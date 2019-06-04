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
  template<int NDim, typename T> std::size_t _element_size(const T& shape)
  {
    if constexpr(NDim > 0)
    {
      auto size = std::accumulate(std::cbegin(shape), std::cend(shape), 1ul,
                                  std::multiplies<std::size_t>());
      return size;
    }
    else
      return 1;
  }
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

    template<typename Dummy = void,
             typename       = std::enable_if_t<(NDim > 0), Dummy>>
    _iterator(double* time, raw_value_type* data,
              const std::vector<std::size_t>& shape)
        : _indexes{time, data, _element_size<NDim>(shape)}, _CurrentValue{time,
                                                                          data,
                                                                          shape}
    {}

    template<typename Dummy = void,
             typename       = std::enable_if_t<(NDim > 0), Dummy>>
    _iterator(double* time, raw_value_type* data,
              const std::array<std::size_t, NDim>& shape)
        : _indexes{time, data, _element_size<NDim>(shape)}, _CurrentValue{time,
                                                                          data,
                                                                          shape}
    {}

    template<typename Dummy = void,
             typename       = std::enable_if_t<NDim == 0, Dummy>>
    _iterator(double* time, raw_value_type* data)
        : _indexes{time, data, 1}, _CurrentValue{time, data}
    {}

    _iterator() = delete;

    _iterator(const _iterator& other) : _indexes{other._indexes}
    {
      _updateValue();
      if constexpr(NDim > 0) _updateShape(other._CurrentValue.shape());
    }

    _iterator(_iterator&& other) : _indexes{other._indexes}
    {
      _updateValue();
      if constexpr(NDim > 0) _updateShape(other._CurrentValue.shape());
    }

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

    void _updateShape(const std::array<std::size_t, NDim>& shape)
    {
      if constexpr(NDim > 0) _CurrentValue.reshape(shape);
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
