#pragma once
#include "Infrastructure/Updates/UpdateDownload.h"
#include "Infrastructure/Updates/UpdateSecurity.h"
#include <QPointer>
#include <QTemporaryDir>
#include <QMainWindow>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <memory>
namespace Infrastructure::Updates {
class UpdateController : public QObject {
public:
    explicit UpdateController(QMainWindow* window);
    void check(bool manual);
private:
    void show_offer(const Manifest& manifest, const QByteArray& json, const QByteArray& signature);
    void download(const Manifest& manifest, const QByteArray& json, const QByteArray& signature);
    void check_startup(const Manifest& manifest);
    void activate(const Manifest& manifest);
    void fail(const QString& message, bool visible);
    QMainWindow* window_;
    UpdateDownload* download_;
    bool busy_ = false;
    bool manual_ = false;
    QPointer<QDialog> dialog_;
    QPointer<QLabel> status_;
    QPointer<QPushButton> action_;
    QPointer<QProgressBar> progress_;
    std::unique_ptr<QTemporaryDir> stage_;
};
}
