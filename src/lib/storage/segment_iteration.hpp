#pragma once

#include "pos_list.hpp"
#include "resolve_type.hpp"
#include "storage/base_segment.hpp"
#include "storage/segment_iterables/any_segment_iterable.hpp"
#include "storage/segment_iterables.hpp"
#include "storage/create_iterable_from_segment.hpp"

/**
 * This file provides the main entry points to read Segment data, irrespective of the underlying encoding.
 *
 * Two main signatures are provided:
 *      segment_with_iterators()    Calls the functor with a begin and end iterator
 *      segment_for_each()          Calls the functor with each value in the segment
 *
 * Both functions optionally take a PosList which allows for selective access to the values in a segment.
 *
 * The template parameter T is either (if known to the caller) the DataType of the values contained in the segment, or
 * ResolveDataTypeTag, if the type is unknown to the caller.
 *
 * The template parameter SegmentIterationTypeErasure specifies if type erasure should be used, which reduces compile
 * time at the cost of run time.
 *
 *
 * ## NOTES REGARDING COMPILE TIME AND BINARY SIZE
 *
 * Calling any of the functions in this file will result in the functor being instantiated many times.
 *   With type erasure:     The functor is instantiated for each DataType
 *   Without type erasure:  The functor is instantiated for each DataType, IterableType and IteratorType combination.
 *
 * Especially when nesting segment iteration, this will lead to a lot of instantiations of the functor, so try to keep
 * them small and use type erase when performance is not crucial.
 */

namespace opossum {

struct ResolveDataTypeTag {};

enum class SegmentIterationTypeErasure { OnlyInDebug, Always };

template <typename T = ResolveDataTypeTag, SegmentIterationTypeErasure type_erasure = SegmentIterationTypeErasure::OnlyInDebug, typename Functor>
void segment_with_iterators(const BaseSegment& base_segment, const Functor& functor) {
  if constexpr (std::is_same_v<T, ResolveDataTypeTag>) {
    resolve_data_type(base_segment.data_type(), [&](const auto data_type_t) {
      using ColumnDataType = typename decltype(data_type_t)::type;
      segment_with_iterators<ColumnDataType, type_erasure>(base_segment, functor);
    });
  } else {
    if constexpr (IS_DEBUG || type_erasure == SegmentIterationTypeErasure::Always) {
      const auto any_segment_iterable = create_any_segment_iterable<T>(base_segment);
      any_segment_iterable.with_iterators(functor);
    } else {
      resolve_segment_type<T>(base_segment, [&](const auto& segment) {
        const auto segment_iterable = create_iterable_from_segment<T>(segment);
        segment_iterable.with_iterators(functor);
      });
    }
  }
}

template <typename T = ResolveDataTypeTag, SegmentIterationTypeErasure type_erasure = SegmentIterationTypeErasure::OnlyInDebug, typename Functor>
void segment_with_iterators(const BaseSegment& base_segment, const std::shared_ptr<const PosList>& position_filter,
                            const Functor& functor) {
  if (!position_filter) {
    segment_with_iterators<T, type_erasure>(base_segment, functor);
    return;
  }

  if constexpr (std::is_same_v<T, ResolveDataTypeTag>) {
    resolve_data_type(base_segment.data_type(), [&](const auto data_type_t) {
      using ColumnDataType = typename decltype(data_type_t)::type;
      segment_with_iterators<ColumnDataType, type_erasure>(base_segment, position_filter, functor);
    });
  } else {
    if constexpr (IS_DEBUG || type_erasure == SegmentIterationTypeErasure::Always) {
      const auto any_segment_iterable = create_any_segment_iterable<T>(base_segment);
      any_segment_iterable.with_iterators(position_filter, functor);
    } else {
      resolve_segment_type<T>(base_segment, [&](const auto& segment) {
        const auto segment_iterable = create_iterable_from_segment<T>(segment);
        if constexpr (is_point_accessible_segment_iterable_v<decltype(segment_iterable)>) {
          segment_iterable.with_iterators(position_filter, functor);
        } else {
          Fail("Cannot access non-PointAccessibleSegmentIterable with position_filter");
        }
      });
    }
  }
}

template <typename T = ResolveDataTypeTag, SegmentIterationTypeErasure type_erasure = SegmentIterationTypeErasure::OnlyInDebug, typename Functor>
void segment_for_each(const BaseSegment& base_segment, const std::shared_ptr<const PosList>& position_filter,
                      const Functor& functor) {
  segment_with_iterators<T, type_erasure>(base_segment, position_filter, [&](auto it, const auto end) {
    while (it != end) {
      functor(*it);
      ++it;
    }
  });
}

template <typename T = ResolveDataTypeTag, SegmentIterationTypeErasure type_erasure = SegmentIterationTypeErasure::OnlyInDebug, typename Functor>
void segment_for_each(const BaseSegment& base_segment, const Functor& functor) {
  segment_with_iterators<T, type_erasure>(base_segment, [&](auto it, const auto end) {
    while (it != end) {
      functor(*it);
      ++it;
    }
  });
}

}  // namespace opossum
