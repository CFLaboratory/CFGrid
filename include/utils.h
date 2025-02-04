/**
 * utils.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_UTILS_H_
#define __CFG_UTILS_H_

#include <algorithm>
#include <vector>

namespace cfg::utils
{
  /**
   * Appends elements onto a vector from another collection.
   *
   * @param dst       The vector that will be appended to.
   * @param src_first Start iterator for the range that elements will be taken from.
   * @param src_last  End iterator for the range that will be taken from.
   * @returns The vector with new elements appended to it.
   */
  template <class T, class I>
  [[nodiscard]] std::vector<T>& append(std::vector<T>& dst, I src_first, I src_last) noexcept
  {
    dst.insert(dst.end(), src_first, src_last);
    return dst;
  }

  /**
   * Tests the stride of consecutive entries matches a specified constant.
   *
   * @param first  The iterator to the start of the range.
   * @param last   The iterator to the end of the range.
   * @param test   A function to compute the stride between two elements.
   * @param stride The expected stride value to test for (default=1).
   * @returns `true` if the `stride` matches the expectation for all elements, `false` otherwise.
   */
  template <class T, class I, class F>
  [[nodiscard]] bool test_stride(I first, I last, F test, const size_t stride = 1) noexcept
  {
    return std::adjacent_find(first,
                              last,
                              // Simple lambda for comparing two elements, short names are clear.
                              // NOLINTNEXTLINE(readability-identifier-length)
                              [stride, test](const T& a, const T& b) -> bool
                              {
                                // adjacent_find looks for equal entries, we want to find "unequal"
                                // entries, i.e. where the stride does not match the expectation.
                                return test(a, b) != stride;
                              }) == last;
  }

  /**
   * A structure describing the parallel environment.
   */
  struct Parallel
  {
   public:
    unsigned int rank;  ///< ID of this processing element (PE) in the parallel environment.
    unsigned int size;  ///< Size of the parallel environment (how many PEs?).
  };

  /**
   * Base partition class - not actually intended for use as it will reject all elements.
   */
  class Partition
  {
   public:
    /**
     * Determines whether an element is in the partition.
     * The base partition class rejects all elements so this method always returns `false`.
     *
     * @param idx The index of the element to test.
     * @returns Whether the element is in the partition or not.
     */
    // NOLINTNEXTLINE(misc-unused-parameters)
    [[nodiscard]] virtual bool pick(const size_t idx) const
    {
      return false;
    }
  };

  /**
   * Serial partition class - for use in the serial case, accepts all elements as part of the
   * partition.
   */
  class SerialPartition : public Partition
  {
   public:
    /**
     * Determines whether an element is in the partition.
     * The serial partition class accepts all elements so this method always returns `true`.
     *
     * @param idx The index of the element to test.
     * @returns Whether the element is in the partition or not.
     */
    // NOLINTNEXTLINE(misc-unused-parameters)
    [[nodiscard]] bool pick(const size_t idx) const override
    {
      return true;
    }
  };

  /**
   * Naive partition class - partitions based on an equal split across ranks, with remainder
   * distributed evenly across the lower ranks.
   */
  class NaivePartition : public Partition
  {
   public:
    /**
     * Constructor for the naive partition class, determines the range of elements that belong to
     * this partition based on the size of the set and the degree of parallelism.
     *
     * @param parallel The description of parallelism.
     * @param n        The size of the set being partitioned.
     */
    NaivePartition(const Parallel& parallel, const size_t n)
        : local_count(n / parallel.size), local_start(parallel.rank * local_count)
    {
      const auto delta = n - parallel.size * local_count;
      if (delta != 0)
      {
        if (parallel.rank < delta)
        {
          local_count += 1;
          local_start += parallel.rank;
        }
        else
        {
          local_start += delta;
        }
      }
    }

    /**
     * Determines whether an element is in the partition.
     *
     * @param idx The index of the element to test.
     * @returns Whether the element is in the partition or not.
     */
    [[nodiscard]] bool pick(const size_t idx) const override
    {
      return (idx >= local_start) && (idx < (local_start + local_count));
    }

    /**
     * Returns the size of the partition.
     */
    [[nodiscard]] size_t size() const
    {
      return local_count;
    }

   private:
    size_t local_count;
    size_t local_start;
  };
}  // namespace cfg::utils

#endif  // __CFG_UTILS_H_
