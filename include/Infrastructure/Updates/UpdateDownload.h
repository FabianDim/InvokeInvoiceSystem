#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QNetworkReply>
#include <functional>
#include <memory>

namespace Infrastructure::Updates {
// One bounded HTTPS request chain. A fresh manager carries no invoice session credentials.
class UpdateDownload : public QObject {
public:
    using Completion = std::function<void(const QString&, const QByteArray&)>;
    explicit UpdateDownload(QObject* parent = nullptr, QNetworkAccessManager* transport = nullptr);
    void start(const QUrl& url, qint64 limit, const QString& destination, Completion completion);
    void cancel();
    std::function<void(qint64, qint64)> progress;
private:
    void request(const QUrl& url, int redirects);
    void finish(const QString& error);
    QNetworkAccessManager* network_;
    QPointer<QNetworkReply> reply_;
    qint64 limit_ = 0;
    qint64 received_ = 0;
    QString destination_;
    QByteArray data_;
    Completion completion_;
    class State;
    std::shared_ptr<State> state_;
};
}
