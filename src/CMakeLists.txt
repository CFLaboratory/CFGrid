# CMakeLists.txt
#
# The CMake configuration for the CFGrid project source.
#
# SPDX-License-Identifier: Apache-2.0

add_library(libcfg reader.cpp)
target_include_directories(libcfg PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(cfgrid main.cpp)
target_link_libraries(cfgrid libcfg)
