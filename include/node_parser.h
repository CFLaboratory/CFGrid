/**
 * node_parser.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_NODE_PARSER_H_
#define __CFG_NODE_PARSER_H_

#include <array>
#include <cstddef>
#include <functional>
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
    size_t natural_idx;       ///< The natural index of the node
    size_t global_idx;        ///< The global index of the node
    std::array<double, d> x;  ///< The node coordinates
  };

  /**
   * Constructs a function that can read a Section of a GMSH file.
   *
   * @param hdr_parser  An object that parses the header of the targeted Section.
   * @param data_parser An object that parses the data of the targeted Section.
   * @param environment Allows passing data from the calling environment to the DataParser.
   * @param validator   An object that validates the read data.
   */
  template <class H, class D, class E, class V>
  auto read_X(const H hdr_parser, const D data_parser, const E environment, const V validator)
  {
    return [hdr_parser, data_parser, environment, validator](
               const cfg::reader::SectionReader& section_reader, std::ifstream& mesh_stream, const Mode mode)
    {
      const auto hdr       = hdr_parser.parse(section_reader, mesh_stream, mode);
      const auto data      = data_parser.parse(section_reader, mesh_stream, mode, hdr, environment);
      validator.validate(data, hdr);
      return data;
    };
  }

  /**
   * Reads the nodes from a mesh file.
   *
   * @param mesh_stream The data stream associated with the mesh file.
   * @param mode        Flag indicating whether the file was opened in ASCII or binary mode.
   * @param parallel    The parallel environment.
   */
  void read_nodes(std::ifstream& mesh_stream, const Mode mode, const cfg::utils::Parallel& parallel);
}  // namespace cfg::parser

#endif  // __CFG_NODE_PARSER_H_
