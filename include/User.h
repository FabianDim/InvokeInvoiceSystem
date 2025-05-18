#pragma once  
#include "pch.h"

class User {

private:
	std::string email;
	std::string password;
	std::string firstName;
	std::string lastName;
	std::string mongoUserOID;

public:
	User(const std::string& email, const std::string& password);
	std::string getEmail() const;
	std::string getFirstName() const;
	std::string getLastName() const;
	std::string getPassword() const;
	std::string getMongoUserID() const;
	void setUserEmail(const std::string& newUsername);  // to track user
	void setPassword(const std::string& newPassword);
	void setFirstName(const std::string& firstName);
	void setLastName(const std::string& lastName);
	void setMongoUserID(const std::string& mongoID); // to track user
};
