#include "View/LegacyMenus/ClientMenu.h"
std::unordered_map<std::string, std::string> ClientMenu::clientMap;


void ClientMenu::displayClientMenu() {
    int choice;
    do {
        std::cout << "\n========== Client Management Menu ==========\n";
        std::cout << "1. Add New Client\n";
        std::cout << "2. View All Clients\n";
        std::cout << "3. Edit Existing Client\n";
        std::cout << "4. Delete a Client\n";
        std::cout << "5. Back to Business Menu\n";
        std::cout << "Please enter your choice: ";
        std::cin >> choice;
        std::cout << std::endl;

        switch (choice) {
        case 1:
            if (clientDetails)clientDetails->collectClientInfo();
            break;
        case 2:
            displayClientMap();
            break;
        case 3:
            break;
        case 4:

            break;
        case 5:
            std::cout << "Returning to business menu...\n";
            break;
        default:
            std::cout << "Invalid choice. Please try again.\n";
            break;
        }
    } while (choice != 5);
}

void ClientMenu::displayClientMap() {
    clientMap = clientManager.fetchBizClients();
    int i = 1;
    for (auto& pair : clientMap) {
        std::cout << i << ": " << pair.second << " | " << pair.first << std::endl;
    }
}
