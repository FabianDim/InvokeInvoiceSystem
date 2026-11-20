#include "Infrastructure/Updates/UpdateDownload.h"
#include "Infrastructure/Updates/UpdateSecurity.h"
#include "Infrastructure/Updates/UpdateStore.h"
#include <QtTest>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <functional>
#include <stdexcept>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

using namespace Infrastructure::Updates;

namespace {
constexpr auto good_sha = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";

QString iso(const QDateTime& value) {
    return value.toUTC().toString(Qt::ISODate);
}

class TestKey {
public:
    TestKey() = default;
    TestKey(const TestKey&) = delete;
    TestKey& operator=(const TestKey&) = delete;
    ~TestKey() { EVP_PKEY_free(key_); }

    bool generate() {
        EVP_PKEY_CTX* context = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr);
        if (!context) return false;
        const bool generated = EVP_PKEY_keygen_init(context) == 1 && EVP_PKEY_keygen(context, &key_) == 1;
        EVP_PKEY_CTX_free(context);
        if (!generated) return false;
        BIO* buffer = BIO_new(BIO_s_mem());
        if (!buffer) return false;
        const bool written = PEM_write_bio_PUBKEY(buffer, key_) == 1;
        BUF_MEM* memory = nullptr;
        BIO_get_mem_ptr(buffer, &memory);
        if (written && memory) public_pem = QByteArray(memory->data, int(memory->length));
        BIO_free(buffer);
        return written && !public_pem.isEmpty();
    }

    QByteArray sign(const QByteArray& data) const {
        if (!key_) return {};
        EVP_MD_CTX* context = EVP_MD_CTX_new();
        if (!context) return {};
        QByteArray signature;
        size_t length = 0;
        const auto* bytes = reinterpret_cast<const unsigned char*>(data.constData());
        if (EVP_DigestSignInit(context, nullptr, nullptr, nullptr, key_) == 1 &&
            EVP_DigestSign(context, nullptr, &length, bytes, size_t(data.size())) == 1) {
            signature.resize(int(length));
            if (EVP_DigestSign(context, reinterpret_cast<unsigned char*>(signature.data()), &length, bytes,
                    size_t(data.size())) == 1) signature.resize(int(length));
            else signature.clear();
        }
        EVP_MD_CTX_free(context);
        return signature;
    }

    QByteArray public_pem;

private:
    EVP_PKEY* key_ = nullptr;
};

struct TestManifest {
    int schema = 1;
    QString repository_name = QString::fromLatin1(repository);
    QString platform = "windows-x64";
    QString version = "0.1.1";
    QString asset;
    qint64 size = 1234;
    QString sha = QString::fromLatin1(good_sha);
    QString published;
    QString expires;
};

TestManifest valid_manifest(const QDateTime& now) {
    TestManifest manifest;
    manifest.asset = QString("InvokeInvoiceSystem-%1-windows-x64.exe").arg(manifest.version);
    manifest.published = iso(now.addSecs(-60));
    manifest.expires = iso(now.addDays(180));
    return manifest;
}

QByteArray to_json(const TestManifest& manifest) {
    QJsonObject object;
    object.insert("schema", manifest.schema);
    object.insert("repository", manifest.repository_name);
    object.insert("platform", manifest.platform);
    object.insert("version", manifest.version);
    object.insert("asset", manifest.asset);
    object.insert("size", manifest.size);
    object.insert("sha256", manifest.sha);
    object.insert("published_utc", manifest.published);
    object.insert("expires_utc", manifest.expires);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool rejected(const QByteArray& json, const QByteArray& signature, const QByteArray& key, const QString& current,
    const QDateTime& now = QDateTime::currentDateTimeUtc(), bool require_fresh = true) {
    try {
        verify_manifest(json, signature, key, current, now, require_fresh);
    } catch (const std::exception&) {
        return true;
    }
    return false;
}

bool rejected(const TestManifest& manifest, const TestKey& key, const QString& current, const QDateTime& now,
    bool require_fresh = true) {
    const auto json = to_json(manifest);
    return rejected(json, key.sign(json), key.public_pem, current, now, require_fresh);
}

bool write_file(const QString& path, const QByteArray& contents) {
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(contents) == contents.size();
}
}

