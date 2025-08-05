cmake_minimum_required(VERSION 3.10...3.24)

if (WIN32)
    project(InvokeInvoiceSystem LANGUAGES CXX)
elseif(UNIX)
    project(InvokeInvoiceSystem)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Qt6_DIR "C:/Qt/6.9.1/msvc2022_64/lib/cmake/Qt6")

set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)


find_package(fmt                CONFIG REQUIRED)
find_package(bsoncxx            CONFIG REQUIRED)
find_package(mongocxx           CONFIG REQUIRED)
find_package(unofficial-libharu CONFIG REQUIRED)
find_package(Iconv              REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
qt_standard_project_setup()

#=================== INCLUSION OF Project Files ====================#
set(FORMS_DIR "${CMAKE_SOURCE_DIR}/forms")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}src/include")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories(${FORMS_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})

file(GLOB_RECURSE SOURCES
    "${FORMS_DIR}/*.ui"
    "${FORMS_DIR}/*.qrc"
    "${INCLUDE_DIR}/*.h"
    "${SOURCE_DIR}/*.cpp"
)

#=================== SETUP EXECTUABLE ====================#

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${FORMS_DIR})

# 2) Passlib static library
add_library(passlib STATIC
  src/Accounts/PasswordHashing/bcrypt.cpp
  src/Accounts/PasswordHashing/blowfish.cpp
)
target_include_directories(passlib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Accounts/PasswordHashing
  ${CMAKE_CURRENT_SOURCE_DIR}src/include
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

qt_add_resources(INVOICE_RESOURCES "${CMAKE_SOURCE_DIR}/forms/Invoice_Resources.qrc")


# Add the executable
if (WIN32) 
    qt_add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${INVOICE_RESOURCES})
elseif(UNIX)
    qt_add_executable(${PROJECT_NAME} ${SOURCES} ${INVOICE_RESOURCES})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${FORMS_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
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

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/pch.h)