#include "AccountManager.h"
#include <iostream>

AccountManager::AccountManager() {
    // Constructor logic

	//load accounts from disk
	//loadencryption module;
}

bool AccountManager::doesAccountExist(const string& username){
	if (accounts.contains(username)) {
		cout << "Username already exists. Please try again or login." << endl;
		return true;
	}
	else {
		cout << "Username is not taken." << endl;
	}
	return false;
}

bool AccountManager::doesPasswordMatch(const string& password){
	return false;
}

bool AccountManager::validatePassword(const string& password){
   bool isDigit = false, isUpper = false, isSpecial = false;
   for (char c : password) {
       if (isdigit(c)) {
           isDigit = true;
       } else if (isupper(c)) {
           isUpper = true;
       } else if (ispunct(c)) {
           isSpecial = true;
       }
   }
   if (isDigit && isUpper && isSpecial) {
	   cout << "Password is valid." << endl;
   }
   else {
	   cout << "Password must contain at least one digit, one uppercase letter, and one special character." << endl;
   }

   return isDigit && isUpper && isSpecial;
}

void AccountManager::createAccount() {
	// Logic to create an account
	string potentialUsername{};
	cout << "Please enter your new username:	";
	cin >> potentialUsername;

	if (doesAccountExist(potentialUsername)) {
		createAccount();
	}
	else {
		accounts[potentialUsername] = ""; // Placeholder for password
		string password{};
		cout << "Please enter your password:	";
		cin >> password;
		if (validatePassword) {
			//password is valid
			//hash the password
			//store the password
			accounts[potentialUsername] = password; // Store the password
		}
		else {
			cout << "Password does not meet the requirements." << endl;
			createAccount();
		}
	}
	std::cout << "Account created." << std::endl;
}