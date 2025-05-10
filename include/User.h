#pragma once  
#include "pch.h"  

class User {  
private:  
   string username;  
   string password;  

public:  
   User(const string& username, const string& password);  
   string getUsername() const;  
   string getPassword() const;  
   void setUsername(const string& newUsername);  
   void setPassword(const string& newPassword);  
};