add_library(passlib STATIC
    src/bcrypt.cpp
    src/blowfish.cpp
)

target_include_directories(passlib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
