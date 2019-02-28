#ifndef TIMESERIES_H
#define TIMESERIES_H
#include <iostream>
#include <memory>
#include <ts_arithmetic.h>
#include <ts_iterator_value.h>
#include <ts_iterators.h>
#include <ts_time.h>
#include <vector>

namespace TimeSeries
{
  class ITimeSerie
  {
  public:
    virtual ~ITimeSerie()                          = default;
    virtual std::size_t size() const               = 0;
    virtual std::size_t size(int index) const      = 0;
    virtual std::vector<std::size_t> shape() const = 0;
  };

  template<typename T> struct TimeSerieView
  {
    T _begin;
    T _end;
    T begin() { return _begin; }
    T end() { return _end; }
    TimeSerieView(T begin, T end) : _begin(begin), _end(end) {}
    TimeSerieView(TimeSerieView&&) = default;
  };

  template<typename RawValueType, typename TimeSerieType, int NDim = 1,
           template<typename val_t, typename...> class container_t =
               std::vector>

  class TimeSerie : public ITimeSerie
  {
    static std::size_t
    _flattenSize(const std::initializer_list<std::size_t>& sizes) noexcept
    {
      std::size_t size = 1.;
      for(auto s : sizes)
      {
        size *= s;
      }
      return size;
    }

  protected:
    container_t<RawValueType> _data;
    container_t<double> _t;
    std::vector<std::size_t> _shape;

    std::size_t _element_size() const
    {
      if constexpr(NDim > 1)
        return std::accumulate(std::cbegin(_shape) + 1, std::cend(_shape), 1.,
                               std::multiplies<std::size_t>());
      else
        return 1;
    }

    std::array<std::size_t, NDim - 1> _element_shape() const
    {
      std::array<std::size_t, NDim - 1> shape;
      std::copy(std::cbegin(_shape) + 1, std::cend(_shape), std::begin(shape));
      return shape;
    }

  public:
    using type = TimeSerie<RawValueType, TimeSerieType, NDim, container_t>;
    using IteratorValue = details::iterators::IteratorValue<RawValueType, type>;
    using raw_value_type = RawValueType;
    using value_type     = typename std::conditional<
        NDim == 1, details::iterators::IteratorValue<RawValueType, type>,
        details::iterators::TimeSerieSlice<RawValueType, type, NDim - 1,
                                           false>>::type;
    using ByIndexIteratorValue =
        details::iterators::IteratorValue<RawValueType, type, false>;

    template<typename val_t, typename... args>
    using container_type = class container_t<val_t, args...>;

    using Iterator_t =
        details::iterators::_iterator<IteratorValue, type, 0, true, false>;
    using ByIndexIterator_t =
        details::iterators::_iterator<ByIndexIteratorValue, type, 0, true,
                                      false>;

    template<int _NDim>
    using IteratorND_t = details::iterators::_iterator<
        details::iterators::TimeSerieSlice<RawValueType, type, _NDim, false>,
        type, _NDim>;

    template<typename _ValueType, typename _ts_type, int _NDim,
             bool compareValue>
    friend struct details::iterators::TimeSerieSlice;

    TimeSerie() : _shape(NDim, 0) {}

    template<typename Dummy = void,
             typename       = std::enable_if_t<NDim == 1, Dummy>>
    TimeSerie(std::size_t size) : _data(size), _t(size)
    {}

    TimeSerie(const std::initializer_list<std::size_t>& sizes)
        : _data(_flattenSize(sizes)), _t(*sizes.begin()), _shape(sizes)
    {
      assert(sizes.size() == NDim);
    }

    template<typename Dummy = void,
             typename       = std::enable_if_t<NDim == 1, Dummy>>
    TimeSerie(container_t<double>&& t, container_t<RawValueType>&& data)
        : _data{data}, _t{t}, _shape{{t.size()}}
    {}

    template<typename Dummy = void,
             typename       = std::enable_if_t<NDim == 1, Dummy>>
    TimeSerie(const container_t<double>& t,
              const container_t<RawValueType>& data)
        : _data{data}, _t{t}, _shape{{t.size()}}
    {}

    template<typename Dummy = void,
             typename       = std::enable_if_t<(NDim > 1), Dummy>>
    TimeSerie(container_t<double>&& t, container_t<RawValueType>&& data,
              const std::initializer_list<std::size_t>& sizes)
        : _data{data}, _t{t}, _shape(sizes)
    {}

    template<typename Dummy = void,
             typename       = std::enable_if_t<(NDim > 1), Dummy>>
    TimeSerie(const container_t<double>& t,
              const container_t<RawValueType>& data,
              const std::initializer_list<std::size_t>& sizes)
        : _data{data}, _t{t}, _shape(sizes)
    {}

