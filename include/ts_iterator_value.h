#ifndef TS_ITERATOR_VALUE_H
#define TS_ITERATOR_VALUE_H
#include <iostream>
#include <memory>
#include <vector>
#include <ts_iterators.h>
#include <ts_time.h>

namespace TimeSeries::details::iterators
{
template <typename ValueType, typename ts_type>
struct IteratorValue : public details::arithmetic::_comparable_object<IteratorValue<ValueType, ts_type>>,
                       public details::arithmetic::_addable_object<IteratorValue<ValueType, ts_type>> {
  //using ts_type = TimeSerie<ValueType, TimeSerieType, NDim, container_t>;
  IteratorValue() = delete;
  virtual ~IteratorValue() noexcept = default;

  explicit IteratorValue(double &t, ValueType &v)
      : _t{std::ref(t)}, _v{std::ref(v)} {}

  explicit IteratorValue(ts_type *ts, std::size_t position)
      : _t(ts->t(position)), _v(ts->v(position)) {}

  IteratorValue(IteratorValue &&other)
      : _t_{other.t()}, _v_{other.v()}, _t{std::ref(_t_)}, _v{std::ref(_v_)} {
  }

  IteratorValue(const IteratorValue &other)
      : _t_{other.t()}, _v_{other.v()}, _t{std::ref(_t_)}, _v{std::ref(_v_)} {
  }

  void operator()(double &t, ValueType &v) {
    _t = std::ref(t);
    _v = std::ref(v);
  }

  IteratorValue &operator=(const IteratorValue &other) {
    _v.get() = other.v();
    _t.get() = other.t();
    return *this;
  }

  IteratorValue &operator=(const std::pair<double, ValueType> &value) {
    _v.get() = value.second;
    _t.get() = value.first;
    return *this;
  }

  IteratorValue &operator=(IteratorValue &&other) {
    _v.get() = other.v();
    _t.get() = other.t();
    return *this;
  }

  IteratorValue &operator=(ValueType value) {
    _v.get() = value;
    return *this;
  }

  IteratorValue &operator=(Second value) {
    _t.get() = static_cast<double>(value);
    return *this;
  }

  bool equals(const IteratorValue &other) const { return v() == other.v(); }

  bool gt(const IteratorValue &other) const { return v() > other.v(); }

  IteratorValue iadd(IteratorValue &other) {
    _t.get() += other.v();
    return *this;
  }

  ValueType v() const { return _v.get(); }
  ValueType &v() { return _v.get(); }
  double t() const { return _t; }
  double &t() { return _t; }

  template <typename T> constexpr auto wrap(T pos, T size) {
    return (size + pos) % size;
  }

  friend ValueType operator-(const IteratorValue &lhs,
                             const IteratorValue &rhs) {
    return lhs.v() - rhs.v();
  }

private:
  double _t_;
  ValueType _v_;
  std::reference_wrapper<double> _t;
  std::reference_wrapper<ValueType> _v;
};

template <typename ValueType, typename ts_type, int NDim = 1>
struct TimeSerieSlice {
private:
  using IteratorT = typename ts_type::IteratorT;
  template <typename T, typename... args>
  using container_t = typename ts_type::template container_type<T, args...>;

  ts_type *_tsd_;
  std::size_t _position;
  double _t_;
  std::reference_wrapper<double> _t;

public:
  TimeSerieSlice(ts_type *ts, std::size_t position)
      : _tsd_{ts}, _position{position}, _t(ts->t(position)) {}

  //    TimeSerieSlice(TimeSerieSlice &&other)
  //        : _tsd_{std::make_unique<TimeSerieType>(other.begin(),
  //        other.end())},
  //          _begin{_tsd_->begin()}, _end{_tsd_->end()}, _t_{other._t_},
  //          _t{std::ref(_t_)} {}

  TimeSerieSlice &operator=(const TimeSerieSlice &other) {
    std::copy(other.begin(), other.end(), this->begin());
    return *this;
  }

  TimeSerieSlice &operator=(container_t<ValueType> value) {
    std::copy(value.begin(), value.end(), this->begin());
    return *this;
  }

  auto begin() const {
    if constexpr (NDim <= 1)
      return details::iterators::_iterator<IteratorValue<ValueType,ts_type>, ts_type>(_tsd_, _position);
    else
      return details::iterators::_iterator<
          TimeSerieSlice<ValueType,ts_type,NDim - 1>,ts_type>(_tsd_,_position);
  }
  auto end() const {
    if constexpr (NDim <= 1)
      return details::iterators::_iterator<IteratorValue<ValueType,ts_type>, ts_type>(_tsd_, _position);
    else
      return details::iterators::_iterator<
          TimeSerieSlice<ValueType,ts_type,NDim - 1>,ts_type>(_tsd_,_position);
  }
};
}

#endif
