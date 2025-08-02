cmake_minimum_required(VERSION 3.24)

# Project definition
project(InvokeInvoiceSystem LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prefix paths for vcpkg and Qt
set(CMAKE_PREFIX_PATH
    "C:/Users/Fabian/dev/vcpkg/installed/x64-windows/share"
    "C:/Qt/6.9.1/msvc2022_64/lib/cmake"
    CACHE STRING "Paths to vcpkg installed/share and Qt6 install prefix"
)

# Dependencies
find_package(fmt CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(unofficial-libharu CONFIG REQUIRED)
find_package(Iconv REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
qt_standard_project_setup()

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)



# Define include paths
set(INCLUDE_DIR "  ${CMAKE_SOURCE_DIR}/src/InvoiceSystem/include")
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
        fmt::fmt
        Iconv::Iconv
        unofficial::libharu::hpdf
        mongo::bsoncxx_shared
        mongo::mongocxx_shared
)

 # Add subdirectory for GUI application
add_subdirectory(src/app)
