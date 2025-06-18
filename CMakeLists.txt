cmake_minimum_required (VERSION 3.24)

project ("Labels" VERSION 1.3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# Tell MSVC to build using multiple processes.
# This may speed up compilation time significantly.
# For more information check:
# https://learn.microsoft.com/en-us/cpp/build/reference/mp-build-with-multiple-processes?view=msvc-170
#
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

# Makes it easier to display some useful info
include(CMakePrintHelpers)

# Uncomment the line below, if you want to specify additional
# locations to be searched by find_package and include.
# For example, a local cmake/ direcory within the project, etc.
# list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Display some useful information
cmake_print_variables(CMAKE_MODULE_PATH)
cmake_print_variables(CMAKE_PREFIX_PATH)

################################################################################
#
# Unit testing
#

# Configure the project for testing with CTest/CDash
# Automatically adds the BUILD_TESTING option and sets it to ON
# If BUILD_TESTING is ON, automatically calls enable_testing().
# Check the following resources for more info:
#   https://cmake.org/cmake/help/latest/module/CTest.html
#   https://cmake.org/cmake/help/latest/command/enable_testing.html
#   https://cmake.org/cmake/help/latest/manual/ctest.1.html
include(CTest)


# Make Catch2 available
if(BUILD_TESTING)

  message(STATUS "Make Catch2 available...")

  if(EXISTS ${CMAKE_SOURCE_DIR}/lib/Catch2)

    # If Catch2's repo has been cloned to the /lib directory, use that    
    add_subdirectory(${CMAKE_SOURCE_DIR}/lib/Catch2)
  
  else()

    # Try to either find a local installation of Catch2,
    # or download it from its repository.
    #
    # You can find more information on how FetchContent works and
    # what is the order of locations being searched in these sources:
    #
    # Using Dependencies Guide
    #   https://cmake.org/cmake/help/latest/guide/using-dependencies/index.html#guide:Using%20Dependencies%20Guide
    # FetchContent examples:
    #   https://cmake.org/cmake/help/latest/module/FetchContent.html#fetchcontent-find-package-integration-examples
    # If necessary, set up FETCHCONTENT_TRY_FIND_PACKAGE_MODE. Check:
    #   https://cmake.org/cmake/help/latest/module/FetchContent.html#variable:FETCHCONTENT_TRY_FIND_PACKAGE_MODE
    # For Catch2's own documentation on CMake integration check:
    #   https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md
    
    include(FetchContent)

    # FIND_PACKAGE_ARGS makes it so that CMake first tries to find
    # CMake with find_package() and if it is NOT found, it will
    # be retrieved from its repository.
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0
        FIND_PACKAGE_ARGS
    )

    FetchContent_MakeAvailable(Catch2)

    # The line below was necessary when Catch2 was obtained with FetchContent,
    # as described here:
    #   https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md)
    # This does not seem to be the case anymore.
    # list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

  endif()

  # Include the Catch module, which provides catch_discover_tests
  include(Catch) 

  # Status messages
  cmake_print_variables(Catch2_DIR)
  cmake_print_variables(catch2_SOURCE_DIR)
  cmake_print_variables(Catch2_SOURCE_DIR)
  cmake_print_variables(CMAKE_MODULE_PATH)

endif()


################################################################################
#
# TCLAP
#

message(STATUS "Make TCLAP available...")

if(EXISTS ${CMAKE_SOURCE_DIR}/lib/tclap)

  # If TCLAP's repo has been cloned to the /lib directory, use that    
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/tclap)

else()

  # Try to either find a local installation of TCLAP,
  # or download it from its repository.
  
  include(FetchContent)

  # FIND_PACKAGE_ARGS makes it so that CMake first tries to find
  # TCLAP with find_package() and if it is NOT found, it will
  # be retrieved from its repository.
  FetchContent_Declare(
      tclap
      GIT_REPOSITORY https://github.com/mirror/tclap.git
      GIT_TAG        v1.2.5
      FIND_PACKAGE_ARGS
  )

  FetchContent_MakeAvailable(tclap)
  
  # TCLAP is header-only, so we need to create an interface target
  if(NOT TARGET tclap::tclap)
    add_library(tclap INTERFACE)
    add_library(tclap::tclap ALIAS tclap)
    target_include_directories(tclap INTERFACE ${tclap_SOURCE_DIR}/include)
  endif()

endif()

# Status messages
cmake_print_variables(tclap_SOURCE_DIR)
cmake_print_variables(tclap_DIR)

################################################################################
#
# Hash-lib
#

message(STATUS "Making hash-lib available...")

set(hlib_SOURCE_DIR ${CMAKE_SOURCE_DIR}/lib/hash-library)

if(NOT EXISTS ${hlib_SOURCE_DIR})
    message(STATUS "hash-library not found in ${hlib_SOURCE_DIR}. Downloading and extracting from zip.")

    set(HLIB_URL "https://github.com/stbrumme/hash-library/archive/refs/tags/hash_library_v8.zip")
    set(HLIB_ZIP_PATH "${CMAKE_BINARY_DIR}/hash-library.zip")
    set(HLIB_EXTRACT_DIR "${CMAKE_SOURCE_DIR}/lib")
    set(HLIB_EXTRACTED_NAME "hash-library-hash_library_v8")

    # Download
    file(DOWNLOAD ${HLIB_URL} ${HLIB_ZIP_PATH} SHOW_PROGRESS)

    # Extract
    file(ARCHIVE_EXTRACT INPUT ${HLIB_ZIP_PATH} DESTINATION ${HLIB_EXTRACT_DIR})

    # Rename
    file(RENAME "${HLIB_EXTRACT_DIR}/${HLIB_EXTRACTED_NAME}" "${hlib_SOURCE_DIR}")

    # Cleanup
    file(REMOVE ${HLIB_ZIP_PATH})
endif()

################################################################################
#
# Targets
#

# Add the src and lib to path
include_directories("src")
include_directories("lib/hash-library")

# Executable and library targets
add_subdirectory(src)
add_subdirectory(lib)

# Unit testing
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
