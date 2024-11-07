# CMakeLists.txt
#
# The CMake configuration for the CFGrid reader test suite.
#
# SPDX-License-Identifier: Apache-2.0

add_executable(detect_format detect_format.cpp)
target_link_libraries(detect_format PRIVATE cfgrid)
add_dependencies(detect_format test_inputs)
target_link_libraries(detect_format PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(detect_format
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/inputs)

add_executable(parse_header parse_header.cpp)
target_link_libraries(parse_header PRIVATE cfgrid)
add_dependencies(parse_header test_inputs)
target_link_libraries(parse_header PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(parse_header
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/inputs)
