#include "Infrastructure/Updates/UpdateSecurity.h"
#include "UpdateTrust.h"
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QVersionNumber>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <memory>
#include <stdexcept>

namespace Infrastructure::Updates {
namespace {
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
}
bool valid_version(const QString& version) {
    static const QRegularExpression pattern("^(0|[1-9][0-9]{0,8})\\.(0|[1-9][0-9]{0,8})\\.(0|[1-9][0-9]{0,8})$");
    return pattern.match(version).hasMatch();
}
bool newer_version(const QString& version, const QString& current) {
    return valid_version(version) && valid_version(current) && QVersionNumber::fromString(version) > QVersionNumber::fromString(current);
}
QString latest_version(const QByteArray& json) {
    require(json.size() <= 1024 * 1024, "Release metadata is too large.");
    const auto doc = QJsonDocument::fromJson(json);
    require(doc.isObject(), "GitHub returned invalid release metadata.");
    const auto release = doc.object();
    require(release.value("draft").isBool() && !release.value("draft").toBool() &&
        release.value("prerelease").isBool() && !release.value("prerelease").toBool(), "This release is not a stable public release.");
    const auto tag = release.value("tag_name").toString();
    require(tag.startsWith('v') && valid_version(tag.mid(1)), "Unsupported release version.");
    return tag.mid(1);
}
bool allowed_url(const QUrl& url) {
    if (!url.isValid() || url.scheme() != "https" || !url.userInfo().isEmpty() || url.hasFragment() ||
        (url.port(-1) != -1 && url.port() != 443)) return false;
    if (url.host() == "api.github.com") return !url.hasQuery() && url.path() == QString("/repos/%1/releases/latest").arg(repository);
    if (url.host() == "release-assets.githubusercontent.com") return url.path().startsWith("/github-production-release-asset/");
    if (url.host() != "github.com" || url.hasQuery()) return false;
    const auto prefix = QString("/%1/releases/download/").arg(repository);
    if (!url.path().startsWith(prefix)) return false;
    const auto parts = url.path().mid(prefix.size()).split('/');
    if (parts.size() != 2 || !parts[0].startsWith('v') || !valid_version(parts[0].mid(1))) return false;
    return parts[1] == "update-windows-x64.json" || parts[1] == "update-windows-x64.sig" ||
        parts[1] == QString("InvokeInvoiceSystem-%1-windows-x64.exe").arg(parts[0].mid(1));
}
QUrl asset_url(const QString& version, const QString& asset) {
    return QUrl(QString("https://github.com/%1/releases/download/v%2/%3").arg(repository, version, asset));
}
QByteArray trusted_public_key() { return QByteArray(INVOKE_UPDATE_PUBLIC_KEY); }
Manifest verify_manifest(const QByteArray& json, const QByteArray& signature, const QByteArray& public_key,
    const QString& current, const QDateTime& now, bool require_fresh) {
    require(!json.isEmpty() && json.size() <= 16384 && signature.size() == 64, "Invalid signed update metadata.");
    std::unique_ptr<BIO, decltype(&BIO_free)> bio(BIO_new_mem_buf(public_key.constData(), public_key.size()), BIO_free);
    require(bool(bio), "Could not initialize update verification.");
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> key(PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr), EVP_PKEY_free);
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    require(key && context && EVP_PKEY_id(key.get()) == EVP_PKEY_ED25519 &&
        EVP_DigestVerifyInit(context.get(), nullptr, nullptr, nullptr, key.get()) == 1 &&
        EVP_DigestVerify(context.get(), reinterpret_cast<const unsigned char*>(signature.constData()), signature.size(),
            reinterpret_cast<const unsigned char*>(json.constData()), json.size()) == 1,
        "The update signature could not be verified. No update was installed.");
    const auto document = QJsonDocument::fromJson(json);
    require(document.isObject(), "Invalid update manifest.");
    const auto obj = document.object();
    require(obj.value("schema").toInt() == 1 && obj.value("repository").toString() == repository &&
        obj.value("platform").toString() == "windows-x64", "This update is for a different application or platform.");
    Manifest result;
    result.version = obj.value("version").toString();
    require(newer_version(result.version, current), "The update must be newer than the running version.");
    result.asset = obj.value("asset").toString();
    require(result.asset == QString("InvokeInvoiceSystem-%1-windows-x64.exe").arg(result.version), "Unexpected update filename.");
    const auto digest = obj.value("sha256").toString();
    require(QRegularExpression("^[0-9a-f]{64}$").match(digest).hasMatch(), "Invalid update checksum.");
    result.sha256 = digest.toLatin1();
    const auto size = obj.value("size");
    result.size = size.toInteger(-1);
    require(size.isDouble() && size.toDouble() == result.size && result.size > 0 && result.size <= maximum_download, "Invalid update size.");
    const auto published = QDateTime::fromString(obj.value("published_utc").toString(), Qt::ISODate);
    const auto expires = QDateTime::fromString(obj.value("expires_utc").toString(), Qt::ISODate);
    require(published.isValid() && expires.isValid() && published < expires && published.daysTo(expires) <= 181,
        "Invalid update validity period.");
    if (require_fresh) require(published <= now.addSecs(86400) && expires > now, "The signed update metadata has expired or the computer clock is incorrect.");
    return result;
}
QByteArray read_bounded(const QString& path, qint64 maximum) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.size() > maximum) return {};
    return file.read(maximum + 1);
}
bool verify_file(const QString& path, const Manifest& manifest) {
    const QFileInfo info(path);
    if (!info.isFile() || info.isSymLink() || info.size() != manifest.size) return false;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QCryptographicHash hash(QCryptographicHash::Sha256);
    return hash.addData(&file) && hash.result().toHex() == manifest.sha256;
}
}
