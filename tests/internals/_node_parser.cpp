/**
 * _node_parser.cpp
 *
 * Tests the internals of the node_parser module.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include "utils.h"

#include <_node_parser.h>

TEST_CASE("Parse Node Header", "[internals]")
{
  std::istringstream header("$Nodes\n1 2 7 5");  // Fake Nodes section with header
  const auto mode = cfg::parser::Mode::ASCII;

  const cfg::reader::SectionReader node_reader("Nodes", header);

  const auto node_header = cfg::parser::parse_node_header(node_reader, header, mode);

  REQUIRE(node_header.n_blocks == 1);
  REQUIRE(node_header.n_nodes == 2);
  REQUIRE(node_header.min_tag == 7);
  REQUIRE(node_header.max_tag == 5);
}

TEST_CASE("Parse Node Blocks", "[internals]")
{
  // Fake Nodes blocks
  std::istringstream node_blocks(
      "$Nodes\n0 1 0 1\n1\n0 0 1\n1 1 0 5\n9\n10\n11\n12\n13\n0 0 0.1\n0 0 0.3\n0 0 0.5\n0 0 0.7\n0 0 0.8");

  const auto mode    = cfg::parser::Mode::ASCII;
  const int n_blocks = 2;
  const int n_nodes  = 6;

  const cfg::reader::SectionReader node_reader("Nodes", node_blocks);

  const auto node_header = []() -> cfg::parser::NodeHeader
  {
    cfg::parser::NodeHeader node_header{};
    node_header.n_blocks = n_blocks;
    node_header.n_nodes  = n_nodes;
    node_header.min_tag  = 1;
    node_header.max_tag  = 13;
    return node_header;
  }();

  // Create a serial parallel configuration
  const auto parallel = []() -> cfg::utils::Parallel
  {
    cfg::utils::Parallel parallel{};
    parallel.size = 1;
    parallel.rank = 0;
    return parallel;
  }();
  const auto nodes = cfg::parser::parse_node_blocks(node_reader, node_header, node_blocks, mode, parallel);

  REQUIRE(nodes.size() == n_nodes);

  REQUIRE(nodes[0].idx == 1);
  REQUIRE(nodes[1].idx == 9);
  REQUIRE(nodes[2].idx == 10);
  REQUIRE(nodes[3].idx == 11);
  REQUIRE(nodes[4].idx == 12);
  REQUIRE(nodes[5].idx == 13);

  REQUIRE(nodes[0].x == std::array<double, 3>{0, 0, 1});
  REQUIRE(nodes[1].x == std::array<double, 3>{0, 0, 0.1});
  REQUIRE(nodes[2].x == std::array<double, 3>{0, 0, 0.3});
  REQUIRE(nodes[3].x == std::array<double, 3>{0, 0, 0.5});
  REQUIRE(nodes[4].x == std::array<double, 3>{0, 0, 0.7});
  REQUIRE(nodes[5].x == std::array<double, 3>{0, 0, 0.8});
}

TEST_CASE("Validate Nodes (continuous)", "[internals]")
{
  const size_t n_nodes = 3;
  const size_t min_tag = 27;
  const size_t max_tag = 29;

  const std::vector<cfg::parser::Node<3>> nodes = []() -> std::vector<cfg::parser::Node<3>>
  {
    std::vector<cfg::parser::Node<3>> nodes(3);
    size_t idx = min_tag;
    const std::array<double, 3> x{1, 2, 3};

    for (auto& n : nodes)
    {
      n.idx = idx;
      n.x   = x;
      idx++;
    }

    return nodes;
  }();

  // Create a serial parallel configuration
  const auto parallel = []() -> cfg::utils::Parallel
  {
    cfg::utils::Parallel parallel{};
    parallel.size = 1;
    parallel.rank = 0;
    return parallel;
  }();

  // Simple constructor to simplify testing, no need to lint
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  auto construct_hdr = [](const int n_nodes, const int min_tag, const int max_tag) -> cfg::parser::NodeHeader
  {
    cfg::parser::NodeHeader hdr{};
    hdr.n_blocks = 1;
    hdr.n_nodes  = n_nodes;
    hdr.min_tag  = min_tag;
    hdr.max_tag  = max_tag;
    return hdr;
  };
  SECTION("Valid nodes don't raise error")
  {
    const auto hdr = construct_hdr(n_nodes, min_tag, max_tag);
    REQUIRE_NOTHROW(cfg::parser::validate_nodes(nodes, hdr, parallel));
  }

  SECTION("Node indices below range raise error")
  {
    const auto hdr = construct_hdr(n_nodes, min_tag + 1, max_tag);
    REQUIRE_THROWS(cfg::parser::validate_nodes(nodes, hdr, parallel));
  }
  SECTION("Node indices above range raise error")
  {
    const auto hdr = construct_hdr(n_nodes, min_tag, max_tag - 1);
    REQUIRE_THROWS(cfg::parser::validate_nodes(nodes, hdr, parallel));
  }
}

TEST_CASE("Validate Nodes (discontinuous)", "[internals]")
{
  const size_t n_nodes = 3;
  const size_t min_tag = 27;
  const size_t max_tag = 31;

  const std::vector<cfg::parser::Node<3>> nodes = []() -> std::vector<cfg::parser::Node<3>>
  {
    std::vector<cfg::parser::Node<3>> nodes(3);
    size_t idx = min_tag;
    const std::array<double, 3> x{1, 2, 3};

    for (auto& n : nodes)
    {
      n.idx = idx;
      n.x   = x;
      idx += 2;
    }

    return nodes;
  }();

  // Create a serial parallel configuration
  const auto parallel = []() -> cfg::utils::Parallel
  {
    cfg::utils::Parallel parallel{};
    parallel.size = 1;
    parallel.rank = 0;
    return parallel;
  }();

  // Simple constructor to simplify testing, no need to lint
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  auto construct_hdr = [](const int n_nodes, const int min_tag, const int max_tag) -> cfg::parser::NodeHeader
  {
    cfg::parser::NodeHeader hdr{};
    hdr.n_blocks = 1;
    hdr.n_nodes  = n_nodes;
    hdr.min_tag  = min_tag;
    hdr.max_tag  = max_tag;
    return hdr;
  };
  SECTION("Valid nodes don't raise error")
  {
    const auto hdr = construct_hdr(n_nodes, min_tag, max_tag);
    REQUIRE_NOTHROW(cfg::parser::validate_nodes(nodes, hdr, parallel));
  }

  SECTION("Node indices below range raise error")
  {
    const auto hdr = construct_hdr(n_nodes, min_tag + 1, max_tag);
    REQUIRE_THROWS(cfg::parser::validate_nodes(nodes, hdr, parallel));
  }
  SECTION("Node indices above range raise error")
  {
    const auto hdr = construct_hdr(n_nodes, min_tag, max_tag - 1);
    REQUIRE_THROWS(cfg::parser::validate_nodes(nodes, hdr, parallel));
  }
}
