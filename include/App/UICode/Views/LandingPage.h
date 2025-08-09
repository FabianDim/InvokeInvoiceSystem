#pragma once

#include "pch.h"
#include <QWidget>
#include <qlayout.h>

class IAccountManager;
namespace App
{
namespace Views 
{
class LandingPage : public QWidget {
	Q_OBJECT
public:
	LandingPage(IAccountManager& accountManager, QWidget* parent = nullptr);
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
