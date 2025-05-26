#pragma once
#include <pch.h>
#include <User.h>
#include <AccountManager.h>
class BusinessRepository {
public:
	BusinessRepository(const std::string bizID);
    const std::string& getBizID() const { return bizID; }
    const std::vector<int>& getArr() const { return arr; }
    const std::string& getAbn() const { return abn; }
    const std::string& getPhone() const { return phone; }
    const std::string& getName() const { return name; }
    const std::string& getBizName() const { return bizName; }
    const std::string& getAddress() const { return address; }
    const std::string& getAcn() const { return acn; }

    // Setters
    void setBizID(const std::string& id) { bizID = id; }
    void setArr(const std::vector<int>& values) { arr = values; }
    void setAbn(const std::string& value) { abn = value; }
    void setPhone(const std::string& value) { phone = value; }
    void setName(const std::string& value) { name = value; }
    void setBizName(const std::string& value) { bizName = value; }
    void setAddress(const std::string& value) { address = value; }
    void setAcn(const std::string& value) { acn = value; }
private:
	std::string bizID;
	std::vector<int> arr;
	std::string abn;
	std::string phone;
	std::string name;
	std::string bizName;
	std::string address;
	std::string acn;

	std::shared_ptr<User> currentUser;
	std::string currentUserID = currentUser->getMongoUserID();
	MongoDBDataManager dbManager;

};