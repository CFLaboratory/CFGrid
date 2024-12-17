/**
 * test_stride.cpp
 *
 * Tests the test_stride utility.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>

#include <utils.h>

TEST_CASE("test_stride", "[utils]")
{
  const std::vector<int> v{1, 2, 3, 4, 5};   // Unit-strided data
  const std::vector<int> v2{1, 3, 5, 7, 9};  // Constant-strided data
  const std::vector<int> c{1, 1, 1, 1, 1};   // Constant (zero-strided) data
  const std::vector<int> f{1, 1, 2, 3, 5};   // Variable-stided data (Fibonacci)

  const auto tst = [](const int& a, const int& b) -> size_t
  {
    return b - a;
  };

  SECTION("Constant strides are accepted")
  {
    REQUIRE(cfg::utils::test_stride<int>(v.begin(), v.end(), tst));       // Default stride is 1
    REQUIRE(cfg::utils::test_stride<int>(v.begin(), v.end(), tst, 1));    // Set stride explicitly
    REQUIRE(cfg::utils::test_stride<int>(v2.begin(), v2.end(), tst, 2));  // Test non-unit stride
    REQUIRE(cfg::utils::test_stride<int>(c.begin(), c.end(), tst, 0));    // Constant set has stride 0
  }

  SECTION("Wrong strides are rejected")
  {
    REQUIRE_FALSE(cfg::utils::test_stride<int>(v.begin(), v.end(), tst, 2));    // Set stride explicitly
    REQUIRE_FALSE(cfg::utils::test_stride<int>(v2.begin(), v2.end(), tst, 1));  // Test non-unit stride
  }

  SECTION("Variable strides are rejected")
  {
    REQUIRE_FALSE(cfg::utils::test_stride<int>(f.begin(), f.end(), tst, 0));
    REQUIRE_FALSE(cfg::utils::test_stride<int>(f.begin(), f.end(), tst, 1));
    REQUIRE_FALSE(cfg::utils::test_stride<int>(f.begin(), f.end(), tst, 2));
  }
}
