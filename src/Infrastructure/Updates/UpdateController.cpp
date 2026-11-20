#include "Infrastructure/Updates/UpdateController.h"
#include "Infrastructure/Updates/UpdateStore.h"
#include "View/UiStyle.h"
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QSettings>
#include <QTimer>
#include <QMessageBox>
#include <QSaveFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDir>
#include <QFile>
#include <QStorageInfo>
#include <QStatusBar>
#include <stdexcept>

namespace Infrastructure::Updates {
namespace {
void write_file(const QString& path, const QByteArray& bytes) {
    QSaveFile file(path); file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        throw std::runtime_error("Could not save update verification files.");
}
}
UpdateController::UpdateController(QMainWindow* window) : QObject(window), window_(window), download_(new UpdateDownload(this)) {
    auto* menu = window->menuBar()->addMenu("&Updates");
    auto* checkAction = menu->addAction("Check for updates...");
    connect(checkAction, &QAction::triggered, this, [this]() { check(true); });
    auto* automatic = menu->addAction("Check on startup");
    automatic->setCheckable(true); automatic->setChecked(QSettings().value("updates/checkOnStartup", true).toBool());
    connect(automatic, &QAction::toggled, this, [](bool enabled) { QSettings().setValue("updates/checkOnStartup", enabled); });
#ifdef Q_OS_WIN
    if (automatic->isChecked()) QTimer::singleShot(1500, this, [this]() { check(false); });
#else
    automatic->setEnabled(false);
#endif
}
void UpdateController::fail(const QString& message, bool visible) {
    busy_ = false;
    if (status_) {
        status_->setText(message);
        if (action_) action_->setEnabled(false);
        if (progress_) progress_->hide();
    } else if (visible) QMessageBox::warning(window_, "Update unavailable", message);
    else window_->statusBar()->showMessage("Update check unavailable. You can try again from Updates.", 10000);
}
void UpdateController::check(bool manual) {
#ifndef Q_OS_WIN
    if (manual) QMessageBox::information(window_, "Updates", "In-app updates are available for Windows x64 releases.");
    return;
#endif
    if (dialog_) { dialog_->raise(); dialog_->activateWindow(); return; }
    if (busy_) return;
    busy_ = true; manual_ = manual;
    if (manual) window_->statusBar()->showMessage("Checking GitHub for updates...", 30000);
    const QUrl url(QString("https://api.github.com/repos/%1/releases/latest").arg(repository));
    download_->start(url, 1024 * 1024, {}, [this](const QString& error, const QByteArray& data) {
        if (!error.isEmpty()) { fail(error, manual_); return; }
        QString version;
        try { version = latest_version(data); }
        catch (const std::exception& e) { fail(QString::fromUtf8(e.what()), manual_); return; }
        if (!newer_version(version, QCoreApplication::applicationVersion())) {
            busy_ = false;
            if (manual_) QMessageBox::information(window_, "Updates", "You are using the latest available version.");
            return;
        }
        if (!manual_ && QSettings().value("updates/skippedVersion").toString() == version) { busy_ = false; return; }
        download_->start(asset_url(version, "update-windows-x64.json"), 16384, {}, [this, version](const QString& error, const QByteArray& json) {
            if (!error.isEmpty()) { fail(error, manual_); return; }
            download_->start(asset_url(version, "update-windows-x64.sig"), 64, {}, [this, version, json](const QString& error, const QByteArray& signature) {
                if (!error.isEmpty()) { fail(error, manual_); return; }
                try {
                    const auto manifest = verify_manifest(json, signature, trusted_public_key(), QCoreApplication::applicationVersion());
                    if (manifest.version != version) throw std::runtime_error("The release and signed update versions do not match.");
                    busy_ = false; show_offer(manifest, json, signature);
                } catch (const std::exception& e) { fail(QString::fromUtf8(e.what()), manual_); }
            });
        });
    });
}
void UpdateController::show_offer(const Manifest& manifest, const QByteArray& json, const QByteArray& signature) {
    auto* dialog = new QDialog(window_); dialog_ = dialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose); dialog->setObjectName("app_update_dialog");
    dialog->setWindowTitle("Application update"); dialog->resize(500, 250);
    auto* layout = new QVBoxLayout(dialog); layout->setContentsMargins(24, 24, 24, 24); layout->setSpacing(14);
    layout->addWidget(App::Views::UiStyle::label("Version " + manifest.version + " is available", dialog, "title"));
    status_ = App::Views::UiStyle::label(QString("Your version: %1. Download size: %2 MB.\nThe update is signed and verified. Your current app and saved data will be kept.")
        .arg(QCoreApplication::applicationVersion()).arg(manifest.size / (1024.0 * 1024.0), 0, 'f', 1), dialog);
    status_->setTextFormat(Qt::PlainText); status_->setWordWrap(true); layout->addWidget(status_);
    progress_ = new QProgressBar(dialog); progress_->setRange(0, 100); progress_->hide(); layout->addWidget(progress_);
    auto* buttons = new QHBoxLayout;
    auto* later = new QPushButton("Later", dialog);
    auto* skip = new QPushButton("Skip this version", dialog);
    action_ = new QPushButton("Download update", dialog);
    App::Views::UiStyle::button(later); App::Views::UiStyle::button(skip); App::Views::UiStyle::button(action_, "primary");
    buttons->addWidget(later); buttons->addWidget(skip); buttons->addWidget(action_); layout->addLayout(buttons);
    connect(later, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(skip, &QPushButton::clicked, dialog, [version = manifest.version, dialog]() {
        QSettings().setValue("updates/skippedVersion", version); dialog->reject();
    });
    connect(dialog, &QDialog::finished, this, [this]() { download_->cancel(); busy_ = false; });
    connect(dialog, &QObject::destroyed, this, [this]() { stage_.reset(); });
    connect(action_, &QPushButton::clicked, this, [=, this]() { download(manifest, json, signature); });
    dialog->show();
}
void UpdateController::download(const Manifest& manifest, const QByteArray& json, const QByteArray& signature) {
    if (busy_ || !dialog_) return;
    busy_ = true; action_->setEnabled(false); progress_->show();
    try {
        const auto root = update_root();
        if (!QDir().mkpath(root)) throw std::runtime_error("Could not create the update folder.");
        if (QStorageInfo(root).bytesAvailable() < manifest.size + 50 * 1024 * 1024)
            throw std::runtime_error("Not enough disk space to safely download this update.");
        stage_ = std::make_unique<QTemporaryDir>(root + "/release-XXXXXX");
        if (!stage_->isValid()) throw std::runtime_error("Could not create a private update download folder.");
        write_file(stage_->path() + "/manifest.json", json); write_file(stage_->path() + "/manifest.sig", signature);
        status_->setText("Downloading and verifying the update. You can cancel and keep using the app.");
        download_->progress = [this](qint64 received, qint64 total) { if (progress_) progress_->setValue(int(received * 100 / total)); };
        download_->start(asset_url(manifest.version, manifest.asset), manifest.size, stage_->path() + "/application.exe",
            [this, manifest](const QString& error, const QByteArray&) {
                if (!dialog_) return;
                if (!error.isEmpty()) { fail(error, true); return; }
                try { verify_staged(stage_->path(), QCoreApplication::applicationVersion(), true); check_startup(manifest); }
                catch (const std::exception& e) { fail(QString::fromUtf8(e.what()), true); }
            });
    } catch (const std::exception& e) { fail(QString::fromUtf8(e.what()), true); }
}
void UpdateController::check_startup(const Manifest& manifest) {
    status_->setText("Verifying that the new version starts correctly...");
    auto* process = new QProcess(dialog_);
    // The self-check writes its version to stdout and never starts the UI or a cached handoff.
    process->setProgram(stage_->path() + "/application.exe");
    process->setArguments({"--update-self-check", "--no-update-handoff"});
    process->setWorkingDirectory(stage_->path());
    auto env = QProcessEnvironment::systemEnvironment(); env.remove("QT_PLUGIN_PATH"); env.remove("QT_QPA_PLATFORM_PLUGIN_PATH");
    env.insert("QT_QPA_PLATFORM", "windows"); env.insert("QT_ASSUME_STDERR_HAS_CONSOLE", "1"); process->setProcessEnvironment(env);
    auto* timeout = new QTimer(process); timeout->setSingleShot(true);
    connect(timeout, &QTimer::timeout, process, [process]() { process->kill(); });
    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) { fail("The new version could not start. Your current version is unchanged.", true); process->deleteLater(); }
    });
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this, process, manifest](int code, QProcess::ExitStatus exitStatus) {
        const auto version = QString::fromUtf8(process->readAllStandardOutput()).trimmed();
        process->deleteLater();
        if (!dialog_ || !stage_) return;
        if (code != 0 || exitStatus != QProcess::NormalExit || version != "InvokeInvoiceSystem " + manifest.version) {
            fail("The new version failed its startup check. Your current version is unchanged.", true); return;
        }
        busy_ = false; progress_->hide();
        status_->setText("Update verified and ready. Finish or save your work before restarting. Unsaved forms and offline demo records will be cleared when the app closes.");
        action_->disconnect(this); action_->setText("Restart and update"); action_->setEnabled(true);
        connect(action_, &QPushButton::clicked, this, [this, manifest]() { activate(manifest); });
    });
    timeout->start(20000); process->start();
}
void UpdateController::activate(const Manifest& manifest) {
    if (!stage_ || !dialog_) return;
    if (QMessageBox::question(dialog_, "Restart to update?", "The app will close and start the verified update. Unsaved forms and offline demo data will be lost. Saved invoices and account data are preserved.",
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Yes) return;
    const auto root = update_root();
    const auto previous = active_directory(root);
    try {
        const auto verified = verify_staged(stage_->path(), QCoreApplication::applicationVersion(), true);
        set_active_directory(root, stage_->path());
        if (!launch_verified(stage_->path(), verified, QCoreApplication::applicationPid())) {
            if (!previous.isEmpty()) set_active_directory(root, previous);
            else QFile::remove(root + "/active");
            throw std::runtime_error("Could not start the verified update. Your current app remains open.");
        }
        stage_->setAutoRemove(false); stage_.reset();
        QApplication::quit();
    } catch (const std::exception& e) { fail(QString::fromUtf8(e.what()), true); }
}
}
