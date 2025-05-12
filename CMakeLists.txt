cmake_minimum_required(VERSION 3.8)

project(InvokeInvoiceSystem LANGUAGES CXX)

# ───── C++ standard ───────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



# ───── bcrypt hashing library ─────────────────────────────────
add_library(bcrypt STATIC
    src/Accounts/PasswordHashing/bcrypt.cpp
    src/Accounts/PasswordHashing/blowfish.cpp
)
target_include_directories(bcrypt PUBLIC
    src/Accounts/PasswordHashing       # bcrypt headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ───── main executable ───────────────────────────────────────
add_executable(${PROJECT_NAME}
    src/InvokeInvoiceSystem.cpp
    src/Accounts/AccountManager.cpp
    src/Accounts/User.cpp
    src/System/InvoiceSystem.cpp
    src/System/Menu/MainMenu.cpp
)



# link bcrypt to the executable
target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt)

# ───── optional pre-compiled header ───────────────────────────
# (remove the next line if you’re not using pch.h)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/pch.h)