class UpdateTests : public QObject {
    Q_OBJECT
  private slots:
    void versions_must_be_strict_and_strictly_newer_data() {
        QTest::addColumn<QString>("version");
        QTest::addColumn<bool>("valid");
        QTest::newRow("release") << "0.1.0" << true;
        QTest::newRow("large-parts") << "12.340.5678" << true;
        QTest::newRow("empty") << "" << false;
        QTest::newRow("two-parts") << "0.1" << false;
        QTest::newRow("four-parts") << "0.1.0.0" << false;
        QTest::newRow("leading-zero") << "0.01.0" << false;
        QTest::newRow("prefix") << "v0.1.0" << false;
        QTest::newRow("suffix") << "0.1.0-beta" << false;
        QTest::newRow("spaces") << " 0.1.0" << false;
        QTest::newRow("overflow") << "0.1.1234567890" << false;
    }

    void versions_must_be_strict_and_strictly_newer() {
        QFETCH(QString, version);
        QFETCH(bool, valid);
        QCOMPARE(Infrastructure::Updates::valid_version(version), valid);
    }

    void newer_version_requires_a_strict_increase() {
        QVERIFY(newer_version("0.1.2", "0.1.1"));
        QVERIFY(newer_version("0.2.0", "0.1.99"));
        QVERIFY(newer_version("1.0.0", "0.99.99"));
        QVERIFY(!newer_version("0.1.1", "0.1.1"));
        QVERIFY(!newer_version("0.1.0", "0.1.1"));
        QVERIFY(!newer_version("0.1.2", "not-a-version"));
        QVERIFY(!newer_version("v0.1.2", "0.1.1"));
    }

    void release_metadata_must_be_a_stable_versioned_release() {
        auto release = [](const QString& tag, bool draft, bool prerelease) {
            QJsonObject object;
            object.insert("tag_name", tag);
            object.insert("draft", draft);
            object.insert("prerelease", prerelease);
            return QJsonDocument(object).toJson(QJsonDocument::Compact);
        };
        QCOMPARE(latest_version(release("v0.1.2", false, false)), QString("0.1.2"));
        const auto reject = [](const QByteArray& json) {
            try { latest_version(json); } catch (const std::exception&) { return true; }
            return false;
        };
        QVERIFY(reject(release("v0.1.2", true, false)));
        QVERIFY(reject(release("v0.1.2", false, true)));
        QVERIFY(reject(release("0.1.2", false, false)));
        QVERIFY(reject(release("v0.1.2-rc1", false, false)));
        QVERIFY(reject("[]"));
        QVERIFY(reject("{"));
        QVERIFY(reject(QByteArray(1024 * 1024 + 1, 'a')));
    }

    void update_urls_are_restricted_to_the_repository_and_https_data() {
        QTest::addColumn<QString>("url");
        QTest::addColumn<bool>("allowed");
        const auto prefix = QString("/%1/releases/download/").arg(repository);
        QTest::newRow("release-api")
            << "https://api.github.com/repos/FabianDim/InvokeInvoiceSystem/releases/latest" << true;
        QTest::newRow("manifest") << "https://github.com" + prefix + "v0.1.2/update-windows-x64.json" << true;
        QTest::newRow("signature") << "https://github.com" + prefix + "v0.1.2/update-windows-x64.sig" << true;
        QTest::newRow("executable")
            << "https://github.com" + prefix + "v0.1.2/InvokeInvoiceSystem-0.1.2-windows-x64.exe" << true;
        QTest::newRow("cdn")
            << "https://release-assets.githubusercontent.com/github-production-release-asset/1/file?sig=x" << true;
        QTest::newRow("http")
            << "http://github.com" + prefix + "v0.1.2/update-windows-x64.json" << false;
        QTest::newRow("other-host") << "https://evil.example.com/update-windows-x64.json" << false;
        QTest::newRow("other-repository")
            << "https://github.com/FabianDim/OtherProject/releases/download/v0.1.2/update-windows-x64.json" << false;
        QTest::newRow("unversioned-tag") << "https://github.com" + prefix + "latest/update-windows-x64.json" << false;
        QTest::newRow("unsupported-asset") << "https://github.com" + prefix + "v0.1.2/update-linux-x64.json" << false;
        QTest::newRow("mismatched-executable")
            << "https://github.com" + prefix + "v0.1.2/InvokeInvoiceSystem-0.1.3-windows-x64.exe" << false;
        QTest::newRow("nested-path") << "https://github.com" + prefix + "v0.1.2/extra/update-windows-x64.json" << false;
        QTest::newRow("query") << "https://github.com" + prefix + "v0.1.2/update-windows-x64.json?x=1" << false;
        QTest::newRow("fragment") << "https://github.com" + prefix + "v0.1.2/update-windows-x64.json#x" << false;
        QTest::newRow("credentials") << "https://user:pass@github.com" + prefix + "v0.1.2/update-windows-x64.json"
            << false;
        QTest::newRow("api-query") << "https://api.github.com/repos/FabianDim/InvokeInvoiceSystem/releases/latest?x=1"
            << false;
        QTest::newRow("empty") << "" << false;
        QTest::newRow("file") << "file:///C:/Windows/System32/calc.exe" << false;
    }

