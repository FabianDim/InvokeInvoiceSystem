#include "User.h"

User::User(const string& email, const string& password) {
	this->email = email;
	this->password = password;
}

string User::getEmail() const {
	return email;
}

string User::getPassword() const {
	return password;
}

void User::setUserEmail(const string& newEmail) {
	this->email = newEmail;
}

void User::setPassword(const string& newPassword) {
	this->password = newPassword;
}

void User::setFirstName(const string& firstName) {
	this->firstName = firstName;
}

void User::setLastName(const string& lastName) {
	this->lastName = lastName;
}
string User::getFirstName() const {
	return firstName;
}

string User::getLastName() const {
	return lastName;
}
