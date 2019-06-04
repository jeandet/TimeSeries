#ifndef TS_ITERATORS_H
#define TS_ITERATORS_H
#include "ts_arithmetic.h"
#include "ts_iterator_indexes.h"

#include <array>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>

namespace TimeSeries::details::iterators
{
  template<typename itValue_t, typename ts_t, int NDim, bool iterTime = true,
           bool isConst = false>
  struct _iterator : public details::arithmetic::_incrementable_object<
                         _iterator<itValue_t, ts_t, NDim, iterTime, isConst>>,
                     public details::arithmetic::_comparable_object<
                         _iterator<itValue_t, ts_t, NDim, iterTime, isConst>>
  {
    using iterator_category = std::random_access_iterator_tag;
    using value_type =
        typename std::conditional<isConst, const itValue_t, itValue_t>::type;
    using difference_type = std::ptrdiff_t;
    using pointer         = void;
    using reference       = value_type&;

    using raw_value_type = typename ts_t::raw_value_type;

    friend ts_t;
    using indexes_t = _iterator_indexes<ts_t, iterTime>;

    std::size_t _element_size(const std::vector<std::size_t>& shape) const
    {
      if constexpr(NDim > 1)
        return std::accumulate(std::cbegin(shape) + 1, std::cend(shape), 1ul,
                               std::multiplies<std::size_t>());
      else
        return 1;
    }

    explicit _iterator(double* time, raw_value_type* data,
                       const std::vector<std::size_t>& shape)
        : _indexes{time, data, _element_size(shape)}
    {
      _updateValue();
    }

    _iterator() = delete;

    _iterator(const _iterator& other) : _indexes{other._indexes}
    {
      _updateValue();
    }

    _iterator(_iterator&& other) : _indexes{other._indexes} { _updateValue(); }

    virtual ~_iterator() noexcept = default;

    _iterator& operator=(const _iterator& other)
    {
      _indexes = other._indexes;
      _updateValue();
      return *this;
    }

    friend void swap(_iterator& lhs, _iterator& rhs)
    {
      std::swap(lhs._indexes, rhs._indexes);
      std::swap(lhs._CurrentValue, rhs._CurrentValue);
    }

    void next(int offset)
    {
      _indexes.next(offset);
      _updateValue();
    }

    void prev(int offset) { next(-offset); }

    bool equals(const _iterator& other) const
    {
      return _indexes.equals(other._indexes);
    }

    bool gt(const _iterator& other) const
    {
      return _indexes.gt(other._indexes);
    }

    std::size_t distance(const _iterator& other) const
    {
      return _indexes.distance(other._indexes);
    }

    void _updateValue()
    {
      _CurrentValue._update_pointers(_indexes.t_ptr(), _indexes.v_ptr());
    }

    const itValue_t* operator->() const { return &_CurrentValue; }
    const itValue_t& operator*() const { return _CurrentValue; }
    itValue_t* operator->() { return &_CurrentValue; }
    itValue_t& operator*() { return _CurrentValue; }
    itValue_t& operator[](int offset) const
    {
      auto copy = *this;
      return *(copy + offset);
    }

  protected:
    indexes_t _indexes;
    value_type _CurrentValue;
  };
} // namespace TimeSeries::details::iterators

#endif
