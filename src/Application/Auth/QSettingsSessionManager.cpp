#include "Application/Auth/QSettingsSessionManager.h"
#include <ctime>
namespace Invoke::Application::Auth {

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
    std::string QSettingsSessionManager::createToken(const std::string& user_id) {
        std::time_t time = std::time(nullptr);
        std::tm* local_time = std::localtime(&time);
        return user_id.substr(2, user_id.size()) + std::to_string(local_time->tm_hour) +
               std::to_string(local_time->tm_min) + std::to_string(local_time->tm_sec);
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
