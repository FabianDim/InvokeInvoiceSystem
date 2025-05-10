#include "User.h"

User::User(const string& email, const string& password) {
	this->email = email;
	this->password = password;
	const string firstName;
	const string lastName;
}

string User::getEmail() const {
	return email;
}

string User::getFirstName() const {
	return firstName;
}

string User::getPassword() const {
	return string();
}

void User::setUsername(const string& newEmail) {
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
