/**
 * detect_format.cpp
 *
 * Tests opening a mesh file and determininig its format.
 * For example using standard extensions the file mesh.msh could be in gmsh format or ANSYS format.
 * Furthermore, the gmsh file could be ASCII or binary.
 * Before proceeding to parsing the file, we need to know which format it is.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>

#include <detect_format.h>

TEST_CASE("Detect gmsh header", "[format]")
{
  const cfg::reader::GmshDetector detector;

  SECTION("Identify GMSH header string")
  {
    REQUIRE(detector.is_gmsh_file(std::string("$MeshFormat")));
    REQUIRE(detector.is_gmsh_file(std::string("$MeshFormat ")));
    REQUIRE(detector.is_gmsh_file(std::string(" $MeshFormat")));
    REQUIRE(detector.is_gmsh_file(std::string(" $MeshFormat ")));
  }

  SECTION("Reject random string")
  {
    REQUIRE_FALSE(detector.is_gmsh_file(std::string("FooBar")));
  }

  SECTION("Reject broken header")
  {
    REQUIRE_FALSE(detector.is_gmsh_file(std::string("$Mesh Format")));
    REQUIRE_FALSE(detector.is_gmsh_file(std::string("$ MeshFormat")));
    REQUIRE_FALSE(detector.is_gmsh_file(std::string("$ Mesh Format")));
  }
}

// These are closer to integration tests

TEST_CASE("Detect gmsh file", "[format]")
{
  const cfg::reader::FormatDetector detector;
  SECTION("Identify ASCII file")
  {
    REQUIRE(detector.get_format("box-txt.msh") == cfg::reader::MeshFormat::GMSH);
  }

  SECTION("Identify binary file")
  {
    REQUIRE(detector.get_format("box-bin.msh") == cfg::reader::MeshFormat::GMSH);
  }
}

TEST_CASE("Error handling in mesh format detection", "[format]")
{
  const cfg::reader::FormatDetector detector;

  SECTION("Unknown mesh format")
  {
    REQUIRE_THROWS_AS(detector.get_format("unknown.msh"), cfg::reader::unknown_format);
  }

  SECTION("Missing mesh file")
  {
    REQUIRE_THROWS_AS(detector.get_format("non-existent.msh"), std::filesystem::filesystem_error);
  }
}
