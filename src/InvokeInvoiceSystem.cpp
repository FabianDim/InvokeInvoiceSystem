// InvokeInvoiceSystem.cpp : Defines the entry point for the application.
//

#include "InvokeInvoiceSystem.h"

using namespace std;

int main()
{
	AccountManager accountManager;
	cout << "Welcome to the Invoke Invoice System(tm)." << endl;
	accountManager.createAccount();

	return 0;
}
