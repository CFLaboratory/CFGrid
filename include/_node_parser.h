/**
 * _node_parser.h
 *
 * Internal components of the node_parser module.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG__NODE_PARSER_H_
#define __CFG__NODE_PARSER_H_

#include <node_parser.h>
#include <vector>

#include <utils.h>

namespace cfg::parser
{
  /**
   * Container for the relevant information from the header of the Nodes section in a GMSH file.
   */
  struct NodeHeader
  {
    size_t n_nodes;   // The number of nodes in the mesh
    size_t n_blocks;  // The number of node blocks in the mesh
    size_t min_tag;   // The minimum node index
    size_t max_tag;   // The maximum node index
  };

  /**
   * Parses the data header of the Nodes segment in a GMSH file: the number of blocks of nodes to
   * read and the global description of the nodes in the mesh.
   *
   * @param node_reader The node reader object for the mesh.
   * @param mesh_stream The mesh data stream.
   * @param mode        Indicates the data mode of the mesh stream, currently either ASCII or BINARY.
   * @returns The global node description header.
   */
  [[nodiscard]] NodeHeader parse_node_header(const cfg::reader::SectionReader& node_reader,
                                             std::istream& mesh_stream,
                                             const Mode mode) noexcept;

  /**
   * Parses each block of nodes.
   *
   * @param node_reader The node reader object for the mesh.
   * @param node_header The global node description header.
   * @param mesh_stream The mesh data stream.
   * @param mode        Indicates the data mode of the mesh stream, currently either ASCII or BINARY.
   * @param parallel    The parallel configuration object.
   * @returns The node vector.
   */
  [[nodiscard]] std::vector<Node<3>> parse_node_blocks(const cfg::reader::SectionReader& node_reader,
                                                       const NodeHeader& node_header,
                                                       std::istream& mesh_stream,
                                                       const Mode mode,
                                                       const cfg::utils::Parallel& parallel) noexcept;

  /**
   * Performs validation of the node data that was read, raising an error if this fails.
   *
   * @param nodes       The vector of nodes.
   * @prarm node_header The global description of the nodes in the mesh that is used to test the data.
   * @param parallel    The parallel configuration object.
   */
  void validate_nodes(const std::vector<Node<3>>& nodes,
                      const NodeHeader& node_header,
                      const cfg::utils::Parallel& parallel);
}  // namespace cfg::parser

#endif  // __CFG__NODE_PARSER_H_
