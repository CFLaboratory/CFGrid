/**
 * parse_header.cpp
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>

#include <gmsh.h>

TEST_CASE("Parse GMSH header", "[parser]")
{
  const cfg::mesh::GmshHeaderParser parser("4.1");

  SECTION("Parse valid binary header")
  {
    const cfg::mesh::GmshHeader header = parser.parse_header(std::string("4.1 1 8"));
    REQUIRE(header.version == "4.1");
    REQUIRE(header.binary == true);
    REQUIRE(header.dsize == 8);
  }
  SECTION("Parse valid ASCII header")
  {
    const cfg::mesh::GmshHeader header = parser.parse_header(std::string("4.1 0 8"));
    REQUIRE(header.version == "4.1");
    REQUIRE(header.binary == false);
    REQUIRE(header.dsize == 8);
  }

  SECTION("Parse invalid binary header")
  {
    REQUIRE_THROWS(cfg::mesh::GmshHeader{parser.parse_header(std::string("3.1 1 8"))});
  }
}
