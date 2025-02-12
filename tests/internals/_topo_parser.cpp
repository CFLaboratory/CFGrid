/**
 * _topo_parser.cpp
 *
 * Tests the internals of the topo_parser module.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <_topo_parser.h>

#include <catch2/catch_test_macros.hpp>

#include <sstream>

#include <section_reader.h>

TEST_CASE("Parse Element Header", "[internals]")
{
  std::istringstream header("$Elements\n42 1729 63 101"); // Fake Elements section with header
  const auto mode = cfg::reader::Mode::ASCII;

  const cfg::reader::SectionReader topo_reader("Elements", header);

  const auto topo_header = cfg::parser::HeaderParser::parse(topo_reader, header, mode);

  REQUIRE(topo_header.n_blocks == 42);
  REQUIRE(topo_header.n_elements == 1729);
  REQUIRE(topo_header.min_tag == 63);
  REQUIRE(topo_header.max_tag == 101);
}
