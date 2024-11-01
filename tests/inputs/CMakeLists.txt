# CMakeLists.txt
#
# The CMake configuration for the CFGrid test inputs.
#
# SPDX-License-Identifier: Apache-2.0

set(CFG_TESTS_INPUTS_DIR ${CFG_TESTS_DIR}/inputs)

## GMSH
set(GMSH_TEST_INPUTS
  ${CFG_TESTS_INPUTS_DIR}/box-txt.msh
  ${CFG_TESTS_INPUTS_DIR}/box-bin.msh)

# Create dependency between the mesh files and geo files, rebuilding if necessary
add_custom_command(OUTPUT ${GMSH_TEST_INPUTS}
  COMMAND gmsh ARGS -3 box.geo -o box-txt.msh
  COMMAND gmsh ARGS -3 -bin box.geo -o box-bin.msh
  DEPENDS box.geo
  WORKING_DIRECTORY ${CFG_TESTS_INPUTS_DIR})
add_custom_target(gmsh_test_inputs DEPENDS ${GMSH_TEST_INPUTS})

## Misc
set(TEST_INPUTS
  ${CFG_TESTS_INPUTS_DIR}/unknown.msh)
add_custom_target(test_inputs DEPENDS ${TEST_INPUTS})
add_dependencies(test_inputs gmsh_test_inputs)
