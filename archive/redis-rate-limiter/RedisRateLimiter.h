#pragma once

#include <memory>
#include "Infrastructure/Security/RateLimiter.h"

namespace sw::redis {
class Redis;
}

class RedisRateLimiter final : public Infrastructure::Security::RateLimiter {
  public:
    RedisRateLimiter();
    ~RedisRateLimiter();

    // Call during backend startup, before accepting requests. Reads REDIS_URL.
    bool initialise_redis();
    Infrastructure::Security::RateLimitResult check(const QString& key, int limit, int window_seconds) override;

  private:
    std::unique_ptr<sw::redis::Redis> redis_;
};
