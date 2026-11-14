#pragma once

#include <QString>

namespace Infrastructure::Security {

enum class RateLimitStatus { Allowed, Limited, Unavailable };

struct RateLimitResult {
    RateLimitStatus status = RateLimitStatus::Unavailable;
    int retry_after_seconds = 0;
};

class RateLimiter {
  public:
    virtual ~RateLimiter() = default;
    // Implementations must be thread-safe. Keys identify an action and a caller,
    // never a password/token. Rejected requests must not extend the window.
    virtual RateLimitResult check(const QString& key, int limit, int window_seconds) = 0;
};

}
