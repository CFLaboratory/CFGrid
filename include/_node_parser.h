/**
 * _node_parser.h
 *
 * Internal components of the node_parser module.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG__NODE_PARSER_H_
#define __CFG__NODE_PARSER_H_

#include <fstream>
#include <functional>
#include <vector>

#include <node_parser.h>
#include <utils.h>

namespace cfg::parser
{
  /**
   * Container for the relevant information from the header of the Nodes section in a GMSH file.
   */
  struct NodeHeader
  {
    size_t n_nodes;   ///< The number of nodes in the mesh
    size_t n_blocks;  ///< The number of node blocks in the mesh
    size_t min_tag;   ///< The minimum node index
    size_t max_tag;   ///< The maximum node index
  };

  /**
   * Parses the data header of the Nodes segment in a GMSH file: the number of blocks of nodes to
   * read and the global description of the nodes in the mesh.
   */
  class HeaderParser
  {
   public:
    /**
     * Parses the header of the Node Section of a GMSH file.
     *
     * @param node_reader The node reader object for the mesh.
     * @param mesh_stream The mesh data stream.
     * @param mode        Indicates the data mode of the mesh stream, currently either ASCII or BINARY.
     * @returns The global node description header.
     */
    [[nodiscard]] static auto parse(const cfg::reader::SectionReader& node_reader,
                                    std::istream& mesh_stream,
                                    const reader::Mode mode)
    {
      if (mode == reader::Mode::BINARY)
      {
        mesh_stream.ignore(1);  // Skip spare char
      }
      const auto node_header = [&node_reader, &mesh_stream, mode]() -> NodeHeader
      {
        NodeHeader node_header{};

        node_header.n_blocks = read_one<size_t>(node_reader, mesh_stream, mode);
        node_header.n_nodes  = read_one<size_t>(node_reader, mesh_stream, mode);
        node_header.min_tag  = read_one<size_t>(node_reader, mesh_stream, mode);
        node_header.max_tag  = read_one<size_t>(node_reader, mesh_stream, mode);

        return node_header;
      }();

      return node_header;
    }
  };

  /**
   * Describes the environment for the node DataParser.
   */
  struct NodeEnvironment
  {
    const utils::Parallel& parallel;  ///< The parallel environment.
  };

  /**
   * Parses each block of nodes.
   */
  class DataParser
  {
   public:
    /**
     * Parses a node data block.
     *
     * @param node_reader The node reader object for the mesh.
     * @param mesh_stream The mesh data stream.
     * @param mode        Indicates the data mode of the mesh stream, currently either ASCII or BINARY.
     * @param node_header The global node description header.
     * @param environment Contains the calling environment, in particular passes the Parallel field
     * @returns The node vector.
     */
    [[nodiscard]] static auto parse(const reader::SectionReader& node_reader,
                                    std::istream& mesh_stream,
                                    const reader::Mode mode,
                                    const NodeHeader& node_header,
                                    const NodeEnvironment& environment)
    {
      std::vector<Node<3>> nodes;

      const utils::NaivePartition partition{environment.parallel, node_header.n_nodes};

      // Read nodes from each block
      size_t ctr = 0;
      for (size_t block = 0; block < node_header.n_blocks; block++)
      {
        const auto [block_dim, block_tag, block_param, block_nodes] =
            parse_node_block_header(node_reader, mesh_stream, mode);
        const auto indices = parse_node_idx(node_reader, block_nodes, mesh_stream, mode);
        const auto coords  = parse_node_coords(node_reader, block_nodes, mesh_stream, mode);
        const auto _nodes  = [&partition](const std::vector<Node<3>>& nodes) -> std::vector<Node<3>>
        {
          std::vector<Node<3>> _nodes;
          for (const auto n : nodes)
          {
            if (partition.pick(n.global_idx))
            {
              _nodes.push_back(n);
            }
          }
          return _nodes;
        }(assemble_nodes(indices, coords, ctr));

        // Append block's nodes to nodes
        nodes = cfg::utils::append(nodes, _nodes.begin(), _nodes.end());
      }

      return nodes;
    }

   private:
    /**
     * Parses the data header of a node block in a GMSH file.
     *
     * @param node_reader The node reader object for the mesh.
     * @param mesh_stream The mesh data stream.
     * @param mode        Indicates the data mode of the mesh stream, currently either ASCII or BINARY.
     * @returns A tuple of the block dimension, block tag, flag indicating whether the block is
     *          parametric and the number of nodes in the block.
     */
    [[nodiscard]] static std::tuple<int, int, bool, size_t> parse_node_block_header(
        const reader::SectionReader& node_reader,
        std::istream& mesh_stream,
        const reader::Mode mode) noexcept
    {
      const auto block_dim   = read_one<int>(node_reader, mesh_stream, mode);
      const auto block_tag   = read_one<int>(node_reader, mesh_stream, mode);
      const auto block_param = read_one<int>(node_reader, mesh_stream, mode);
      const auto block_nodes = read_one<size_t>(node_reader, mesh_stream, mode);

      return {block_dim, block_tag, bool{static_cast<bool>(block_param)}, block_nodes};
    }

    /**
     * Parses the indices of the nodes in a block in a GMSH file.
     *
     * @param node_reader The node reader object for the mesh.
     * @param block_nodes The number of nodes in the block.
     * @param mesh_stream The mesh data stream.
     * @param mode        Indicates the data mode of the mesh stream, currently either ASCII or BINARY.
     * @returns A vector of node indices.
     */
    [[nodiscard]] static std::vector<size_t> parse_node_idx(const cfg::reader::SectionReader& node_reader,
                                                            const size_t block_nodes,
                                                            std::istream& mesh_stream,
                                                            const reader::Mode mode) noexcept
    {
      std::vector<size_t> indices(block_nodes);

      for (size_t node = 0; node < block_nodes; node++)
      {
        indices[node] = read_one<size_t>(node_reader, mesh_stream, mode);
      }

      return indices;
    }

    /**
     * Parses the coordinates of the nodes in a block in a GMSH file.
     *
     * @param node_reader The node reader object for the mesh.
     * @param block_nodes The number of nodes in the block.
     * @param mesh_stream The mesh data stream.
     * @param mode        Indicates the data mode of the mesh stream, currently either ASCII or BINARY.
     * @returns A vector of node coordinates.
     */
    [[nodiscard]] static std::vector<std::array<double, 3>> parse_node_coords(
        const cfg::reader::SectionReader& node_reader,
        const size_t block_nodes,
        std::istream& mesh_stream,
        const reader::Mode mode) noexcept
    {
      std::vector<std::array<double, 3>> coords(block_nodes);

      auto pop_component = [&node_reader, &mesh_stream, mode]() -> double
      {
        return read_one<double>(node_reader, mesh_stream, mode);
      };

      for (size_t node = 0; node < block_nodes; node++)
      {
        coords[node] = {pop_component(), pop_component(), pop_component()};
      }

      return coords;
    }

    /**
     * Assembles collections of node indices and node coordinates into a collection of nodes.
     *
     * @param indices The vector of node indices.
     * @param coords  The vector of node coordinates.
     * @returns A vector of nodes.
     */
    [[nodiscard]] static std::vector<Node<3>> assemble_nodes(const std::vector<size_t>& indices,
                                                             const std::vector<std::array<double, 3>>& coords,
                                                             size_t& ctr) noexcept
    {
      std::vector<Node<3>> _nodes(indices.size());
      std::transform(indices.begin(),
                     indices.end(),
                     coords.begin(),
                     _nodes.begin(),
                     [&ctr](const size_t idx, const std::array<double, 3> coo) -> Node<3>
                     {
                       return Node<3>{idx, ctr++, coo};
                     });
      return _nodes;
    }
  };

  /**
   * Performs validation of the node data that was read, raising an error if this fails.
   *
   * @param nodes       The vector of nodes.
   * @param node_header The global description of the nodes in the mesh that is used to test the data.
   * @param parallel    The parallel configuration object.
   */
  void validate_nodes(const std::vector<Node<3>>& nodes,
                      const NodeHeader& node_header,
                      const cfg::utils::Parallel& parallel);

  /**
   * Utility to construct a node reader.
   *
   * @param parallel The parallel environment.
   * @returns A function to read nodes from a GMSH file.
   */
  std::function<std::vector<Node<3>>(const cfg::reader::SectionReader&, std::ifstream&, const reader::Mode)> make_node_reader(
      const cfg::utils::Parallel& parallel);
}  // namespace cfg::parser

#endif  // __CFG__NODE_PARSER_H_
