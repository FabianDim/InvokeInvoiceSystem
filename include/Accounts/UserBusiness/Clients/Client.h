#pragma once
#include <string>

class Client {
private:
    std::string clientID;
    std::string name;
    std::string phoneNumber;
    std::string email;
    std::string address;

public:
    // --- Getters ---
    std::string getClientID() const { return clientID; }
    std::string getName() const { return name; }
    std::string getPhoneNumber() const { return phoneNumber; }
    std::string getEmail() const { return email; }
    std::string getAddress() const { return address; }

    // --- Setters ---
    void setClientID(const std::string& id) { clientID = id; }
    void setName(const std::string& newName) { name = newName; }
    void setPhoneNumber(const std::string& number) { phoneNumber = number; }
    void setEmail(const std::string& newEmail) { email = newEmail; }
    void setAddress(const std::string& newAddress) { address = newAddress; }
};
