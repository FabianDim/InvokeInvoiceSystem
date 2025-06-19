#pragma once
#include <string>
#include <vector>

class Client {
private:
    std::string clientID;
    std::string name;
    std::string phoneNumber;
    std::string email;
    std::string address;
    std::vector<std::string> clientStockIDs;

public:
    // --- Getters ---
    std::string getClientID() const { return clientID; }
    std::string getName() const { return name; }
    std::string getPhoneNumber() const { return phoneNumber; }
    std::string getEmail() const { return email; }
    std::string getAddress() const { return address; }
    const std::vector<std::string>& getClientStockIDs() const { return clientStockIDs; }

    // --- Setters ---
    void setClientID(const std::string& id) { clientID = id; }
    void setName(const std::string& newName) { name = newName; }
    void setPhoneNumber(const std::string& number) { phoneNumber = number; }
    void setEmail(const std::string& newEmail) { email = newEmail; }
    void setAddress(const std::string& newAddress) { address = newAddress; }
    void setClientStockIDs(const std::vector<std::string>& stockIDs) { clientStockIDs = stockIDs; }
};
