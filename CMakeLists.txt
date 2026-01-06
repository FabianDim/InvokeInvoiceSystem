cmake_minimum_required(VERSION 3.10...3.24)

project(InvokeInvoiceSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Release configs you use
set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

# ---------- Dependencies ----------
find_package(fmt                CONFIG REQUIRED)
find_package(bsoncxx            CONFIG REQUIRED)
find_package(mongocxx           CONFIG REQUIRED)
find_package(unofficial-libharu CONFIG REQUIRED)
find_package(Iconv              REQUIRED)

# IMPORTANT: you link Qt6::Network and will use Qt6::Test for QTest
find_package(Qt6 REQUIRED COMPONENTS Widgets Network HttpServer Test)

find_package(Qt6Keychain CONFIG REQUIRED)

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
  include/Infrastructure/Database/Services/InvoiceServices.h
)
target_include_directories(passlib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Infrastructure/Security/PasswordHashing
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

# ---------- Auto file pickup ----------
file(GLOB_RECURSE PROJECT_CPP CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)
file(GLOB_RECURSE PROJECT_HDR CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/include/*.h
)
file(GLOB_RECURSE PROJECT_UI CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/forms/*.ui
)

# ---------- Split: App library (everything except main) ----------
# CHANGE THIS if your main() lives elsewhere:
set(APP_MAIN_CPP ${CMAKE_SOURCE_DIR}/src/View/InvokeInvoiceSystem.cpp)

# Safety: normalize paths so list(REMOVE_ITEM) works reliably
get_filename_component(APP_MAIN_CPP_ABS "${APP_MAIN_CPP}" ABSOLUTE)
set(PROJECT_CPP_ABS "")
foreach(f IN LISTS PROJECT_CPP)
  get_filename_component(fabs "${f}" ABSOLUTE)
  list(APPEND PROJECT_CPP_ABS "${fabs}")
endforeach()

# Remove main() translation unit from the library sources
list(REMOVE_ITEM PROJECT_CPP_ABS "${APP_MAIN_CPP_ABS}")

add_library(InvokeInvoiceSystem_lib STATIC
    ${PROJECT_CPP_ABS}
    ${PROJECT_HDR}
    ${PROJECT_UI}
)

target_include_directories(InvokeInvoiceSystem_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(InvokeInvoiceSystem_lib PRIVATE
    passlib
    Qt6::Widgets
    Qt6::Network
    fmt::fmt
    Iconv::Iconv
    unofficial::libharu::hpdf
    $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
    $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
    Qt6Keychain::Qt6Keychain
    Qt6::HttpServer
)

# If you use a PCH for the whole app, it can also be used for tests
target_precompile_headers(InvokeInvoiceSystem_lib PRIVATE include/pch.h)

# ---------- App executable (just main + resources, links the lib) ----------
qt_add_executable(InvokeInvoiceSystem MANUAL_FINALIZATION
    ${APP_MAIN_CPP_ABS}
    ${PROJECT_UI}
)
  target_link_libraries(InvokeInvoiceSystem PRIVATE InvokeInvoiceSystem_lib)
# Qt resources
qt_add_resources(InvokeInvoiceSystem app_resources
  FILES forms/Invoice_Resources.qrc
)


set_target_properties(InvokeInvoiceSystem PROPERTIES
  WIN32_EXECUTABLE ON
  MACOSX_BUNDLE   ON
)

qt_finalize_executable(InvokeInvoiceSystem)

# ---------- Tests (GoogleTest + QtTest/QTest) ----------
include(CTest)
enable_testing()

# You can toggle tests off with -DBUILD_TESTING=OFF
if(BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  )
  # Helps avoid MSVC CRT mismatch headaches
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_executable(InvokeInvoiceSystem_tests
    tests/PDFGenTests.cpp
  )


  target_include_directories(InvokeInvoiceSystem_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests
  )

  target_link_libraries(InvokeInvoiceSystem_tests PRIVATE
    InvokeInvoiceSystem_lib
    Qt6::Widgets
    Qt6::Test
    Qt6::Network
    unofficial::libharu::hpdf
    Iconv::Iconv
  )

  target_precompile_headers(InvokeInvoiceSystem_tests PRIVATE include/pch.h)

  add_test(NAME InvokeInvoiceSystem_tests COMMAND InvokeInvoiceSystem_tests)
endif()
