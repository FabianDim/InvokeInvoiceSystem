#pragma once  
#include "pch.h"  

class User {  
private:  
   string email;  
   string password;
   string firstName;
   string lastName;

public:  
   User(const string& email, const string& password);  
   string getEmail() const;  
   string getFirstName() const;
   string getPassword() const;  
   void setUsername(const string& newUsername);  
   void setPassword(const string& newPassword);  
   void setFirstName(const string& firstName);
   void setLastName(const string& lastName);
};