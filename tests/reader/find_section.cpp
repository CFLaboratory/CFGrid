/**
 * Tests locating a section of a mesh file: nodes, elements, etc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <sstream>

#include <section_reader.h>

TEST_CASE("Locate section", "[section]")
{
  // A section of a GMSH file.
  // Note this contains three sections: MeshFormat, Entities and Nodes; the contents of each section
  // may not be complete/valid, however the MeshFormat and Entities sections are both closed by the
  // `$EndXXX` sygil, however Nodes is not.
  std::stringstream ss(
    "$MeshFormat\n"
    "4.1 0 8\n"
    "$EndMeshFormat\n"
    "$Entities\n"
    "8 12 6 1\n"
    "1 0 0 1 0\n"
    "2 0 0 0 0\n"
    "5 -1e-07 -9.999999994736442e-08 0.9999999000000001 1e-07 1.0000001 1.0000001 0 2 1 -3\n"
    "6 0.9999999000000001 -9.999999994736442e-08 0.9999999000000001 1.0000001 1.0000001 1.0000001 0 2 5 -7\n"
    "7 0.9999999000000001 0.9999999000000001 -9.999999994736442e-08 1.0000001 1.0000001 1.0000001 0 2 8 -7\n"
    "8 0.9999999000000001 -9.999999994736442e-08 -1e-07 1.0000001 1.0000001 1e-07 0 2 6 -8\n"
    "9 -9.999999994736442e-08 -9.999999994736442e-08 -9.999999994736442e-08 1.0000001 1.0000001 1.0000001 0 6 1 2 3 4 5 6 \n"
    "$EndEntities\n"
    "$Nodes\n"
    "27 363 1 363\n"
    "0 1 0 1\n"
    "1\n"
    "0 0 1\n"
    "0 2 0 1\n"
    );

  const cfg::reader::SectionReader format_reader("MeshFormat", ss);
  const cfg::reader::SectionReader entities_reader("Entities", ss);
  const cfg::reader::SectionReader nodes_reader("Nodes", ss);

  SECTION("Find section start")
  {
    std::string line;

    format_reader.seekg(ss, 0);
    format_reader(ss) >> line;
    REQUIRE(line == "$MeshFormat");

    entities_reader.seekg(ss, 0);
    entities_reader(ss) >> line;
    REQUIRE(line == "$Entities");

    nodes_reader.seekg(ss, 0);
    nodes_reader(ss) >> line;
    REQUIRE(line == "$Nodes");
  }

  SECTION("Unordered section search")
  {
    std::string line;

    nodes_reader.seekg(ss, 0);
    nodes_reader(ss) >> line;
    REQUIRE(line == "$Nodes");

    entities_reader.seekg(ss, 0);
    entities_reader(ss) >> line;
    REQUIRE(line == "$Entities");

    format_reader.seekg(ss, 0);
    format_reader(ss) >> line;
    REQUIRE(line == "$MeshFormat");
  }

  SECTION("Read until section end")
  {
    std::string line;

    format_reader.seekg(ss, 0);
    while(format_reader(ss) >> line) {}
    REQUIRE(line == "$EndMeshFormat");

    entities_reader.seekg(ss, 0);
    while(entities_reader(ss) >> line) {}
    REQUIRE(line == "$EndEntities");
  }

  SECTION("Read section without end")
  {
    nodes_reader.seekg(ss, 0);
    auto run = [&]()
    {
      std::string line;
      while(nodes_reader(ss) >> line) {}
    };
    REQUIRE_THROWS(run());
  }

  SECTION("Read line from a section")
  {
    format_reader.seekg(ss, 0);
    const auto firstline = format_reader.getline(ss);
    REQUIRE(firstline == "$MeshFormat");
    const auto bodyline = format_reader.getline(ss);
    REQUIRE(bodyline == "4.1 0 8");
  }
}

// These are closer to integration tests
TEST_CASE("Read and locate section in ASCII mesh", "[section, ASCII]")
{
  std::ifstream ifs{"box-txt.msh"}; // Open ASCII mesh file for reading

  const cfg::reader::SectionReader format_reader("MeshFormat", ifs);
  const cfg::reader::SectionReader entities_reader("Entities", ifs);
  const cfg::reader::SectionReader nodes_reader("Nodes", ifs);

  SECTION("Find section start")
  {
    std::string line;

    format_reader.seekg(ifs, 0);
    format_reader(ifs) >> line;
    REQUIRE(line == "$MeshFormat");

    entities_reader.seekg(ifs, 0);
    entities_reader(ifs) >> line;
    REQUIRE(line == "$Entities");

    nodes_reader.seekg(ifs, 0);
    nodes_reader(ifs) >> line;
    REQUIRE(line == "$Nodes");
  }

  SECTION("Unordered section search")
  {
    std::string line;

    nodes_reader.seekg(ifs, 0);
    nodes_reader(ifs) >> line;
    REQUIRE(line == "$Nodes");

    entities_reader.seekg(ifs, 0);
    entities_reader(ifs) >> line;
    REQUIRE(line == "$Entities");

    format_reader.seekg(ifs, 0);
    format_reader(ifs) >> line;
    REQUIRE(line == "$MeshFormat");
  }

  SECTION("Read until section end")
  {
    std::string line;

    format_reader.seekg(ifs, 0);
    while(format_reader(ifs) >> line) {}
    REQUIRE(line == "$EndMeshFormat");

    entities_reader.seekg(ifs, 0);
    while(entities_reader(ifs) >> line) {}
    REQUIRE(line == "$EndEntities");

    nodes_reader.seekg(ifs, 0);
    while(nodes_reader(ifs) >> line) {}
    REQUIRE(line == "$EndNodes");
  }
}

TEST_CASE("Read and locate section in binary mesh", "[section, binary]")
{
  std::ifstream ifs{"box-bin.msh", std::ios::binary}; // Open binary mesh file

  const cfg::reader::SectionReader format_reader("MeshFormat", ifs);
  const cfg::reader::SectionReader entities_reader("Entities", ifs);
  const cfg::reader::SectionReader nodes_reader("Nodes", ifs);

  SECTION("Find section start")
  {
    std::string line;

    format_reader.seekg(ifs, 0);
    format_reader(ifs) >> line;
    REQUIRE(line == "$MeshFormat");

    entities_reader.seekg(ifs, 0);
    entities_reader(ifs) >> line;
    REQUIRE(line == "$Entities");

    nodes_reader.seekg(ifs, 0);
    nodes_reader(ifs) >> line;
    REQUIRE(line == "$Nodes");
  }

  SECTION("Unordered section search")
  {
    std::string line;

    nodes_reader.seekg(ifs, 0);
    nodes_reader(ifs) >> line;
    REQUIRE(line == "$Nodes");

    entities_reader.seekg(ifs, 0);
    entities_reader(ifs) >> line;
    REQUIRE(line == "$Entities");

    format_reader.seekg(ifs, 0);
    format_reader(ifs) >> line;
    REQUIRE(line == "$MeshFormat");
  }

  SECTION("Read until section end")
  {
    std::string line;

    format_reader.seekg(ifs, 0);
    while(format_reader(ifs) >> line) {}
    REQUIRE(line == "$EndMeshFormat");

    entities_reader.seekg(ifs, 0);
    while(entities_reader(ifs) >> line) {}
    REQUIRE(line == "$EndEntities");

    nodes_reader.seekg(ifs, 0);
    while(nodes_reader(ifs) >> line) {}
    REQUIRE(line == "$EndNodes");
  }
}
