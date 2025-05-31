#include "Accounts/UserBusiness/SetBusinessFromDB.h"
std::shared_ptr<BusinessRepository> SetBusiness::setUpBusiness(const std::string businessID) {
	MongoDBDataManager dbManager;
	auto result = dbManager.findOne("Business", make_document(kvp("BusinessID", businessID)));
	if (result) {
		auto view = result->view();
		auto business = std::make_shared<BusinessRepository>(BusinessRepository(businessID));

		//skip businessID and UserID array

		auto abnV = view["ABN"];
		auto phoneV = view["Phone"];
		auto nameV = view["BusinessName"];
		auto addressV = view["BusinessAddress"];
		auto acnV = view["ACN"];
		std::string abn{ abnV.get_utf8().value };
		std::string phone{ phoneV.get_utf8().value };
		std::string name{ nameV.get_utf8().value };
		std::string address{ addressV.get_utf8().value };
		std::string acn{ acnV.get_utf8().value };

		business->setAbn(abn);
		business->setPhone(phone);
		business->setBizName(name);
		business->setAcn(acn);

		return business;
	}
	std::cerr << "No business found" << std::endl;
	return nullptr;
}
