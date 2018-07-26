#pragma once

#include <memory>

#include "abstract_histogram.hpp"
#include "types.hpp"

namespace opossum {

class Table;

template <typename T>
class EqualWidthHistogram : public AbstractHistogram<T> {
 public:
  using AbstractHistogram<T>::AbstractHistogram;

  HistogramType histogram_type() const override;
  uint64_t total_count_distinct() const override;
  uint64_t total_count() const override;
  size_t num_buckets() const override;

 protected:
  void _generate(const ColumnID column_id, const size_t max_num_buckets) override;

  BucketID _bucket_for_value(const T value) const override;
  BucketID _lower_bound_for_value(const T value) const override;
  BucketID _upper_bound_for_value(const T value) const override;

  T _bucket_min(const BucketID index) const override;
  T _bucket_max(const BucketID index) const override;
  uint64_t _bucket_count(const BucketID index) const override;
  uint64_t _bucket_count_distinct(const BucketID index) const override;

  // Overriding because it would otherwise recursively call itself.
  T _bucket_width(const BucketID index) const override;
  // TODO(tim): ask experts how this works
  // std::enable_if_t<std::is_integral_v<T>, T> _bucket_width(const BucketID index) const override;

  uint64_t _string_bucket_width(const BucketID index) const;

 private:
  T _min;
  T _max;
  std::vector<uint64_t> _counts;
  std::vector<uint64_t> _distinct_counts;
  uint64_t _num_buckets_with_larger_range;
};

}  // namespace opossum
