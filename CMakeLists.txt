cmake_minimum_required(VERSION 3.8)

project(InvokeInvoiceSystem LANGUAGES CXX)

# ───── C++ standard ───────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "C:/mongo-cxx-driver")
find_package(bsoncxx REQUIRED)



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

# ───── MongoDB example target ────────────────────────────────
set(MONGO_EXAMPLES_DIR
    "C:/Users/Fabian Di Mento/mongo-cxx-driver-r4.1.0/examples")

add_executable(view_and_value
    "${MONGO_EXAMPLES_DIR}/bsoncxx/view_and_value.cpp")

target_include_directories(view_and_value PRIVATE
    "C:/mongo-cxx-driver/include")          # or the path you installed to

target_link_libraries(view_and_value PRIVATE
    mongo::bsoncxx_shared)                 

# link bcrypt to the executable
target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt)

# ───── optional pre-compiled header ───────────────────────────
# (remove the next line if you’re not using pch.h)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/pch.h)
