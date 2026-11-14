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
    // Use a consistent policy per key. The first allowed request starts a fixed
    // window; subsequent checks use its expiry and the supplied limit.
    // Invalid input or exhausted storage returns Unavailable, which callers must
    // fail closed. Limited includes a positive, rounded-up retry delay.
    virtual RateLimitResult check(const QString& key, int limit, int window_seconds) = 0;
};

}
