# Find Qt again explicitly (best practice for subdirs)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Define local paths
set(APP_FORMS_DIR   "${CMAKE_SOURCE_DIR}/src/InvoiceSystem/forms")
set(APP_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/InvoiceSystem/include")
set(APP_SOURCE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${APP_INCLUDE_DIR})

# GUI Sources
file(GLOB_RECURSE APP_SOURCES
    "${APP_SOURCE_DIR}/*.cpp"
    "${APP_FORMS_DIR}/*.ui"
    "${APP_FORMS_DIR}/*.qrc"
    "${APP_INCLUDE_DIR}/app/*.h"  # assuming GUI-related headers
)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${APP_FORMS_DIR})
# Create GUI executable
if(WIN32)
    qt_add_executable(InvokeInvoiceSystem  MANUAL_FINALIZATION WIN32

        "./main.cpp"

        # Core Invoice System logic
        ../InvoiceSystem/InvokeInvoiceSystem.cpp
        ../InvoiceSystem/InvoiceManager.cpp

        # Accounts
        ../InvoiceSystem/Accounts/AccountManager.cpp
        ../InvoiceSystem/Accounts/User.cpp

        # Accounts > PasswordHashing
        ../InvoiceSystem/Accounts/PasswordHashing/bcrypt.cpp
        ../InvoiceSystem/Accounts/PasswordHashing/blowfish.cpp

        # Accounts > UserBusiness
        ../InvoiceSystem/Accounts/UserBusiness/BusinessManager.cpp
        ../InvoiceSystem/Accounts/UserBusiness/BusinessRepository.cpp

        # Accounts > UserBusiness > Clients
        ../InvoiceSystem/Accounts/UserBusiness/Clients/ClientManager.cpp
        ../InvoiceSystem/Accounts/UserBusiness/Clients/ClientDetails.cpp
        ../InvoiceSystem/Accounts/UserBusiness/Clients/SetClientFromDB.cpp

        # System > Database
        ../InvoiceSystem/System/Database/MongoDBDataManager.cpp

        # System > Menu
        ../InvoiceSystem/System/Menu/InvoiceMenu.cpp
        ../InvoiceSystem/System/Menu/BusinessMenu.cpp
        ../InvoiceSystem/System/Menu/ClientMenu.cpp
        ../InvoiceSystem/System/Menu/StockMenu.cpp

        # Stock
        ../InvoiceSystem/Stock/StockManager.cpp
        ../InvoiceSystem/Stock/StockDetails.cpp
        ../InvoiceSystem/Stock/ClientStockItem.cpp
        ../InvoiceSystem/Stock/SetStockFromDB.cpp

        # InvoiceData (you may need to add more here if you have .cpps under InvoiceData)
        # Example:
        # ../InvoiceSystem/InvoiceData/SomeInvoiceCpp.cpp

        # PDF (if you have .cpp files under PDF, add here)
        # Example:
        # ../InvoiceSystem/PDF/InvoicePDF.cpp

        # Add any other relevant .cpp files as needed!
    )
endif()

# Include paths
target_include_directories(${PROJECT_NAME} PRIVATE
    ${APP_FORMS_DIR}
    ${APP_INCLUDE_DIR}
    ${APP_SOURCE_DIR}
)

# Link GUI executable with Core library and Qt Widgets
target_link_libraries(${PROJECT_NAME} PRIVATE
    InvoiceSystemLib
    Qt6::Widgets
)

# Platform-specific settings
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

# Precompiled headers (optional but recommended) - only if pch.h actually exists
if(EXISTS "${APP_INCLUDE_DIR}/pch.h")
    target_precompile_headers(${PROJECT_NAME} PRIVATE ${APP_INCLUDE_DIR}/pch.h)
endif()
