cmake_minimum_required(VERSION 3.10...3.24)

project(InvokeInvoiceSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If Qt isn't found via PATH/CMAKE_PREFIX_PATH, keep this hint:
set(Qt6_DIR "C:/Qt/6.9.1/msvc2022_64/lib/cmake/Qt6")

# Release configs you use
set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

# ---------- Dependencies ----------
find_package(fmt                CONFIG REQUIRED)
find_package(bsoncxx            CONFIG REQUIRED)
find_package(mongocxx           CONFIG REQUIRED)
find_package(unofficial-libharu CONFIG REQUIRED)
find_package(Iconv              REQUIRED)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
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
)
target_include_directories(passlib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Infrastructure/Security/PasswordHashing
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

# ---------- Auto file pickup ----------
# Define globs FIRST
file(GLOB_RECURSE PROJECT_CPP CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)
file(GLOB_RECURSE PROJECT_HDR CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/include/*.h
)
file(GLOB_RECURSE PROJECT_UI CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/forms/*.ui
)


# ---------- App executable ----------
qt_add_executable(InvokeInvoiceSystem MANUAL_FINALIZATION
    ${PROJECT_CPP}         # must include src/View/InvokeInvoiceSystem.cpp with main()
    ${PROJECT_UI}
)

# Register headers so IntelliSense and AUTOMOC can see them
target_sources(InvokeInvoiceSystem PRIVATE
    ${PROJECT_HDR}
)

# Qt resources
qt_add_resources(InvokeInvoiceSystem app_resources
  FILES forms/Invoice_Resources.qrc
)

# Public include root
target_include_directories(InvokeInvoiceSystem PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(InvokeInvoiceSystem PRIVATE
    passlib
    Qt6::Widgets
    fmt::fmt
    Iconv::Iconv
    unofficial::libharu::hpdf
    $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
    $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
)

set_target_properties(InvokeInvoiceSystem PROPERTIES
  WIN32_EXECUTABLE ON
  MACOSX_BUNDLE   ON
)

target_precompile_headers(InvokeInvoiceSystem PRIVATE include/pch.h)
qt_finalize_executable(InvokeInvoiceSystem)
