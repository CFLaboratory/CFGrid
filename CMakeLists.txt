# CMakeLists.txt
#
# The CMake configuration for the CFGrid project.
#
# SPDX-License-Identifier: Apache-2.0

## General configuration
cmake_minimum_required(VERSION 3.5)

project(cfgrid
  LANGUAGES CXX
  VERSION 0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## Tests
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
    
## Documentation
