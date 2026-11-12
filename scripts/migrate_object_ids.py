"""Convert legacy entity references to existing MongoDB _id values.

Dry-run by default. Requires pymongo. Stop application writers and take a backup
before --apply; apply requires a replica set/Atlas because all changes commit in
one transaction. No new entity IDs are generated and no documents are deleted.
"""

import argparse
import copy
import os

from bson import ObjectId
from pymongo import MongoClient


PRIMARY = {"Users": "UserID", "Business": "BusinessID", "Clients": "ClientID",
           "Stock": "StockID", "Invoices": "InvoiceID"}


def build_plan(snapshot):
    indexes = {}
    for collection, old_key in PRIMARY.items():
        lookup = {}
        for doc in snapshot.get(collection, []):
            oid = doc.get("_id")
            if not isinstance(oid, ObjectId):
                raise ValueError(f"{collection}: non-ObjectId _id; manual repair required")
            for key in (str(oid), doc.get(old_key)):
                if key is None:
                    continue
                if not isinstance(key, str) or not key:
                    raise ValueError(f"{collection}: invalid legacy identity")
                if key in lookup and lookup[key] != oid:
                    raise ValueError(f"{collection}: ambiguous legacy identity; manual repair required")
                lookup[key] = oid
        indexes[collection] = lookup

    def reference(value, collection):
        if not isinstance(value, (str, ObjectId)) or str(value) not in indexes[collection]:
            raise ValueError(f"Unresolved reference to {collection}; no writes performed")
        return indexes[collection][str(value)]

    def references(value, collection):
        if not isinstance(value, list):
            raise ValueError(f"Expected an array of references to {collection}")
        # Early console builds accidentally stored a nested BusinessIDs array.
        flattened = []
        for item in value:
            if isinstance(item, list):
                flattened.extend(references(item, collection))
            else:
                flattened.append(reference(item, collection))
        return list(dict.fromkeys(flattened))

    converted = {}
    for collection, old_key in PRIMARY.items():
        converted[collection] = []
        for original in snapshot.get(collection, []):
            doc = copy.deepcopy(original)
            doc.pop(old_key, None)
            if collection == "Users":
                doc["BusinessIDs"] = references(doc.get("BusinessIDs", []), "Business")
            elif collection == "Business":
                owners = doc.get("UserID")
                doc["UserID"] = (references(owners, "Users") if isinstance(owners, list)
                                 else reference(owners, "Users"))
                if doc["UserID"] == []:
                    raise ValueError("Business has no owner")
                doc["ClientIDs"] = references(doc.get("ClientIDs", []), "Clients")
                doc["StockIDs"] = references(doc.get("StockIDs", []), "Stock")
            elif collection in ("Clients", "Stock"):
                business = doc.get("BusinessID")
                if isinstance(business, list):
                    business = references(business, "Business")
                    if len(business) != 1:
                        raise ValueError("Child record has multiple/no businesses; manual repair required")
                    business = business[0]
                doc["BusinessID"] = reference(business, "Business")
                if "UserID" in doc:
                    doc["UserID"] = reference(doc["UserID"], "Users")
                if collection == "Clients":
                    doc["ClientStockIDs"] = references(doc.get("ClientStockIDs", []), "Stock")
                    for item in doc.get("clientStockArray", []):
                        item["StockID"] = reference(item["StockID"], "Stock")
            else:
                doc["ClientID"] = reference(doc["ClientID"], "Clients")
                for key, target in (("UserID", "Users"), ("BusinessID", "Business")):
                    if key in doc:
                        doc[key] = reference(doc[key], target)
                for item in doc.get("StockItems", []):
                    item["StockID"] = reference(item["StockID"], "Stock")
            converted[collection].append(doc)

    # Never turn conflicting legacy links into a new authorization grant.
    by_id = {name: {doc["_id"]: doc for doc in docs} for name, docs in converted.items()}
    for user in converted["Users"]:
        for bid in user["BusinessIDs"]:
            owner = by_id["Business"][bid]["UserID"]
            if user["_id"] not in (owner if isinstance(owner, list) else [owner]):
                raise ValueError("User/business ownership conflict; manual repair required")
    for business in converted["Business"]:
        for key, target in (("ClientIDs", "Clients"), ("StockIDs", "Stock")):
            for child in business[key]:
                if by_id[target][child]["BusinessID"] != business["_id"]:
                    raise ValueError("Business/child ownership conflict; manual repair required")
    for client in converted["Clients"]:
        stock_ids = client["ClientStockIDs"] + [item["StockID"] for item in client.get("clientStockArray", [])]
        for sid in stock_ids:
            if by_id["Stock"][sid]["BusinessID"] != client["BusinessID"]:
                raise ValueError("Client/stock ownership conflict; manual repair required")
    for invoice in converted["Invoices"]:
        bid = by_id["Clients"][invoice["ClientID"]]["BusinessID"]
        if "BusinessID" in invoice and invoice["BusinessID"] != bid:
            raise ValueError("Invoice/client ownership conflict; manual repair required")
        for item in invoice.get("StockItems", []):
            if by_id["Stock"][item["StockID"]]["BusinessID"] != bid:
                raise ValueError("Invoice/stock ownership conflict; manual repair required")

    plan = []
    for collection, docs in converted.items():
        for before, after in zip(snapshot.get(collection, []), docs):
            if before != after:
                plan.append((collection, after))
    return plan


def migrate(database, apply=False, session=None):
    snapshot = {name: list(database[name].find({}, session=session)) for name in PRIMARY}
    plan = build_plan(snapshot)
    if apply:
        for collection, document in plan:
            result = database[collection].replace_one({"_id": document["_id"]}, document, session=session)
            if result.matched_count != 1:
                raise RuntimeError("A migration record disappeared; transaction must abort")
    return {name: sum(collection == name for collection, _ in plan) for name in PRIMARY}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--database", required=True, help="Explicit target database name")
    parser.add_argument("--apply", action="store_true", help="Apply atomically; otherwise only validate/report")
    args = parser.parse_args()
    uri = os.environ.get("MONGODB_URI")
    if not uri:
        parser.error("Set MONGODB_URI; no default/live database is assumed")
    with MongoClient(uri) as client:
        database = client[args.database]
        if args.apply:
            with client.start_session() as session:
                counts = session.with_transaction(lambda s: migrate(database, True, s))
        else:
            counts = migrate(database)
        print("Applied" if args.apply else "Dry run", counts)


if __name__ == "__main__":
    main()
