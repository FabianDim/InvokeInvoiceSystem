cmake_minimum_required(VERSION 3.8)

project(InvokeInvoiceSystem)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Collect source files
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Use precompiled headers (optional)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/pch.h)