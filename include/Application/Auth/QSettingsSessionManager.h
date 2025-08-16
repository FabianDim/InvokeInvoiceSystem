#pragma once

#include "pch.h"
#include "Domain/Accounts/User.h"
#include "Application/Auth/ISessionManager.h"
#include <QSettings>
#include <string>
#include <unordered_map>
#include <any>
#include <QUuid>

namespace Invoke::Application::Auth {

    class QSettingsSessionManager final : public Infrastructure::Auth::ISessionManager {
      private:
        QSettings* settings_;
        std::string current_user_id_;
        bool session_active_;
        std::unordered_map<std::string, std::any> session_data_;

        ~QSettingsSessionManager() override = default;

        std::string create_token(const std::string& user_id);

        std::string hash_token(const std::string& token);

        void save_secture_token(const QString& token);

        QString settings_file_;

        QApplication* app_;

      public:
        QSettingsSessionManager(QApplication* main_app_);

        void start_session(const std::string& user_id) override;
        void end_session() override;

        std::string get_session_token() const override; // Missing in your original, but in ISessionManager

        std::string get_current_user_id() const override;

        bool is_session_active() const override;

        void set_session_data(const std::string& key, const std::any& value) override;
        std::any get_session_data(const std::string& key) const override;
    };

} // namespace Invoke::Application::Auth