    void update_urls_are_restricted_to_the_repository_and_https() {
        QFETCH(QString, url);
        QFETCH(bool, allowed);
        QCOMPARE(Infrastructure::Updates::allowed_url(QUrl(url)), allowed);
    }

    void asset_urls_are_built_only_inside_the_release_path() {
        const auto url = asset_url("0.1.2", "update-windows-x64.json");
        QCOMPARE(url.toString(),
            QString("https://github.com/FabianDim/InvokeInvoiceSystem/releases/download/v0.1.2/update-windows-x64.json"));
        QVERIFY(allowed_url(url));
    }

    void signed_manifest_is_accepted_and_reports_its_fields() {
        TestKey key;
        QVERIFY(key.generate());
        const auto now = QDateTime::currentDateTimeUtc();
        const auto json = to_json(valid_manifest(now));
        const auto signature = key.sign(json);
        QCOMPARE(signature.size(), 64);
        const auto manifest = verify_manifest(json, signature, key.public_pem, "0.1.0", now);
        QCOMPARE(manifest.version, QString("0.1.1"));
        QCOMPARE(manifest.asset, QString("InvokeInvoiceSystem-0.1.1-windows-x64.exe"));
        QCOMPARE(manifest.size, qint64(1234));
        QCOMPARE(manifest.sha256, QByteArray(good_sha));
        // Five-minute clock skew and a 180-day validity window are both acceptable.
        QVERIFY(!rejected(json, signature, key.public_pem, "0.1.0", now.addSecs(-300)));
    }

    void manifest_must_be_signed_by_the_trusted_key() {
        TestKey trusted;
        TestKey attacker;
        QVERIFY(trusted.generate());
        QVERIFY(attacker.generate());
        const auto now = QDateTime::currentDateTimeUtc();
        const auto json = to_json(valid_manifest(now));
        QVERIFY(rejected(json, attacker.sign(json), trusted.public_pem, "0.1.0", now));
        QVERIFY(rejected(json, trusted.sign(json), attacker.public_pem, "0.1.0", now));
        QVERIFY(rejected(json, trusted.sign(json), "", "0.1.0", now));
        auto signature = trusted.sign(json);
        signature[0] = char(signature[0] ^ 0x01);
        QVERIFY(rejected(json, signature, trusted.public_pem, "0.1.0", now));
        QVERIFY(rejected(json, signature.left(63), trusted.public_pem, "0.1.0", now));
        QVERIFY(rejected(json, signature + 'x', trusted.public_pem, "0.1.0", now));
        QVERIFY(rejected(QByteArray(), QByteArray(), trusted.public_pem, "0.1.0", now));
    }

    void tampered_signed_fields_are_rejected() {
        TestKey key;
        QVERIFY(key.generate());
        const auto now = QDateTime::currentDateTimeUtc();
        const auto json = to_json(valid_manifest(now));
        const auto signature = key.sign(json);
        auto tampered = json;
        tampered.replace("\"0.1.1\"", "\"0.1.2\"");
        QVERIFY(rejected(tampered, signature, key.public_pem, "0.1.0", now));
        tampered = json;
        tampered.replace(good_sha, QByteArray(64, 'b'));
        QVERIFY(rejected(tampered, signature, key.public_pem, "0.1.0", now));
        tampered = json;
        tampered.replace("\"size\":1234", "\"size\":1");
        QVERIFY(rejected(tampered, signature, key.public_pem, "0.1.0", now));
    }

    void signed_manifest_must_match_this_app_repository_and_platform() {
        TestKey key;
        QVERIFY(key.generate());
        const auto now = QDateTime::currentDateTimeUtc();
        auto manifest = valid_manifest(now);
        manifest.repository_name = "SomeoneElse/InvokeInvoiceSystem";
        QVERIFY(rejected(manifest, key, "0.1.0", now));
        manifest = valid_manifest(now);
        manifest.platform = "linux-x64";
        QVERIFY(rejected(manifest, key, "0.1.0", now));
        manifest = valid_manifest(now);
        manifest.schema = 2;
        QVERIFY(rejected(manifest, key, "0.1.0", now));
        manifest = valid_manifest(now);
        manifest.asset = "InvokeInvoiceSystem-0.1.2-windows-x64.exe";
        QVERIFY(rejected(manifest, key, "0.1.0", now));
        manifest = valid_manifest(now);
        manifest.version = "0.1";
        QVERIFY(rejected(manifest, key, "0.1.0", now));
        manifest = valid_manifest(now);
        manifest.sha = QString::fromLatin1(good_sha).toUpper();
        QVERIFY(rejected(manifest, key, "0.1.0", now));
        manifest = valid_manifest(now);
        manifest.size = 0;
        QVERIFY(rejected(manifest, key, "0.1.0", now));
        manifest = valid_manifest(now);
        manifest.size = maximum_download + 1;
        QVERIFY(rejected(manifest, key, "0.1.0", now));
    }

