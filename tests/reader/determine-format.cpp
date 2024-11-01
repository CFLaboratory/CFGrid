/**
 * determine-format.cpp
 *
 * Tests opening a mesh file and determininig its format.
 * For example using standard extensions the file mesh.msh could be in gmsh format or ANSYS format.
 * Furthermore, the gmsh file could be ASCII or binary.
 * Before proceeding to parsing the file, we need to know which format it is.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Read gmsh ASCII file", "[format]")
{
  REQUIRE(false);
}

TEST_CASE("Read gmsh binary file", "[format]")
{
  REQUIRE(false);
}

TEST_CASE("Reading an unknown format should fail", "[format]")
{
  REQUIRE(false);
}
