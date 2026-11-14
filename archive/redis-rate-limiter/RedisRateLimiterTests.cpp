#include <QtTest>
#include <QHostAddress>
#include <QTcpServer>
#include <QUrl>
#include <QUuid>
#include <future>
#include <sw/redis++/redis++.h>
#include "Infrastructure/Security/RedisRateLimiter.h"

using Infrastructure::Security::RateLimitStatus;

class RedisRateLimiterTests : public QObject {
    Q_OBJECT
  private slots:
    void initTestCase() {
        test_url_ = qgetenv("INVOICE_TEST_REDIS_URL");
        if (test_url_.isEmpty())
            QSKIP("Set INVOICE_TEST_REDIS_URL to a disposable loopback Redis instance.");
        const QUrl url(QString::fromUtf8(test_url_));
        QVERIFY2(url.isValid() && QHostAddress(url.host()).isLoopback() &&
            (url.scheme() == "redis" || url.scheme() == "tcp") &&
            (url.path().isEmpty() || url.path() == "/0"), "Redis tests require an explicit loopback URL using database 0.");
        saved_url_ = qgetenv("REDIS_URL");
        had_url_ = qEnvironmentVariableIsSet("REDIS_URL");
        configured_ = true;
        sw::redis::ConnectionOptions options;
        options.host = url.host().toStdString();
        options.port = url.port(6379);
        options.password = url.password(QUrl::FullyDecoded).toStdString();
        if (!url.userName().isEmpty()) options.user = url.userName(QUrl::FullyDecoded).toStdString();
        options.connect_timeout = std::chrono::milliseconds(1000);
        options.socket_timeout = std::chrono::milliseconds(1000);
        raw_ = std::make_unique<sw::redis::Redis>(options);
        QCOMPARE(raw_->ping(), std::string("PONG"));
    }

    void init() {
        qputenv("REDIS_URL", test_url_);
        key_ = "invoice:test:rate-limit:" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    void cleanup() {
        // Never flush a shared instance. Delete only this test's unique key.
        if (raw_) raw_->del(key_.toStdString());
    }

    void cleanupTestCase() {
        if (!configured_) return;
        if (had_url_) qputenv("REDIS_URL", saved_url_);
        else qunsetenv("REDIS_URL");
    }

    void missing_or_invalid_configuration_and_uninitialised_client_fail_closed() {
        RedisRateLimiter limiter;
        QCOMPARE(limiter.check(key_, 2, 1).status, RateLimitStatus::Unavailable);
        qunsetenv("REDIS_URL");
        QVERIFY(!limiter.initialise_redis());
        qputenv("REDIS_URL", "https://example.invalid");
        QVERIFY(!limiter.initialise_redis());
        QCOMPARE(limiter.check(key_, 2, 1).status, RateLimitStatus::Unavailable);
    }

    void failed_connection_is_reported() {
        QTcpServer reservation;
        QVERIFY(reservation.listen(QHostAddress::LocalHost, 0));
        const auto port = reservation.serverPort();
        reservation.close();
        qputenv("REDIS_URL", QString("redis://127.0.0.1:%1").arg(port).toUtf8());
        RedisRateLimiter limiter;
        QVERIFY(!limiter.initialise_redis());
    }

    void counters_are_shared_between_clients_and_keys_are_independent() {
        RedisRateLimiter first, second;
        QVERIFY(first.initialise_redis());
        QVERIFY(second.initialise_redis());
        QCOMPARE(first.check(key_, 2, 5).status, RateLimitStatus::Allowed);
        QCOMPARE(second.check(key_, 2, 5).status, RateLimitStatus::Allowed);
        const auto blocked = first.check(key_, 2, 5);
        QCOMPARE(blocked.status, RateLimitStatus::Limited);
        QVERIFY(blocked.retry_after_seconds >= 1 && blocked.retry_after_seconds <= 5);
        const auto other = key_ + ":other";
        QCOMPARE(second.check(other, 2, 5).status, RateLimitStatus::Allowed);
        raw_->del(other.toStdString());
    }

    void rejected_requests_do_not_extend_expiry_or_grow_counter() {
        RedisRateLimiter limiter;
        QVERIFY(limiter.initialise_redis());
        QCOMPARE(limiter.check(key_, 1, 1).status, RateLimitStatus::Allowed);
        QTest::qSleep(600);
        for (int i = 0; i < 10; ++i)
            QCOMPARE(limiter.check(key_, 1, 1).status, RateLimitStatus::Limited);
        QCOMPARE(*raw_->get(key_.toStdString()), std::string("1"));
        QTest::qSleep(550);
        QCOMPARE(limiter.check(key_, 1, 1).status, RateLimitStatus::Allowed);
    }

    void concurrent_clients_cannot_exceed_limit() {
        RedisRateLimiter first, second;
        QVERIFY(first.initialise_redis());
        QVERIFY(second.initialise_redis());
        std::vector<std::future<Infrastructure::Security::RateLimitResult>> requests;
        for (int i = 0; i < 40; ++i)
            requests.push_back(std::async(std::launch::async, [&, i] { return (i % 2 ? first : second).check(key_, 7, 30); }));
        int allowed = 0;
        for (auto& result : requests) {
            const auto status = result.get().status;
            QVERIFY(status != RateLimitStatus::Unavailable);
            if (status == RateLimitStatus::Allowed) ++allowed;
        }
        QCOMPARE(allowed, 7);
        QCOMPARE(*raw_->get(key_.toStdString()), std::string("7"));
        QVERIFY(raw_->pttl(key_.toStdString()) > 0);
    }

    void missing_expiry_is_repaired_and_redis_script_errors_fail_closed() {
        RedisRateLimiter limiter;
        QVERIFY(limiter.initialise_redis());
        raw_->set(key_.toStdString(), "2");
        QCOMPARE(limiter.check(key_, 2, 5).status, RateLimitStatus::Limited);
        QVERIFY(raw_->pttl(key_.toStdString()) > 0);
        raw_->set(key_.toStdString(), "not-a-counter");
        QCOMPARE(limiter.check(key_, 2, 5).status, RateLimitStatus::Unavailable);
        QCOMPARE(limiter.check(key_, 0, 5).status, RateLimitStatus::Unavailable);
        QCOMPARE(limiter.check(key_, 2, 0).status, RateLimitStatus::Unavailable);
    }

  private:
    QByteArray test_url_, saved_url_;
    bool had_url_ = false, configured_ = false;
    QString key_;
    std::unique_ptr<sw::redis::Redis> raw_;
};

QTEST_GUILESS_MAIN(RedisRateLimiterTests)
#include "RedisRateLimiterTests.moc"
