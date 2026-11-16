#include "KeyChainClass.h"
class LocalAccountKeychain {
    using Clock = std::chrono::steady_clock;
    using Now = std::function<Clock::time_point()>;
    explicit LocalAccountKeychain(Now now = Clock::now) : now_(now ? std::move(now) : Now(Clock::now)) {}

  public:
    bool remember_email(const QString& email);
    bool remember_password(const QString& password);

  private:
    Now now_;
};