#pragma once

#include "pch.h"
#include <QWidget>

class IAccountManager;

class LandingPage : public QWidget {
	Q_OBJECT
public:
	LandingPage(IAccountManager& accountManager, QWidget* parent = nullptr);
	~LandingPage() = default;

};