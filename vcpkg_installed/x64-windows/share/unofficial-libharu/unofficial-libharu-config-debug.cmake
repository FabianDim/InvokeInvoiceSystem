#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::libharu::hpdf" for configuration "Debug"
set_property(TARGET unofficial::libharu::hpdf APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(unofficial::libharu::hpdf PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/hpdf.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/hpdf.dll"
  )

list(APPEND _cmake_import_check_targets unofficial::libharu::hpdf )
list(APPEND _cmake_import_check_files_for_unofficial::libharu::hpdf "${_IMPORT_PREFIX}/debug/lib/hpdf.lib" "${_IMPORT_PREFIX}/debug/bin/hpdf.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
