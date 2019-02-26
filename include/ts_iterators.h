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

    explicit _iterator(const raw_value_it_t& begin, std::size_t pos,
                       const time_it_t& t_begin,
                       const std::array<std::size_t, NDim>& shape,
                       std::size_t increment)
        : _begin{begin}, _t_begin{t_begin},
          _CurrentValue{begin, *t_begin, shape}, _position{pos}, _increment{
                                                                     increment}
    {}

    explicit _iterator(const raw_value_it_t& begin, std::size_t pos, double& t,
                       const std::array<std::size_t, NDim>& shape,
                       std::size_t increment)
        : _begin{begin}, _CurrentValue{begin, t, shape}, _position{pos},
          _increment{increment}
    {}

    explicit _iterator(const raw_value_it_t& begin, std::size_t pos,
                       const time_it_t& t_begin)
        : _begin{begin}, _t_begin{t_begin},
          _CurrentValue{*t_begin, *begin}, _position{pos}, _increment{1}
    {}

    explicit _iterator(const raw_value_it_t& begin, std::size_t pos, double& t)
        : _begin{begin}, _CurrentValue{t, *begin}, _position{pos}, _increment{1}
    {}

//    _iterator(const _iterator& other)
//        : _begin{other._begin}, _t_begin{other._t_begin},
//          _position{other._position}, _increment{other._increment}
//    {}

    virtual ~_iterator() noexcept = default;

    _iterator& operator=(_iterator other)
    {
      std::swap(_CurrentValue, other._CurrentValue);
      return *this;
    }

    friend void swap(const _iterator& lhs, const _iterator& rhs)
    {
      std::swap(lhs._CurrentValue, rhs._CurrentValue);
    }

    void next(int offset)
    {
      this->_position += offset * _increment;
      if constexpr(iterTime)
      {
        this->_CurrentValue._update(*(_t_begin + (_position / _increment)),
                                    _begin + _position);
      }
      else
      {
        this->_CurrentValue._update(_begin + _position);
      }
    }
    void prev(int offset) { next(-offset); }

    bool equals(const _iterator& other) const
    {
      return _position == other._position;
    }

    bool gt(const _iterator& other) const
    {
      return _position > other._position;
    }

    std::size_t distance(const _iterator& other) const
    {
      return (_position - other._position) / _increment;
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
    raw_value_it_t _begin;
    time_it_t _t_begin;
    itValue_t _CurrentValue;
    std::size_t _position;
    std::size_t _increment = 1;
  };
} // namespace TimeSeries::details::iterators

#endif
