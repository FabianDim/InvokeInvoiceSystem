cmake_minimum_required(VERSION 3.10...3.24)
project(InvokeInvoiceSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1) Find external libraries
find_package(fmt      CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(unofficial-libharu CONFIG REQUIRED) 

# 2) Passlib static library
add_library(passlib STATIC
  src/Accounts/PasswordHashing/bcrypt.cpp
  src/Accounts/PasswordHashing/blowfish.cpp
)
target_include_directories(passlib PUBLIC
  src/Accounts/PasswordHashing
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

# 3) Main Application Executable (DO NOT add test files here!)
add_executable(${PROJECT_NAME}
  src/InvokeInvoiceSystem.cpp
  src/Accounts/AccountManager.cpp
  src/Accounts/User.cpp
  src/System/InvoiceSystem.cpp
  src/System/Menu/MainMenu.cpp
  src/System/Database/MongoDBHandler.cpp
  src/System/Database/MongoDBDataManager.cpp
  src/Accounts/SetUserFromDB.cpp
  src/Accounts/UserBusiness/BusinessDetails.cpp
 src/InvoiceSystem/PDF/InvoicePdfGenerator.cpp 
 src/Accounts/UserBusiness/BusinessRepository.cpp
 src/System/Menu/InvoiceMenu.cpp 
 src/Accounts/UserBusiness/SetBusinessFromDB.cpp
 )

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    passlib
    fmt::fmt
    unofficial::libharu::hpdf
    $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,
       mongo::bsoncxx_static,
       mongo::bsoncxx_shared>
    $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,
       mongo::mongocxx_static,
       mongo::mongocxx_shared>
)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/pch.h)

# 4) GoogleTest fetch & setup (after app target so build is ready)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 5) Test Executable (ONLY test files)
add_executable(InvoiceMenuTests
  tests/InvoiceMenuTests.cpp
  src/Accounts/AccountManager.cpp
  src/Accounts/User.cpp
  src/System/Database/MongoDBHandler.cpp
  src/System/Database/MongoDBDataManager.cpp
  src/Accounts/SetUserFromDB.cpp
  src/Accounts/UserBusiness/BusinessDetails.cpp
  src/InvoiceSystem/PDF/InvoicePdfGenerator.cpp
  src/Accounts/UserBusiness/BusinessRepository.cpp
  src/System/InvoiceSystem.cpp
  src/System/Menu/MainMenu.cpp
  src/InvokeInvoiceSystem.cpp
  # add other .cpps your tests use
 src/Accounts/UserBusiness/SetBusinessFromDB.cpp 
 )
target_include_directories(InvoiceMenuTests PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
)
target_link_libraries(InvoiceMenuTests
  PRIVATE
    passlib
    fmt::fmt
    GTest::gtest
    GTest::gtest_main
    unofficial::libharu::hpdf
    $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,
       mongo::bsoncxx_static,
       mongo::bsoncxx_shared>
    $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,
       mongo::mongocxx_static,
       mongo::mongocxx_shared>
)
target_precompile_headers(InvoiceMenuTests PRIVATE include/pch.h)

# 6) Enable test discovery
include(GoogleTest)
gtest_discover_tests(InvoiceMenuTests)
