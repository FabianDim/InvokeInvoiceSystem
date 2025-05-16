#pragma once  
#include "pch.h"


class User {  

private:  
   string email;  
   string password;
   string firstName;
   string lastName;
   string mongoUserOID;


public:  
   User(const string& email, const string& password);  
   string getEmail() const;  
   string getFirstName() const;
   string getLastName() const;
   string getPassword() const;
   string getMongoUserID()const;
   void setUserEmail(const string& newUsername);  //to track uer
   void setPassword(const string& newPassword);  
   void setFirstName(const string& firstName);
   void setLastName(const string& lastName);
   void setMongoUserID(const string& mongoID); // to track user
};