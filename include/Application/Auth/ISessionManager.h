#pragma once
#include "pch.h"
#include <any>

namespace Invoke::Infrastructure::Auth {
    class ISessionManager {
      public:
        virtual ~ISessionManager() = default;

        virtual void startSession(const std::string& rUserId) = 0;
        virtual void endSession() = 0;

        virtual std::string getCurrentUserId() const = 0;

        virtual bool isSessionActive() const = 0;

        virtual void setSessionData(const std::string& rKey, const std::any& rValue) = 0;
        virtual std::any getSessionData(const std::string& rKey) const = 0;
    };
} // namespace Invoke::Infrastructure::Auth
