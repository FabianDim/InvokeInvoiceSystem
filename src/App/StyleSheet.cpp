#include "App/StyleSheet.h"

namespace Styles {
	const std::unordered_map<std::string, QString> widgetStyles = {
        {"button", QStringLiteral(
		)},
        {"loginButton", QStringLiteral(
            "QPushButton#loginButton {"
                "font-size: 18px;"
                "padding: 10px;"
                "background-color: transparent;"
                "color: white;"
                "border-radius: 10px;"
                "border-style: solid;"
                "border-color: white;"
                "border-width: 2px;"
            "}"
        )},
        {"registerButton", QStringLiteral(
            "QPushButton#registerButton {"
                "font-size: 18px;"
                "padding: 10px;"
                "background-color: #2196F3;"
                "color: white;"
                "border-radius: 10px;"
                "border-style: solid;"
                "border-color: #2196F3;"
                "border-width: 2px;"
            "}"
        )},
        {"titleLabel", QStringLiteral(
            "QLabel#titleLabel {"
                "font-size: 32px;"
                "font-weight: bold;"
                "color: white;"
                "padding: 20px;"
                "background-color: #6F8FAF;"
                "border-radius: 20px;"
            "}"
        )}
	};
}