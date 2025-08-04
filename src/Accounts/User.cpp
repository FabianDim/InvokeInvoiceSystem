#include "InvoiceSystem/Accounts/User.h"

User::User(const std::string& email, const std::string& password) {
	this->email = email;
	this->password = password;
}

std::string User::getEmail() const {
	return email;
}

std::string User::getPassword() const {
	return password;
}

std::string User::getMongoUserID() const {
	return mongoUserID;
}

void User::setUserEmail(const std::string& newEmail) {
	this->email = newEmail;
}

void User::setPassword(const std::string& newPassword) {
	this->password = newPassword;
}

void User::setFirstName(const std::string& firstName) {
	this->firstName = firstName;
}

void User::setLastName(const std::string& lastName) {
	this->lastName = lastName;
}

void User::setMongoUserID(const std::string& mongoID) {
	this->mongoUserID = mongoID;
}

std::string User::getFirstName() const {
	return firstName;
}

std::string User::getLastName() const {
	return lastName;
}
