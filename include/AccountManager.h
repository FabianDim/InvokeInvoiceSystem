#pragma once

//using namespace std;
class AccountManager {
public:
    AccountManager();
	bool doesAccountExist(const string& username); // we eventually want to hash the password.
    bool doesPasswordMatch(const string& password);
	bool validatePassword(const string& password);
    void createAccount();//create the account and store it.
    void login();//login
    string getAccount(string username);

private:
	unordered_map<string, string> accounts;
};
