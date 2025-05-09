cmake_minimum_required(VERSION 3.8)

project(InvokeInvoiceSystem)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add source files explicitly
add_executable(${PROJECT_NAME}
    src/InvokeInvoiceSystem.cpp
    src/Accounts/AccountManager.cpp
    src/System/InvoiceSystem.cpp
 "src/System/Menu/MainMenu.cpp" "include/MainMenu.h")

# Precompiled header (optional)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/pch.h)