    void signed_manifest_must_be_newer_than_the_running_version() {
        TestKey key;
        QVERIFY(key.generate());
        const auto now = QDateTime::currentDateTimeUtc();
        QVERIFY(rejected(valid_manifest(now), key, "0.1.1", now));
        QVERIFY(rejected(valid_manifest(now), key, "0.2.0", now));
        QVERIFY(rejected(valid_manifest(now), key, "", now));
    }

    void signed_manifest_validity_window_is_enforced() {
        TestKey key;
        QVERIFY(key.generate());
        const auto now = QDateTime::currentDateTimeUtc();
        auto manifest = valid_manifest(now);
        manifest.published = iso(now.addDays(2));
        QVERIFY(rejected(manifest, key, "0.1.0", now));
        manifest = valid_manifest(now);
        manifest.published = iso(now.addDays(-200));
        manifest.expires = iso(now.addDays(10));
        QVERIFY(rejected(manifest, key, "0.1.0", now));
        manifest = valid_manifest(now);
        manifest.published = iso(now.addDays(-10));
        manifest.expires = iso(now.addSecs(-1));
        QVERIFY(rejected(manifest, key, "0.1.0", now));
        // Offline recovery may reuse an already verified, expired manifest.
        const auto json = to_json(manifest);
        QVERIFY(!rejected(json, key.sign(json), key.public_pem, "0.1.0", now, false));
        manifest = valid_manifest(now);
        manifest.published = "not-a-date";
        QVERIFY(rejected(manifest, key, "0.1.0", now));
        manifest = valid_manifest(now);
        manifest.expires = iso(now.addDays(182));
        QVERIFY(rejected(manifest, key, "0.1.0", now));
        // A 180-day window plus the signer's timestamp rounding is the allowance.
        manifest = valid_manifest(now);
        manifest.expires = iso(now.addDays(181));
        QVERIFY(!rejected(manifest, key, "0.1.0", now));
        manifest = valid_manifest(now);
        manifest.published = iso(now.addDays(1));
        manifest.expires = iso(now.addDays(-1));
        QVERIFY(rejected(manifest, key, "0.1.0", now));
    }

    void oversized_signed_manifest_is_rejected() {
        TestKey key;
        QVERIFY(key.generate());
        const auto now = QDateTime::currentDateTimeUtc();
        auto json = to_json(valid_manifest(now));
        json += QByteArray(16384, ' ');
        QVERIFY(rejected(json, key.sign(json), key.public_pem, "0.1.0", now));
    }

