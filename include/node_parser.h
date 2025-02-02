/**
 * node_parser.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_NODE_PARSER_H_
#define __CFG_NODE_PARSER_H_

#include <array>
#include <cstddef>
#include <istream>

#include <section_reader.h>
#include <utils.h>

namespace cfg::parser
{
  /**
   * Identifies which mode the mesh file is read in: ASCII (formatted) or BINARY (unformatted).
   */
  enum class Mode
  {
    ASCII,
    BINARY
  };

  /**
   * Reads a single item from the reader, according to the mode.
   */
  template <class C>
  [[nodiscard]] C read_one(const cfg::reader::SectionReader& node_reader, std::istream& mesh_stream, const Mode mode)
  {
    C val;
    if (mode == Mode::ASCII)
    {
      node_reader(mesh_stream) >> val;
    }
    else
    {
      const size_t nchar = sizeof(C) / sizeof(char);
      mesh_stream.read((char*)&val, nchar);
    }
    return val;
  }

  /**
   * A mesh node of arbitrary dimension `d`. This stores the node's index and coordinates.
   */
  template <unsigned int d>
  struct Node
  {
    size_t idx;
    std::array<double, d> x;
  };

  void read_nodes(std::ifstream& mesh_stream, const Mode mode, const cfg::utils::Parallel& parallel);
}  // namespace cfg::parser

#endif  // __CFG_NODE_PARSER_H_
