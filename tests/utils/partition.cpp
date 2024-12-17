/**
 * partition.cpp
 *
 * Tests the partition classes.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>

#include <utils.h>

TEST_CASE("Partition base", "[utils]")
{
  cfg::utils::Partition partition;

  // Reject 0 and 1
  REQUIRE_FALSE(partition.pick(0));
  REQUIRE_FALSE(partition.pick(1));

  // Reject any value
  int idx = 1;
  while (idx < std::numeric_limits<int>::max() / 2)
  {
    REQUIRE_FALSE(partition.pick(idx));      // Even
    REQUIRE_FALSE(partition.pick(idx + 1));  // Odd
    idx *= 2;
  }
}

TEST_CASE("Partition serial", "[utils]")
{
  cfg::utils::SerialPartition partition;

  // Take 0 and 1
  REQUIRE(partition.pick(0));
  REQUIRE(partition.pick(1));

  // Take any value
  int idx = 1;
  while (idx < std::numeric_limits<int>::max() / 2)
  {
    REQUIRE(partition.pick(idx));      // Even
    REQUIRE(partition.pick(idx + 1));  // Odd
    idx *= 2;
  }
}

TEST_CASE("Partition naive", "[utils]")
{
  // Build a "curried" constructor for the Parallel type.
  // Returns a constructor for Parallel objects on different ranks which will all be based on a
  // communicator of the same size.
  const auto make_parallel_constructor = [](const int size)
  {
    return [size](const int rank) -> cfg::utils::Parallel
    {
      cfg::utils::Parallel parallel{};
      parallel.size = size;
      parallel.rank = rank;

      return parallel;
    };
  };

  SECTION("Even partition")
  {
    const auto n         = 1 << 16;
    const int size       = 2;  // Size of the "communicator"
    const auto parallel  = make_parallel_constructor(size);
    const auto parallel0 = parallel(0);  // Parallel context for "rank" 0
    const auto parallel1 = parallel(1);  // Parallel context for "rank" 1

    cfg::utils::NaivePartition partition0(parallel0, n);
    cfg::utils::NaivePartition partition1(parallel1, n);

    // Check split
    int idx = 1;
    while (idx < n)
    {
      // Even indices
      if (idx < n / 2)
      {
        REQUIRE(partition0.pick(idx));
        REQUIRE_FALSE(partition1.pick(idx));
      }
      else
      {
        REQUIRE_FALSE(partition0.pick(idx));
        REQUIRE(partition1.pick(idx));
      }

      // Odd indices
      if ((idx + 1) < n / 2)
      {
        REQUIRE(partition0.pick(idx + 1));
        REQUIRE_FALSE(partition1.pick(idx + 1));
      }
      else if ((idx + 1 < n))
      {
        REQUIRE_FALSE(partition0.pick(idx + 1));
        REQUIRE(partition1.pick(idx + 1));
      }
      else
      {
        REQUIRE_FALSE(partition0.pick(idx + 1));
        REQUIRE_FALSE(partition1.pick(idx + 1));
      }

      idx *= 2;
    }
  }

  SECTION("Odd partition")
  {
    const auto n         = (1 << 16) + 1;
    const int size       = 2;  // Size of the "communicator"
    const auto parallel  = make_parallel_constructor(size);
    const auto parallel0 = parallel(0);  // Parallel context for "rank" 0
    const auto parallel1 = parallel(1);  // Parallel context for "rank" 1

    cfg::utils::NaivePartition partition0(parallel0, n);
    cfg::utils::NaivePartition partition1(parallel1, n);

    // Check split
    int idx = 1;
    while (idx < n)
    {
      // Even indices
      if (idx <= n / 2)
      {
        REQUIRE(partition0.pick(idx));
        REQUIRE_FALSE(partition1.pick(idx));
      }
      else
      {
        REQUIRE_FALSE(partition0.pick(idx));
        REQUIRE(partition1.pick(idx));
      }

      // Odd indices
      if ((idx + 1) <= n / 2)
      {
        REQUIRE(partition0.pick(idx + 1));
        REQUIRE_FALSE(partition1.pick(idx + 1));
      }
      else if ((idx + 1) < n)
      {
        REQUIRE_FALSE(partition0.pick(idx + 1));
        REQUIRE(partition1.pick(idx + 1));
      }
      else
      {
        REQUIRE_FALSE(partition0.pick(idx + 1));
        REQUIRE_FALSE(partition1.pick(idx + 1));
      }

      idx *= 2;
    }
  }

  SECTION("Three-way partition")
  {
    const auto n         = (1 << 16);
    const int size       = 3;  // Size of the "communicator"
    const auto parallel  = make_parallel_constructor(size);
    const auto parallel0 = parallel(0);  // Parallel context for "rank" 0
    const auto parallel1 = parallel(1);  // Parallel context for "rank" 1
    const auto parallel2 = parallel(2);  // Parallel context for "rank" 2

    cfg::utils::NaivePartition partition0(parallel0, n);
    cfg::utils::NaivePartition partition1(parallel1, n);
    cfg::utils::NaivePartition partition2(parallel2, n);

    // Check split
    int idx = 1;
    while (idx < n)
    {
      // Even indices
      if (idx <= n / 3)
      {
        REQUIRE(partition0.pick(idx));
        REQUIRE_FALSE(partition1.pick(idx));
        REQUIRE_FALSE(partition2.pick(idx));
      }
      else if (idx < 2 * (n / 3))
      {
        REQUIRE_FALSE(partition0.pick(idx));
        REQUIRE(partition1.pick(idx));
        REQUIRE_FALSE(partition2.pick(idx));
      }
      else if (idx < n)
      {
        REQUIRE_FALSE(partition0.pick(idx));
        REQUIRE_FALSE(partition1.pick(idx));
        REQUIRE(partition2.pick(idx));
      }
      else
      {
        REQUIRE_FALSE(partition0.pick(idx));
        REQUIRE_FALSE(partition1.pick(idx));
        REQUIRE_FALSE(partition2.pick(idx));
      }

      // Odd indices
      if (idx + 1 <= n / 3)
      {
        REQUIRE(partition0.pick(idx + 1));
        REQUIRE_FALSE(partition1.pick(idx + 1));
        REQUIRE_FALSE(partition2.pick(idx + 1));
      }
      else if (idx + 1 < 2 * (n / 3))
      {
        REQUIRE_FALSE(partition0.pick(idx + 1));
        REQUIRE(partition1.pick(idx + 1));
        REQUIRE_FALSE(partition2.pick(idx + 1));
      }
      else if (idx + 1 < n)
      {
        REQUIRE_FALSE(partition0.pick(idx + 1));
        REQUIRE_FALSE(partition1.pick(idx + 1));
        REQUIRE(partition2.pick(idx + 1));
      }
      else
      {
        REQUIRE_FALSE(partition0.pick(idx + 1));
        REQUIRE_FALSE(partition1.pick(idx + 1));
        REQUIRE_FALSE(partition2.pick(idx + 1));
      }

      idx *= 2;
    }
  }
}
