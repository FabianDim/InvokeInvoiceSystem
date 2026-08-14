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
    static constexpr auto KEY_USER_ID = "invoke_current_user_id";
    static constexpr auto MONTH_IN_SECONDS = 2592000;

    QSettingsSessionManager::QSettingsSessionManager() : settings_(new QSettings), session_active_(false) {}

    void QSettingsSessionManager::start_new_session(const std::string& user_id) {
        try {
            if (!user_id.empty()) {
                current_user_id_ = user_id;
                session_active_ = true;
                save_secure_token(create_token());
            } else {
                throw std::invalid_argument("User ID cannot be empty.");
            }
        } catch (const std::exception& e) {
            qWarning() << "Failed to start session: " << e.what();
            return;
        }
    }
    void QSettingsSessionManager::restart_session(const std::unordered_map<std::string, std::string>& session_info) {
        try {
            if (is_session_data_valid(session_info)) {
                current_user_id_ = session_info.at(KEY_USER_ID);
                session_active_ = true;
            } else {
                delete_session_data();
                throw std::runtime_error("Invalid or expired session data.");
            }
        } catch (const std::exception& e) {
            qWarning() << "Failed to restart session: " << e.what();
            return;
        }
    }
    QString QSettingsSessionManager::create_token() {
        const auto uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        return uuid;
    }

    bool
    QSettingsSessionManager::is_session_data_valid(std::unordered_map<std::string, std::string> keychain_info) const {
        if (keychain_info.contains(KEY_ISSUED_AT) && keychain_info.contains(KEY_TOKEN) &&
            keychain_info.contains(KEY_USER_ID)) {
            const auto issuedAtStr = keychain_info.at(KEY_ISSUED_AT);
            const auto token = keychain_info.at(KEY_TOKEN);
            const auto userId = keychain_info.at(KEY_USER_ID);
            if (!issuedAtStr.empty() && !token.empty() && !userId.empty()) {
                bool ok = false;
                const qint64 issuedAtSecs = QString::fromStdString(issuedAtStr).toLongLong(&ok);
                if (ok) {
                    const qint64 currentSecs = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
                    if ((currentSecs - issuedAtSecs) < MONTH_IN_SECONDS) {
                        return true;
                    } else {
                        qWarning() << "Session token expired.";
                        return false;
                    }
                } else {
                    qWarning() << "Invalid issued_at timestamp.";
                    return false;
                }
            } else {
                qWarning() << "Token or user ID is empty.";
                return false;
            }
        }
        return false;
    }

    bool QSettingsSessionManager::save_secure_token(const QString& token) {
        auto* job = new WritePasswordJob(SERVICE);
        job->setAutoDelete(false);
        job->setKey(KEY_TOKEN);
        job->setTextData(token);

        QEventLoop loop;
        QObject::connect(job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);

        job->start();
        loop.exec();

        const bool ok = (job->error() == QKeychain::NoError);
        if (ok) {
            const qint64 nowSecs = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
            settings_->setValue(KEY_ISSUED_AT, nowSecs);
            settings_->setValue(KEY_USER_ID, QString::fromStdString(current_user_id_));
            settings_->sync();
        } else {
            qWarning() << "Keychain write error:" << job->errorString();
        }
        delete job;
        return ok;
    }

    void QSettingsSessionManager::end_session() {
        session_active_ = false;
        current_user_id_.clear();
    }

    bool Invoke::Application::Auth::QSettingsSessionManager::delete_session_data() {
        auto* job = new DeletePasswordJob(SERVICE);
        job->setKey(KEY_TOKEN);

        QEventLoop loop;
        QObject::connect(job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);

        job->start(); // <-- start it
        loop.exec();  // <-- wait

        const bool ok = (job->error() == QKeychain::NoError || job->error() == QKeychain::EntryNotFound);

        if (ok) {
            settings_->remove(KEY_ISSUED_AT);
            settings_->remove(KEY_USER_ID);
            settings_->sync();
            delete job;
            return true;
        } else {
            qWarning() << "Keychain delete error:" << job->errorString();
            delete job;
            return false;
        }
        return true;
    }

    std::optional<std::unordered_map<std::string, std::string>>
    Invoke::Application::Auth::QSettingsSessionManager::get_session_token() const {
        auto* job = new ReadPasswordJob(SERVICE);
        job->setAutoDelete(false);
        job->setKey(KEY_TOKEN);

        QEventLoop loop;
        QObject::connect(job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);

        job->start(); // <-- start it
        loop.exec();  // <-- wait

        const bool ok = (job->error() == QKeychain::NoError);
        if (ok) {
            const auto token = job->textData().toStdString();
            const qint64 issuedAtSecs = settings_->value(KEY_ISSUED_AT, 0).toLongLong();
            const auto issuedAtStr = (issuedAtSecs > 0) ? QString::number(issuedAtSecs).toStdString() : std::string{};

            delete job;
            return std::unordered_map<std::string, std::string>{
                {KEY_ISSUED_AT, issuedAtStr},
                {KEY_TOKEN, token},
                {KEY_USER_ID, current_user_id_ != "" ? current_user_id_ : "User not found"}};
        } else {
            qWarning() << "Keychain read error:" << job->errorString();
            delete job;
            return std::nullopt;
        }
    }

    std::string QSettingsSessionManager::get_current_user_id() const {
        return current_user_id_;
    }

    bool QSettingsSessionManager::is_session_active() const {
        return session_active_;
    }

    void QSettingsSessionManager::set_session_data(const std::string& key, const std::any& value) {}

    std::any QSettingsSessionManager::get_session_data(const std::string& key) const {
        return {};
    }

} // namespace Invoke::Application::Auth
