cmake_minimum_required(VERSION 3.10...3.24)

if (WIN32)
    project(InvokeInvoiceSystem LANGUAGES CXX)
elseif(UNIX)
    project(InvokeInvoiceSystem)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH
    "C:/Users/Fabian/dev/vcpkg/installed/x64-windows/share"
    "C:/Qt/6.9.1/msvc2022_64/lib/cmake"
    CACHE STRING "Paths to vcpkg installed/share and Qt6 install prefix"
)

set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

find_package(fmt                CONFIG REQUIRED)
find_package(bsoncxx            CONFIG REQUIRED)
find_package(mongocxx           CONFIG REQUIRED)
find_package(unofficial-libharu CONFIG REQUIRED)
find_package(Iconv              REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
qt_standard_project_setup()

add_subdirectory(libs/passlib)
add_subdirectory(src)

