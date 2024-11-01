# CMakeLists.txt
#
# The CMake configuration for the CFGrid tests.
#
# SPDX-License-Identifier: Apache-2.0

set(CFG_TESTS_DIR ${CMAKE_SOURCE_DIR}/tests)

enable_testing()
add_subdirectory(inputs)

find_package(Catch2 3 REQUIRED)

include(Catch)
add_subdirectory(reader)
