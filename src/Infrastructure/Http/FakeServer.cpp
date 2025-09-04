#include "Infrastructure/Http/FakeServer.h"
#include <QtHttpServer>

void Server::start_server() {
    QHttpServer httpServer;

    const auto port = httpServer.listen(QHostAddress::Any);

    if (!port)
        return 0;

    qDebug() << QCoreApplication::translate("QHttpServerExample",
                                            "Running on http://127.0.0.1:%1/ (Press CTRL+C to quit)")
                    .arg(port);
}
