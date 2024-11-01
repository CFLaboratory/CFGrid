# CMakeLists.txt
#
# The CMake configuration for the CFGrid project.
#
# SPDX-License-Identifier: Apache-2.0

## Toolchain settings
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -Og -Wall -Wpedantic -Werror")

## General configuration
cmake_minimum_required(VERSION 3.5)

project(cfgrid
  LANGUAGES CXX
  VERSION 0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## Source
include_directories(include)

## Tests
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
    
## Documentation
add_custom_target(doc
  doxygen .doxyfile
  COMMAND make -C man/latex
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
