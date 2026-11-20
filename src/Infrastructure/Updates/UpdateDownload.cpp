#include "Infrastructure/Updates/UpdateDownload.h"
#include "Infrastructure/Updates/UpdateSecurity.h"
#include <QNetworkRequest>
#include <QSaveFile>
#include <QTimer>
#include <QFile>
#include <memory>

namespace Infrastructure::Updates {
class UpdateDownload::State {
public:
    std::unique_ptr<QSaveFile> file;
    QPointer<QTimer> deadline;
};
UpdateDownload::UpdateDownload(QObject* parent, QNetworkAccessManager* transport) : QObject(parent),
    network_(transport ? transport : new QNetworkAccessManager(this)) {}
void UpdateDownload::start(const QUrl& url, qint64 limit, const QString& destination, Completion completion) {
    cancel();
    limit_ = limit; received_ = 0; data_.clear(); destination_ = destination; completion_ = std::move(completion);
    state_ = std::make_shared<State>();
    if (limit <= 0 || limit > maximum_download || !allowed_url(url)) { finish("Update download URL or size is not permitted."); return; }
    if (!destination.isEmpty()) {
        state_->file = std::make_unique<QSaveFile>(destination);
        state_->file->setDirectWriteFallback(false);
        if (!state_->file->open(QIODevice::WriteOnly)) { finish("Could not create the update download file."); return; }
    }
    auto* timer = new QTimer(this); timer->setSingleShot(true); state_->deadline = timer;
    connect(timer, &QTimer::timeout, this, [this]() { finish("Update download timed out. Please try again later."); });
    timer->start(destination.isEmpty() ? 30000 : 300000);
    request(url, 0);
}
void UpdateDownload::request(const QUrl& url, int redirects) {
    if (!allowed_url(url) || redirects > 4) { finish("The update redirected to an untrusted location."); return; }
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("User-Agent", "InvokeInvoiceSystem-Updater");
    networkRequest.setRawHeader("Accept", url.host() == "api.github.com" ? "application/vnd.github+json" : "application/octet-stream");
    networkRequest.setRawHeader("Accept-Encoding", "identity");
    networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    networkRequest.setAttribute(QNetworkRequest::CookieLoadControlAttribute, QNetworkRequest::Manual);
    networkRequest.setAttribute(QNetworkRequest::CookieSaveControlAttribute, QNetworkRequest::Manual);
    networkRequest.setTransferTimeout(15000);
    auto* reply = network_->get(networkRequest); reply_ = reply; reply->setReadBufferSize(65536);
    connect(reply, &QIODevice::readyRead, this, [this, reply]() {
        if (reply_ != reply) return;
        const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status != 200) { reply->readAll(); return; }
        if (reply->header(QNetworkRequest::ContentLengthHeader).toLongLong() > limit_) {
            finish("The update exceeds its permitted download size."); return;
        }
        const auto chunk = reply->readAll(); received_ += chunk.size();
        if (received_ > limit_) { finish("The update exceeds its permitted download size."); return; }
        if (state_->file) {
            if (state_->file->write(chunk) != chunk.size()) { finish("Could not write the update download. Check available disk space."); return; }
        } else data_ += chunk;
        if (progress) progress(received_, limit_);
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, url, redirects]() {
        if (reply_ != reply) return;
        const auto redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status >= 300 && status <= 399 && !redirect.isEmpty()) {
            reply_ = nullptr; reply->deleteLater(); request(url.resolved(redirect), redirects + 1); return;
        }
        if (reply->error() != QNetworkReply::NoError || status != 200) {
            finish(status == 404 ? "No signed update is available for this release yet." : "Could not securely download the update. Please try again later."); return;
        }
        if (state_->file && !state_->file->commit()) { finish("Could not save the update download."); return; }
        finish({});
    });
}
void UpdateDownload::finish(const QString& error) {
    if (!completion_) return;
    if (reply_) {
        auto* reply = reply_.data(); reply_ = nullptr;
        reply->disconnect(this); reply->abort(); reply->deleteLater();
    }
    if (state_ && state_->deadline) { state_->deadline->stop(); state_->deadline->deleteLater(); }
    if (!error.isEmpty() && state_ && state_->file) state_->file->cancelWriting();
    auto callback = std::move(completion_); completion_ = {};
    const auto data = data_;
    state_.reset();
    callback(error, data);
}
void UpdateDownload::cancel() { finish("Update cancelled."); }
}
