#pragma once
#include <QObject>
#include <string>

class IAccountManager : public QObject {
    Q_OBJECT

public:
    explicit IAccountManager(QObject* parent = nullptr)
        : QObject(parent)
        , m_isLoggedIn(false)
        , m_needsAccountSetup(false)
    {
    }

    ~IAccountManager() override = default;

    // public interface for UI
    virtual bool validEmail(const std::string& email) = 0;
    virtual bool validName(const std::string& name) = 0;
    virtual bool doesAccountExist(const std::string& username) = 0;
    virtual bool doesPasswordMatch(const std::string& password) = 0;
    virtual void createAccount(std::string & userEmail, std::string & userPassword) = 0;
    virtual void login() = 0;
    virtual void logOut() = 0;
    virtual bool isLoggedIn() const = 0;
    virtual bool needsAccountSetup(const std::string& email) = 0;

protected:
    bool m_isLoggedIn;
    bool m_needsAccountSetup;
};