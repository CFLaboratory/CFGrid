/**
 * append.cpp
 *
 * Tests appending collection types - initially vectors.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>

#include <utils.h>

TEST_CASE("Append", "[utils]")
{
  const std::vector<int> vec_a = {1, 2, 3, 4, 5};
  const std::vector<int> vec_empty = {};

  SECTION("Append vectors")
  {
    std::vector<int> vec_new = {6, 7, 8, 9, 10};
    std::vector<int> vec_expect = {6, 7, 8, 9, 10, 1, 2, 3, 4, 5};

    vec_new = cfg::utils::append(vec_new, vec_a.begin(), vec_a.end());
    REQUIRE(vec_new == vec_expect);
  }

  SECTION("Append reverse vector")
  {
    std::vector<int> vec_new = {6, 7, 8, 9, 10};
    std::vector<int> vec_expect = {6, 7, 8, 9, 10, 5, 4, 3, 2, 1};

    vec_new = cfg::utils::append(vec_new, vec_a.rbegin(), vec_a.rend());
    REQUIRE(vec_new == vec_expect);
  }

  SECTION("Append empty vector")
  {
    std::vector<int> vec_new = {6, 7, 8, 9, 10};
    std::vector<int> vec_expect = {6, 7, 8, 9, 10};

    vec_new = cfg::utils::append(vec_new, vec_empty.begin(), vec_empty.end());
    REQUIRE(vec_new == vec_expect);
  }

  SECTION("Append to empty vector")
  {
    std::vector<int> vec_new = {};

    vec_new = cfg::utils::append(vec_new, vec_a.begin(), vec_a.end());
    REQUIRE(vec_new == vec_a);
  }
}
