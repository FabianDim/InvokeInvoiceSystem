#include <QtTest/QtTest>
#include <QElapsedTimer>
#include <QUuid>
#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/options/find.hpp>
#include "Infrastructure/Database/Mongo/MongoDBHandler.h"
#include <chrono>
#include <memory>

class DatabaseTests : public QObject {
    // This is a live test environment.
    Q_OBJECT

  private slots:
    void initTestCase();
    void heartbeat_data();
    void heartbeat();
    void authenticatedSession();
    void writablePrimary();
    void signupCollectionReadable();
    void cleanupTestCase();

  private:
    std::unique_ptr<MongoDBHandler> handler;
};

void DatabaseTests::initTestCase() {
    QVERIFY2(!qEnvironmentVariableIsEmpty("MONGODB_URI"),
             "Set MONGODB_URI in the test process environment. Restart the IDE after changing Windows variables.");
    try {
        handler = std::make_unique<MongoDBHandler>();
    } catch (const std::exception&) {
        // URI parsing errors can contain credentials, so do not print exception text here.
        QFAIL("Could not initialize MongoDB. Check URI syntax, password encoding, and SRV DNS resolution.");
    }
}

void DatabaseTests::heartbeat_data() {
    QTest::addColumn<int>("sample");
    QTest::newRow("first-ping") << 1;
    QTest::newRow("second-ping") << 2;
    QTest::newRow("third-ping") << 3;
}

void DatabaseTests::heartbeat() {
    QFETCH(int, sample);
    try {
        QElapsedTimer timer;
        timer.start();
        const auto reply = handler->getDatabase().run_command(make_document(kvp("ping", 1)));
        const auto ok = reply.view()["ok"];
        QVERIFY2(ok, "Ping response is missing status.");
        switch (ok.type()) {
        case bsoncxx::type::k_double:
            QCOMPARE(ok.get_double().value, 1.0);
            break;
        case bsoncxx::type::k_int32:
            QCOMPARE(ok.get_int32().value, 1);
            break;
        case bsoncxx::type::k_int64:
            QCOMPARE(ok.get_int64().value, std::int64_t{1});
            break;
        default:
            QFAIL("Ping response status is not numeric.");
        }
        qInfo() << "[DatabaseTests] Heartbeat" << sample << "succeeded in" << timer.elapsed() << "ms";
    } catch (const mongocxx::exception& error) {
        const auto message = QString("Heartbeat failed: %1. Check credentials/authSource for bad auth; "
                                     "check DNS, IP allowlist, and network for connection timeouts.")
                                 .arg(QString::fromUtf8(error.what()));
        QFAIL(qPrintable(message));
    }
}

void DatabaseTests::authenticatedSession() {
    try {
        // A ping alone does not prove that the connection has an authenticated database user.
        const auto reply = handler->getDatabase().run_command(make_document(kvp("connectionStatus", 1)));
        const auto authInfo = reply.view()["authInfo"];
        QVERIFY2(authInfo && authInfo.type() == bsoncxx::type::k_document, "Missing authentication status.");
        const auto users = authInfo.get_document().view()["authenticatedUsers"];
        QVERIFY2(users && users.type() == bsoncxx::type::k_array, "Missing authenticated user list.");
        const auto authenticatedUsers = users.get_array().value;
        QVERIFY2(authenticatedUsers.begin() != authenticatedUsers.end(),
                 "MongoDB is reachable, but no database user is authenticated on this connection.");
    } catch (const mongocxx::exception& error) {
        const auto message = QString("Authentication check failed: %1").arg(QString::fromUtf8(error.what()));
        QFAIL(qPrintable(message));
    }
}

void DatabaseTests::writablePrimary() {
    try {
        const auto reply = handler->getDatabase().run_command(make_document(kvp("hello", 1)));
        const auto writable = reply.view()["isWritablePrimary"];
        QVERIFY2(writable && writable.type() == bsoncxx::type::k_bool, "Missing primary status in hello response.");
        QVERIFY2(writable.get_bool().value, "Connected server is not writable; signup needs a writable primary.");
    } catch (const mongocxx::exception& error) {
        const auto message = QString("Primary check failed: %1").arg(QString::fromUtf8(error.what()));
        QFAIL(qPrintable(message));
    }
}

void DatabaseTests::signupCollectionReadable() {
    try {
        const auto probeEmail = ("db-health-" + QUuid::createUuid().toString(QUuid::WithoutBraces)
                                 + "@example.invalid").toStdString();
        mongocxx::options::find options;
        options.projection(make_document(kvp("_id", 1)));
        options.max_time(std::chrono::seconds(5));
        // Exercise signup's actual collection and field without reading user details or writing data.
        const auto result = handler->getDatabase()["Users"].find_one(
            make_document(kvp("UserEmail", probeEmail)), options);
        QVERIFY2(!result, "Unexpected document matched the unique health-check email.");
    } catch (const mongocxx::exception& error) {
        const auto message = QString("Users collection read failed: %1. Check authentication and read permissions.")
                                 .arg(QString::fromUtf8(error.what()));
        QFAIL(qPrintable(message));
    }
}

void DatabaseTests::cleanupTestCase() {
    handler.reset();
}

QTEST_GUILESS_MAIN(DatabaseTests)
#include "DatabaseTests.moc"
