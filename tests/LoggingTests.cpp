#include "Infrastructure/Logging/HttpLogging.h"
#include <QtTest>
#include <QHttpServer>
#include <QNetworkAccessManager>
#include <QTcpServer>
#include <QTemporaryDir>

using namespace Infrastructure::Logging;

namespace {
QStringList messages;
void capture(QtMsgType, const QMessageLogContext& context, const QString& message) {
    messages.append(QString::fromLatin1(context.category) + ' ' + message + " request_id=" + requestContext);
}
struct Capture {
    QtMessageHandler previous = qInstallMessageHandler(capture);
    Capture() { messages.clear(); }
    ~Capture() { qInstallMessageHandler(previous); }
};
}

class LoggingTests : public QObject {
    Q_OBJECT
private slots:
    void correlationAndPayloadPrivacy() {
        QHttpServer server;
        auto* tcp = new QTcpServer(&server);
        QVERIFY(tcp->listen(QHostAddress::LocalHost, 0));
        QVERIFY(server.bind(tcp));
        server.route("/test", QHttpServerRequest::Method::Post,
            tracedRoute("/test", [](const QHttpServerRequest&) {
                Operation operation(database(), "test.lookup");
                return QHttpServerResponse("application/json", "{\"secret\":\"response-secret\"}",
                    QHttpServerResponder::StatusCode::Unauthorized);
            }));
        QNetworkAccessManager network;
        const auto id = newRequestId();
        QNetworkRequest request(QUrl(QString("http://127.0.0.1:%1/test?secret=query-secret").arg(tcp->serverPort())));
        request.setRawHeader("X-Request-ID", id);
        request.setRawHeader("Authorization", "Bearer token-secret");
        Capture capture;
        auto* reply = network.post(request, "password=body-secret");
        traceReply(reply);
        QSignalSpy finished(reply, &QNetworkReply::finished);
        QVERIFY(finished.wait(5000));
        QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 401);
        QCOMPARE(reply->rawHeader("X-Request-ID"), id);
        QCOMPARE(reply->readAll(), QByteArray("{\"secret\":\"response-secret\"}"));
        const auto logs = messages.join('\n');
        QVERIFY(logs.contains("server.request.started"));
        QVERIFY(logs.contains("server.request.failed"));
        QVERIFY(logs.contains("client.request.failed"));
        QVERIFY(logs.contains("test.lookup"));
        QVERIFY(logs.contains("elapsed_ms="));
        QVERIFY(logs.contains(id));
        for (const auto& secret : {"query-secret", "token-secret", "body-secret", "response-secret"})
            QVERIFY2(!logs.contains(secret), secret);
        QVERIFY(requestContext.isEmpty());
        reply->deleteLater();
    }

    void invalidRequestIdsAndNestedContext() {
        const auto malicious = QByteArray("untrusted\r\nforged-log-line");
        const auto safe = requestId(malicious);
        QCOMPARE(safe.size(), 36);
        QVERIFY(safe != malicious);
        QCOMPARE(requestId(safe), safe);
        {
            RequestContext outer(safe);
            {
                RequestContext inner(newRequestId());
                QVERIFY(requestContext != safe);
            }
            QCOMPARE(requestContext, safe);
        }
        QVERIFY(requestContext.isEmpty());
    }

    void operationExceptionsAndFiltering() {
        Capture capture;
        try {
            Operation operation(pdf(), "test.render");
            throw std::runtime_error("private-exception-detail");
        } catch (const std::exception&) {}
        const auto logs = messages.join('\n');
        QVERIFY(logs.contains("operation.exception"));
        QVERIFY(!logs.contains("private-exception-detail"));
        QLoggingCategory::setFilterRules("invoice.pdf.debug=false");
        messages.clear();
        { Operation operation(pdf(), "test.filtered"); }
        const bool empty = messages.isEmpty();
        QLoggingCategory::setFilterRules(QString{});
        QVERIFY(empty);
    }

    void fileSinkEscapesAndRotates() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto path = directory.filePath("application.log");
        QFile seed(path);
        QVERIFY(seed.open(QIODevice::WriteOnly));
        QVERIFY(seed.resize(5 * 1024 * 1024));
        seed.close();
        sink().file.setFileName(path);
        QVERIFY(sink().file.open(QIODevice::WriteOnly | QIODevice::Append));
        messageHandler(QtDebugMsg, QMessageLogContext(__FILE__, __LINE__, Q_FUNC_INFO, "invoice.app"),
                       "event=test.first\nforged\rline");
        sink().file.close();
        QVERIFY(QFile::exists(path + ".1"));
        QFile result(path);
        QVERIFY(result.open(QIODevice::ReadOnly));
        const auto content = result.readAll();
        QCOMPARE(content.count('\n'), 1);
        QVERIFY(content.contains("\\nforged\\rline"));
    }
};

QTEST_GUILESS_MAIN(LoggingTests)
#include "LoggingTests.moc"
