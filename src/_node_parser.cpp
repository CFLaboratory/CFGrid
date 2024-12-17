/**
 * _node_parser.cpp
 *
 * Implements the internal components of the node_parser module.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <_node_parser.h>

#include <algorithm>
#include <stdexcept>

#include <utils.h>

namespace cfg::parser
{
  [[nodiscard]] NodeHeader parse_node_header(const cfg::reader::SectionReader& node_reader,
                                             std::istream& mesh_stream,
                                             const Mode mode) noexcept
  {
    if (mode == Mode::BINARY)
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

  /**
   * Parses the data header of a node block in a GMSH file.
   *
   * @param node_reader The node reader object for the mesh.
   * @param mesh_stream The mesh data stream.
   * @param mode        Indicates the data mode of the mesh stream, currently either ASCII or BINARY.
   * @returns A tuple of the block dimension, block tag, flag indicating whether the block is
   *          parametric and the number of nodes in the block.
   */
  [[nodiscard]] std::tuple<int, int, bool, size_t> parse_node_block_header(
      const cfg::reader::SectionReader& node_reader,
      std::istream& mesh_stream,
      const Mode mode) noexcept
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
  [[nodiscard]] std::vector<size_t> parse_node_idx(const cfg::reader::SectionReader& node_reader,
                                                   const size_t block_nodes,
                                                   std::istream& mesh_stream,
                                                   const Mode mode) noexcept
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
  [[nodiscard]] std::vector<std::array<double, 3>> parse_node_coords(const cfg::reader::SectionReader& node_reader,
                                                                     const size_t block_nodes,
                                                                     std::istream& mesh_stream,
                                                                     const Mode mode) noexcept
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
  [[nodiscard]] std::vector<Node<3>> assemble_nodes(const std::vector<size_t>& indices,
                                                    const std::vector<std::array<double, 3>>& coords) noexcept
  {
    std::vector<Node<3>> _nodes(indices.size());
    std::transform(indices.begin(),
                   indices.end(),
                   coords.begin(),
                   _nodes.begin(),
                   [](const size_t idx, const std::array<double, 3> coo) -> Node<3>
                   {
                     return Node<3>{idx, coo};
                   });
    return _nodes;
  }

  [[nodiscard]] std::vector<Node<3>> parse_node_blocks(const cfg::reader::SectionReader& node_reader,
                                                       const NodeHeader& node_header,
                                                       std::istream& mesh_stream,
                                                       const Mode mode,
                                                       const cfg::utils::Parallel& parallel) noexcept
  {
    std::vector<Node<3>> nodes;

    // Without any better information we will (initially) partition the nodes naively
    cfg::utils::NaivePartition partition{parallel, node_header.n_nodes};

    // Read nodes from each block
    size_t ctr = 0;
    for (size_t block = 0; block < node_header.n_blocks; block++)
    {
      const auto [block_dim, block_tag, block_param, block_nodes] =
          parse_node_block_header(node_reader, mesh_stream, mode);
      const auto indices = parse_node_idx(node_reader, block_nodes, mesh_stream, mode);
      const auto coords  = parse_node_coords(node_reader, block_nodes, mesh_stream, mode);
      const auto _nodes  = [&partition, &ctr](const std::vector<Node<3>>& nodes) -> std::vector<Node<3>>
      {
        std::vector<Node<3>> _nodes;
        for (const auto n : nodes)
        {
          if (partition.pick(ctr))
          {
            _nodes.push_back(n);
          }
          ctr++;
        }
        return _nodes;
      }(assemble_nodes(indices, coords));

      // Append block's nodes to nodes
      nodes = cfg::utils::append(nodes, _nodes.begin(), _nodes.end());
    }

    return nodes;
  }

  /**
   * Performs validation of the strides in node indices. Either the nodes are contiguous in which
   * case the indices should be stride one, otherwise if there are holes in the index range we
   * expect that there are no duplicate indices. An error is raised if the validation fails.
   *
   * @param nodes       The vector of nodes.
   * @prarm node_header The global description of the nodes in the mesh that is used to test the data.
   */
  void validate_strides(const std::vector<Node<3>>& nodes, const NodeHeader& node_header)
  {
    // Utility function to check whether the node range covers the number of
    // nodes, therefore nodes should be contiguous (when sorted)
    auto should_be_contiguous = [](const NodeHeader& node_header) -> bool
    {
      return (node_header.max_tag - (node_header.min_tag - 1)) == node_header.n_nodes;
    };

    // Utility function to compute the stride in indexes between two nodes, short names are clear.
    // NOLINTNEXTLINE(readability-identifier-length)
    auto test_node_stride = [](const Node<3>& a, const Node<3>& b) -> size_t
    {
      return (b.idx - a.idx);
    };

    // Create a sorted copy of the nodes based on their indices for checking strides
    auto nodes_copy = nodes;
    std::sort(nodes_copy.begin(),
              nodes_copy.end(),
              [](const Node<3>& node_a, const Node<3>& node_b) -> bool
              {
                return node_a.idx < node_b.idx;
              });
    if (should_be_contiguous(node_header))
    {
      // Nodes should be contiguous when sorted
      if (!cfg::utils::test_stride<Node<3>>(nodes_copy.begin(), nodes_copy.end(), test_node_stride))
      {
        throw std::runtime_error("Expected contiguous node indices, yet sorted indices were non-contiguous");
      }
    }
    else
    {
      // Nodes should at least not contain any duplicates
      using NodesIterator = std::vector<Node<3>>::iterator;
      auto test_nodup     = [test_node_stride](NodesIterator first, NodesIterator last) -> bool
      {
        return !cfg::utils::test_stride<Node<3>>(first, last, test_node_stride, 0);
      };

      if (!test_nodup(nodes_copy.begin(), nodes_copy.end()))
      {
        throw std::runtime_error("Expected non-contiguous node indices: duplicate indices were found");
      }
    }
  }

  void validate_nodes(const std::vector<Node<3>>& nodes,
                      const NodeHeader& node_header,
                      const cfg::utils::Parallel& parallel)
  {
    // Validate that we read enough data based on the naive partition
    const cfg::utils::NaivePartition partition{parallel, node_header.n_nodes};
    if (nodes.size() != partition.size())
    {
      throw std::runtime_error("The number of nodes does not match expectation");
    }

    // Validate data

    // Utility function to get the minimum and maximum of a vector
    using const_NodesIterator = std::vector<Node<3>>::const_iterator;
    auto get_min_max          = [](const_NodesIterator first, const_NodesIterator last) -> std::pair<size_t, size_t>
    {
      const auto [it_min, it_max] = std::minmax_element(first,
                                                        last,
                                                        [](const Node<3>& node1, const Node<3>& node2)
                                                        {
                                                          return node1.idx < node2.idx;
                                                        });
      return {(*it_min).idx, (*it_max).idx};
    };

    const auto [idx_min, idx_max] = get_min_max(nodes.begin(), nodes.end());
    if (idx_min < node_header.min_tag)
    {
      throw std::runtime_error("The node indices are below the expected range");
    }
    if (idx_max > node_header.max_tag)
    {
      throw std::runtime_error("The node indices are above the expected range");
    }
  }
}  // namespace cfg::parser
