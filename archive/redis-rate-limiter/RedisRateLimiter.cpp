
#include "Infrastructure/Security/RedisRateLimiter.h"
#include "Infrastructure/Logging/Logging.h"
#include <QUrl>
#include <chrono>
#include <sw/redis++/redis++.h>

RedisRateLimiter::RedisRateLimiter() = default;
RedisRateLimiter::~RedisRateLimiter() = default;

Infrastructure::Security::RateLimitResult RedisRateLimiter::check(const QString& key, int limit, int window_seconds) {
    using Infrastructure::Security::RateLimitStatus;
    if (!redis_ || key.isEmpty() || key.size() > 256 || limit < 1 || window_seconds < 1 || window_seconds > 86400)
        return {};
    // A single Redis operation makes the counter and expiry atomic across dynos.
    // Saturate at the limit, retain the original expiry, and repair a key without
    // an expiry so even an old/misconfigured counter cannot permanently lock out a user.
    static constexpr auto script = R"lua(
local count = tonumber(redis.call('GET', KEYS[1]) or '0')
local ttl = redis.call('PTTL', KEYS[1])
if count >= tonumber(ARGV[1]) then
    if ttl < 0 then
        redis.call('PEXPIRE', KEYS[1], ARGV[2])
        ttl = tonumber(ARGV[2])
    end
    return math.max(1, math.ceil(ttl / 1000))
end
local updated = redis.call('INCR', KEYS[1])
if updated == 1 or ttl < 0 then
    redis.call('PEXPIRE', KEYS[1], ARGV[2])
end
return 0
)lua";
    try {
        const auto wait = redis_->eval<long long>(
            script, {key.toStdString()}, {std::to_string(limit), std::to_string(window_seconds * 1000LL)});
        if (wait < 0 || wait > 86400)
            return {};
        return {wait == 0 ? RateLimitStatus::Allowed : RateLimitStatus::Limited, static_cast<int>(wait)};
    } catch (const sw::redis::TimeoutError&) {
        qCWarning(Infrastructure::Logging::auth) << "event=rate_limit.failed reason=timeout";
    } catch (const sw::redis::IoError&) {
        qCWarning(Infrastructure::Logging::auth) << "event=rate_limit.failed reason=connection_error";
    } catch (const sw::redis::ReplyError&) {
        qCWarning(Infrastructure::Logging::auth) << "event=rate_limit.failed reason=server_rejected_command";
    } catch (const sw::redis::Error&) {
        qCWarning(Infrastructure::Logging::auth) << "event=rate_limit.failed reason=redis_error";
    }
    return {}; // Fail closed; callers return 503 rather than bypassing throttling.
}

bool RedisRateLimiter::initialise_redis() {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::database(),
                                                 "RedisRateLimiter.initialise_redis");
    redis_.reset();
    const auto configured_url = qEnvironmentVariable("REDIS_URL");
    if (configured_url.isEmpty()) {
        qCWarning(Infrastructure::Logging::database) << "event=redis.initialization_failed reason=missing_redis_url";
        return false;
    }

    // redis++ 1.3.x does not parse rediss:// itself. Use explicit options so
    // hosted Redis URLs enable TLS and percent-encoded credentials are decoded.
    const QUrl url(configured_url, QUrl::StrictMode);
    const auto scheme = url.scheme();
    const int port = url.port(6379);
    const auto path = url.path();
    bool valid_database = true;
    const int database = path.isEmpty() || path == "/" ? 0 : path.mid(1).toInt(&valid_database);
    if (!url.isValid() || url.host().isEmpty() || (scheme != "redis" && scheme != "rediss" && scheme != "tcp") ||
        port < 1 || port > 65535 || !valid_database || database < 0 || url.hasQuery() || url.hasFragment()) {
        qCWarning(Infrastructure::Logging::database) << "event=redis.initialization_failed reason=invalid_redis_url";
        return false;
    }

    try {
        sw::redis::ConnectionOptions options;
        options.host = url.host().toStdString();
        options.port = port;
        options.db = database;
        const auto username = url.userName(QUrl::FullyDecoded);
        if (!username.isEmpty())
            options.user = username.toStdString();
        options.password = url.password(QUrl::FullyDecoded).toStdString();
        options.connect_timeout = std::chrono::milliseconds(2000);
        options.socket_timeout = std::chrono::milliseconds(2000);
        options.tls.enabled = scheme == "rediss";
        if (options.tls.enabled) {
            options.tls.sni = options.host;
            // Keep certificate verification enabled. Supply the provider's trusted
            // CA bundle here when its certificate is not in the system trust store.
            options.tls.cacert = qEnvironmentVariable("REDIS_CA_CERT_FILE").toStdString();
        }

        auto connection = std::make_unique<sw::redis::Redis>(options);
        // Construction alone does not establish a working connection.
        if (connection->ping() != "PONG") {
            qCWarning(Infrastructure::Logging::database)
                << "event=redis.initialization_failed reason=unexpected_ping_reply";
            return false;
        }
        redis_ = std::move(connection);
        qCInfo(Infrastructure::Logging::database) << "event=redis.ready tls=" << options.tls.enabled;
        return true;
    } catch (const sw::redis::TimeoutError&) {
        qCWarning(Infrastructure::Logging::database) << "event=redis.initialization_failed reason=timeout";
    } catch (const sw::redis::IoError&) {
        qCWarning(Infrastructure::Logging::database) << "event=redis.initialization_failed reason=connection_error";
    } catch (const sw::redis::ReplyError&) {
        qCWarning(Infrastructure::Logging::database)
            << "event=redis.initialization_failed reason=server_rejected_command";
    } catch (const sw::redis::Error&) {
        qCWarning(Infrastructure::Logging::database) << "event=redis.initialization_failed reason=redis_error";
    }
    // Do not log the URL or raw exception text: they may contain credentials.
    return false;
}
