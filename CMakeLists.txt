# CMakeLists.txt
#
# The CMake configuration for the CFGrid project.
#
# SPDX-License-Identifier: Apache-2.0

## Toolchain settings
set(CMAKE_CXX_FLAGS_INIT "-Wall -Wpedantic -Werror")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -Og")

## General configuration
cmake_minimum_required(VERSION 3.5)

project(cfgrid
  LANGUAGES CXX
  VERSION 0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## Source
add_subdirectory(src)

## Tests
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
    
## Documentation
add_custom_target(doc
  doxygen .doxyfile
  # COMMAND make -k -C man/latex
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
