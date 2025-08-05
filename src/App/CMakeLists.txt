#app/cmakelist CMakeLists.txt for InvoiceSystem GUI application
set(Qt6_DIR "C:/Qt/6.9.1/msvc2022_64/lib/cmake/Qt6")
# Find Qt again explicitly (best practice for subdirs)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

set(APP_EXECUTABLE_NAME InvoiceSystemGUI)

# Define local paths
set(APP_FORMS_DIR   "${CMAKE_SOURCE_DIR}/forms")
set(APP_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(APP_SOURCE_DIR  "${CMAKE_SOURCE_DIR}/src/InvoiceSystem" "${CMAKE_SOURCE_DIR}/src/App")

include_directories(${APP_INCLUDE_DIR})

# GUI Sources
file(GLOB_RECURSE APP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/UICode/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/InvoiceSystem/"
    "${APP_FORMS_DIR}/*.ui"
    "${APP_FORMS_DIR}/*.qrc"
)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${APP_FORMS_DIR})

message(STATUS "App sources: ${APP_SOURCES}")
# Create GUI executable
if(WIN32)
    add_executable(${APP_EXECUTABLE_NAME} 
    ${APP_SOURCES}
    )
endif()
qt_standard_project_setup()
qt_finalize_executable(${APP_EXECUTABLE_NAME})

# Include paths
target_include_directories(${APP_EXECUTABLE_NAME} PRIVATE
    ${APP_FORMS_DIR}
    ${APP_INCLUDE_DIR}
    ${APP_SOURCE_DIR}
)

# Link GUI executable with Core library and Qt Widgets
target_link_libraries(${APP_EXECUTABLE_NAME} PRIVATE
    InvoiceSystemLib
    Qt6::Widgets
)

# Platform-specific settings
set_target_properties(${APP_EXECUTABLE_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

# Precompiled headers (optional but recommended) - only if pch.h actually exists
if(EXISTS "${APP_INCLUDE_DIR}/pch.h")
    target_precompile_headers(${APP_EXECUTABLE_NAME} PRIVATE ${APP_INCLUDE_DIR}/pch.h)
endif()
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)
