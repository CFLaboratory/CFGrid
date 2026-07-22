/**
 * topo_parser.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_TOPO_PARSER_H_
#define __CFG_TOPO_PARSER_H_

#include <cstddef>
#include <fstream>
#include <stdexcept>

#include <node_parser.h>
#include <utils.h>

#include <iostream>

namespace cfg::parser
{

  template <class T>
  class NestedVector
  {
   public:
    NestedVector() = default;
    NestedVector(const std::vector<size_t>& ptr, std::vector<T> val) : _val(val)
    {
      if (!ptr.empty())
      {
	if (ptr.back() > val.size())
	{
	  throw std::runtime_error("The last entry of ptr must be <= val.size()");
	}

        _size = ptr.size() - 1;
	size_t prev = 0;
        for (const auto o : ptr)
        {
	  if (o < prev)
	  {
	    throw std::runtime_error("The offsets ptr must be increasing");
	  }
	  prev = o;
          _ptr.push_back(_val.begin() + o);
        }
      }
      else
      {
        _ptr.push_back(_val.begin());
        _ptr.push_back(_val.end());
        _size = 0;
      }
    }

    auto operator[](std::size_t idx)
    {
      return Iterator(_ptr.begin() + idx, _ptr.end() - 1);      
    }        

    void push_back(std::vector<T> vals)
    {
      _size += 1;
      for (const auto& v : vals)
      {
	_val.push_back(v);        
      }        
      _ptr.push_back(_val.begin() + vals.size());
    }        

    [[nodiscard]] size_t size() const
    {
      return _size;
    }

    [[nodiscard]] auto begin() const
    {
      return Iterator(_ptr.begin(), _ptr.end() - 1);
    }

    [[nodiscard]] auto end() const
    {
      if (size() == 0)
      {
        return begin();
      }

      return Iterator(_ptr.end() - 1, _ptr.end() - 1);
    }

   private:
    using IterVec = std::vector<typename std::vector<T>::const_iterator>;
    IterVec _ptr{};         // The row start vector
    std::vector<T> _val{};  // The row values vector
    size_t _size{};

    class Iterator
    {
      using NestedIterator = typename IterVec::const_iterator;

     public:
      Iterator(NestedIterator itstart, const NestedIterator last) : curr(itstart), next(itstart), _last(last)
      {
        if (next != _last)
        {
          next++;
        }
      }

      [[nodiscard]] auto begin() const
      {
        return *curr;
      }

      [[nodiscard]] auto end() const
      {
        return *next;
      }

      [[nodiscard]] bool operator!=(const Iterator& other)
      {
        return this->curr != other.curr;
      }

      Iterator& operator++()
      {
        // Advance starting iterators
        curr = next;

        // Advance ending iterators if we haven't hit the end
        if (next != _last)
        {
          std::advance(next, 1);
        }

        return *this;
      }

      Iterator& operator*()
      {
        return *this;
      }

     private:
      NestedIterator curr;   //< The current position in the NestedVector
      NestedIterator next;   //< The next position in the NestedVector
      NestedIterator _last;  //< The end of the NestedVector
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
