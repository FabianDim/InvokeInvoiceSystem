#pragma once
#include "pch.h"
#include "Domain/Accounts/User.h"
#include "Application/Auth/IsessionManager.h"
#include <QSettings>

using namespace Invoke::Infrastructure::Auth;

namespace Invoke::Application::Auth {
    class QSettingsSessionManager final : public ISessionManager {
      private:
        QSettings* mpQSettings;

        std::string mCurrentUserId;
        bool mSessionActive;
        std::unordered_map<std::string, std::any> mSessionData;
    };
} // namespace Invoke::Application::Auth
