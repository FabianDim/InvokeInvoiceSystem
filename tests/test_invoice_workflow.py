"""Real HTTP/MongoDB tests. Requires an explicitly selected local replica set.

python -m pip install pymongo
INVOICE_TEST_MONGODB_URI=mongodb://127.0.0.1:27028/?directConnection=true
INVOICE_TEST_BACKEND=<path to InvokeInvoiceBackend>
python -m unittest discover -s tests -p test_invoice_workflow.py -v
"""
import json
import os
import socket
import subprocess
import time
import unittest
import urllib.error
import urllib.request
from concurrent.futures import ThreadPoolExecutor

from bson import ObjectId
from pymongo import MongoClient
from pymongo.uri_parser import parse_uri


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
            sock.bind(("127.0.0.1", 0))
            port = sock.getsockname()[1]
        self.base = f"http://127.0.0.1:{port}"
        env = dict(os.environ, PORT=str(port), MONGODB_URI=uri, MONGODB_DATABASE=self.db.name)
        self.backend = subprocess.Popen([os.environ["INVOICE_TEST_BACKEND"]], env=env,
                                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                                        creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        for _ in range(100):
            try:
                with urllib.request.urlopen(self.base, timeout=.2):
                    return
            except (OSError, urllib.error.URLError):
                if self.backend.poll() is not None:
                    self.fail("Backend exited before listening")
                time.sleep(.05)
        self.fail("Backend did not start")

    def stop_backend(self):
        if self.backend.poll() is None:
            self.backend.terminate()
        self.backend.wait(timeout=10)

    def request(self, path, data=None, token=None):
        headers = {"Content-Type": "application/json"}
        if token:
            headers["Authorization"] = "Bearer " + token
        request = urllib.request.Request(self.base + path,
                                         None if data is None else json.dumps(data).encode(), headers)
        try:
            response = urllib.request.urlopen(request, timeout=20)
        except urllib.error.HTTPError as error:
            response = error
        with response:
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

    def test_prefix_and_sequence_survive_restart_and_blank_prefix(self):
        self.assertEqual(self.begin(), "INV-000001")
        settings = {"BusinessID": self.business, "InvoicePrefix": "JOB-"}
        self.assertEqual(self.request("/business/invoice-settings", settings, self.other)[0], 400)
        self.assertEqual(self.request("/business/invoice-settings", settings, self.token)[0], 200)
        self.assertEqual(self.begin(), "JOB-000002")
        self.stop_backend()
        self.start_backend(os.environ["INVOICE_TEST_MONGODB_URI"])
        self.token = self.login("owner@example.invalid")
        status, businesses = self.request("/business/list", token=self.token)
        self.assertEqual(status, 200)
        self.assertEqual(businesses[self.business]["InvoicePrefix"], "JOB-")
        self.assertEqual(self.begin(), "JOB-000003")
        settings["InvoicePrefix"] = ""
        self.assertEqual(self.request("/business/invoice-settings", settings, self.token)[0], 200)
        self.assertEqual(self.begin(), "000004")
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
        self.assertEqual(self.request("/invoices/stock-list", failed, self.token)[0], 400)
        self.assertEqual(self.quantity(stock), 4)
        self.assertEqual(self.quantity(empty), 0)
        self.assertEqual(self.db.Invoices.count_documents({}), 1)
        self.assertEqual(self.request("/invoices/stock-list", [dict(line, Quantity=5)], self.token)[0], 400)
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
        self.assertEqual(sorted(results), [200, 400])
        self.assertEqual(self.quantity(stock), 0)

    def test_foreign_stock_is_rejected_without_deducting_own_stock(self):
        own = self.save("stock", {"BusinessID": self.business, "Name": "Own", "Price": 1, "Quantity": 5})
        foreign_business = self.save("business", {"Business name": "Other business"})
        foreign = self.save("stock", {"BusinessID": foreign_business, "Name": "Foreign", "Price": 1, "Quantity": 5})
        self.begin()
        lines = [{"_id": id, "Name": "Item", "Quantity": 1, "Price": 1} for id in (own, foreign)]
        self.assertEqual(self.request("/invoices/stock-list", lines, self.token)[0], 400)
        self.assertEqual(self.quantity(own), 5)
        self.assertEqual(self.quantity(foreign), 5)


if __name__ == "__main__":
    unittest.main()
