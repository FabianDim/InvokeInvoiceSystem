#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QUuid>
#include <cstdio>
#include <exception>
#include <mutex>

namespace Infrastructure::Logging {
// Function-local categories are shared by all targets, including small test binaries.
inline const QLoggingCategory& app() { static QLoggingCategory c("invoice.app"); return c; }
inline const QLoggingCategory& ui() { static QLoggingCategory c("invoice.ui"); return c; }
inline const QLoggingCategory& http() { static QLoggingCategory c("invoice.http"); return c; }
inline const QLoggingCategory& auth() { static QLoggingCategory c("invoice.auth"); return c; }
inline const QLoggingCategory& database() { static QLoggingCategory c("invoice.database"); return c; }
inline const QLoggingCategory& pdf() { static QLoggingCategory c("invoice.pdf"); return c; }
inline const QLoggingCategory& offline() { static QLoggingCategory c("invoice.offline"); return c; }

inline thread_local QByteArray requestContext;
inline QByteArray newRequestId() { return QUuid::createUuid().toByteArray(QUuid::WithoutBraces); }

// Only UUIDs are accepted from callers: arbitrary headers must not enter logs.
inline QByteArray requestId(const QByteArray& candidate) {
    if (candidate.size() == 36) {
        const QUuid id(QString::fromLatin1(candidate));
        if (!id.isNull() && id.toByteArray(QUuid::WithoutBraces) == candidate.toLower())
            return candidate.toLower();
    }
    return newRequestId();
}

class RequestContext {
    QByteArray previous_;
public:
    explicit RequestContext(const QByteArray& id) : previous_(requestContext) { requestContext = id; }
    ~RequestContext() { requestContext = previous_; }
    RequestContext(const RequestContext&) = delete;
    RequestContext& operator=(const RequestContext&) = delete;
};

// 'returned' only means control returned normally, not that validation succeeded.
class Operation {
    const QLoggingCategory& category_;
    const char* name_;
    QElapsedTimer timer_;
    int exceptions_ = std::uncaught_exceptions();
public:
    Operation(const QLoggingCategory& category, const char* name) : category_(category), name_(name) {
        timer_.start();
        qCDebug(category_).noquote() << "event=operation.started operation=" << name_;
    }
    ~Operation() {
        if (std::uncaught_exceptions() > exceptions_)
            qCWarning(category_).noquote() << "event=operation.exception operation=" << name_ << "elapsed_ms=" << timer_.elapsed();
        else
            qCDebug(category_).noquote() << "event=operation.returned operation=" << name_ << "elapsed_ms=" << timer_.elapsed();
    }
    Operation(const Operation&) = delete;
    Operation& operator=(const Operation&) = delete;
};

struct Sink {
    std::mutex mutex;
    QFile file;
};
inline Sink& sink() { static Sink value; return value; }

inline void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    auto line = qFormatLogMessage(type, context, message);
    line.replace('\r', QStringLiteral("\\r"));
    line.replace('\n', QStringLiteral("\\n"));
    if (!requestContext.isEmpty())
        line += " request_id=" + QString::fromLatin1(requestContext);
    const auto bytes = (line + '\n').toUtf8();
    auto& output = sink();
    const std::lock_guard lock(output.mutex);
    std::fwrite(bytes.constData(), 1, static_cast<size_t>(bytes.size()), stderr);
    std::fflush(stderr);
    if (output.file.isOpen()) {
        if (output.file.size() + bytes.size() > 5 * 1024 * 1024) {
            const auto path = output.file.fileName();
            output.file.close();
            // Keep one previous file; if rotation fails, continue on stderr.
            const auto backup = path + ".1";
            const bool removed = !QFile::exists(backup) || QFile::remove(backup);
            if (removed && QFile::rename(path, backup))
                output.file.open(QIODevice::WriteOnly | QIODevice::Append);
            if (!output.file.isOpen())
                std::fputs("Invoice log rotation failed; continuing on stderr.\n", stderr);
        }
        if (output.file.isOpen() &&
            (output.file.write(bytes) != bytes.size() || !output.file.flush())) {
            output.file.close();
            std::fputs("Invoice log file write failed; continuing on stderr.\n", stderr);
        }
    }
}

// Call once, after Q(Core)Application and its application/organization names.
inline void initialize(bool desktop) {
    qSetMessagePattern("%{time yyyy-MM-ddTHH:mm:ss.zzz} %{type} [%{category}] pid=%{pid} %{message}");
    QString path = qEnvironmentVariable("INVOICE_LOG_FILE");
    if (path.isEmpty() && desktop)
        path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs/application.log";
    auto& output = sink();
    if (!path.isEmpty()) {
        QDir().mkpath(QFileInfo(path).absolutePath());
        output.file.setFileName(path);
        output.file.open(QIODevice::WriteOnly | QIODevice::Append);
    }
    qInstallMessageHandler(messageHandler);
    if (!path.isEmpty() && !output.file.isOpen())
        qCWarning(app) << "event=logging.file_unavailable continuing_on_stderr=true";
    qCInfo(app) << "event=logging.ready file_enabled=" << output.file.isOpen();
}
} // namespace Infrastructure::Logging
