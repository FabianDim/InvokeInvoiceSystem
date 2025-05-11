cmake_minimum_required(VERSION 3.8)

project(InvokeInvoiceSystem)



# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bcrypt src/Accounts/PasswordHashing/bcrypt.cpp src/Accounts/PasswordHashing/blowfish.cpp)
target_include_directories(bcrypt PRIVATE src/Accounts/PasswordHashing/)
target_include_directories(bcrypt PUBLIC include/)

# Include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


# Add source files explicitly
add_executable(${PROJECT_NAME}
    src/InvokeInvoiceSystem.cpp
    src/Accounts/AccountManager.cpp
    src/System/InvoiceSystem.cpp
 "src/System/Menu/MainMenu.cpp" "include/MainMenu.h" "src/Accounts/User.cpp" "include/User.h")
# Precompiled header (optional)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/pch.h)