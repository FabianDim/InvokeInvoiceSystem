#pragma once
#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QUrl>

namespace Infrastructure::Updates {
inline constexpr auto repository = "FabianDim/InvokeInvoiceSystem";
inline constexpr qint64 maximum_download = 256 * 1024 * 1024;
struct Manifest {
    QString version;
    QString asset;
    QByteArray sha256;
    qint64 size = 0;
};
bool valid_version(const QString& version);
bool newer_version(const QString& version, const QString& current);
QString latest_version(const QByteArray& json);
bool allowed_url(const QUrl& url);
QUrl asset_url(const QString& version, const QString& asset);
QByteArray trusted_public_key();
Manifest verify_manifest(const QByteArray& json, const QByteArray& signature, const QByteArray& public_key,
    const QString& current, const QDateTime& now = QDateTime::currentDateTimeUtc(), bool require_fresh = true);
QByteArray read_bounded(const QString& path, qint64 maximum);
bool verify_file(const QString& path, const Manifest& manifest);
}
