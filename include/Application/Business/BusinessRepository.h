#pragma once
#include "pch.h"
#include "Domain/Accounts/User.h"
#include "Application/Accounts/AccountManager.h"

class BusinessRepository {

  public:
    explicit BusinessRepository() : current_user_(AccountManager::currentUser) {
        if (current_user_) {
            current_user_id_ = current_user_->getMongoUserID();
        } else {
            current_user_id_.clear();
        }
    }
    const std::string& getBizID() const {
        return biz_id;
    }
    const std::unordered_set<std::string>& getClients() const {
        return clients;
    }
    const std::vector<std::string>& getStock() const {
        return stock;
    }
    const std::string& getAbn() const {
        return abn;
    }
    const std::string& getPhone() const {
        return phone;
    }
    const std::string& getName() const {
        return name;
    }
    const std::string& getBizName() const {
        return biz_name;
    }
    const std::string& getAddress() const {
        return address;
    }
    const std::string& getAcn() const {
        return acn;
    }
    const std::string& get_website_url() const {
        return website_url_;
    }
    const std::string& get_biz_logo_url() const {
        return biz_logo_;
    }

    // Setters
    void setBizID(const std::string& id) {
        biz_id = id;
    }
    void setClients(const std::unordered_set<std::string>& values) {
        clients = values;
    } // maybe set size one day
    void setStock(const std::vector<std::string>& values) {
        stock = values;
    }
    void setAbn(const std::string& value) {
        abn = value;
    }
    void setPhone(const std::string& value) {
        phone = value;
    }
    void setName(const std::string& value) {
        name = value;
    }
    void setBizName(const std::string& value) {
        biz_name = value;
    }
    void setAddress(const std::string& value) {
        address = value;
    }
    void setAcn(const std::string& value) {
        acn = value;
    }
    void set_website_url(const std::string& value) {
        website_url_ = value;
    }
    void set_biz_logo_url(const std::string& value) {
        biz_logo_ = value;
    }

  private:
    std::string biz_id;
    std::vector<std::string> stock;
    std::unordered_set<std::string> clients;
    std::string abn;
    std::string phone;
    std::string name;
    std::string biz_name;
    std::string address;
    std::string acn;
    std::string website_url_;
    std::string biz_logo_;

    std::shared_ptr<User> current_user_;
    std::string current_user_id_;
};