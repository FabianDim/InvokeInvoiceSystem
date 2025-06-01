# Optional: Set fallback path where you built libpostal manually
set(LIBPOSTAL_HINT_PATH "C:/msys64/home/Fabian/libpostal")

find_path(LIBPOSTAL_INCLUDE_DIR
  NAMES libpostal/libpostal.h
  PATHS
    "${LIBPOSTAL_HINT_PATH}/src"
    "${LIBPOSTAL_HINT_PATH}/include"
    "$ENV{LIBPOSTAL_DIR}/include"
    "C:/libpostal/include"
    "/usr/local/include"
)

find_library(LIBPOSTAL_LIBRARY
  NAMES postal libpostal-1
  PATHS
    "${LIBPOSTAL_HINT_PATH}/src/.libs"
    "${LIBPOSTAL_HINT_PATH}/lib"
    "$ENV{LIBPOSTAL_DIR}/lib"
    "C:/libpostal/lib"
    "/usr/local/lib"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libpostal DEFAULT_MSG LIBPOSTAL_INCLUDE_DIR LIBPOSTAL_LIBRARY)

if(LIBPOSTAL_FOUND)
  set(LIBPOSTAL_INCLUDE_DIRS ${LIBPOSTAL_INCLUDE_DIR})
  set(LIBPOSTAL_LIBRARIES ${LIBPOSTAL_LIBRARY})
endif()
