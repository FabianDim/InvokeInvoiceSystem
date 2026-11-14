#pragma once
#include <QHash>
#include <chrono>
#include <functional>
#include <mutex>
#include <utility>
#include "RateLimiter.h"
namespace Infrastructure::Security {

// Bounded fixed windows, isolated to this instance. Share one instance between
// callers that should share quotas; restarting the server resets its counters.
class LocalRateLimiter final : public RateLimiter {
  public:
    using Clock = std::chrono::steady_clock;
    using Now = std::function<Clock::time_point()>;

    // The optional clock must be monotonic. It is sampled under the mutex so
    // concurrent checks cannot observe timestamps in a different order.
    explicit LocalRateLimiter(Now now = Clock::now) : now_(now ? std::move(now) : Now(Clock::now)) {}

    RateLimitResult check(const QString& key, int limit, int window_seconds) override {
        if (key.isEmpty() || key.size() > 256 || limit < 1 || window_seconds < 1)
            return {};
        const std::lock_guard lock(mutex_);
        const auto now = now_();
        for (auto it = counters_.begin(); it != counters_.end();) {
            if (it->expires <= now)
                it = counters_.erase(it);
            else
                ++it;
        }
        auto it = counters_.find(key);
        if (it == counters_.end()) {
            if (counters_.size() >= 4096)
                return {};
            counters_.insert(key, {1, now + std::chrono::seconds(window_seconds)});
            return {RateLimitStatus::Allowed, 0};
        }
        if (it->count >= limit) {
            const auto remaining = std::chrono::ceil<std::chrono::seconds>(it->expires - now).count();
            return {RateLimitStatus::Limited, static_cast<int>(remaining)};
        }
        ++it->count;
        return {RateLimitStatus::Allowed, 0};
    }

  private:
    struct Counter {
        int count;
        std::chrono::steady_clock::time_point expires;
    };
    std::mutex mutex_;
    Now now_;
    QHash<QString, Counter> counters_;
};

} // namespace Infrastructure::Security
