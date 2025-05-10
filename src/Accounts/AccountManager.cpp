#include "AccountManager.h"
#include <iostream>
MainMenu mainMenu;
AccountManager::AccountManager() {
    // Constructor logic

	//load accounts from disk
	//loadencryption module;
}

bool AccountManager::validName(string& name) {
	for (char& c : name) {
		if (isdigit(c) or ispunct(c)) {
			cout << "Names cannot include numbers or punctuation" << endl;
			return false;
		}
	}
	
	return true;
}

bool AccountManager::doesAccountExist(const string& username){
	if (accounts.contains(username)) {
		cout << "Username already exists. Please try again or login." << endl;
		return true;
	}
	else {
		cout << "Username is valid." << endl;
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
	string username;

	// Prompt until a valid (unused) username is entered
	while (true) {
		cout << "Please enter your new email (or * to cancel): ";
		cin >> username;

		if (username == "*") {
			cout << "Account creation canceled.\n";
			return;
		}

		if (!doesAccountExist(username)) {
			break; // valid and not taken
		}
	}

	// Password setup loop
	string password;
	while (true) {
		cout << "Please enter your password: ";
		cin >> password;

		if (validatePassword(password)) {
			break;
		}
	}

	string firstName;
	string lastName;

	while (true) {
		cout << "Please enter your first name: ";
		cin >> firstName;
		cout << "Please enter your last name: ";
		cin >> lastName;
		if (validName(lastName) && validName(firstName)) {
			break;
		}

	}

	// Create and store user
	User* user = new User(username, password);
	accounts[username] = password;
	user->setPassword(password);
	user->setFirstName(firstName);
	user->setFirstName(lastName);
	cout << "Account created successfully!\n";
	mainMenu.displayMenu(user);
}


void AccountManager::login() {
	string username;
	string password;
	bool isReal = false;
	while (true) {
		cout << "Please enter your username (or * to cancel): ";
		cin >> username;
		if (username == "*") {
			cout << "Account creation canceled.\n";
			return;
		}
		break;
	}
	if (!accounts.contains(username)) {
		isReal = false;
	}
	else {
		while (true) {
			cout << "Please enter your password (or * to cancel): ";
			cin >> password;
			auto& user = accounts[username];

			if (user == password) {
				currentUser = new User(username, password);
				return;
			}
			else {
				cout << "That username and password combo does not exist retry.(or * to cancel)" << endl;
				login();
			}
		}
		//welcome back first name.
	}
	cout << "Please enter your password: ";
	cin >> password;
	cout << "That username and password combo does not exist. Retry or type * into the usern name or password field to exit" << endl;
}

User* AccountManager::getAccount() {
	return currentUser ? currentUser : nullptr;
}

bool AccountManager::isLoggedIn() {
	return currentUser != nullptr;
}
