#pragma once  
#include "pch.h"


class User {  

private:  
   string email;  
   string password;
   string firstName;
   string lastName;
   string mongoOID;


public:  
   User(const string& email, const string& password);  
   string getEmail() const;  
   string getFirstName() const;
   string getLastName() const;
   string getPassword() const;
   string getMongoID()const;
   void setUserEmail(const string& newUsername);  //to track uer
   void setPassword(const string& newPassword);  
   void setFirstName(const string& firstName);
   void setLastName(const string& lastName);
   void setMongoID(const string& mongoID); // to track user
};