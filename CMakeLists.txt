#root cmake list file for the Invoice System project
cmake_minimum_required(VERSION 3.15)

# Project definition
project(InvokeInvoiceSystem2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prefix paths for vcpkg and Qt
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
find_package(fmt CONFIG REQUIRED)
set(Qt6_DIR "C:/Qt/6.9.1/msvc2022_64/lib/cmake/Qt6")

# Dependencies
find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(unofficial-libharu CONFIG REQUIRED)
find_package(Iconv REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)


qt_add_resources(INVOICE_RESOURCES "${CMAKE_SOURCE_DIR}/forms/Invoice_Resources.qrc")

# Define include paths
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SOURCE_DIR  "${CMAKE_SOURCE_DIR}/src/InvoiceSystem")

include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})

# Create Passlib static library
add_library(passlib STATIC
    ${SOURCE_DIR}/Accounts/PasswordHashing/bcrypt.cpp
    ${SOURCE_DIR}/Accounts/PasswordHashing/blowfish.cpp
)

target_include_directories(passlib PUBLIC
    ${SOURCE_DIR}/Accounts/PasswordHashing
    ${INCLUDE_DIR}
)

# Invoice System Core Sources (Non-GUI)
file(GLOB_RECURSE INVOICE_SYSTEM_SOURCES
    "${INCLUDE_DIR}/*.h"
    "${SOURCE_DIR}/*.cpp"
)

# Create InvoiceSystem library
add_library(InvoiceSystemLib STATIC ${INVOICE_SYSTEM_SOURCES})

target_include_directories(InvoiceSystemLib PUBLIC ${INCLUDE_DIR})

target_link_libraries(InvoiceSystemLib
    PUBLIC
        passlib
        Qt6::Widgets
        fmt::fmt
        Iconv::Iconv
        unofficial::libharu::hpdf
        mongo::bsoncxx_shared
        mongo::mongocxx_shared
)

 # Add subdirectory for GUI application
add_subdirectory(src/app)
