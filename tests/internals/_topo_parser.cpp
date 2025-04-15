/**
 * _topo_parser.cpp
 *
 * Tests the internals of the topo_parser module.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <_topo_parser.h>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <sstream>

#include <node_parser.h>
#include <section_reader.h>

TEST_CASE("Empty NestedVector", "[internals]")
{
  const std::vector<size_t> ptr{};
  const std::vector<size_t> val{};
  const cfg::parser::NestedVector<size_t> csr{ptr, val};

  SECTION("Check row count")
  {
    auto ctr = 0;
    for (auto row : csr)
    {
      // Silence unused variable warning
      auto a = *row.begin();
      a = a + 0;

      ctr++;
    }
    REQUIRE(ctr == 0);
  }

  SECTION("Check steps")
  {
    auto ctr = 0;
    for (auto row : csr)
    {
      for (auto entry : row)
      {
	auto a = entry;
	a = a + 0;

	ctr++;
      }
    }
    REQUIRE(ctr == 0);
  }
}

TEST_CASE("NestedVector", "[internals]")
{
  const std::vector<size_t> ptr{0, 3, 7, 10};
  const std::vector<size_t> val{1, 2, 3, 4, 5, 6, 7, 8, 9};
  const cfg::parser::NestedVector<size_t> csr{ptr, val};

  SECTION("Check row count")
  {
    auto ctr = 0;
    for (auto row : csr)
    {
      // Silence unused variable warning
      auto a = *row.begin();
      a = a + 0;

      ctr++;
    }
    REQUIRE(ctr == 3);
  }
  
  SECTION("Check strides")
  {
    SECTION("Check begins")
    {
      auto ctr = 0;
      for (auto row : csr)
      {
        REQUIRE(*row.begin() == val[ptr[ctr++]]);
      }
    }

    SECTION("Check ends")
    {
      auto ctr = 0;
      for (auto row : csr)
      {
	if (ctr < 2)
	{
	  REQUIRE(*row.end() == val[ptr[++ctr]]);
	}
      }
    }
  }

  SECTION("Check steps")
  {
    auto ctr = 0;
    for (auto row : csr)
    {
      auto ctr2 = 0;
      for (const auto entry : row)
      {
        REQUIRE(entry == val[ptr[ctr] + ctr2]);
        ctr2++;
      }
      ctr++;
    }
  }
}

TEST_CASE("Parse Element Header", "[internals]")
{
  std::istringstream header("$Elements\n42 1729 63 101");  // Fake Elements section with header
  const auto mode = cfg::reader::Mode::ASCII;

  const cfg::reader::SectionReader topo_reader("Elements", header);

  const auto topo_header = cfg::parser::HeaderParser::parse(topo_reader, header, mode);

  REQUIRE(topo_header.n_blocks == 42);
  REQUIRE(topo_header.n_elements == 1729);
  REQUIRE(topo_header.min_tag == 63);
  REQUIRE(topo_header.max_tag == 101);
}

TEST_CASE("Parse Element Blocks", "[internals]")
{
  // Fake Elements blocks
  const auto mode        = cfg::reader::Mode::ASCII;
  const int n_blocks     = 4;  // points, edges, facets, cells
  const int n_elt        = 23;
  const int min_tag      = 1;
  const int max_tag      = 23;
  const auto element_hdr = []() -> cfg::parser::TopoHeader
  {
    cfg::parser::TopoHeader element_hdr{};
    element_hdr.n_blocks   = n_blocks;
    element_hdr.n_elements = n_elt;
    element_hdr.min_tag    = min_tag;
    element_hdr.max_tag    = max_tag;
    return element_hdr;
  }();

  const std::string section_hdr = "$Elements\n";

  const std::string points_hdr = "0 1 15 5\n";
  const std::string points_elt = "1 1\n2 2\n3 3\n4 4\n5 5\n";
  const std::string points_blk = points_hdr + points_elt;

  const std::string edges_hdr = "1 2 1 9\n";
  const std::string edges_elt = "6 1 2\n7 1 3\n8 1 4\n9 2 3\n10 3 4\n11 4 2\n12 2 5\n13 3 5\n14 4 5\n";
  const std::string edges_blk = edges_hdr + edges_elt;

  const std::string faces_hdr = "2 3 2 7\n";
  const std::string faces_elt = "15 1 2 3\n16 1 3 4\n17 1 4 2\n18 2 3 4\n19 2 5 3\n20 3 5 4\n21 4 5 2\n";
  const std::string faces_blk = faces_hdr + faces_elt;

  const std::string cells_hdr = "3 3 4 2\n";
  const std::string cells_elt = "22 1 2 3 4\n23 2 3 4 5\n";
  const std::string cells_blk = cells_hdr + cells_elt;

  std::istringstream element_blocks{section_hdr + points_blk + edges_blk + faces_blk + cells_blk};
  const cfg::reader::SectionReader topo_reader("Elements", element_blocks);

  SECTION("No nodes ownership")
  {
    const auto nodes = []() -> std::vector<cfg::parser::Node<3>>
    {
      std::vector<cfg::parser::Node<3>> nodes{};
      return nodes;
    }();
    std::cout << "YO" << std::endl;
    const auto topo = cfg::parser::DataParser::parse(topo_reader, element_blocks, mode, element_hdr, nodes);
    std::cout << "YOyoyo" << std::endl;

    REQUIRE(topo.nodes().size() == 0);
    REQUIRE(topo.nodes().local().size() == 0);
    REQUIRE(topo.nodes().halo().size() == 0);
  }

  SECTION("Node 0 ownership")
  {
    const auto nodes = []() -> std::vector<cfg::parser::Node<3>>
    {
      std::vector<cfg::parser::Node<3>> nodes;
      nodes.push_back({42, 0, {0.0, 0.0, 0.0}});
      return nodes;
    }();
    const auto topo = cfg::parser::DataParser::parse(topo_reader, element_blocks, mode, element_hdr, nodes);

    REQUIRE(topo.nodes().size() == 0);
    REQUIRE(topo.nodes().local().size() == 0);
    REQUIRE(topo.nodes().halo().size() == 0);
  }

  SECTION("Node 1 ownership")
  {
    const auto nodes = []() -> std::vector<cfg::parser::Node<3>>
    {
      std::vector<cfg::parser::Node<3>> nodes;
      nodes.push_back({1, 0, {0.0, 0.0, 0.0}});
      return nodes;
    }();
    const auto topo = cfg::parser::DataParser::parse(topo_reader, element_blocks, mode, element_hdr, nodes);

    REQUIRE(topo.nodes().size() == 4);
    REQUIRE(topo.nodes().local().size() == 1);
    REQUIRE(topo.nodes().halo().size() == 3);
  }
}
