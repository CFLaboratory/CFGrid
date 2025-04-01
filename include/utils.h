/**
 * utils.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_UTILS_H_
#define __CFG_UTILS_H_

#include <mpi.h>
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
    MPI_Comm comm; ///< MPI parallel communicator
  };

 }  // namespace cfg::utils

#endif  // __CFG_UTILS_H_
