#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "RSLogKit::RSLogKit" for configuration "Release"
set_property(TARGET RSLogKit::RSLogKit APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(RSLogKit::RSLogKit PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libRSLogKit.so.1.4.1"
  IMPORTED_SONAME_RELEASE "libRSLogKit.so.1"
  )

list(APPEND _cmake_import_check_targets RSLogKit::RSLogKit )
list(APPEND _cmake_import_check_files_for_RSLogKit::RSLogKit "${_IMPORT_PREFIX}/lib/libRSLogKit.so.1.4.1" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
