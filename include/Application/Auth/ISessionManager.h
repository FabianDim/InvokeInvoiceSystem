#pragma once

#include "pch.h"
#include <any>
#include <string>

namespace Invoke::Infrastructure::Auth {

    class ISessionManager {
      public:
        virtual ~ISessionManager() = default;

        virtual void start_new_session(const std::string& user_id) = 0;

        virtual void restart_session(const std::unordered_map<std::string, std::string>& session_info) = 0;

        virtual void end_session() = 0;

        virtual std::optional<std::unordered_map<std::string, std::string>> get_session_token() const = 0;
        virtual std::string get_current_user_id() const = 0;

        virtual bool is_session_active() const = 0;

        virtual bool delete_session_data() = 0;

        virtual void set_session_data(const std::string& key, const std::any& value) = 0;
        virtual std::any get_session_data(const std::string& key) const = 0;
    };

} // namespace Invoke::Infrastructure::Auth
