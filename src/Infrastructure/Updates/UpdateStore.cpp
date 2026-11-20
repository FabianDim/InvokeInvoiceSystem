#include "Infrastructure/Updates/UpdateStore.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <stdexcept>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#endif
namespace Infrastructure::Updates {
namespace {
bool safe_directory(const QString& directory) {
    QFileInfo info(directory);
    if (!info.isDir() || info.isSymLink()) return false;
#ifdef Q_OS_WIN
    auto path = info.absoluteFilePath();
    while (!path.isEmpty()) {
        const auto attributes = GetFileAttributesW(reinterpret_cast<LPCWSTR>(path.utf16()));
        if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_REPARSE_POINT)) return false;
        QDir parent(path);
        if (!parent.cdUp() || parent.absolutePath() == path) break;
        path = parent.absolutePath();
    }
#endif
    return true;
}
bool valid_directory_name(const QString& name) {
    static const QRegularExpression pattern("^release-[A-Za-z0-9]{6}$");
    return pattern.match(name).hasMatch();
}
}
QString update_root() { return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/updates"; }
QString active_directory(const QString& root) {
    if (!safe_directory(root)) return {};
    const auto name = QString::fromUtf8(read_bounded(root + "/active", 100));
    if (!valid_directory_name(name)) return {};
    const auto directory = root + '/' + name;
    return safe_directory(directory) ? directory : QString{};
}
void set_active_directory(const QString& root, const QString& directory) {
    const QFileInfo info(directory);
    if (!safe_directory(root) || !safe_directory(directory) || !valid_directory_name(info.fileName()) ||
        QDir(info.absolutePath()) != QDir(root)) throw std::runtime_error("Invalid update staging directory.");
    QSaveFile pointer(root + "/active"); pointer.setDirectWriteFallback(false);
    const auto value = info.fileName().toUtf8();
    if (!pointer.open(QIODevice::WriteOnly) || pointer.write(value) != value.size() || !pointer.commit())
        throw std::runtime_error("Could not activate the verified update.");
}
Manifest verify_staged(const QString& directory, const QString& current, bool fresh) {
    if (!safe_directory(directory)) throw std::runtime_error("The update directory is not safe to use.");
    const auto manifest = verify_manifest(read_bounded(directory + "/manifest.json", 16384),
        read_bounded(directory + "/manifest.sig", 64), trusted_public_key(), current, QDateTime::currentDateTimeUtc(), fresh);
    if (!verify_file(directory + "/application.exe", manifest)) throw std::runtime_error("The downloaded update did not match its signed checksum.");
    return manifest;
}
bool launch_verified(const QString& directory, const Manifest& manifest, qint64 wait_for_pid) {
#ifdef Q_OS_WIN
    if (!safe_directory(directory)) return false;
    const auto path = directory + "/application.exe";
    // Deny writes and deletion between hashing and CreateProcess. The running image
    // remains protected by the Windows loader after this handle is closed.
    HANDLE file = CreateFileW(reinterpret_cast<LPCWSTR>(path.utf16()), GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    bool started = false;
    if (verify_file(path, manifest)) {
        QProcess process;
        process.setProgram(path);
        process.setWorkingDirectory(directory);
        auto env = QProcessEnvironment::systemEnvironment();
        env.remove("QT_PLUGIN_PATH"); env.remove("QT_QPA_PLATFORM_PLUGIN_PATH"); env.insert("QT_QPA_PLATFORM", "windows");
        process.setProcessEnvironment(env);
        process.setArguments({"--update-wait-pid", QString::number(wait_for_pid)});
        started = process.startDetached();
    }
    CloseHandle(file);
    return started;
#else
    Q_UNUSED(directory); Q_UNUSED(manifest); Q_UNUSED(wait_for_pid);
    return false;
#endif
}
bool launch_cached_update() {
#ifdef Q_OS_WIN
    const auto directory = active_directory(update_root());
    if (directory.isEmpty()) return false;
    try {
        const auto manifest = verify_staged(directory, QCoreApplication::applicationVersion());
        return launch_verified(directory, manifest, QCoreApplication::applicationPid());
    } catch (const std::exception&) { return false; } // Keep the original usable, even offline or after corruption.
#else
    return false;
#endif
}
bool wait_for_launcher(qint64 pid) {
#ifdef Q_OS_WIN
    if (pid <= 0 || pid > MAXDWORD || pid == GetCurrentProcessId()) return false;
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(pid));
    if (!process) return GetLastError() == ERROR_INVALID_PARAMETER;
    const auto result = WaitForSingleObject(process, 30000);
    CloseHandle(process);
    return result == WAIT_OBJECT_0;
#else
    Q_UNUSED(pid); return false;
#endif
}
}
