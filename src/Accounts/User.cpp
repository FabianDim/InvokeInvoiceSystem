#include "User.h"

User::User(const string& username, const string& password) {
	this->username = username;
	this->password = password;
}

string User::getUsername() const {
	return username;
}

string User::getPassword() const {
	return string();
}

void User::setUsername(const string& newUsername) {
	this->username = newUsername;
}

void User::setPassword(const string& newPassword) {
	this->password = newPassword;
}
