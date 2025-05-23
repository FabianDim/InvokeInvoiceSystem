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
# 2) Your own bcrypt library (renamed to avoid clash with Windows' bcrypt.lib)
add_library(passlib STATIC
  src/Accounts/PasswordHashing/bcrypt.cpp
  src/Accounts/PasswordHashing/blowfish.cpp
)
target_include_directories(passlib PUBLIC
  src/Accounts/PasswordHashing
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

# 3) Your executable and MongoDB support
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
 tests/InvoiceMenuTests.cpp)

# 4) Link in fmt, passlib and the MongoDB C++ driver
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    passlib
    fmt::fmt
    unofficial::libharu::hpdf
    # pick the static bsoncxx if available else fall back to shared
    $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,
       mongo::bsoncxx_static,
       mongo::bsoncxx_shared>

    # pick the static mongocxx if available else fall back to shared
    $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,
       mongo::mongocxx_static,
       mongo::mongocxx_shared>
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 5) Optional precompiled header
target_precompile_headers(${PROJECT_NAME} PRIVATE include/pch.h)
