/**
 * _topo_parser.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG__TOPO_PARSER_H_
#define __CFG__TOPO_PARSER_H_

#include <cstddef>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <utility>

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
    [[nodiscard]] const static auto parse(const reader::SectionReader& topo_reader,
                                          std::istream& mesh_stream,
                                          const reader::Mode mode)
    {
      if (mode == reader::Mode::BINARY)
      {
        mesh_stream.ignore(1);  // Skip spare char
      }

      return _parse(topo_reader, mesh_stream, mode);
    }

   private:
    [[nodiscard]] static TopoHeader _parse(const reader::SectionReader& topo_reader,
                                           std::istream& mesh_stream,
                                           const reader::Mode mode)
    {
      TopoHeader topo_header{};

      topo_header.n_blocks   = reader::read_one<size_t>(topo_reader, mesh_stream, mode);
      topo_header.n_elements = reader::read_one<size_t>(topo_reader, mesh_stream, mode);
      topo_header.min_tag    = reader::read_one<size_t>(topo_reader, mesh_stream, mode);
      topo_header.max_tag    = reader::read_one<size_t>(topo_reader, mesh_stream, mode);

      return topo_header;
    }
  };

  template <class T>
  class ConnectivityVector
  {
   public:
    ConnectivityVector() = default;
    ConnectivityVector(const unsigned int dim, const std::vector<size_t>& ptr, std::vector<T> val)
        : _dim(dim), conn(ptr, std::move(val)) {};
    ConnectivityVector(const int dim, const std::vector<size_t>& ptr, std::vector<T> val)
        : ConnectivityVector(static_cast<unsigned int>(dim), ptr, std::move(val))
    {
      // Could this be written as a
      if (dim < 0)
      {
        throw std::runtime_error("The dimensionality must be >= 0");
      }
    };

    [[nodiscard]] auto size() const
    {
      return conn.size();
    }

    [[nodiscard]] auto begin() const
    {
      return conn.begin();
    }
    [[nodiscard]] auto end() const
    {
      return conn.end();
    }

    unsigned int _dim{};  // Dimensionality of the connected components
    NestedVector<size_t> conn;
  };

  template <typename F>
  [[nodiscard]] static ConnectivityVector<size_t> take_elts(const std::vector<Node<3>>& nodes,
                                                            const ConnectivityVector<size_t>& block_elts,
                                                            const F pred)
  {
    std::vector<size_t> ptr;
    std::vector<size_t> nds;
    ptr.push_back(0);

    for (const auto& elt : block_elts)
    {
      const auto take = [pred, &elt](const Node<3>& node) -> bool
      {
        return pred(elt, node);
      };

      if (std::find_if(nodes.begin(), nodes.end(), take) != nodes.end())
      {
        // Push element nodes into NestedVector and increment pointer.
        auto next = ptr.back();
        for (const auto& n : elt)
        {
          nds.push_back(n);
          next++;
        }
        ptr.push_back(next);
      }
    }

    return {block_elts._dim, ptr, nds};
  };

  const auto element_is_local = [](const auto& elt, const Node<3>& node) -> bool
  {
    const auto node_is_in_element = [node](const size_t n) -> bool
    {
      return n == node.natural_idx;
    };

    return std::find_if(elt.begin(), elt.end(), node_is_in_element) != elt.end();
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

      // Find local elements
      auto local_elts = [](const reader::SectionReader& topo_reader,
                           std::istream& mesh_stream,
                           const reader::Mode mode,
                           const TopoHeader& hdr,
                           const std::vector<Node<3>>& nodes) -> std::map<unsigned int, NestedVector<size_t>>
      {
        const auto append_blocklocal_elts =
            [](const auto& elts_, std::vector<size_t>& ptr, std::vector<size_t>& nds) -> void
        {
          for (const auto& elt : elts_)
          {
            auto next = ptr.back();
            for (const auto& e_nd : elt)
            {
              nds.push_back(e_nd);
              next++;
            }
            ptr.push_back(next);
          }
        };

        // Loop over Blocks, read elements of the block and take local elements - return maps of CSR
        // ptr/values as a pair (using element dimension as the map key)
        std::map<unsigned int, std::vector<size_t>> ptr;
        std::map<unsigned int, std::vector<size_t>> nds;
        for (size_t block = 0; block < hdr.n_blocks; block++)
        {
          // Extract the local elements from the block
          const auto block_hdr  = parse_topo_block_header(topo_reader, mesh_stream, mode);
          const auto block_elts = parse_elts(topo_reader, block_hdr, mesh_stream, mode);
          const auto elts_      = take_elts(nodes, block_elts, element_is_local);

          const auto dim = block_hdr.dim;
          if (ptr[dim].size() == 0)
          {
            ptr[dim].push_back(0);
          }
          append_blocklocal_elts(elts_, ptr[dim], nds[dim]);
        }

        std::map<unsigned int, NestedVector<size_t>> elt_map{};
        for (const auto& itr : ptr)
        {
          const auto dim = itr.first;
          elt_map[dim]   = {ptr[dim], nds[dim]};
        }
        return elt_map;
      }(topo_reader, mesh_stream, mode, hdr, nodes);

      // Extract non-local nodes from element-node lists. We only need to query the nodes making up
      // the highest-dimension elements and compare this against the local node list to find the
      // non-local nodes.
      unsigned int dim_max = 0;
      for (const auto& itr : local_elts)
      {
        const auto dim     = itr.first;  // Map is keyed by dimension
        const auto n_nodes = itr.second.size();
        if (n_nodes > 0)
        {
          dim_max = std::max(dim_max, dim);
        }
      }
      std::cout << "dim " << dim_max << std::endl;

      const auto n_halo_max = local_elts[dim_max].size() - nodes.size();  // Upper bound for non-local node count
      std::cout << local_elts[dim_max].size() << " " << nodes.size() << std::endl;
      std::cout << n_halo_max << std::endl;
      // std::vector<Node<3>> halo_nodes{};
      // halo_nodes.reserve(n_halo_max);

      // Connect elements

      return topo;
    }

   private:
    struct BlockHeader
    {
      int dim;   // Dimension of elements in block
      int tag;   // Block tag/ID
      int type;  // Type of elements in block
      size_t n;  // Number of elements in block
    };

    [[nodiscard]] static BlockHeader parse_topo_block_header(const reader::SectionReader& topo_reader,
                                                             std::istream& mesh_stream,
                                                             const reader::Mode mode) noexcept
    {
      const auto block_dim = reader::read_one<int>(topo_reader, mesh_stream, mode);
      const auto block_tag = reader::read_one<int>(topo_reader, mesh_stream, mode);
      const auto elt_type  = reader::read_one<int>(topo_reader, mesh_stream, mode);
      const auto block_elt = reader::read_one<size_t>(topo_reader, mesh_stream, mode);

      return {block_dim, block_tag, elt_type, block_elt};
    }

    [[nodiscard]] static ConnectivityVector<size_t> parse_elts(const reader::SectionReader& topo_reader,
                                                               const BlockHeader block,
                                                               std::istream& mesh_stream,
                                                               const reader::Mode mode)
    {
      std::vector<size_t> nodes{};
      std::vector<size_t> elt_start{};

      const auto parse_1elt = [](std::istringstream& elt_iss) -> std::vector<size_t>
      {
        size_t elt_idx = {};
        elt_iss >> elt_idx;  // Pop element index

        std::vector<size_t> nodes{};
        size_t elt_nid = {};
        while (elt_iss >> elt_nid)
        {
          nodes.push_back(elt_nid);
        }

        return nodes;
      };

      const auto read_1elt = [&topo_reader](std::istream& mesh_stream) -> std::istringstream
      {
        const auto elt_str = topo_reader.getline(mesh_stream);
        return std::istringstream(elt_str);
      };

      // Set size upfront
      elt_start.resize(block.n);

      {                                                       // Read element-node connectivity for this block
        const auto dummy = topo_reader.getline(mesh_stream);  // Read empty data
        size_t ctr       = 0;

        for (auto& es : elt_start)
        {
          es = ctr;

          auto elt_iss         = read_1elt(mesh_stream);
          const auto elt_nodes = parse_1elt(elt_iss);
          for (const auto n : elt_nodes)
          {
            nodes.push_back(n);
          }
          ctr += elt_nodes.size();
        }
        elt_start.push_back(ctr);
      }

      return {block.dim, elt_start, nodes};
    }
  };

  class Validator
  {
   public:
    void validate(const Topo& topo, const TopoHeader& hdr) const {};
  };

}  // namespace cfg::parser

#endif  // __CFG__TOPO_PARSER_H_
