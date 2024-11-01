# CMakeLists.txt
#
# The CMake configuration for the CFGrid project source.
#
# SPDX-License-Identifier: Apache-2.0

add_library(cfgrid reader.cpp)
target_include_directories(cfgrid PUBLIC ${CMAKE_SOURCE_DIR}/include)
