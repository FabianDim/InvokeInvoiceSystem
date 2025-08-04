set(FORMS_DIR "${CMAKE_SOURCE_DIR}/forms")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

file(GLOB_RECURSE SOURCES
    "${FORMS_DIR}/*.ui"
    "${FORMS_DIR}/*.qrc"
    "${INCLUDE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${FORMS_DIR})

if (WIN32)
    qt_add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
elseif(UNIX)
    qt_add_executable(${PROJECT_NAME} ${SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${FORMS_DIR}
    ${INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

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

target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/pch.h)

