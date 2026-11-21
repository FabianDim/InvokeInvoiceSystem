"""Real HTTP/MongoDB tests. Requires an explicitly selected local replica set.

python -m pip install pymongo
INVOICE_TEST_MONGODB_URI=mongodb://127.0.0.1:27028/?directConnection=true
INVOICE_TEST_BACKEND=<path to InvokeInvoiceBackend>
python -m unittest discover -s tests -p test_invoice_workflow.py -v
"""
import json
import os
import pathlib
import re
import socket
import subprocess
import tempfile
import time
import unittest
import urllib.error
import urllib.request
import zlib
from concurrent.futures import ThreadPoolExecutor

from bson import ObjectId
from pymongo import MongoClient
from pymongo.uri_parser import parse_uri


def pdf_page_streams(body):
    """Inspect libharu's compressed drawing commands without a PDF dependency."""
    pages = []
    for match in re.finditer(rb"stream\r?\n(.*?)\r?\nendstream", body, re.S):
        try:
            content = zlib.decompress(match[1])
        except zlib.error:
            continue
        if b"(Tax Invoice)" in content or b"(Invoice)" in content:
            pages.append(content)
    return pages


@unittest.skipUnless(os.getenv("INVOICE_TEST_MONGODB_URI") and os.getenv("INVOICE_TEST_BACKEND"),
                     "Select an isolated local replica set and backend binary")
