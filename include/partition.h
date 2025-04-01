/**
 * partition.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <mpi.h>
#include <parhip_interface.h>
#include <topo_parser.h>
#include <utils.h>

namespace cfg::partition
{
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
    NaivePartition(const cfg::utils::Parallel& parallel, const size_t n)
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


}  // namespace cfg::partition