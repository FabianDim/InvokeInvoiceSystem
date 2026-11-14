#pragma once

#include "Infrastructure/Security/RateLimiter.h"
#include <QHash>
#include <chrono>
#include <mutex>

namespace Infrastructure::Security {

// Bounded, process-local protection for the embedded desktop server. The
// standalone backend explicitly injects Redis; it never falls back to this.
class LocalRateLimiter final : public RateLimiter {
  public:
    RateLimitResult check(const QString& key, int limit, int window_seconds) override {
        if (key.isEmpty() || key.size() > 256 || limit < 1 || window_seconds < 1)
            return {};
        const std::lock_guard lock(mutex_);
        const auto now = std::chrono::steady_clock::now();
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
    QHash<QString, Counter> counters_;
};

}
