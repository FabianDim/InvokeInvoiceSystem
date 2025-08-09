#include "App/UICode/Views/LandingPage.h"
#include <qlabel.h>
#include <QVBoxLayout>
#include <QPushButton>
#include "App/StyleSheet.h"
#include <QGraphicsDropShadowEffect>
App::Views::LandingPage::LandingPage(Invoke::Domain::Accounts::IAccountManager& accountManager, QWidget* parent)
{
    titleWidget = new QWidget(this);
    innerLayout = new QVBoxLayout(titleWidget);
	setParent(parent);
	setWindowTitle("Landing Page");
	createPageLayout();
	setFixedSize(800, 600);

    //wire the actions last
}
QWidget* App::Views::LandingPage::createPageLayout() {
    QLabel* titleLabel = new QLabel("Welcome to the Invoke Invoice System", titleWidget);
    titleLabel->setAlignment(Qt::AlignCenter);
	loginButton = new QPushButton("Login", titleWidget);
	registerButton = new QPushButton("Register", titleWidget);
	loginButton->setObjectName("loginButton");
	registerButton->setObjectName("registerButton");
	titleLabel->setObjectName("titleLabel");
    
	// Styles
	QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(titleWidget);
	effect->setBlurRadius(16);
	effect->setOffset(0, 6);
	effect->setColor(QColor(0, 0, 0, 90));

	loginButton->setStyleSheet(Styles::widgetStyles.at("loginButton")
		+ Styles::widgetStyles.at("button"));
	registerButton->setStyleSheet(Styles::widgetStyles.at("registerButton")
		+ Styles::widgetStyles.at("button"));
	titleLabel->setStyleSheet(Styles::widgetStyles.at("titleLabel"));


	
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);

    // Layout for the titleWidget itself
    innerLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
	innerLayout->addLayout(buttonLayout);

    return titleWidget;
}