class InvoiceWorkflowTests(unittest.TestCase):
    def setUp(self):
        uri = os.environ["INVOICE_TEST_MONGODB_URI"]
        if any(host not in ("localhost", "127.0.0.1", "::1") for host, _ in parse_uri(uri)["nodelist"]):
            raise ValueError("Integration tests require a loopback MongoDB instance")
        self.mongo = MongoClient(uri, serverSelectionTimeoutMS=5000)
        self.db = self.mongo["invoice_workflow_test_" + str(ObjectId())]
        self.addCleanup(self.mongo.close)
        self.addCleanup(lambda: self.mongo.drop_database(self.db.name))
        self.start_backend(uri)
        self.addCleanup(self.stop_backend)
        self.token = self.account("owner@example.invalid")
        self.other = self.account("other@example.invalid")
        self.business = self.save("business", {"Business name": "Test business"})
        self.client = self.save("client", {"BusinessID": self.business, "Name": "Customer"})

    def start_backend(self, uri):
        with socket.socket() as sock:
            # Match the backend's wildcard listener. A loopback-only reservation
            # can choose a port already occupied on another interface on Windows.
            if hasattr(socket, "SO_EXCLUSIVEADDRUSE"):
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_EXCLUSIVEADDRUSE, 1)
            sock.bind(("0.0.0.0", 0))
            port = sock.getsockname()[1]
        self.base = f"http://127.0.0.1:{port}"
        env = dict(os.environ, PORT=str(port), MONGODB_URI=uri, MONGODB_DATABASE=self.db.name)
        self.backend_log = tempfile.TemporaryFile()
        self.addCleanup(self.backend_log.close)
        self.backend = subprocess.Popen([os.environ["INVOICE_TEST_BACKEND"]], env=env,
                                        stdout=self.backend_log, stderr=subprocess.STDOUT,
                                        creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        for _ in range(100):
            try:
                with urllib.request.urlopen(self.base, timeout=.2):
                    return
            except (OSError, urllib.error.URLError):
                if self.backend.poll() is not None:
                    self.backend_log.seek(0)
                    self.fail("Backend exited before listening: " + self.backend_log.read().decode(errors="replace"))
                time.sleep(.05)
        self.fail("Backend did not start")

    def stop_backend(self):
        if self.backend.poll() is None:
            self.backend.terminate()
        self.backend.wait(timeout=10)

    def request(self, path, data=None, token=None, api_version=None):
        headers = {"Content-Type": "application/json"}
        if api_version is not None:
            headers["X-Invoice-API-Version"] = api_version
        if token:
            headers["Authorization"] = "Bearer " + token
        request = urllib.request.Request(self.base + path,
                                         None if data is None else json.dumps(data).encode(), headers)
        try:
            response = urllib.request.urlopen(request, timeout=20)
        except urllib.error.HTTPError as error:
            response = error
        with response:
            self.last_headers = response.headers
            body = response.read()
            return response.status, body if body.startswith(b"%PDF") else json.loads(body)

    def account(self, email):
        credentials = {"email": email, "password": "Synthetic1!"}
        self.assertEqual(self.request("/auth/signup", credentials)[0], 201)
        return self.login(email)

    def login(self, email):
        status, data = self.request("/auth/login", {"email": email, "password": "Synthetic1!"})
        self.assertEqual(status, 200)
        return data["token"]

    def save(self, resource, data):
        status, result = self.request("/data/" + resource, data, self.token)
        self.assertEqual(status, 200, result)
        return result["_id"]

    def begin(self, token=None, **extra):
        status, result = self.request("/invoices/invoice_start",
                                      dict(BusinessID=self.business, ClientID=self.client, **extra),
                                      token or self.token)
        self.assertEqual(status, 200, result)
        return result["invoice_number"]

    def quantity(self, stock):
        return self.db.Stock.find_one({"_id": ObjectId(stock)})["Quantity"]

    def test_api_version_rejection_prevents_writes_and_session_changes(self):
        stock = self.save("stock", {"BusinessID": self.business, "Name": "Original", "Price": 10, "Quantity": 8})
        update = dict(_id=stock, BusinessID=self.business, Name="Edited", Quantity=12,
                      ExpectedQuantity=8, Price=14.5, Margin=0.25, Unit="box", Keywords="Copper, pipe")
        client_count = self.db.Clients.count_documents({})
        for path, data in [
            ("/stock/update", update),
            ("/data/client", dict(BusinessID=self.business, Name="Must not be created")),
            ("/auth/logout", {}),
        ]:
            with self.subTest(path=path):
                status, body = self.request(path, data, self.token, api_version="2")
                self.assertEqual(status, 400)
                self.assertEqual(body["code"], "unsupported_api_version")
                self.assertEqual(body["supported_api_versions"], ["1"])
                self.assertEqual(self.last_headers.get("Cache-Control"), "no-store")
        self.assertEqual(self.quantity(stock), 8)
        self.assertEqual(self.db.Stock.find_one({"_id": ObjectId(stock)})["Name"], "Original")
        self.assertEqual(self.db.Clients.count_documents({}), client_count)
        # Rejected logout must leave the existing session usable.
        self.assertEqual(self.request("/business/list", token=self.token, api_version="1")[0], 200)
        self.assertEqual(self.request("/stock/update", update, self.token, api_version="1")[0], 200)
        self.assertEqual(self.quantity(stock), 12)

    def test_business_and_client_edits_preserve_ownership_and_persist(self):
        original = self.db.Business.find_one({"_id": ObjectId(self.business)})
        self.db.Business.update_one({"_id": ObjectId(self.business)}, {"$set": {"InvoicePrefix": "KEEP-", "InvoiceSequence": 7}})
        second = self.save("business", {"Business name": "Second"})
        cases = [
            ("business", dict(_id=self.business, BusinessID=self.business, BusinessName="Edited business",
                              ABN="123", ACN="", Phone="0412345678", BusinessAddress="New address",
                              Website="https://example.invalid", UserID=str(ObjectId()), InvoiceSequence=999,
                              InvoicePrefix="OVERWRITE", ClientIDs=[], GSTRegistered=True, PaymentTerms="30",
                              BankName="Updated bank")),
            ("client", dict(_id=self.client, BusinessID=self.business, Name="Edited client", Phone="",
                            Email="edited@example.invalid", Address="Client address", UserID=str(ObjectId()))),
        ]
        for resource, update in cases:
            path = f"/{resource}/update"
            with self.subTest(resource=resource):
                self.assertEqual(self.request(path, update)[0], 401)
                self.assertEqual(self.request(path, update, self.other)[0], 409)
                self.assertEqual(self.request(path, dict(update, BusinessID=second), self.token)[0], 409)
                self.assertEqual(self.request(path, dict(update, _id="bad"), self.token)[0], 400)
                name = "BusinessName" if resource == "business" else "Name"
                invalid_values = [{name: ""}, {name: "a" * 201}, {"Phone": []}]
                if resource == "business":
                    invalid_values += [{"PaymentTerms": "99"}, {"GSTRegistered": "yes"},
                                       {"Website": "file:///bad"}, {"LogoData": "not an image"}]
                for invalid in invalid_values:
                    self.assertEqual(self.request(path, dict(update, **invalid), self.token)[0], 400)
                self.assertEqual(self.request(path, update, self.token)[0], 200)
                self.assertEqual(self.request(path, update, self.token)[0], 200)
        business = self.db.Business.find_one({"_id": ObjectId(self.business)})
        self.assertEqual(business["BusinessName"], "Edited business")
        self.assertEqual(business["BusinessAddress"], "New address")
        self.assertEqual(business["UserID"], original["UserID"])
        self.assertEqual(business["ClientIDs"], original["ClientIDs"])
        self.assertEqual(business["InvoicePrefix"], "KEEP-")
        self.assertEqual(business["InvoiceSequence"], 7)
        self.assertTrue(business["GSTRegistered"])
        self.assertEqual(business["PaymentTerms"], "30")
        self.assertEqual(business["BankName"], "Updated bank")
        client = self.db.Clients.find_one({"_id": ObjectId(self.client)})
        self.assertEqual(client["ClientName"], "Edited client")
        self.assertEqual(client["BusinessID"], ObjectId(self.business))
        self.assertNotIn("UserID", client)
        self.assertEqual(self.db.Business.count_documents({"UserID": original["UserID"]}), 2)
        self.assertEqual(self.db.Clients.count_documents({"BusinessID": ObjectId(self.business)}), 1)
        self.stop_backend()
        self.start_backend(os.environ["INVOICE_TEST_MONGODB_URI"])
        self.token = self.login("owner@example.invalid")
        status, records = self.request("/business/items?BusinessID=" + self.business, token=self.token)
        self.assertEqual(status, 200)
        self.assertEqual(records["businesses"][0]["BusinessName"], "Edited business")
        self.assertEqual(records["clients"][0]["Name"], "Edited client")
        self.assertEqual(records["clients"][0]["Email"], "edited@example.invalid")

    def test_stock_edits_persist_and_enforce_ownership_and_quantity_conflicts(self):
        stock = self.save("stock", {"BusinessID": self.business, "Name": "Original", "Price": 10, "Quantity": 8})
        update = dict(_id=stock, BusinessID=self.business, Name="Edited", Quantity=12,
                      ExpectedQuantity=8, Price=14.5, Margin=0.25, Unit="box", Keywords="Copper, pipe")
        self.assertEqual(self.request("/stock/update", update)[0], 401)
        self.assertEqual(self.request("/stock/update", update, self.other)[0], 409)
        second = self.save("business", {"Business name": "Second"})
        self.assertEqual(self.request("/stock/update", dict(update, BusinessID=second), self.token)[0], 409)
        for invalid in (dict(Price=-1), dict(Quantity=1.5), dict(Name=""), dict(ExpectedQuantity="8")):
            self.assertEqual(self.request("/stock/update", dict(update, **invalid), self.token)[0], 400)
        self.assertEqual(self.quantity(stock), 8)
        self.assertEqual(self.request("/stock/update", update, self.token)[0], 200)
        self.assertEqual(self.request("/stock/update", update, self.token)[0], 409)
        saved = self.db.Stock.find_one({"_id": ObjectId(stock)})
        self.assertEqual(saved["Name"], "Edited")
        self.assertEqual(saved["Quantity"], 12)
        self.assertEqual(saved["StdPrice"], 14.5)
        self.assertEqual(saved["ProfitMargin"], 0.25)
        self.assertEqual(saved["ProductKeywords"], ["copper", "pipe"])
        self.assertEqual(saved["BusinessID"], ObjectId(self.business))
        self.assertEqual(self.db.Stock.count_documents({"BusinessID": ObjectId(self.business)}), 1)
        self.stop_backend()
        self.start_backend(os.environ["INVOICE_TEST_MONGODB_URI"])
        self.token = self.login("owner@example.invalid")
        status, records = self.request("/business/items?BusinessID=" + self.business, token=self.token)
        self.assertEqual(status, 200)
        self.assertEqual(records["stocks"][0]["Name"], "Edited")

    def test_archive_survives_restart_and_regeneration_preserves_original_details(self):
        stock = self.save("stock", {"BusinessID": self.business, "Name": "Widget", "Price": 10, "Quantity": 8})
        self.db.Business.update_one({"_id": ObjectId(self.business)}, {"$set": {"PaymentTerms": "14"}})
        number = self.begin(date_created="2026-01-01", date_due="2026-01-15", paid=True)
        lines = [{"_id": stock, "Name": "Widget", "Quantity": 2, "Price": 10}]
        status, original = self.request("/invoices/stock-list", lines, self.token)
        self.assertEqual(status, 200, original)
        original_pages = pdf_page_streams(original)
        self.assertEqual(len(original_pages), 1)
        self.assertIn(b"(Due date: 15 Jan 2026)", original_pages[0])
        self.assertIn(b"(Payment: PAID)", original_pages[0])
        self.assertEqual(self.request("/invoices/stock-list", lines, self.token)[0], 200)
        self.assertEqual(self.db.Invoices.count_documents({}), 1)
        saved = self.db.Invoices.find_one()
        self.assertTrue(saved["Paid"])
        snapshot = json.loads(saved["SnapshotJson"])
        self.assertEqual(snapshot["DateDue"], "2026-01-15")
        self.assertEqual(snapshot["Client"]["Name"], "Customer")
        self.assertEqual(snapshot["Total"], 20)
        self.assertEqual(snapshot["Items"], lines)
        # Regeneration must not rely on current stock or contact records.
        self.db.Clients.delete_one({"_id": ObjectId(self.client)})
        self.db.Business.update_one({"_id": ObjectId(self.business)}, {"$set": {"BusinessName": "Changed business"}})
        self.stop_backend()
        self.start_backend(os.environ["INVOICE_TEST_MONGODB_URI"])
        self.token = self.login("owner@example.invalid")
        status, listing = self.request(f"/invoices/list?BusinessID={self.business}", token=self.token)
        self.assertEqual(status, 200, listing)
        self.assertEqual(self.last_headers["Cache-Control"], "no-store")
        row = listing["invoices"][0]
        self.assertEqual(row["InvoiceNumber"], number)
        self.assertEqual(row["ClientName"], "Customer")
        self.assertTrue(row["Paid"])
        self.assertFalse(row["LegacyCopy"])
        self.assertNotIn("Items", row)
        payment = {"InvoiceID": row["_id"], "BusinessID": self.business, "Paid": False}
        self.assertEqual(self.request("/invoices/payment", payment, self.token)[0], 200)
        self.assertFalse(self.db.Invoices.find_one()["Paid"])
        self.assertEqual(self.db.Invoices.find_one()["SnapshotJson"], saved["SnapshotJson"])
        path = f"/invoices/pdf?BusinessID={self.business}&InvoiceID={row['_id']}"
        status, regenerated = self.request(path, token=self.token)
        self.assertEqual(status, 200, regenerated)
        self.assertTrue(regenerated.startswith(b"%PDF-"))
        regenerated_pages = pdf_page_streams(regenerated)
        self.assertEqual(len(regenerated_pages), 1)
        self.assertIn(b"(Due date: 15 Jan 2026)", regenerated_pages[0])
        self.assertIn(b"(Payment: UNPAID)", regenerated_pages[0])
        self.assertIn(b"(Test business)", regenerated_pages[0])
        self.assertIn(b"(Customer)", regenerated_pages[0])
        self.assertNotIn(b"Changed business", regenerated_pages[0])
        self.assertEqual(self.quantity(stock), 6)
        self.assertEqual(self.db.Invoices.count_documents({}), 1)
        # A second login cannot bypass the account-wide backend wait.
        second_session = self.login("owner@example.invalid")
        self.assertEqual(self.request(path, token=second_session)[0], 429)
        wait = int(self.last_headers["Retry-After"])
        self.assertGreaterEqual(wait, 1)
        self.assertLessEqual(wait, 10)
        time.sleep(wait + .05)
        self.assertEqual(self.request(path, token=second_session)[0], 200)
        self.assertEqual(self.quantity(stock), 6)
        if directory := os.getenv("INVOICE_PDF_PREVIEW_DIR"):
            destination = pathlib.Path(directory)
            destination.mkdir(parents=True, exist_ok=True)
            (destination / "original-paid.pdf").write_bytes(original)
            (destination / "regenerated-unpaid.pdf").write_bytes(regenerated)

    def test_archive_authorization_and_input_validation(self):
        for invalid in ({"paid": "yes"}, {"date_due": "2026-02-30"}, {"date_created": "not a date"}):
            data = dict(BusinessID=self.business, ClientID=self.client, **invalid)
            self.assertEqual(self.request("/invoices/invoice_start", data, self.token)[0], 400)
        self.begin(date_created="2026-01-01", date_due="2026-01-15")
        self.assertEqual(self.request("/invoices/stock-list", [{"Name": "Service", "Quantity": 1, "Price": 25}], self.token)[0], 200)
        saved = self.db.Invoices.find_one()
        self.assertFalse(saved["Paid"])
        path = f"/invoices/pdf?BusinessID={self.business}&InvoiceID={saved['_id']}"
        self.assertEqual(self.request(path)[0], 401)
        self.assertEqual(self.request(path, token=self.other)[0], 404)
        listing = f"/invoices/list?BusinessID={self.business}"
        self.assertEqual(self.request(listing)[0], 401)
        self.assertEqual(self.request(listing, token=self.other)[1]["invoices"], [])
        self.assertEqual(self.request(listing + "&offset=-1", token=self.token)[0], 400)
        payment = {"InvoiceID": str(saved["_id"]), "BusinessID": self.business, "Paid": True}
        self.assertEqual(self.request("/invoices/payment", payment)[0], 401)
        self.assertEqual(self.request("/invoices/payment", payment, self.other)[0], 404)
        self.assertEqual(self.request("/invoices/payment", dict(payment, Paid="true"), self.token)[0], 400)
        self.assertEqual(self.request("/invoices/payment", dict(payment, BusinessID=str(ObjectId())), self.token)[0], 404)
        self.assertFalse(self.db.Invoices.find_one()["Paid"])

    def test_archive_pagination_and_legacy_records(self):
        owner = self.db.Business.find_one({"_id": ObjectId(self.business)})["UserID"]
        records = [{"_id": ObjectId(), "UserID": owner, "BusinessID": ObjectId(self.business),
                    "ClientID": ObjectId(self.client), "InvoiceNumber": f"OLD-{index:03}",
                    "ItemsJson": json.dumps([{"Name": "Legacy service", "Quantity": 1, "Price": 10}])}
                   for index in range(51)]
        self.db.Invoices.insert_many(records)
        path = f"/invoices/list?BusinessID={self.business}"
        status, first = self.request(path, token=self.token)
        self.assertEqual(status, 200, first)
        self.assertEqual(len(first["invoices"]), 50)
        self.assertTrue(first["has_more"])
        row = first["invoices"][0]
        self.assertTrue(row["LegacyCopy"])
        self.assertEqual(row["DateDue"], "")
        self.assertFalse(row["Paid"])
        self.assertEqual(self.db.Invoices.count_documents({"Paid": False}), 51)
        _, second = self.request(path + "&offset=50", token=self.token)
        self.assertEqual(len(second["invoices"]), 1)
        self.assertFalse(second["has_more"])
        self.assertNotIn(second["invoices"][0]["_id"], [item["_id"] for item in first["invoices"]])
        status, pdf = self.request(f"/invoices/pdf?BusinessID={self.business}&InvoiceID={row['_id']}", token=self.token)
        self.assertEqual(status, 200)
        self.assertIn(b"(Due date: Not recorded)", pdf_page_streams(pdf)[0])
        self.assertIn(b"Legacy copy:", pdf_page_streams(pdf)[0])

    def test_pdf_due_date_and_status_on_every_page(self):
        self.db.Business.update_one({"_id": ObjectId(self.business)}, {"$set": {"PaymentTerms": "30"}})
        self.begin(date_created="2026-11-01", date_due="2026-11-30", paid=False)
        lines = [{"Name": f"Service {index}", "Quantity": 1, "Price": 25} for index in range(60)]
        status, pdf = self.request("/invoices/stock-list", lines, self.token)
        self.assertEqual(status, 200)
        pages = pdf_page_streams(pdf)
        self.assertGreater(len(pages), 1)
        for page in pages:
            self.assertIn(b"(Due date: 01 Dec 2026)", page)
            self.assertIn(b"(Issued: 01 Nov 2026)", page)
            self.assertIn(b"(Payment: UNPAID)", page)

    def test_gst_terms_and_bank_details_persist_and_snapshot(self):
        preferences = {"GSTRegistered": True, "PaymentTerms": "30", "BankName": "Example Bank",
                       "BankAccountName": "Test business", "BankBSB": "012-345", "BankAccountNumber": "00123456"}
        settings = dict(preferences, BusinessID=self.business, InvoicePrefix="INV-")
        self.assertEqual(self.request("/business/invoice-settings", settings, self.token)[0], 200)
        # A partial update must not erase preferences.
        self.assertEqual(self.request("/business/invoice-settings",
                         {"BusinessID": self.business, "InvoicePrefix": "INV-"}, self.token)[0], 200)
        self.stop_backend()
        self.start_backend(os.environ["INVOICE_TEST_MONGODB_URI"])
        self.token = self.login("owner@example.invalid")
        status, businesses = self.request("/business/list", token=self.token)
        self.assertEqual(status, 200)
        for key, value in preferences.items():
            self.assertEqual(businesses[self.business][key], value)
        for invalid in [{"GSTRegistered": "yes"}, {"PaymentTerms": "7"}, {"BankBSB": 123},
                        {"BankName": "x" * 121}]:
            self.assertEqual(self.request("/business/invoice-settings", dict(settings, **invalid), self.token)[0], 400)
        self.begin(date_created="2026-12-20", date_due="2026-12-21")
        lines = [{"Name": "Service", "Quantity": 2, "Price": 19.99},
                 {"Name": "Small line", "Quantity": 1, "Price": 0.05},
                 {"Name": "Second small line", "Quantity": 1, "Price": 0.05}]
        status, pdf = self.request("/invoices/stock-list", lines, self.token)
        self.assertEqual(status, 200)
        saved = self.db.Invoices.find_one()
        snapshot = json.loads(saved["SnapshotJson"])
        self.assertEqual(snapshot["DateDue"], "2027-01-19")
        self.assertEqual(snapshot["Subtotal"], 40.08)
        self.assertEqual(snapshot["GST"], 4.02)
        self.assertEqual(snapshot["Total"], 44.10)
        self.assertEqual(saved["Total"], 44.10)
        text = b" ".join(pdf_page_streams(pdf))
        self.assertIn(b"(Tax Invoice)", text)
        self.assertIn(b"(BSB: 012-345)", text)
        self.assertIn(b"(Account number: 00123456)", text)
        self.assertIn(b"(Total: $44.10)", text)
        self.assertEqual(self.request("/business/invoice-settings",
                         dict(settings, GSTRegistered=False, PaymentTerms="COD", BankBSB="999-999"), self.token)[0], 200)
        status, archived = self.request(f"/invoices/pdf?BusinessID={self.business}&InvoiceID={saved['_id']}", token=self.token)
        self.assertEqual(status, 200)
        archived_text = b" ".join(pdf_page_streams(archived))
        self.assertIn(b"(BSB: 012-345)", archived_text)
        self.assertIn(b"(Total: $44.10)", archived_text)
        self.assertIn(b"(Due date: 19 Jan 2027)", archived_text)
        self.begin(date_created="2026-12-20")
        status, untaxed = self.request("/invoices/stock-list", lines, self.token)
        self.assertEqual(status, 200)
        untaxed_text = b" ".join(pdf_page_streams(untaxed))
        self.assertNotIn(b"(Tax Invoice)", untaxed_text)
        self.assertIn(b"(Total: $40.08)", untaxed_text)
        self.assertIn(b"(Due date: 20 Dec 2026)", untaxed_text)

    def test_prefix_and_sequence_survive_restart_and_blank_prefix(self):
        self.assertEqual(self.begin(), "INV-000001")
        terms = "Payment within 14 days.\nPlease quote your invoice number."
        settings = {"BusinessID": self.business, "InvoicePrefix": "JOB-", "InvoiceTerms": terms,
                    "InvoiceFilenameTemplate": "{business_name}_{invoice_number}"}
        self.assertEqual(self.request("/business/invoice-settings", settings, self.other)[0], 400)
        self.assertEqual(self.request("/business/invoice-settings", settings, self.token)[0], 200)
        self.assertEqual(self.begin(), "JOB-000002")
        self.stop_backend()
        self.start_backend(os.environ["INVOICE_TEST_MONGODB_URI"])
        self.token = self.login("owner@example.invalid")
        status, businesses = self.request("/business/list", token=self.token)
        self.assertEqual(status, 200)
        self.assertEqual(businesses[self.business]["InvoicePrefix"], "JOB-")
        self.assertEqual(businesses[self.business]["InvoiceTerms"], terms)
        self.assertEqual(businesses[self.business]["InvoiceFilenameTemplate"], "{business_name}_{invoice_number}")
        for invalid in [123, "{unknown}", "../file", "x" * 201]:
            self.assertEqual(self.request("/business/invoice-settings", dict(settings, InvoiceFilenameTemplate=invalid), self.token)[0], 400)
        for invalid in [123, None, "x" * 4001]:
            self.assertEqual(self.request("/business/invoice-settings", dict(settings, InvoiceTerms=invalid), self.token)[0], 400)
        self.assertEqual(self.request("/business/invoice-settings", {"BusinessID": self.business, "InvoicePrefix": "JOB-"}, self.token)[0], 200)
        self.assertEqual(self.db.Business.find_one({"_id": ObjectId(self.business)})["InvoiceTerms"], terms)
        self.assertEqual(self.begin(), "JOB-000003")
        settings["InvoicePrefix"] = ""
        settings["InvoiceTerms"] = ""
        self.assertEqual(self.request("/business/invoice-settings", settings, self.token)[0], 200)
        self.assertEqual(self.begin(), "000004")
        self.assertEqual(self.db.Business.find_one({"_id": ObjectId(self.business)})["InvoiceTerms"], "")
        another_business = self.save("business", {"Business name": "Independent sequence"})
        another_client = self.save("client", {"BusinessID": another_business, "Name": "Second customer"})
        self.business, self.client = another_business, another_client
        self.assertEqual(self.begin(), "INV-000001")

    def test_stock_deducted_once_and_failed_invoice_is_atomic(self):
        stock = self.save("stock", {"BusinessID": self.business, "Name": "Widget", "Price": 10, "Quantity": 8})
        empty = self.save("stock", {"BusinessID": self.business, "Name": "Empty", "Price": 5, "Quantity": 0})
        line = {"_id": stock, "Name": "Widget", "Quantity": 2, "Price": 10}
        self.begin()
        # Repeated lines must be aggregated, including in the stock availability check.
        status, pdf = self.request("/invoices/stock-list", [line, line], self.token)
        self.assertEqual(status, 200, pdf)
        self.assertTrue(pdf.startswith(b"%PDF"))
        self.assertEqual(self.quantity(stock), 4)
        self.assertEqual(self.db.Invoices.count_documents({}), 1)
        self.assertEqual(self.request("/invoices/stock-list", [line, line], self.token), (200, pdf))
        self.assertEqual(self.quantity(stock), 4)
        self.assertEqual(self.request("/invoices/stock-list", [line], self.token)[0], 400)
        self.begin()
        failed = [line, {"_id": empty, "Name": "Empty", "Quantity": 1, "Price": 5}]
        self.assertEqual(self.request("/invoices/stock-list", failed, self.token)[0], 409)
        self.assertEqual(self.quantity(stock), 4)
        self.assertEqual(self.quantity(empty), 0)
        self.assertEqual(self.db.Invoices.count_documents({}), 1)
        self.assertEqual(self.request("/invoices/stock-list", [dict(line, Quantity=5)], self.token)[0], 409)
        self.assertEqual(self.quantity(stock), 4)
        # Ad hoc invoice lines are allowed and do not affect inventory.
        self.assertEqual(self.request("/invoices/stock-list", [{"Name": "Service", "Quantity": 1, "Price": 20}], self.token)[0], 200)
        self.assertEqual(self.quantity(stock), 4)

    def test_concurrent_sessions_do_not_oversell_or_reuse_numbers(self):
        stock = self.save("stock", {"BusinessID": self.business, "Name": "Last one", "Price": 10, "Quantity": 1})
        tokens = [self.token, self.login("owner@example.invalid")]
        with ThreadPoolExecutor(2) as pool:
            numbers = list(pool.map(lambda token: self.begin(token), tokens))
        self.assertEqual(set(numbers), {"INV-000001", "INV-000002"})
        with ThreadPoolExecutor(2) as pool:
            results = list(pool.map(lambda token: self.request("/invoices/stock-list",
                [{"_id": stock, "Name": "Last one", "Quantity": 1, "Price": 10}], token)[0], tokens))
        self.assertEqual(sorted(results), [200, 409])
        self.assertEqual(self.quantity(stock), 0)

    def test_foreign_stock_is_rejected_without_deducting_own_stock(self):
        own = self.save("stock", {"BusinessID": self.business, "Name": "Own", "Price": 1, "Quantity": 5})
        foreign_business = self.save("business", {"Business name": "Other business"})
        foreign = self.save("stock", {"BusinessID": foreign_business, "Name": "Foreign", "Price": 1, "Quantity": 5})
        self.begin()
        lines = [{"_id": id, "Name": "Item", "Quantity": 1, "Price": 1} for id in (own, foreign)]
        for policy in ("reject", "zero", "negative"):
            payload = {"items": lines, "stockShortagePolicy": policy}
            self.assertEqual(self.request("/invoices/stock-list", payload, self.token)[0], 400)
        self.assertEqual(self.quantity(own), 5)
        self.assertEqual(self.quantity(foreign), 5)

    def test_shortage_choices_are_atomic_and_retries_do_not_deduct_twice(self):
        for policy, balance in (("zero", 0), ("negative", -3)):
            with self.subTest(policy=policy):
                stock = self.save("stock", {"BusinessID": self.business, "Name": "Widget", "Price": 10, "Quantity": 5})
                enough = self.save("stock", {"BusinessID": self.business, "Name": "Enough", "Price": 1, "Quantity": 10})
                line = {"_id": stock, "Name": "Widget", "Quantity": 4, "Price": 10}
                lines = [line, line, {"_id": enough, "Name": "Enough", "Quantity": 2, "Price": 1}]
                self.begin()
                count = self.db.Invoices.count_documents({})
                status, error = self.request("/invoices/stock-list", lines, self.token)
                self.assertEqual(status, 409, error)
                self.assertEqual(error["code"], "stock_shortage")
                self.assertEqual(error["items"], [{"_id": stock, "Name": "Widget", "Available": 5, "Requested": 8}])
                self.assertEqual(self.quantity(stock), 5)
                self.assertEqual(self.quantity(enough), 10)
                self.assertEqual(self.db.Invoices.count_documents({}), count)
                payload = {"items": lines, "stockShortagePolicy": policy}
                status, pdf = self.request("/invoices/stock-list", payload, self.token)
                self.assertEqual(status, 200, pdf)
                self.assertTrue(pdf.startswith(b"%PDF"))
                self.assertEqual(self.quantity(stock), balance)
                self.assertEqual(self.quantity(enough), 8)
                self.assertEqual(self.db.Invoices.count_documents({}), count + 1)
                self.assertEqual(self.request("/invoices/stock-list", payload, self.token), (200, pdf))
                self.assertEqual(self.request("/invoices/stock-list", lines, self.token), (200, pdf))
                self.assertEqual(self.quantity(stock), balance)
                self.assertEqual(self.quantity(enough), 8)
                # Listing must support the signed 64-bit quantities stored by completion.
                status, records = self.request("/stock/list?BusinessID=" + self.business, token=self.token)
                self.assertEqual(status, 200)
                self.assertEqual(next(row["Quantity"] for row in records if row["_id"] == stock), balance)
                self.begin()
                self.assertEqual(self.request("/invoices/stock-list", [line], self.token)[0], 409)
                self.assertEqual(self.request("/invoices/stock-list",
                    {"items": [line], "stockShortagePolicy": "negative"}, self.token)[0], 200)
                self.assertEqual(self.quantity(stock), balance - 4)

    def test_invalid_shortage_choices_leave_inventory_unchanged(self):
        stock = self.save("stock", {"BusinessID": self.business, "Name": "Widget", "Price": 10, "Quantity": 1})
        self.begin()
        lines = [{"_id": stock, "Name": "Widget", "Quantity": 2, "Price": 10}]
        for payload in ({"items": lines}, {"items": lines, "stockShortagePolicy": "invalid"},
                        {"items": lines, "stockShortagePolicy": True},
                        {"items": {}, "stockShortagePolicy": "zero"}):
            self.assertEqual(self.request("/invoices/stock-list", payload, self.token)[0], 400)
        self.assertEqual(self.quantity(stock), 1)
        self.assertEqual(self.db.Invoices.count_documents({}), 0)


if __name__ == "__main__":
    unittest.main()
