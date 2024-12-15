# CMakeLists.txt
#
# The CMake configuration for the CFGrid reader test suite.
#
# SPDX-License-Identifier: Apache-2.0

define_test(detect_format detect_format.cpp)
define_test(parse_header parse_header.cpp)
define_test(find_section find_section.cpp)
