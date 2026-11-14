#include "Infrastructure/Security/LocalRateLimiter.h"
#include <QtTest>
#include <future>
#include <limits>
#include <vector>

using namespace Infrastructure::Security;
using namespace std::chrono_literals;

class LocalRateLimiterTests : public QObject {
    Q_OBJECT
  private slots:
    void limits_are_exact_and_keys_and_instances_are_independent() {
        LocalRateLimiter limiter;
        RateLimiter& interface = limiter;
        for (int i = 0; i < 3; ++i) {
            const auto result = interface.check("login:a", 3, 60);
            QCOMPARE(result.status, RateLimitStatus::Allowed);
            QCOMPARE(result.retry_after_seconds, 0);
        }
        QCOMPARE(interface.check("login:a", 3, 60).status, RateLimitStatus::Limited);
        QCOMPARE(interface.check("login:b", 3, 60).status, RateLimitStatus::Allowed);
        LocalRateLimiter other;
        QCOMPARE(other.check("login:a", 3, 60).status, RateLimitStatus::Allowed);
    }

    void invalid_inputs_fail_closed_without_consuming_quota_data() {
        QTest::addColumn<QString>("key");
        QTest::addColumn<int>("limit");
        QTest::addColumn<int>("window");
        QTest::newRow("empty-key") << QString() << 1 << 60;
        QTest::newRow("long-key") << QString(257, 'a') << 1 << 60;
        QTest::newRow("zero-limit") << QString("key") << 0 << 60;
        QTest::newRow("negative-limit") << QString("key") << -1 << 60;
        QTest::newRow("minimum-limit") << QString("key") << std::numeric_limits<int>::min() << 60;
        QTest::newRow("zero-window") << QString("key") << 1 << 0;
        QTest::newRow("negative-window") << QString("key") << 1 << -1;
    }

    void invalid_inputs_fail_closed_without_consuming_quota() {
        QFETCH(QString, key);
        QFETCH(int, limit);
        QFETCH(int, window);
        LocalRateLimiter limiter;
        const auto result = limiter.check(key, limit, window);
        QCOMPARE(result.status, RateLimitStatus::Unavailable);
        QCOMPARE(result.retry_after_seconds, 0);
        QCOMPARE(limiter.check("key", 1, 60).status, RateLimitStatus::Allowed);
        QCOMPARE(limiter.check(QString(256, 'a'), 1, 60).status, RateLimitStatus::Allowed);
    }

    void fixed_window_expires_exactly_and_retries_round_up() {
        auto now = LocalRateLimiter::Clock::time_point{};
        LocalRateLimiter limiter([&] { return now; });
        QCOMPARE(limiter.check("key", 2, 2).status, RateLimitStatus::Allowed);
        now += 500ms;
        // An allowed request must not slide the expiry either.
        QCOMPARE(limiter.check("key", 2, 2).status, RateLimitStatus::Allowed);
        auto result = limiter.check("key", 2, 2);
        QCOMPARE(result.status, RateLimitStatus::Limited);
        QCOMPARE(result.retry_after_seconds, 2);
        now += 500ms;
        QCOMPARE(limiter.check("key", 2, 2).retry_after_seconds, 1);
        now += 999ms;
        for (int i = 0; i < 10; ++i) {
            result = limiter.check("key", 2, 2);
            QCOMPARE(result.status, RateLimitStatus::Limited);
            QCOMPARE(result.retry_after_seconds, 1);
        }
        now += 1ms;
        result = limiter.check("key", 2, 2);
        QCOMPARE(result.status, RateLimitStatus::Allowed);
        QCOMPARE(result.retry_after_seconds, 0);
        QCOMPARE(limiter.check("key", 2, 2).status, RateLimitStatus::Allowed);
        QCOMPARE(limiter.check("key", 2, 2).retry_after_seconds, 2);
    }

    void maximum_window_does_not_overflow() {
        auto now = LocalRateLimiter::Clock::time_point{};
        LocalRateLimiter limiter([&] { return now; });
        const int maximum = std::numeric_limits<int>::max();
        QCOMPARE(limiter.check("key", 1, maximum).status, RateLimitStatus::Allowed);
        QCOMPARE(limiter.check("key", 1, maximum).retry_after_seconds, maximum);
        now += std::chrono::seconds(maximum);
        QCOMPARE(limiter.check("key", 1, maximum).status, RateLimitStatus::Allowed);
    }

    void capacity_preserves_live_counters_and_reclaims_expired_entries() {
        auto now = LocalRateLimiter::Clock::time_point{};
        LocalRateLimiter limiter([&] { return now; });
        QCOMPARE(limiter.check("live", 2, 60).status, RateLimitStatus::Allowed);
        for (int i = 0; i < 4095; ++i)
            QCOMPARE(limiter.check(QString::number(i), 1, 1).status, RateLimitStatus::Allowed);
        QCOMPARE(limiter.check("overflow", 1, 60).status, RateLimitStatus::Unavailable);
        QCOMPARE(limiter.check("live", 2, 60).status, RateLimitStatus::Allowed);
        QCOMPARE(limiter.check("live", 2, 60).status, RateLimitStatus::Limited);
        now += 1s;
        QCOMPARE(limiter.check("overflow", 1, 60).status, RateLimitStatus::Allowed);
        QCOMPARE(limiter.check("live", 2, 60).status, RateLimitStatus::Limited);
        for (int i = 0; i < 4094; ++i)
            QCOMPARE(limiter.check(QString::number(i), 1, 60).status, RateLimitStatus::Allowed);
        QCOMPARE(limiter.check("still-full", 1, 60).status, RateLimitStatus::Unavailable);
    }

    void concurrent_requests_cannot_exceed_quota_or_capacity_data() {
        QTest::addColumn<bool>("fill_capacity");
        QTest::newRow("shared-key") << false;
        QTest::newRow("new-keys-at-capacity") << true;
    }

    void concurrent_requests_cannot_exceed_quota_or_capacity() {
        QFETCH(bool, fill_capacity);
        // Freeze time: this test checks serialization, regardless of machine load.
        LocalRateLimiter limiter([] { return LocalRateLimiter::Clock::time_point{}; });
        if (fill_capacity) {
            for (int i = 0; i < 4095; ++i)
                QCOMPARE(limiter.check(QString::number(i), 1, 60).status, RateLimitStatus::Allowed);
        }
        std::promise<void> start;
        const auto ready = start.get_future().share();
        std::vector<std::future<RateLimitResult>> requests;
        for (int i = 0; i < 32; ++i) {
            requests.push_back(std::async(std::launch::async, [&, i, ready] {
                ready.wait();
                return limiter.check(fill_capacity ? QString("new:%1").arg(i) : QString("shared"), 7, 60);
            }));
        }
        start.set_value();
        int allowed = 0;
        for (auto& request : requests) {
            const auto result = request.get();
            if (result.status == RateLimitStatus::Allowed) {
                ++allowed;
            } else if (fill_capacity) {
                QCOMPARE(result.status, RateLimitStatus::Unavailable);
            } else {
                QCOMPARE(result.status, RateLimitStatus::Limited);
                QCOMPARE(result.retry_after_seconds, 60);
            }
        }
        QCOMPARE(allowed, fill_capacity ? 1 : 7);
    }
};

QTEST_GUILESS_MAIN(LocalRateLimiterTests)
#include "LocalRateLimiterTests.moc"
