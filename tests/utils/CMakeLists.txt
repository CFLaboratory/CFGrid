# CMakeLists.txt
#
# The CMake configuration for the CFGrid utils test suite.
#
# SPDX-License-Identifier: Apache-2.0

define_test(append append.cpp)
define_test(test_stride test_stride.cpp)
define_test(partition partition.cpp)
