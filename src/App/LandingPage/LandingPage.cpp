#include <App/UICode/LandingPage/LandingPage.h>

LandingPage::LandingPage(IAccountManager& accountManager, QWidget* parent)
{
	setParent(parent);
	setWindowTitle("Landing Page");
	setFixedSize(800, 600);
}
LandingPage::~LandingPage() {
	// Cleanup if necessary
}