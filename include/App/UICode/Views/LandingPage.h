#pragma once

#include "pch.h"
#include <QWidget>
#include <qlayout.h>

class Invoke::Domain::Accounts::IAccountManager;
namespace App
{
namespace Views 
{
class LandingPage : public QWidget {
	Q_OBJECT
public:
	LandingPage(Invoke::Domain::Accounts::IAccountManager& accountManager, QWidget* parent = nullptr);
	~LandingPage() = default;
private:
	QWidget* createPageLayout();
	QWidget* titleWidget;
	QVBoxLayout* innerLayout;

	QPushButton* loginButton;
	QPushButton* registerButton;

};
}
}
