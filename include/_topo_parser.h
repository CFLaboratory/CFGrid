/**
 * _topo_parser.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG__TOPO_PARSER_H_
#define __CFG__TOPO_PARSER_H_

#include <cstddef>

#include <reader.h>
#include <section_reader.h>

namespace cfg::parser
{
  /**
   * Container for the relevant information from the header of the Elements (topology) section in a
   * GMSH file.
   */
  struct TopoHeader
  {
    size_t n_elements;  ///< The number of elements in the mesh --- N.B. elements span all dimensions
    size_t n_blocks;    ///< The number of node blocks in the mesh
    size_t min_tag;     ///< The minimum node index
    size_t max_tag;     ///< The maximum node index
  };

  class HeaderParser
  {
   public:
    [[nodiscard]] static auto parse(const reader::SectionReader& topo_reader,
                                    std::istream& mesh_stream,
                                    const reader::Mode mode)
    {
      if (mode == reader::Mode::BINARY)
      {
        mesh_stream.ignore(1);  // Skip spare char
      }
      const auto topo_header = [&topo_reader, &mesh_stream, mode]() -> TopoHeader
      {
        TopoHeader topo_header{};

        topo_header.n_blocks   = reader::read_one<size_t>(topo_reader, mesh_stream, mode);
        topo_header.n_elements = reader::read_one<size_t>(topo_reader, mesh_stream, mode);
        topo_header.min_tag    = reader::read_one<size_t>(topo_reader, mesh_stream, mode);
        topo_header.max_tag    = reader::read_one<size_t>(topo_reader, mesh_stream, mode);

        return topo_header;
      }();

      return topo_header;
    }
  };

}  // namespace cfg::parser

#endif  // __CFG__TOPO_PARSER_H_
