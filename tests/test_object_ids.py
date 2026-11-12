"""Run with Python + pymongo: python -m unittest discover -s tests -p test_object_ids.py -v.

Set INVOICE_TEST_MONGODB_URI (loopback only) and INVOICE_TEST_BACKEND to also run
the actual backend against a newly created, isolated database. That database is
removed afterwards; existing databases and application processes are untouched.
"""
import copy
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import time
import unittest
import urllib.error
import urllib.request

from bson import ObjectId
from pymongo import MongoClient
from pymongo.uri_parser import parse_uri

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from scripts.migrate_object_ids import build_plan, migrate


def legacy_fixture():
    return {
        "Users": [{"_id": ObjectId(), "UserID": "USR1", "BusinessIDs": [["BUS1"]]}],
        "Business": [{"_id": ObjectId(), "BusinessID": "BUS1", "UserID": ["USR1"],
                      "ClientIDs": ["CLI1"], "StockIDs": ["STK1"]}],
        "Clients": [{"_id": ObjectId(), "ClientID": "CLI1", "BusinessID": ["BUS1"],
                     "UserID": "USR1", "ClientStockIDs": ["STK1"],
                     "clientStockArray": [{"StockID": "STK1", "ClientPrice": 5.0}]}],
        "Stock": [{"_id": ObjectId(), "StockID": "STK1", "BusinessID": "BUS1"}],
        "Invoices": [{"_id": ObjectId(), "InvoiceID": "INV1", "ClientInvoiceID": "2026-123",
                      "ClientID": "CLI1", "StockItems": [{"StockID": "STK1", "Quantity": 2}]}],
    }


class MigrationTests(unittest.TestCase):
    def test_preserves_ids_and_invoice_numbers_converts_all_references_and_is_idempotent(self):
        fixture = legacy_fixture()
        original = copy.deepcopy(fixture)
        converted = {name: [doc] for name, doc in build_plan(fixture)}
        self.assertEqual(fixture, original)
        for name in original:
            self.assertEqual(converted[name][0]["_id"], original[name][0]["_id"])
        user, business, client, stock, invoice = [converted[n][0] for n in original]
        self.assertNotIn("UserID", user)
        self.assertNotIn("BusinessID", business)
        self.assertNotIn("ClientID", client)
        self.assertNotIn("StockID", stock)
        self.assertNotIn("InvoiceID", invoice)
        self.assertEqual(user["BusinessIDs"], [business["_id"]])
        self.assertEqual(business["UserID"], [user["_id"]])
        self.assertEqual(business["ClientIDs"], [client["_id"]])
        self.assertEqual(business["StockIDs"], [stock["_id"]])
        self.assertEqual(client["BusinessID"], business["_id"])
        self.assertEqual(client["clientStockArray"][0]["StockID"], stock["_id"])
        self.assertEqual(invoice["StockItems"][0]["StockID"], stock["_id"])
        self.assertEqual(invoice["ClientInvoiceID"], "2026-123")
        self.assertEqual(build_plan(converted), [])

    def test_duplicate_legacy_ids_are_rejected(self):
        fixture = legacy_fixture()
        fixture["Users"].append({"_id": ObjectId(), "UserID": "USR1"})
        with self.assertRaisesRegex(ValueError, "ambiguous"):
            build_plan(fixture)

    def test_missing_references_are_rejected(self):
        fixture = legacy_fixture()
        fixture["Users"][0]["BusinessIDs"] = ["missing"]
        with self.assertRaisesRegex(ValueError, "Unresolved"):
            build_plan(fixture)

    def test_conflicting_owner_is_rejected(self):
        fixture = legacy_fixture()
        fixture["Users"].append({"_id": ObjectId(), "UserID": "USR2", "BusinessIDs": ["BUS1"]})
        with self.assertRaisesRegex(ValueError, "ownership conflict"):
            build_plan(fixture)

    def test_non_objectid_primary_key_is_rejected(self):
        fixture = legacy_fixture()
        fixture["Users"][0]["_id"] = "old-string-key"
        with self.assertRaisesRegex(ValueError, "non-ObjectId"):
            build_plan(fixture)


@unittest.skipUnless(os.getenv("INVOICE_TEST_MONGODB_URI") and os.getenv("INVOICE_TEST_BACKEND"),
                     "Set explicit local MongoDB URI and backend binary for integration tests")
