#include "AccountManager.h"
#include <iostream>
#include <iso646.h>
#include <cctype> 

MainMenu mainMenu;
AccountManager::AccountManager() {
    // Constructor logic

	//load accounts from disk
	//loadencryption module;
}

bool AccountManager::validEmail(string& email) {
	for (char& c : email) {
		if (isspace(c)) {
			cout << "Email cannot include spaces" << endl;
			return false;
		}
	}
	
	return true;
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
	if (accounts.find(username) != accounts.end()) {
		cout << "Email already exists. Please try again or login." << endl;
		return true;
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
	string userEmail;

	// Prompt until a valid (unused) username is entered
	while (true) {
		cout << "Please enter your new email (or * to cancel): ";
		getline(std::cin >> std::ws, userEmail);

		if (userEmail == "*") {
			cout << "Account creation canceled.\n";
			return;
		}

		if (!doesAccountExist(userEmail) && validEmail(userEmail)) {
			break; // valid and not taken
		}
	}

	// Password setup loop
	string userPassword;
	string storedHash;
	while (true) {
		cout << "Please enter your password: ";
		cin >> userPassword;

		if (validatePassword(userPassword)) {
			storedHash = bcrypt::generateHash(userPassword);
			//store the password in the db along with username
			break;
		}
	}
	string firstName;
	string lastName;

	while (true) {
		cout << "Please enter your first name: ";
		getline(std::cin >> std::ws, firstName);
		cout << "Please enter your last name: ";
		getline(std::cin >> std::ws, lastName);
		if (validName(lastName) && validName(firstName)) {
			break;
		}

	}
	AccountManager accountMangager;
	MongoDBDataManager dataManager;

	// Create and store user
	auto user = make_shared<User>(User(userEmail, userPassword));



	user->setUserEmail(userEmail);
	user->setPassword(storedHash);
	user->setFirstName(firstName);
	user->setLastName(lastName);
	dataManager.buildNewUser(user);
	accounts[userEmail] = move(user);
	currentUser = accounts[userEmail];
	cout << "Account created successfully!\n";
	mainMenu.loggedInMenu(*this);
}


void AccountManager::login() {
	while (true) {
		string userEmail;
		string userPassword;
		while (true) {
			cout << "Please enter your email (or * to cancel): ";
			getline(std::cin >> std::ws, userEmail);

			if (userEmail == "*") {
				cout << "Login canceled.\n";
				return;
			}

			if (validEmail(userEmail)) {
				break;
			}
			//maybe eventually add some sort of convenience check like must include @

		}
		while (true) {
			cout << "Please enter your password (or * to cancel): ";
			getline(std::cin >> std::ws, userPassword);

			if (userPassword == "*") {
				cout << "Login canceled.\n";
				return;
			}

			break;
		}

		MongoDBDataManager dataManager;
		if (dataManager.validPassword(userPassword, userEmail)) {
			mainMenu.loggedInMenu(*this);
			break;
		}
		else {
			cout << "That username/password combo does not exist: try again.\n\n";
		}
	}
}

shared_ptr<User> AccountManager::getAccount() {
	if (!currentUser) cout << "no user" << endl;
	return currentUser ? currentUser : nullptr;
}

bool AccountManager::isLoggedIn() {
	return currentUser != nullptr;
}
