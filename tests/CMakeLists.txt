# CMakeLists.txt
#
# The CMake configuration for the CFGrid tests.
#
# SPDX-License-Identifier: Apache-2.0

enable_testing()

find_package(Catch2 3 REQUIRED)

include(Catch)
add_subdirectory(reader)
