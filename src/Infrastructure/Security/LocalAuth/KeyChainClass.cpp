#include <QDebug>

#include <memory>

#include "Infrastructure/Security/LocalAuth/KeyChainClass.h"

namespace Infrastructure::Security {

static const QString service = "keychain.invoke.invoice.app";

KeyChainClass::KeyChainClass(QObject* parent) : QObject(parent) {}

void KeyChainClass::readKey(const QString& key) {
    auto readCredentialJob = std::make_unique<QKeychain::ReadPasswordJob>(service);
    readCredentialJob->setAutoDelete(true);
    readCredentialJob->setKey(key);

    QObject::connect(
        readCredentialJob.get(), &QKeychain::ReadPasswordJob::finished, this, [this, key](QKeychain::Job* job) {
            auto j = static_cast<QKeychain::ReadPasswordJob*>(job);
            if (j->error() == QKeychain::NoError) {
                emit keyRestored(key, j->textData());
            } else {
                emit error(tr("Read key failed: %1").arg(qPrintable(j->errorString())));
            }
            // no delete needed, autoDelete takes care of it
        });

    readCredentialJob->start();
}

void KeyChainClass::writeKey(const QString& key, const QString& value) {
    auto writeCredentialJob = std::make_unique<QKeychain::WritePasswordJob>(service);
    writeCredentialJob->setAutoDelete(true);
    writeCredentialJob->setKey(key);
    writeCredentialJob->setTextData(value);

    QObject::connect(
        writeCredentialJob.get(), &QKeychain::WritePasswordJob::finished, this, [this, key](QKeychain::Job* job) {
            auto j = static_cast<QKeychain::WritePasswordJob*>(job);
            if (j->error() == QKeychain::NoError) {
                emit keyStored(key);
            } else {
                emit error(tr("Write key failed: %1").arg(qPrintable(j->errorString())));
            }
        });

    writeCredentialJob->start();
}

void KeyChainClass::deleteKey(const QString& key) {
    auto deleteCredentialJob = std::make_unique<QKeychain::DeletePasswordJob>(service);
    deleteCredentialJob->setAutoDelete(true);
    deleteCredentialJob->setKey(key);

    QObject::connect(
        deleteCredentialJob.get(), &QKeychain::DeletePasswordJob::finished, this, [this, key](QKeychain::Job* job) {
            auto j = static_cast<QKeychain::DeletePasswordJob*>(job);
            if (j->error() == QKeychain::NoError) {
                emit keyDeleted(key);
            } else {
                emit error(tr("Delete key failed: %1").arg(qPrintable(j->errorString())));
            }
        });

    deleteCredentialJob->start();
}

} // namespace Infrastructure::Security
