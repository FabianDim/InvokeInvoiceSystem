#include "View/StyleSheet.h"

namespace Styles {
    const std::unordered_map<std::string, QString> widgetStyles = {
        {"button", QStringLiteral("")},

        {"login_button_",
         QStringLiteral("QPushButton#login_button_ {"
                        "  font-size: 18px;"
                        "  padding: 10px;"
                        "  background-color: transparent;"
                        "  color: white;"
                        "  border-radius: 10px;"
                        "  border: 2px solid white;"
                        "}"
                        "QPushButton#login_button_:hover {"
                        "  background: white;"
                        "  color: #222;"
                        "  border: 2px solid #222;"
                        "}"
                        "QPushButton#login_button_:pressed {"
                        "  background: grey;"
                        "  color: white;"
                        "}")},

        {"register_button_",
         QStringLiteral("QPushButton#register_button_ {"
                        "  font-size: 18px;"
                        "  padding: 10px;"
                        "  background-color: #2196F3;"
                        "  color: white;"
                        "  border-radius: 10px;"
                        "}"
                        "QPushButton#register_button_:hover {"
                        "  filter: none;" /* harmless placeholder; QSS has no CSS filters */
                        "}"
                        "QPushButton#register_button_:pressed {"
                        "  background: white;"
                        "  color: #222;"
                        "  border: 2px solid #222;"
                        "}")},

        {"titleLabel",
         QStringLiteral("QLabel#titleLabel {"
                        "  font-size: 32px;"
                        "  font-weight: bold;"
                        "  color: white;"
                        "}")}};
}
