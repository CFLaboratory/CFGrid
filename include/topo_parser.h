/**
 * topo_parser.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_TOPO_PARSER_H_
#define __CFG_TOPO_PARSER_H_

#include <fstream>
#include <utility>

#include <node_parser.h>
#include <utils.h>

namespace cfg::parser
{

  template <class T>
  class NestedVector
  {
   public:
    NestedVector() = default;
    NestedVector(std::vector<size_t> ptr, std::vector<T> val) : ptr(std::move(ptr)), val(val) {}
    
    [[nodiscard]] size_t size() const
    {
      if (ptr.empty())
      {
        return 0;
      }
      return ptr.size() - 1;  // ptr contains n+1 offsets
    }

    [[nodiscard]] auto begin() const
    {
      return iterator(0, ptr, val);
    }

    [[nodiscard]] auto end() const
    {
      if (size() == 0)
      {
        return begin();
      }

      return iterator(size() + 1, ptr, val);
    }

   private:
    std::vector<size_t> ptr{};  // The row start vector
    std::vector<T> val{};  // The row values vector

    class iterator
    {
     public:
      iterator(const size_t idx, const std::vector<T>& ptr, const std::vector<T>& val)
          : offset(ptr.begin() + idx), last(ptr.end() - 1), _begin(val.begin() + *offset), _end(val.begin() + *(offset + 1))
      {
        if (ptr.empty())
        {
          _end = _begin;
        }
      }

      [[nodiscard]] auto begin() const
      {
        return _begin;
      }

      [[nodiscard]] auto end() const
      {
        return _end;
      }

      bool operator!=(const iterator& a)
      {
        return this->offset != a.offset;
      }

      iterator& operator++()
      {
	// Move beginning to end
        _begin =_end;
	
	// Advance offset if not at last entry, storing previous location
	const auto tmp = offset;
	if (offset != last)
	{
	  offset++;
	}

	// Advance end by step between offset values
	const auto delta = *offset - *tmp;
        _end += delta;

        return *this;
      }

      iterator& operator*()
      {
        return *this;
      }

     private:
      typename std::vector<T>::const_iterator offset;
      typename std::vector<T>::const_iterator last;
      typename std::vector<T>::const_iterator _begin;
      typename std::vector<T>::const_iterator _end;
    };
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
