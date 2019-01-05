#ifndef TIMESERIES_H
#define TIMESERIES_H
#include <iostream>
#include <memory>
#include <ts_arithmetic.h>
#include <ts_iterator_value.h>
#include <ts_iterators.h>
#include <ts_time.h>
#include <vector>

namespace TimeSeries {
class ITimeSerie {
public:
  virtual std::size_t size() = 0;
};

template <typename T> struct TimeSerieView {
  T _begin;
  T _end;
  inline T begin() { return _begin; }
  inline T end() { return _end; }
  TimeSerieView(T begin, T end) : _begin(begin), _end(end) {}
};

template <typename ValueType, typename TimeSerieType, int NDim = 1,
          template <typename val_t, typename...> class container_t =
              std::vector>

class TimeSerie : public ITimeSerie {
  static std::size_t
  _flattenSize(const std::initializer_list<std::size_t> &sizes) noexcept {
    std::size_t size = 1.;
    for (auto s : sizes) {
      size *= s;
    }
    return size;
  }

protected:
  container_t<ValueType> _data;
  container_t<double> _t;
  std::vector<std::size_t> _shape;

public:
  using type = TimeSerie<ValueType, TimeSerieType, NDim, container_t>;
  using IteratorValue = details::iterators::IteratorValue<ValueType, type>;
  template <typename val_t, typename... args>
  using container_type = class container_t<val_t, args...>;

  using value_type = ValueType;

  using IteratorT = details::iterators::_iterator<
      IteratorValue, TimeSerie<ValueType, TimeSerieType, NDim, container_t>>;
  using IteratorNDT = details::iterators::_iterator<
      details::iterators::TimeSerieSlice<ValueType, type, NDim - 1>, type>;

  TimeSerie() = default;
  TimeSerie(std::size_t size) : _data(size), _t(size) {}

  TimeSerie(const std::initializer_list<std::size_t> &sizes)
      : _data(_flattenSize(sizes)), _t(_flattenSize(sizes)), _shape{sizes} {}

  TimeSerie(container_t<double> &&t, container_t<ValueType> &&data)
      : _data{data}, _t{t} {}

  TimeSerie(const IteratorT &begin, const IteratorT &end) {
    this->resize(std::distance(begin, end));
    std::copy(begin, end, this->begin());
  }

  auto operator[](const std::size_t &position) {
    if constexpr (NDim == 1)
      return _data[position];
    else {
      return details::iterators::TimeSerieSlice<ValueType, type, NDim - 1>();
    }
  }

  auto operator[](const std::pair<int, int> &range) {
    auto b = begin();
    auto e = begin();
    if (range.first < 0)
      b = end() + range.first;
    else
      b = begin() + range.first;

    if (range.second < 0)
      e = end() + range.second;
    else
      e = begin() + range.second;

    return TimeSerieView(b, e);
  }

  void resize(std::size_t newSize) {
    _t.resize(newSize);
    _data.resize(newSize);
  }

  double t(const std::size_t &position) const { return _t[position]; }
  double &t(const std::size_t &position) { return _t[position]; }
  ValueType v(const std::size_t &position) const { return _data[position]; }
  ValueType &v(const std::size_t &position) { return _data[position]; }

  auto begin() {
    if constexpr (NDim == 1)
      return IteratorT(this, 0);
    else
      return IteratorNDT(this, 0);
  }
  auto end() {
    if constexpr (NDim == 1)
      return IteratorT(this, size());
    else
      return IteratorNDT(this, size());
  }

  std::size_t size() override { return _t.size(); }
};

#define _DECLARE_TS(name, DataType)                                            \
  class name : public TimeSerie<DataType, name> {                              \
  public:                                                                      \
    ~name() = default;                                                         \
    name() {}                                                                  \
    using TimeSerie::TimeSerie;                                                \
  };

_DECLARE_TS(ScalarTs, double)

struct Vector {
  double x;
  double y;
  double z;
};

_DECLARE_TS(VectorTs, Vector)

} // namespace TimeSeries

#define USER_DECLARE_TS(name, DataType)                                        \
  class name : public TimeSeries::TimeSerie<DataType, name> {                  \
  public:                                                                      \
    ~name() = default;                                                         \
    name() {}                                                                  \
    using TimeSerie::TimeSerie;                                                \
  };

#define USER_DECLARE_TS_ND(name, DataType, Dimmensions)                        \
  class name : public TimeSeries::TimeSerie<DataType, name, Dimmensions> {     \
  public:                                                                      \
    ~name() = default;                                                         \
    name() {}                                                                  \
    using TimeSerie::TimeSerie;                                                \
  };

#endif // TIMESERIES_H
