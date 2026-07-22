/**
 * topo_parser.cpp
 *
 * Tests the public interface for the topology objects.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <topo_parser.h>

#include <catch2/catch_test_macros.hpp>

namespace
{
  template <class T>
  void nested_vector(const std::vector<T> ptr, const std::vector<T> val)
  {
    cfg::parser::NestedVector<T> csr{ptr, val};
    const auto nrows = ptr.size() > 0 ? ptr.size() - 1 : 0;
    const auto nel   = val.size();

    SECTION("Check row count")
    {
      auto ctr = 0;
      for (auto row : csr)
      {
        // Silence unused variable warning
        auto a = *row.begin();
        a      = a + 0;

        ctr++;
      }
      REQUIRE(ctr == nrows);
    }

    SECTION("Check element count")
    {
      auto ctr = 0;
      for (auto row : csr)
      {
        for (auto e : row)
        {
          // Silence unused variable warning
          auto v = e;
          v      = v + 0;

          ctr++;
        }
      }
      REQUIRE(ctr == nel);
    }

    SECTION("Check row element count")
    {
      auto ctr = 0;
      for (auto row : csr)
      {
        const auto rowsize = ptr[ctr + 1] - ptr[ctr];
        REQUIRE(std::distance(row.begin(), row.end()) == rowsize);
        ctr++;
      }
    }

    SECTION("Check strides")
    {
      SECTION("Check begins")
      {
        auto startitr = ptr.begin();
        for (auto row : csr)
        {
          REQUIRE(*row.begin() == val[*startitr]);
          startitr++;
        }
      }

      SECTION("Check ends")
      {
        auto enditr = ptr.begin() + 1;
        for (auto row : csr)
        {
          if (enditr != ptr.end() - 1)
          {  // Don't check past the end of val
            REQUIRE(*row.end() == val[*enditr]);
            enditr++;
          }
        }
      }
    }

    SECTION("Check steps")
    {
      auto ctr = 0;
      for (auto row : csr)
      {
        auto ctr2 = 0;
        for (const auto entry : row)
        {
          REQUIRE(entry == val[ptr[ctr] + ctr2]);
          ctr2++;
        }
        ctr++;
      }
    }

    SECTION("Check push_back")
    {
      const auto _nvals = 3;
      const auto _vals  = []() -> std::vector<T>
      {
        std::vector<T> _vals{};
        for (auto i = 0; i < _nvals; i++)
        {
          _vals.push_back({});
        }

        return _vals;
      }();
      csr.push_back(_vals);
 
      // The CSR should now contain one more row
      // The size of the new row should be _nvals
      REQUIRE(csr.size() == nrows + 1);
      const auto row = csr[nrows];
      REQUIRE(std::distance(row.begin(), row.end()) == _nvals); // XXX: breaks because we have invalidated our pointers
    }
  }
}  // namespace

TEST_CASE("Empty NestedVector", "[internals]")
{
  const std::vector<size_t> ptr{};
  const std::vector<size_t> val{};
  nested_vector(ptr, val);
}

TEST_CASE("NestedVector", "[internals]")
{
  const std::vector<size_t> ptr{0, 3, 7, 9};
  const std::vector<size_t> val{1, 2, 3, 4, 5, 6, 7, 8, 9};
  nested_vector(ptr, val);
}
