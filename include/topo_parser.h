/**
 * topo_parser.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_TOPO_PARSER_H_
#define __CFG_TOPO_PARSER_H_

#include <fstream>

#include <node_parser.h>
#include <utils.h>

namespace cfg::parser
{
  template <class T>
  class NestedVector
  {
   public:
    [[nodiscard]] size_t size() const
    {
      if (ptr.empty())
      {
        return 0;
      }
      return ptr.size() - 1;
    }

   private:
    std::vector<T> ptr{};  // The row start vector
    std::vector<T> val{};  // The row values vector
  };

  template <class T>
  class Connectivity
  {
   public:
    Connectivity() = default;
    Connectivity(const size_t nlocal,
                 const size_t nhalo,
                 const NestedVector<T>& edges,
                 const NestedVector<T>& boundaries)
        : nlocal{nlocal}, nhalo{nhalo}, edges{edges}, boundaries{boundaries} {};

    [[nodiscard]] Connectivity local() const
    {
      return Connectivity{nlocal, 0, edges, boundaries};
    }
    [[nodiscard]] Connectivity halo() const
    {
      // Return a new Connectivity restricted to just the halo section
      return Connectivity<T>{nhalo, 0, edges, boundaries};
    }

    [[nodiscard]] auto size() const
    {
      return (std::max(edges.size(), boundaries.size()));
    }

   private:
    size_t nlocal{};               // How many elements are local to this process?
    size_t nhalo{};                // How many elements are non-local to this process?
    NestedVector<T> edges{};       // Connectivity to higher-dimensional elements
    NestedVector<T> boundaries{};  // Connectivity to lower-dimensional elements
  };

  class Topo
  {
   public:
    [[nodiscard]] const Connectivity<size_t>& nodes() const
    {
      return _nodes;
    }

   private:
    Connectivity<size_t> _nodes{};
  };

  std::ostream& operator<<(std::ostream& os, const Topo& topo);

  /**
   * Reads the topology from a mesh file
   */
  [[nodiscard]] Topo read_topo(std::ifstream& mesh_stream,
                               const reader::Mode mode,
                               const std::vector<Node<3>>& nodes,
                               const cfg::utils::Parallel& parallel);
}  // namespace cfg::parser

#endif  // __CFG_TOPO_PARSER_H_
