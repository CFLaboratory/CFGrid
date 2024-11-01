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

#include <reader.h>

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
  REQUIRE_THROWS_AS(cfg::reader::reader("unknown.msh"),
		    cfg::reader::unknown_format);
}

TEST_CASE("Attempting to read a non-existent file should throw a filesystem error", "[format]")
{
  REQUIRE_THROWS_AS(cfg::reader::reader("non-existent.msh"),
		    std::filesystem::filesystem_error);
}
