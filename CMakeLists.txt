cmake_minimum_required(VERSION 3.10...3.24)

project(InvokeInvoiceSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If Qt isn't found via PATH/CMAKE_PREFIX_PATH, keep this hint:
set(Qt6_DIR "C:/Qt/6.9.1/msvc2022_64/lib/cmake/Qt6")

# Release configs you use
set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

# ---------- Dependencies ----------
find_package(fmt                CONFIG REQUIRED)
find_package(bsoncxx            CONFIG REQUIRED)
find_package(mongocxx           CONFIG REQUIRED)
find_package(unofficial-libharu CONFIG REQUIRED)
find_package(Iconv              REQUIRED)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
qt_standard_project_setup()  # enables AUTOMOC/AUTORCC/AUTOUIC

# Let AUTOUIC find ui files under /forms
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_AUTOUIC_SEARCH_PATHS}
    ${CMAKE_SOURCE_DIR}/forms
)

# ---------- passlib static ----------
add_library(passlib STATIC
  src/Infrastructure/Security/PasswordHashing/bcrypt.cpp
  src/Infrastructure/Security/PasswordHashing/blowfish.cpp
)
target_include_directories(passlib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Infrastructure/Security/PasswordHashing
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

# ---------- App executable ----------
qt_add_executable(InvokeInvoiceSystem
  # Sources
  src/App/InvokeInvoiceSystem.cpp
  src/App/MainWindow.cpp
  src/App/UICode/Views/LandingPage.cpp
  #src/App/UICode/Widgets/AnimatedButton.cpp
  src/App/StyleSheet.cpp

  src/Application/Accounts/AccountManager.cpp
  src/Application/Accounts/SetUserFromDB.cpp
  src/Application/Business/BusinessManager.cpp
  src/Application/Business/SetBusinessFromDB.cpp
  src/Application/Business/Clients/ClientManager.cpp
  src/Application/Business/Clients/SetClientFromDB.cpp
  src/Application/Invoices/InvoiceManager.cpp
  src/Application/Invoices/SetInvoiceFromDB.cpp
  src/Application/Stock/StockManager.cpp
  src/Application/Stock/SetStockFromDB.cpp

  src/Domain/Accounts/User.cpp
  src/Domain/Business/BusinessDetails.cpp
  src/Domain/Business/Clients/ClientDetails.cpp
  src/Domain/Invoices/InvoiceDetails.cpp
  src/Domain/Stock/ClientStockItem.cpp
  src/Domain/Stock/StockDetails.cpp

  src/Infrastructure/Database/Mongo/MongoDBDataManager.cpp
  src/Infrastructure/Database/Mongo/MongoDBHandler.cpp
  src/Infrastructure/Pdf/InvoicePdfGenerator.cpp

  # legacy console menus (kept)
  src/App/LegacyMenus/BusinessMenu.cpp
  src/App/LegacyMenus/ClientMenu.cpp
  src/App/LegacyMenus/InvoiceMenu.cpp
  src/App/LegacyMenus/InvoiceSystem.cpp
  src/App/LegacyMenus/MainMenu.cpp
  src/App/LegacyMenus/StockMenu.cpp

  # UI files
  forms/UI/MainWindow.ui
)

# IMPORTANT: headers with Q_OBJECT so AUTOMOC generates moc_*.cpp
target_sources(InvokeInvoiceSystem PRIVATE
  include/App/MainWindow.h
  include/App/UICode/Views/LandingPage.h
  include/App/UICode/Widgets/AnimatedButton.h
)

# Qt resources (compile the .qrc that refers to icons etc.)
qt_add_resources(InvokeInvoiceSystem app_resources
  FILES forms/Invoice_Resources.qrc
)

# Public include root (so #include "Domain/..." works)
target_include_directories(InvokeInvoiceSystem
  PUBLIC ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(InvokeInvoiceSystem
  PRIVATE
    passlib
    Qt6::Widgets
    fmt::fmt
    Iconv::Iconv
    unofficial::libharu::hpdf
    $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,
       mongo::bsoncxx_static,
       mongo::bsoncxx_shared>
    $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,
       mongo::mongocxx_static,
       mongo::mongocxx_shared>
)

set_target_properties(InvokeInvoiceSystem PROPERTIES
  WIN32_EXECUTABLE ON
  MACOSX_BUNDLE   ON
)

# Precompiled header
target_precompile_headers(InvokeInvoiceSystem PRIVATE include/pch.h)

# Finalize Qt app (Qt6 best practice)
qt_finalize_executable(InvokeInvoiceSystem)
