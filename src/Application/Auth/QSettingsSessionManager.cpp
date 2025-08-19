#include "Application/Auth/QSettingsSessionManager.h"
#include <ctime>
#include <QStandardPaths>
#include <QDir>
#include <Infrastructure/Security/PasswordHashing/bcrypt.h>
#include <qt6keychain/keychain.h>
#include <qeventloop.h>
using namespace QKeychain;
namespace Invoke::Application::Auth {

    static constexpr auto SERVICE = "InvokeInvoiceSystem";
    static constexpr auto KEY_TOKEN = "invoke_session_token";
    static constexpr auto KEY_ISSUED_AT = "invoke_session_token_issued_at";

    QSettingsSessionManager::QSettingsSessionManager(QApplication* main_app_)
        : settings_(new QSettings), session_active_(false), app_(main_app_) {}

    void QSettingsSessionManager::start_session(const std::string& user_id) {
        try {
            if (!user_id.empty()) {
                current_user_id_ = user_id;
                session_active_ = true;

            } else {
                throw std::invalid_argument("User ID cannot be empty.");
            }
        } catch (const std::exception& e) {
            qWarning() << "Failed to start session: " << e.what();
            return;
        }
    }
    std::string QSettingsSessionManager::create_token(const std::string&) {
        const auto uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        return uuid.toStdString();
    }
    bool QSettingsSessionManager::save_secure_token(const QString& token) {
        auto* job = new WritePasswordJob(SERVICE);
        job->setAutoDelete(false);
        job->setKey(KEY_TOKEN);
        job->setTextData(token);

        QEventLoop loop;

        QObject::connect(job,                       // 1) sender
                         &QKeychain::Job::finished, // 2) signal
                         &loop,                     // 3) receiver
                         &QEventLoop::quit          // 4) slot (method to call)
        );

        const bool ok = (job->error() == QKeychain::NoError);

        if (ok) {
            settings_->setValue(KEY_ISSUED_AT, QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
            settings_->sync();
        } else {
            qWarning() << "Keychain write error:" << job->errorString();
        }
        delete job;
        return ok;
    }

    void QSettingsSessionManager::end_session() {}

    std::string QSettingsSessionManager::get_session_token() const {
        // Placeholder – implement actual token retrieval if required
        auto* job = new ReadPasswordJob(SERVICE);
        job->setAutoDelete(false);
        job->setKey(KEY_TOKEN);

        QEventLoop loop;

        QObject::connect(job,                       // 1) sender
                         &QKeychain::Job::finished, // 2) signal
                         &loop,                     // 3) receiver
                         &QEventLoop::quit          // 4) slot (method to call)
        );

        return;
    }

    std::string QSettingsSessionManager::get_current_user_id() const {
        return "";
    }

    bool QSettingsSessionManager::is_session_active() const {
        return session_active_;
    }

    void QSettingsSessionManager::set_session_data(const std::string& key, const std::any& value) {}

    std::any QSettingsSessionManager::get_session_data(const std::string& key) const {
        return {};
    }

} // namespace Invoke::Application::Auth
