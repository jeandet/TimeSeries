#ifndef TIMESERIES_H
#define TIMESERIES_H
#include "ts_arithmetic.h"
#include "ts_iterator_indexes.h"
#include "ts_iterator_value.h"
#include "ts_iterators.h"
#include "ts_time.h"

#include <iostream>
#include <memory>
#include <vector>

namespace TimeSeries
{
  class ITimeSerie
  {
  public:
    virtual ~ITimeSerie()                                          = default;
    virtual std::size_t size() const                               = 0;
    virtual std::size_t size(int index) const                      = 0;
    virtual std::vector<std::size_t> shape() const                 = 0;
    virtual double t(const std::size_t& position) const            = 0;
    virtual double& t(const std::size_t& position)                 = 0;
    virtual const std::string& unit(unsigned int axis_index) const = 0;
    virtual std::string& unit(unsigned int axis_index)             = 0;
    virtual std::vector<double>& axis(unsigned int axis_index)     = 0;
    virtual const std::vector<double>& axis(unsigned int axis_index) const = 0;
    virtual std::pair<double, double>
    axis_range(unsigned int axis_index) const = 0;
  };

  template<typename T> struct TimeSerieView
  {
    T _begin;
    T _end;
    T begin() { return _begin; }
    T end() { return _end; }
    TimeSerieView(const T& begin, const T& end) : _begin(begin), _end(end) {}
    TimeSerieView(TimeSerieView&&) = default;
  };

  template<typename RawValueType, typename TimeSerieType, int NDim = 1,
           template<typename val_t, typename...> class container_t =
               std::vector>

  class TimeSerie : public ITimeSerie
  {
    template<template<typename val_t, typename...> class T>
    static std::size_t _flattenSize(const T<std::size_t>& sizes) noexcept
    {
      std::size_t size = 1.;
      for(auto s : sizes)
      {
        size *= s;
      }
      return size;
    }

    void _sanity_check()
    {
      assert(NDim == _shape.size());
      assert(NDim == _axes.size());
      for(int i = 0; i < NDim; i++)
      {
        assert(_axes[i].size() == _shape[i]);
      }
    }

  protected:
    container_t<RawValueType> _data;
    std::vector<std::size_t> _shape;
    std::array<std::vector<double>, NDim> _axes;
    std::array<std::string, NDim + 1> _units;

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
    using raw_value_type = RawValueType;
    using axis_t         = std::vector<double>;
    using data_t         = decltype(_data);

    using iterator_value_1d = details::iterators::IteratorValue<raw_value_type>;

    using iterator_value_nd =
        details::iterators::TimeSerieSlice<type, NDim - 1>;

    using value_type = typename std::conditional_t<NDim == 1, iterator_value_1d,
                                                   iterator_value_nd>;

    using iterator_value_1d_by_index =
        details::iterators::IteratorValue<raw_value_type,
                                          details::iterators::compare_time>;
    using iterator_value_nd_by_index =
        details::iterators::TimeSerieSlice<type, NDim - 1,
                                           details::iterators::compare_time>;

    using iterator_t = typename std::conditional_t<
        NDim == 1,
        details::iterators::_iterator<iterator_value_1d, 0,
                                      details::iterators::iter_time>,
        details::iterators::_iterator<iterator_value_nd, NDim - 1,
                                      details::iterators::iter_time>>;

    using iterator_by_index_t = typename std::conditional_t<
        NDim == 1,
        details::iterators::_iterator<iterator_value_1d_by_index, 0,
                                      details::iterators::iter_time>,
        details::iterators::_iterator<iterator_value_nd_by_index, NDim - 1,
                                      details::iterators::iter_time>>;

    TimeSerie() : _shape(NDim, 0) {}

    template<typename Dummy = void,
             typename       = std::enable_if_t<NDim == 1, Dummy>>
    TimeSerie(std::size_t size) : _data(size), _shape(NDim, size)
    {
      _axes[0].resize(size);
      _sanity_check();
    }

    TimeSerie(const std::initializer_list<std::size_t>& sizes)
        : _data(_flattenSize(sizes)), _shape(sizes)
    {
      assert(sizes.size() == NDim);
      for(int i = 0; i < static_cast<int>(sizes.size()); i++)
      {
        _axes[i].resize(*(sizes.begin() + i));
      }
      _sanity_check();
    }

    TimeSerie(const std::vector<std::size_t>& sizes)
        : _data(_flattenSize(sizes)), _shape(sizes)
    {
      for(int i = 0; i < sizes.size(); i++)
      {
        _axes[i].resize(*(sizes.begin() + i));
      }
      _sanity_check();
    }

    template<typename Dummy = void,
             typename       = std::enable_if_t<NDim == 1, Dummy>>
    TimeSerie(container_t<double>&& t, container_t<RawValueType>&& data)
        : _data{data}, _shape{{t.size()}}
    {
      _axes[0] = std::move(t);
      _sanity_check();
    }

    template<typename Dummy = void,
             typename       = std::enable_if_t<NDim == 1, Dummy>>
    TimeSerie(const container_t<double>& t,
              const container_t<RawValueType>& data)
        : _data{data}, _shape{{t.size()}}
    {
      _axes[0] = t;
    }

    template<typename Dummy = void,
             typename       = std::enable_if_t<(NDim > 1), Dummy>>
    TimeSerie(container_t<double>&& t, container_t<RawValueType>&& data,
              const std::initializer_list<std::size_t>& sizes)
        : _data{data}, _shape(sizes)
    {
      _axes[0] = t;
      for(int i = 1; i < NDim; i++)
      {
        _axes[i].resize(_shape[i]);
      }
      _sanity_check();
    }