    TimeSerie(const Iterator_t& begin, const Iterator_t& end)
    {
      this->resize(std::distance(begin, end));
      std::copy(begin, end, this->begin());
    }

    auto operator[](const std::size_t& position)
    {
      if constexpr(NDim == 1)
        return _data[position];
      else
      {
        return details::iterators::TimeSerieSlice<RawValueType, type, NDim - 1>(
            _t[position], std::begin(_data) + (position * _element_size()),
            _element_shape());
      }
    }

    auto operator[](const std::pair<int, int>& range)
    {
      auto b = begin();
      auto e = begin();
      if(range.first < 0)
        b = end() + range.first;
      else
        b = begin() + range.first;

      if(range.second < 0)
        e = end() + range.second;
      else
        e = begin() + range.second;

      return TimeSerieView(b, e);
    }

    void resize(std::size_t newSize)
    {
      _t.resize(newSize);
      _data.resize(newSize * _element_size());
    }

    double t(const std::size_t& position) const { return _t[position]; }
    double& t(const std::size_t& position) { return _t[position]; }
    auto v(const std::size_t& position) { return (begin() + position)->v(); }

    auto byIndex()
    {
      if constexpr(NDim == 1)
        return TimeSerieView(
            ByIndexIterator_t(std::begin(_t), std::begin(_data)),
            ByIndexIterator_t(std::end(_t), std::end(_data)));
      else
        return TimeSerieView(begin(), end());
    }

    auto begin()
    {
      if constexpr(NDim == 1)
        return Iterator_t(std::begin(_t), std::begin(_data));
      else
        return IteratorND_t<NDim - 1>(std::begin(_t), std::begin(_data),
                                      _element_shape(), _element_size());
    }
    auto end()
    {
      if constexpr(NDim == 1)
        return Iterator_t(std::end(_t), std::end(_data));
      else
        return IteratorND_t<NDim - 1>(std::end(_t), std::end(_data),
                                      _element_shape(), _element_size());
    }

    auto front() { return *begin(); }
    auto back() { return *(end() - 1); }

    template<class T>
    auto push_back(const T& value) -> decltype(T(1., 2.), value.v(), void())
    {
      _data.push_back(value.v());
      _t.push_back(value.t());
    }

    template<class T>
    auto push_back(const T& value) -> decltype(T(1., 2.), value.first, void())
    {
      _data.push_back(value.second);
      _t.push_back(value.first);
    }
    // template<class T> void push_back(T&& value) { push_back(value); }

    template<class T>
    auto push_back(const T& value) -> decltype(value.flat_begin(), void())
    {
      std::copy(value.flat_begin(), value.flat_end(),
                std::back_inserter(_data));
      _t.push_back(value.t());
    }

    template<class T>
    auto push_back(const T& iterator)
        -> decltype(iterator._raw_values_it, void())
    {
      std::copy(iterator._raw_values_it,
                iterator._raw_values_it + iterator._increment,
                std::back_inserter(_data));
      _t.push_back(*(iterator._time_it));
    }

    std::size_t size() const override { return _t.size(); }
    std::size_t size(int index) const override { return _shape[index]; }
    std::vector<std::size_t> shape() const override { return _shape; }
  };

#define _DECLARE_TS(name, DataType)                                            \
  class name : public TimeSerie<DataType, name>                                \
  {                                                                            \
  public:                                                                      \
    ~name() = default;                                                         \
    name() {}                                                                  \
    using TimeSerie::TimeSerie;                                                \
  };

  _DECLARE_TS(ScalarTs, double)

  struct Vector
  {
    double x;
    double y;
    double z;
  };

  _DECLARE_TS(VectorTs, Vector)

} // namespace TimeSeries

#define USER_DECLARE_TS(name, DataType)                                        \
  class name : public TimeSeries::TimeSerie<DataType, name>                    \
  {                                                                            \
  public:                                                                      \
    ~name() = default;                                                         \
    name() {}                                                                  \
    using TimeSerie::TimeSerie;                                                \
  }

#define USER_DECLARE_TS_ND(name, DataType, Dimmensions)                        \
  class name : public TimeSeries::TimeSerie<DataType, name, Dimmensions>       \
  {                                                                            \
  public:                                                                      \
    ~name() = default;                                                         \
    name() {}                                                                  \
    using TimeSerie::TimeSerie;                                                \
  }

namespace std
{
  template<typename T> auto begin(TimeSeries::TimeSerieView<T>&& tsv)
  {
    return tsv.begin();
  }
  template<typename T> auto end(TimeSeries::TimeSerieView<T>&& tsv)
  {
    return tsv.end();
  }
} // namespace std

#endif // TIMESERIES_H