    void downloaded_file_must_match_the_signed_size_and_checksum() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto path = directory.filePath("application.exe");
        const QByteArray contents("verified update payload");
        QVERIFY(write_file(path, contents));
        Manifest manifest;
        manifest.size = contents.size();
        manifest.sha256 = QCryptographicHash::hash(contents, QCryptographicHash::Sha256).toHex();
        QVERIFY(verify_file(path, manifest));
        manifest.size -= 1;
        QVERIFY(!verify_file(path, manifest));
        manifest.size = contents.size();
        manifest.sha256[0] = manifest.sha256[0] == 'a' ? 'b' : 'a';
        QVERIFY(!verify_file(path, manifest));
        QVERIFY(!verify_file(directory.filePath("missing.exe"), manifest));
        QVERIFY(!verify_file(directory.path(), manifest));
    }

    void bounded_reads_reject_oversized_and_missing_files() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto path = directory.filePath("manifest.json");
        const QByteArray contents(100, 'x');
        QVERIFY(write_file(path, contents));
        QCOMPARE(read_bounded(path, 100), contents);
        QVERIFY(read_bounded(path, 99).isEmpty());
        QVERIFY(read_bounded(directory.filePath("missing"), 100).isEmpty());
    }

    void download_refuses_untrusted_urls_and_unsafe_sizes_before_any_request() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto missing_destination = directory.filePath("missing/application.exe");
        const auto run = [&](const QUrl& url, qint64 limit, const QString& destination) {
            UpdateDownload download;
            bool completed = false;
            QString error;
            download.start(url, limit, destination, [&](const QString& message, const QByteArray&) {
                completed = true;
                error = message;
            });
            return std::make_pair(completed, error);
        };
        const auto executable = asset_url("0.1.2", "InvokeInvoiceSystem-0.1.2-windows-x64.exe");
        QVERIFY(run(executable, 0, missing_destination).first);
        QVERIFY(!run(executable, 0, missing_destination).second.isEmpty());
        QVERIFY(run(executable, maximum_download + 1, missing_destination).first);
        QVERIFY(run(QUrl("http://github.com/FabianDim/InvokeInvoiceSystem/releases/download/v0.1.2/"
                          "InvokeInvoiceSystem-0.1.2-windows-x64.exe"),
                    1024, missing_destination)
                        .first);
        QVERIFY(run(QUrl("https://evil.example.com/application.exe"), 1024, missing_destination).first);
        // An accepted URL still fails closed when its destination cannot be created.
        const auto result = run(executable, 1024, missing_destination);
        QVERIFY(result.first);
        QVERIFY(!result.second.isEmpty());
    }

    void cancelling_an_idle_download_is_harmless() {
        UpdateDownload download;
        download.cancel();
        bool completed = false;
        download.start(QUrl("https://evil.example.com/application.exe"), 1024, {},
            [&](const QString&, const QByteArray&) { completed = true; });
        QVERIFY(completed);
        download.cancel();
    }

    void staged_updates_must_carry_a_signed_manifest_and_matching_executable() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QVERIFY_EXCEPTION_THROWN(verify_staged(directory.path(), "0.1.0"), std::runtime_error);
        QVERIFY(write_file(directory.filePath("manifest.json"), "{}"));
        QVERIFY(write_file(directory.filePath("manifest.sig"), QByteArray(64, 'x')));
        QVERIFY(write_file(directory.filePath("application.exe"), "payload"));
        QVERIFY_EXCEPTION_THROWN(verify_staged(directory.path(), "0.1.0"), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(verify_staged(directory.filePath("missing"), "0.1.0"), std::runtime_error);
    }

    void active_pointer_accepts_only_direct_release_directories() {
        QTemporaryDir root;
        QTemporaryDir elsewhere;
        QVERIFY(root.isValid() && elsewhere.isValid());
        const auto release = root.filePath("release-abc123");
        QVERIFY(QDir().mkpath(release));
        QVERIFY(active_directory(root.path()).isEmpty());
        set_active_directory(root.path(), release);
        QCOMPARE(QDir(active_directory(root.path())), QDir(release));

        for (const auto& name : {QString("release-abc12"), QString("release-abc1234"), QString("release-abc12_"),
                 QString("release-abc123/.."), QString("other-abc123"), QString("release-ABC123.exe")}) {
            const auto directory = root.filePath(name);
            QVERIFY(QDir().mkpath(directory));
            QVERIFY_EXCEPTION_THROWN(set_active_directory(root.path(), directory), std::runtime_error);
        }

        const auto foreign = elsewhere.filePath("release-abc123");
        QVERIFY(QDir().mkpath(foreign));
        QVERIFY_EXCEPTION_THROWN(set_active_directory(root.path(), foreign), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(set_active_directory(elsewhere.filePath("missing"), release), std::runtime_error);
    }

    void corrupt_active_pointer_is_ignored() {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(write_file(root.filePath("active"), "release-../../Windows"));
        QVERIFY(active_directory(root.path()).isEmpty());
        QVERIFY(write_file(root.filePath("active"), "release-abc123"));
        QVERIFY(active_directory(root.path()).isEmpty());
        QVERIFY(QDir().mkpath(root.filePath("release-abc123")));
        QVERIFY(!active_directory(root.path()).isEmpty());
    }

    void trusted_public_key_is_an_ed25519_pem() {
        const auto key = trusted_public_key();
        QVERIFY(key.startsWith("-----BEGIN PUBLIC KEY-----"));
        QVERIFY(key.contains("-----END PUBLIC KEY-----"));
        BIO* buffer = BIO_new_mem_buf(key.constData(), key.size());
        QVERIFY(buffer);
        EVP_PKEY* parsed = PEM_read_bio_PUBKEY(buffer, nullptr, nullptr, nullptr);
        BIO_free(buffer);
        QVERIFY(parsed);
        QCOMPARE(EVP_PKEY_id(parsed), EVP_PKEY_ED25519);
        EVP_PKEY_free(parsed);
    }
};

QTEST_GUILESS_MAIN(UpdateTests)
#include "UpdateTests.moc"
