# CMakeLists.txt
#
# The CMake configuration for the CFGrid reader test suite.
#
# SPDX-License-Identifier: Apache-2.0

add_executable(determine-format determine-format.cpp)
target_link_libraries(determine-format PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(determine-format)
