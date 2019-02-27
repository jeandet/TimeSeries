#ifndef TS_ITERATORS_H
#define TS_ITERATORS_H
#include <array>
#include <functional>
#include <iterator>
#include <ts_arithmetic.h>

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
    using pointer         = value_type*;
    using reference       = value_type&;
    template<typename T, typename... args>
    using container_t = typename ts_t::template container_type<T, args...>;

    using raw_value_it_t = decltype(
        std::declval<container_t<typename ts_t::value_type>>().begin());

    using time_it_t = decltype(std::declval<container_t<double>>().begin());

    explicit _iterator(const raw_value_it_t& it, const time_it_t& time_it,
                       const std::array<std::size_t, NDim>& shape,
                       std::size_t increment)
        : _raw_values_it{it}, _time_it{time_it}, _CurrentValue{it, *time_it,
                                                               shape},
          /*_position{pos * increment},*/ _increment{increment}
    {}

    explicit _iterator(const raw_value_it_t& it, double& t,
                       const std::array<std::size_t, NDim>& shape,
                       std::size_t increment)
        : _raw_values_it{it}, _CurrentValue{it, t, shape},
          /*_position{pos * increment},*/ _increment{increment}
    {}

    explicit _iterator(const raw_value_it_t& it, const time_it_t& time_it)
        : _raw_values_it{it}, _time_it{time_it}, _CurrentValue{*time_it, *it},
          /*_position{pos},*/ _increment{1}
    {}

    explicit _iterator(const raw_value_it_t& it, double& t)
        : _raw_values_it{it}, _CurrentValue{t, *it},
          /*_position{pos},*/ _increment{1}
    {}

    template<int N             = NDim,
             typename MultiDim = typename std::enable_if_t<NDim >= 2>,
             typename P        = MultiDim>
    _iterator(const _iterator& other)
        : _raw_values_it{other._begin}, _CurrentValue{other, true},
          _time_it{other._t_begin}, /*_position{other._position},*/
          _increment{other._increment}
    {}

    template<int N           = NDim,
             typename oneDim = typename std::enable_if_t<NDim == 1>>
    _iterator(const _iterator& other)
        : _raw_values_it{other._begin}, _CurrentValue{other, true},
          _time_it{other._t_begin} /*, _position{other._position}*/,
          _increment{other._increment}
    {}

    virtual ~_iterator() noexcept = default;

    _iterator& operator=(const _iterator& other)
    {
      _time_it       = other._time_it;
      _raw_values_it = other._raw_values_it;
      _increment     = other._increment;
      next(0);
      return *this;
    }

    friend void swap(_iterator& lhs, _iterator& rhs)
    {
      // std::swap(lhs._CurrentValue, rhs._CurrentValue);
      std::swap(lhs._time_it, rhs._time_it);
      std::swap(lhs._raw_values_it, rhs._raw_values_it);
      std::swap(lhs._increment, rhs._increment);
      lhs.next(0);
      rhs.next(0);
    }

    void next(int offset)
    {
      this->_raw_values_it += offset * _increment;
      if constexpr(iterTime)
      {
        this->_time_it += offset;
        this->_CurrentValue._update(*_time_it, _raw_values_it);
      }
      else
      {
        this->_CurrentValue._update(_raw_values_it);
      }
    }

    void prev(int offset) { next(-offset); }

    bool equals(const _iterator& other) const
    {
      return _raw_values_it == other._raw_values_it;
    }

    bool gt(const _iterator& other) const
    {
      return _raw_values_it > other._raw_values_it;
    }

    std::size_t distance(const _iterator& other) const
    {
      return std::distance(other._raw_values_it, _raw_values_it) / _increment;
    }

    const itValue_t* operator->() const { return &_CurrentValue; }
    const itValue_t& operator*() const { return _CurrentValue; }
    itValue_t* operator->() { return &_CurrentValue; }
    itValue_t& operator*() { return _CurrentValue; }
    itValue_t& operator[](int offset) const
    {
      return _CurrentValue.advance(offset);
    }

  private:
    raw_value_it_t _raw_values_it;
    time_it_t _time_it;
    itValue_t _CurrentValue;
    std::size_t _increment = 1;
  };
} // namespace TimeSeries::details::iterators

#endif