    template<typename Dummy = void,
             typename       = std::enable_if_t<(NDim > 1), Dummy>>
    TimeSerie(const container_t<double>& t,
              const container_t<RawValueType>& data,
              const std::initializer_list<std::size_t>& sizes)
        : _data{data}, _shape(sizes)
    {
      _axes[0] = t;
      for(int i = 1; i < NDim; i++)
      {
        _axes[i].resize(_shape[i]);
      }
      _sanity_check();
    }

    template<typename Dummy = void,
             typename       = std::enable_if_t<(NDim > 1), Dummy>>
    TimeSerie(const container_t<double>& t,
              const container_t<RawValueType>& data,
              const std::vector<std::size_t>& sizes)
        : _data{data}, _shape(sizes)
    {
      _axes[0] = t;
      for(int i = 1; i < NDim; i++)
      {
        _axes[i].resize(_shape[i]);
      }
      _sanity_check();
    }

    TimeSerie(const iterator_t& begin_it, const iterator_t& end_it)
    {
      auto count = std::distance(begin_it, end_it);
      this->_shape.resize(count);
      if constexpr(NDim > 1)
      {
        auto shape = begin_it->shape();
        std::copy(std::cbegin(shape), std::cend(shape), std::begin(_shape) + 1);
      }
      this->resize(count);
      std::copy(begin_it, end_it, this->begin());
    }

    typename std::conditional_t<NDim == 1, raw_value_type&, iterator_value_nd>
    operator[](std::size_t position)
    {
      if constexpr(NDim == 1)
        return _data[position];
      else
      {
        return iterator_value_nd(_axes[0].data() + position,
                                 _data.data() + (position * _element_size()),
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
      _axes[0].resize(newSize);
      _data.resize(newSize * _element_size());
      _shape[0] = _axes[0].size();
    }

    double t(const std::size_t& position) const override
    {
      return _axes[0][position];
    }
    double& t(const std::size_t& position) override
    {
      return _axes[0][position];
    }
    auto v(const std::size_t& position) { return (begin() + position)->v(); }

    auto by_index() { return TimeSerieView(ibegin(), iend()); }

    auto ibegin() noexcept
    {
      if constexpr(NDim == 1)
        return iterator_by_index_t(_axes[0].data(), _data.data());
      else
        return iterator_by_index_t(_axes[0].data(), _data.data(),
                                   _element_shape());
    }

    auto begin() noexcept
    {
      if constexpr(NDim == 1)
        return iterator_t(_axes[0].data(), _data.data());
      else
        return iterator_t(_axes[0].data(), _data.data(), _element_shape());
    }

    auto end() noexcept
    {
      if constexpr(NDim == 1)
        return iterator_t(_axes[0].data() + size(), _data.data() + size());
      else
        return iterator_t(_axes[0].data() + _axes[0].size(),
                          _data.data() + _data.size(), _element_shape());
    }

    auto iend() noexcept
    {
      if constexpr(NDim == 1)
        return iterator_by_index_t(_axes[0].data() + size(),
                                   _data.data() + size());
      else
        return iterator_by_index_t(_axes[0].data() + _axes[0].size(),
                                   _data.data() + _data.size(),
                                   _element_shape());
    }

    std::string& unit(unsigned int axis_index) override
    {
      assert(axis_index <= NDim);
      return _units[axis_index];
    }

    const std::string& unit(unsigned int axis_index) const override
    {
      assert(axis_index <= NDim);
      return _units[axis_index];
    }

    std::vector<double>& axis(unsigned int axis_index) override
    {
      assert(axis_index < NDim);
      return _axes[axis_index];
    }

    std::pair<double, double> axis_range(unsigned int axis_index) const override
    {
      auto& ax = axis(axis_index);
      if(ax.size()) return {ax.front(), ax.back()};
      return {std::numeric_limits<double>::quiet_NaN(),
              std::numeric_limits<double>::quiet_NaN()};
    }

    const std::vector<double>& axis(unsigned int axis_index) const override
    {
      assert(axis_index < NDim);
      return _axes[axis_index];
    }

    auto front() { return *begin(); }
    auto back() { return *(end() - 1); }

    template<class T> TimeSerie& operator<<(T&& obj)
    {
      this->push_back(std::forward<T>(obj));
      return *this;
    }

    template<class T>
    auto push_back(T&& value)
        -> decltype(T{std::pair{1., RawValueType{}}}, value.v(), void())
    {
      _data.push_back(value.v());
      _axes[0].push_back(value.t());
    }

    template<class T>
    auto push_back(T&& value)
        -> decltype(T{1., RawValueType{}}, value.first, void())
    {
      _data.push_back(value.second);
      _axes[0].push_back(value.first);
      _shape[0] = _axes[0].size();
    }

    template<class T>
    auto push_back(T&& value) -> decltype(value.flat_begin(), void())
    {
      std::copy(value.flat_begin(), value.flat_end(),
                std::back_inserter(_data));
      _axes[0].push_back(value.t());
      _shape[0] = _axes[0].size();
#if __cplusplus > 201703L
      [[unlikely]]
#endif
      if(_axes[0].size() == 1)
      {
        const auto sh = value.shape();
        std::copy(std::cbegin(sh), std::cend(sh), std::begin(_shape) + 1);
      }
    }

    template<class T>
    auto push_back(const T&& iterator)
        -> decltype(iterator.raw_value_ptr_type, void())
    {
      _data.push_back(iterator->v());
      _axes[0].push_back(iterator->t());
      _shape[0] = _axes[0].size();
    }

    template<class T>
    auto push_back(const T&& value) -> decltype(value.v(), void())
    {
      _data.push_back(value.v());
      _axes[0].push_back(value.t());
      _shape[0] = _axes[0].size();
    }

    std::size_t size() const override { return _axes[0].size(); }
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
