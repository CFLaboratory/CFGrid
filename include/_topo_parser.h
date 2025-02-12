/**
 * _topo_parser.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG__TOPO_PARSER_H_
#define __CFG__TOPO_PARSER_H_

#include <cstddef>
#include <iostream>

#include <reader.h>
#include <section_reader.h>
#include <topo_parser.h>

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

  class DataParser
  {
   public:
    [[nodiscard]] static Topo parse(const reader::SectionReader& topo_reader,
                                    std::istream& mesh_stream,
                                    const reader::Mode mode,
                                    const TopoHeader& hdr,
                                    const std::vector<Node<3>>& nodes)
    {
      Topo topo{};

      for (size_t block = 0; block < hdr.n_blocks; block++)
      {
        const auto [block_dim, block_tag, elt_type, block_elt] =
            parse_topo_block_header(topo_reader, mesh_stream, mode);
        const auto elt = parse_elt(topo_reader, block_elt, mesh_stream, mode);

        for (const auto e : elt)
        {
	  std::cout << "hihi" << std::endl;
          const auto element_is_local = [e](const Node<3>& node) -> bool
          {
            return std::find_if(e.begin(),
                                e.end(),
                                [node](const size_t n) -> bool
                                {
				  std::cout << "hola" << std::endl;
                                  return n == node.natural_idx;
                                }) != e.end();
          };

          if (std::find_if(nodes.begin(), nodes.end(), element_is_local) != nodes.end())
          {
            std::cout << "Fount it!" << std::endl;
          }
	  std::cout << "Buenos noches" << std::endl;
        }
      }

      return topo;
    }

   private:
    [[nodiscard]] static std::tuple<int, int, int, size_t> parse_topo_block_header(
        const reader::SectionReader& topo_reader,
        std::istream& mesh_stream,
        const reader::Mode mode) noexcept
    {
      const auto block_dim = reader::read_one<int>(topo_reader, mesh_stream, mode);
      const auto block_tag = reader::read_one<int>(topo_reader, mesh_stream, mode);
      const auto elt_type  = reader::read_one<int>(topo_reader, mesh_stream, mode);
      const auto block_elt = reader::read_one<size_t>(topo_reader, mesh_stream, mode);

      return {block_dim, block_tag, elt_type, block_elt};
    }

    [[nodiscard]] static NestedVector<size_t> parse_elt(const reader::SectionReader& topo_reader,
                                                        const size_t block_elt,
                                                        std::istream& mesh_stream,
                                                        const reader::Mode mode)
    {
      return {};
    }
  };

  class Validator
  {
   public:
    void validate(const Topo& topo, const TopoHeader& hdr) const {};
  };

}  // namespace cfg::parser

#endif  // __CFG__TOPO_PARSER_H_