class BackendObjectIdTests(unittest.TestCase):
    def setUp(self):
        uri = os.environ["INVOICE_TEST_MONGODB_URI"]
        if any(host not in ("127.0.0.1", "localhost", "::1") for host, _ in parse_uri(uri)["nodelist"]):
            raise ValueError("Tests require a loopback MongoDB instance")
        self.mongo = MongoClient(uri, serverSelectionTimeoutMS=5000)
        self.database_name = "invoice_objectid_test_" + str(ObjectId())
        self.db = self.mongo[self.database_name]
        self.addCleanup(self.mongo.close)
        self.addCleanup(lambda: self.mongo.drop_database(self.database_name))
        with socket.socket() as sock:
            sock.bind(("127.0.0.1", 0))
            port = sock.getsockname()[1]
        self.base = f"http://127.0.0.1:{port}"
        env = dict(os.environ, MONGODB_URI=uri, MONGODB_DATABASE=self.database_name, PORT=str(port))
        env.pop("INVOICE_LOG_FILE", None)
        self.backend = subprocess.Popen([os.environ["INVOICE_TEST_BACKEND"]], env=env,
                                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                                        creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        self.addCleanup(self.stop_backend)
        for _ in range(100):
            try:
                if urllib.request.urlopen(self.base, timeout=.2).status == 200:
                    break
            except (OSError, urllib.error.URLError):
                if self.backend.poll() is not None:
                    self.fail("Backend exited before listening")
                time.sleep(.05)
        else:
            self.fail("Backend did not start")

    def stop_backend(self):
        if self.backend.poll() is None:
            self.backend.terminate()
        self.backend.wait(timeout=10)

    def request(self, path, data=None, token=None):
        headers = {"Content-Type": "application/json"}
        if token:
            headers["Authorization"] = "Bearer " + token
        req = urllib.request.Request(self.base + path,
                                     None if data is None else json.dumps(data).encode(), headers)
        try:
            response = urllib.request.urlopen(req, timeout=10)
        except urllib.error.HTTPError as error:
            response = error
        with response:
            body = response.read()
            return response.status, body if body.startswith(b"%PDF") else json.loads(body)

    def account(self, email):
        credentials = {"email": email, "password": "Synthetic1!"}
        self.assertEqual(self.request("/auth/signup", credentials)[0], 201)
        status, login = self.request("/auth/login", credentials)
        self.assertEqual(status, 200)
        user = self.db.Users.find_one({"UserEmail": email})
        self.assertEqual(login["user"]["_id"], str(user["_id"]))
        self.assertNotIn("UserID", user)
        return login["token"], user["_id"]

    def test_insert_result_identity_bson_relationships_and_tenant_isolation(self):
        token, uid = self.account("a@example.invalid")
        other, other_uid = self.account("b@example.invalid")
        # Even colliding legacy aliases must never determine authenticated identity.
        self.db.Users.update_many({}, {"$set": {"UserID": "DUPLICATED_LEGACY_ID"}})
        for email, expected in (("a@example.invalid", uid), ("b@example.invalid", other_uid)):
            status, login = self.request("/auth/login", {"email": email, "password": "Synthetic1!"})
            self.assertEqual(status, 200)
            self.assertEqual(login["user"]["_id"], str(expected))
        injected = str(ObjectId())
        status, saved = self.request("/data/business", {"Business name": "A", "_id": injected,
                                    "BusinessID": "BUSLEGACY", "UserID": str(other_uid)}, token)
        self.assertEqual(status, 200)
        bid = ObjectId(saved["_id"])
        self.assertNotEqual(str(bid), injected)
        business = self.db.Business.find_one({"_id": bid})
        self.assertNotIn("BusinessID", business)
        self.assertEqual(business["UserID"], uid)
        self.assertEqual(self.db.Users.find_one({"_id": uid})["BusinessIDs"], [bid])
        ids = {}
        for resource, collection, legacy in (("client", "Clients", "ClientID"), ("stock", "Stock", "StockID")):
            status, saved = self.request("/data/" + resource, {"BusinessID": str(bid), "Name": "Item",
                                        "Price": 12.5, "Quantity": 2, "_id": injected, legacy: "old"}, token)
            self.assertEqual(status, 200)
            oid = ObjectId(saved["_id"])
            ids[resource] = str(oid)
            doc = self.db[collection].find_one({"_id": oid})
            self.assertNotIn(legacy, doc)
            self.assertEqual(doc["BusinessID"], bid)
            self.assertNotEqual(str(oid), injected)
            self.assertEqual(self.db.Business.find_one({"_id": bid})[legacy + "s"], [oid])
        status, items = self.request("/business/items?BusinessID=" + str(bid), token=token)
        self.assertEqual(status, 200)
        self.assertEqual(items["businesses"][0]["_id"], str(bid))
        self.assertEqual(items["clients"][0]["_id"], ids["client"])
        self.assertEqual(items["stocks"][0]["_id"], ids["stock"])
        self.assertEqual(self.request("/business/items?BusinessID=" + str(bid), token=other)[0], 403)
        before = self.db.Clients.count_documents({})
        self.assertGreaterEqual(self.request("/data/client", {"BusinessID": str(bid), "Name": "Forbidden"}, other)[0], 400)
        self.assertEqual(self.db.Clients.count_documents({}), before)
        for bad in ("BUSLEGACY", "z" * 24, ""):
            self.assertEqual(self.request("/data/client", {"BusinessID": bad, "Name": "Invalid"}, token)[0], 400)
        self.assertGreaterEqual(self.request("/data/client", {"BusinessID": str(ObjectId()), "Name": "Missing"}, token)[0], 400)
        self.assertEqual(self.db.Clients.count_documents({}), before)
        status, _ = self.request("/invoices/invoice_start", {"BusinessID": str(bid), "ClientID": ids["client"],
                                "invoice_number": "TEST-1"}, token)
        self.assertEqual(status, 200)
        status, pdf = self.request("/invoices/stock-list", [{"_id": ids["stock"], "Name": "Item", "Quantity": 1, "Price": 12.5}], token)
        self.assertEqual(status, 200)
        self.assertTrue(pdf.startswith(b"%PDF"))

    def test_transactional_migration_preserves_existing_data_and_is_repeatable(self):
        fixture = legacy_fixture()
        for name, docs in fixture.items():
            self.db[name].insert_many(docs)
        counts = migrate(self.db)
        self.assertEqual(sum(counts.values()), 5)
        self.assertIn("UserID", self.db.Users.find_one({}))
        with self.mongo.start_session() as session:
            session.with_transaction(lambda s: migrate(self.db, True, s))
        self.assertEqual(sum(migrate(self.db).values()), 0)
        for name in fixture:
            self.assertEqual(self.db[name].find_one({})["_id"], fixture[name][0]["_id"])

    def test_existing_account_can_sign_in_and_load_records_after_migration(self):
        token, uid = self.account("existing@example.invalid")
        _, created = self.request("/data/business", {"Business name": "Existing business"}, token)
        bid = ObjectId(created["_id"])
        _, created = self.request("/data/client", {"BusinessID": str(bid), "Name": "Existing client"}, token)
        cid = ObjectId(created["_id"])
        self.db.Users.update_one({"_id": uid}, {"$set": {"UserID": "USR1", "BusinessIDs": ["BUS1"]}})
        self.db.Business.update_one({"_id": bid}, {"$set": {"BusinessID": "BUS1", "UserID": "USR1", "ClientIDs": ["CLI1"]}})
        self.db.Clients.update_one({"_id": cid}, {"$set": {"ClientID": "CLI1", "BusinessID": "BUS1"}})
        with self.mongo.start_session() as session:
            session.with_transaction(lambda s: migrate(self.db, True, s))
        status, login = self.request("/auth/login", {"email": "existing@example.invalid", "password": "Synthetic1!"})
        self.assertEqual(status, 200)
        self.assertEqual(login["user"]["_id"], str(uid))
        status, businesses = self.request("/business/list", token=login["token"])
        self.assertEqual(status, 200)
        self.assertEqual(businesses[str(bid)]["_id"], str(bid))
        status, items = self.request("/business/items?BusinessID=" + str(bid), token=login["token"])
        self.assertEqual(status, 200)
        self.assertEqual(items["clients"][0]["_id"], str(cid))


if __name__ == "__main__":
    unittest.main()
