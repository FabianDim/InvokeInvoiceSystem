#pragma once
#include <QWidget>

class AccountManager;

class IAccountManager : public QObject {
	Q_OBJECT

public:
	virtual ~IAccountManager() = default;
private:
	QString currentUserName() const override;

	bool isLoggedIn = false;
};