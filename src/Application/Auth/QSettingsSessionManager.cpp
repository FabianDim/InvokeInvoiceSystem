#include "Application/Auth/QSettingsSessionManager.h"
#include <ctime>
#include <QStandardPaths>
#include <QDir>
#include <Infrastructure/Security/PasswordHashing/bcrypt.h>
#include <qt6keychain/keychain.h>
using namespace QKeychain;
namespace Invoke::Application::Auth {
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
            qWarning() << "Failed to start session:" << e.what();
            return;
        }
    }
    std::string QSettingsSessionManager::create_token(const std::string& user_id) {
        std::time_t time = std::time(nullptr);
        std::tm* local_time = std::localtime(&time);
        return hash_token(user_id.substr(2, user_id.size()) + std::to_string(local_time->tm_hour) +
                          std::to_string(local_time->tm_min) + std::to_string(local_time->tm_sec));
    }

    std::string QSettingsSessionManager::hash_token(const std::string& token) {
        return bcrypt::generateHash(token);
    }
    void QSettingsSessionManager::save_secture_token(const QString& token) {
        auto* job = new WritePasswordJob("InvokeInvoiceSystem");
        job->setKey("session_token");
        job->setTextData(token);
    }

    void QSettingsSessionManager::end_session() {}

    std::string QSettingsSessionManager::get_session_token() const {
        // Placeholder – implement actual token retrieval if required
        return {};
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
