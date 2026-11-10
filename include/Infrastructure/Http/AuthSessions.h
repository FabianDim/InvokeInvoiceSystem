#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QRandomGenerator>
#include <memory>
#include <string>

namespace Infrastructure::Services { class InvoiceServices; }

namespace Infrastructure::Http {
// Sessions belong to this backend process. A restart requires signing in again.
class AuthSessions {
  public:
    struct Session {
        std::string user_id;
        QDateTime expires;
        std::shared_ptr<Services::InvoiceServices> invoice;
    };

    QByteArray create(const std::string& user_id, const QDateTime& now = QDateTime::currentDateTimeUtc()) {
        for (auto it = sessions_.begin(); it != sessions_.end();) {
            if (it.value()->expires <= now)
                it = sessions_.erase(it);
            else
                ++it;
        }
        if (user_id.empty() || sessions_.size() >= 1000)
            return {};
        QByteArray token;
        do {
            QByteArray random(32, Qt::Uninitialized);
            for (int i = 0; i < random.size(); i += 4) {
                const auto value = QRandomGenerator::system()->generate();
                for (int j = 0; j < 4; ++j)
                    random[i + j] = static_cast<char>(value >> (8 * j));
            }
            token = random.toHex();
        } while (sessions_.contains(token));
        auto session = std::make_shared<Session>();
        session->user_id = user_id;
        session->expires = now.addSecs(8 * 60 * 60);
        sessions_.insert(token, session);
        return token;
    }

    std::shared_ptr<Session> find(const QByteArray& token, const QDateTime& now = QDateTime::currentDateTimeUtc()) {
        auto it = sessions_.find(token);
        if (it == sessions_.end())
            return {};
        if (it.value()->expires <= now) {
            sessions_.erase(it);
            return {};
        }
        return it.value();
    }

    void revoke(const QByteArray& token) { sessions_.remove(token); }

  private:
    QHash<QByteArray, std::shared_ptr<Session>> sessions_;
};
}
