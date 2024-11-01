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

TEST_CASE("Parse GMSH header", "[parser]")
{
  const cfg::reader::gmsh_header_parser parser("4.1");

  SECTION("Parse valid binary header")
  {
    const cfg::reader::gmsh_header header = parser.parse_header(std::string("4.1 1 8"));
    REQUIRE(header.version == "4.1");
    REQUIRE(header.binary == true);
    REQUIRE(header.dsize == 8);
  }
  SECTION("Parse valid ASCII header")
  {
    const cfg::reader::gmsh_header header = parser.parse_header(std::string("4.1 0 8"));
    REQUIRE(header.version == "4.1");
    REQUIRE(header.binary == false);
    REQUIRE(header.dsize == 8);
  }

  SECTION("Parse invalid binary header")
  {
    REQUIRE_THROWS(cfg::reader::gmsh_header{parser.parse_header(std::string("3.1 1 8"))});
  }
}

TEST_CASE("Read gmsh file", "[format]")
{
  cfg::reader::reader reader;

  SECTION("Identify ASCII file")
  {
    REQUIRE_NOTHROW(reader = cfg::reader::reader("box-txt.msh"));
    REQUIRE(reader.format() == cfg::reader::MeshFormat::GMSH_ASCII);
  }

  SECTION("Identify binary file")
  {
    REQUIRE_NOTHROW(reader = cfg::reader::reader("box-bin.msh"));
    REQUIRE(reader.format() == cfg::reader::MeshFormat::GMSH_BIN);
  }
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
