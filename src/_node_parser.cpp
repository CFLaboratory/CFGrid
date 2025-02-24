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
      return (b.natural_idx - a.natural_idx);
    };

    // Create a sorted copy of the nodes based on their indices for checking strides
    auto nodes_copy = nodes;
    std::sort(nodes_copy.begin(),
              nodes_copy.end(),
              [](const Node<3>& node_a, const Node<3>& node_b) -> bool
              {
                return node_a.natural_idx < node_b.natural_idx;
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
                                                          return node1.natural_idx < node2.natural_idx;
                                                        });
      return {(*it_min).natural_idx, (*it_max).natural_idx};
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

  std::function<std::vector<Node<3>>(const reader::SectionReader&, std::ifstream&, const reader::Mode)> make_node_reader(
      const cfg::utils::Parallel& parallel)
  {
    class Validator
    {
     public:
      Validator(const utils::Parallel& parallel) : parallel{parallel} {}
      void validate(const std::vector<Node<3>>& nodes, const NodeHeader& node_header) const
      {
        validate_nodes(nodes, node_header, parallel);
      }

     private:
      const utils::Parallel& parallel;
    };

    // Return the node reader function
    return read_X(HeaderParser{}, DataParser{}, NodeEnvironment{parallel}, Validator{parallel});
  }
}  // namespace cfg::parser
