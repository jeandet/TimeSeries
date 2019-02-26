#ifndef TS_ITERATOR_VALUE_H
#define TS_ITERATOR_VALUE_H
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <ts_iterators.h>
#include <ts_time.h>
#include <vector>

namespace TimeSeries::details::iterators
{
  template<typename ValueType, typename ts_type, bool compareValue = true>
  struct IteratorValue : public details::arithmetic::_comparable_object<
                             IteratorValue<ValueType, ts_type, compareValue>>,
                         public details::arithmetic::_addable_object<
                             IteratorValue<ValueType, ts_type, compareValue>>
  {
    IteratorValue()                   = delete;
    virtual ~IteratorValue() noexcept = default;

    explicit IteratorValue(double& t, ValueType& v)
        : _t{std::ref(t)}, _v{std::ref(v)}
    {}

    explicit IteratorValue(ts_type* ts, std::size_t position)
        : _t(ts->t(position)), _v(ts->v(position))
    {}

    IteratorValue(IteratorValue&& other)
        : _t_{other.t()}, _v_{other.v()}, _t{std::ref(_t_)}, _v{std::ref(_v_)}
    {}

    IteratorValue(const IteratorValue& other)
        : _t_{other.t()}, _v_{other.v()}, _t{std::ref(_t_)}, _v{std::ref(_v_)}
    {}

    void _update(ts_type* ts, std::size_t position)
    {
      _t = ts->t(position);
      _v = ts->v(position);
    }

    IteratorValue& operator=(const IteratorValue& other)
    {
      _v.get() = other.v();
      _t.get() = other.t();
      return *this;
    }

    IteratorValue& operator=(const std::pair<double, ValueType>& value)
    {
      _v.get() = value.second;
      _t.get() = value.first;
      return *this;
    }

    IteratorValue& operator=(IteratorValue&& other)
    {
      _v.get() = other.v();
      _t.get() = other.t();
      return *this;
    }

    IteratorValue& operator=(ValueType value)
    {
      _v.get() = value;
      return *this;
    }

    IteratorValue& operator=(Second value)
    {
      _t.get() = static_cast<double>(value);
      return *this;
    }

    bool equals(const IteratorValue& other) const
    {
      return (v() == other.v()) && (t() == other.t());
    }

    bool gt(const IteratorValue& other) const
    {
      if constexpr(compareValue)
        return v() > other.v();
      else
        return t() > other.t();
    }

    IteratorValue iadd(IteratorValue& other)
    {
      _t.get() += other.v();
      return *this;
    }

    ValueType v() const { return _v.get(); }
    ValueType& v() { return _v.get(); }
    double t() const { return _t; }
    double& t() { return _t; }

    template<typename T> constexpr auto wrap(T pos, T size)
    {
      return (size + pos) % size;
    }

    friend ValueType operator-(const IteratorValue& lhs,
                               const IteratorValue& rhs)
    {
      return lhs.v() - rhs.v();
    }

  private:
    double _t_;
    ValueType _v_;
    std::reference_wrapper<double> _t;
    std::reference_wrapper<ValueType> _v;
  };

  template<typename ValueType, typename ts_type, int NDim = 1,
           bool compareValue = false>
  struct TimeSerieSlice
      : public details::arithmetic::_comparable_object<
            TimeSerieSlice<ValueType, ts_type, NDim, compareValue>>
  {
  private:
    using Iterator_t = details::iterators::_iterator<
        details::iterators::IteratorValue<ValueType, ts_type>, ts_type, 1,
        false>;

    template<int _NDim>
    using IteratorND_t = details::iterators::_iterator<
        details::iterators::TimeSerieSlice<ValueType, ts_type, _NDim, false>,
        ts_type, _NDim, false>;

    template<typename T, typename... args>
    using container_t = typename ts_type::template container_type<T, args...>;
    using container_it_t =
        decltype(std::declval<container_t<ValueType>>().begin());

    using sub_slice_t = TimeSerieSlice<ValueType, ts_type, NDim - 1>;

    using IteratorValue = details::iterators::IteratorValue<ValueType, ts_type>;

    // ts_type* _tsd_        = nullptr;
    // std::size_t _position = 0;
    double _t_;
    container_t<ValueType>* _v = nullptr;
    std::reference_wrapper<double> _t;
    const int _NDim = NDim;
    std::array<std::size_t, NDim> _shape;
    bool _isDetached = false;
    container_it_t _begin;
    // container_it_t _end;

    std::size_t _flatSize() const
    {
      return std::accumulate(_shape.begin(), _shape.end(), 1,
                             std::multiplies<std::size_t>());
    }

    template<int dim, typename T> auto _container_begin(T& t)
    {
      if constexpr(dim == 1)
        return t.begin();
      else
        return _container_begin<dim - 1>(*t.begin());
    }

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
    TimeSerieSlice() = delete;
    TimeSerieSlice(const container_it_t& begin,
                   const std::reference_wrapper<double> t,
                   const std::array<std::size_t, NDim>& shape)
        : _begin{begin}, _t{t}, _shape{shape}
    {}

    TimeSerieSlice(const TimeSerieSlice& other)
        : _t_{other.t()}, _t{std::ref(_t_)}, _shape{other._shape}
    {
      _v     = new container_t<ValueType>(other._flatSize());
      _begin = std::begin(*_v);
      std::copy(other._begin, other._begin + _v->size(), _begin);
    }

    TimeSerieSlice(TimeSerieSlice&& other) : _t{other.t()}, _shape{other._shape}
    {
      if(other._v) { std::swap(other._v, _v); }
      else
      {
        _v = new container_t<ValueType>(other._flatSize());
        std::copy(other._begin, other._begin + _v->size(), std::begin(*_v));
      }
      _begin   = std::begin(*_v);
      _t.get() = other.t();
    }

    //    TimeSerieSlice(ts_type* ts, std::size_t position)
    //        : _tsd_{ts}, _position{position}, _t(ts->t(position))
    //    {
    //      _v = &ts->_data;
    //      std::copy(std::end(ts->_shape) - NDim, std::end(ts->_shape),
    //                std::begin(_shape));
    //    }

    //    TimeSerieSlice(container_t<ValueType>* data, double t, std::size_t
    //    position,
    //                   const std::array<std::size_t, NDim>& shape)
    //        : _tsd_{nullptr}, _position{position},
    //          _t(t), _shape{shape}, _isDetached{true}
    //    {
    //      _v = new container_t<ValueType>(_flatSize());
    //      std::copy(std::begin(*data), std::begin(*data) + _v->size(),
    //                std::begin(*_v));
    //    }

    //    TimeSerieSlice(const TimeSerieSlice& other)
    //        : _tsd_{other._tsd_}, _position{other._position}, _t_{other.t()},
    //          _t{std::ref(_t_)}, _shape{other._shape}, _isDetached{true}
    //    {
    //      _v     = new container_t<ValueType>(other._flatSize());
    //      auto s = _v->size();
    //      std::copy(std::begin(*other._v), std::begin(*other._v) + _v->size(),
    //                std::begin(*_v));
    //    }

    //    TimeSerieSlice(TimeSerieSlice&& other)
    //        : _tsd_{nullptr}, _position{0}, _t_{other.t()}, _t{std::ref(_t_)},
    //          _shape{other._shape}, _isDetached{true}
    //    {
    //      _v = new container_t<ValueType>(other._flatSize());
    //      if(other._isDetached) { std::swap(this->_v, other._v); }
    //      else
    //      {
    //        std::copy(std::begin(*other._v), std::begin(*other._v) +
    //        _v->size(),
    //                  std::begin(*_v));
    //      }
    //    }

    ~TimeSerieSlice()
    {
      if(_isDetached && _v) delete _v;
    }

    TimeSerieSlice& operator=(const TimeSerieSlice& other)
    {
      assert(_NDim == other._NDim);
      assert(_flatSize() == other._flatSize());
      std::copy(other._begin, other._begin + other._flatSize(), this->_begin);
      _t.get() = other.t();
      return *this;
    }

    TimeSerieSlice& operator=(TimeSerieSlice&& other)
    {
      assert(_NDim == other._NDim);
      assert(_flatSize() == other._flatSize());
      if(other._v)
      {
        _v       = other._v;
        other._v = nullptr;
      }
      else
      {
        _v = new container_t<ValueType>(other._flatSize());
        std::copy(other._begin, other._begin + _v->size(), std::begin(*_v));
      }
      _begin   = std::begin(*_v);
      _t.get() = other.t();
      return *this;
    }

    void _update(const container_it_t& begin)
    {
      [[unlikely]] if(_isDetached)
      {
        delete _v;
        _isDetached = false;
      }
      _begin = begin;
      //      _v        = &ts->_data;
      //      _tsd_     = ts;
      //      _position = position;
    }

    TimeSerieSlice& operator=(const container_t<ValueType>& value)
    {
      std::copy(value.cbegin(), value.cend(), _container_begin<NDim>(*this));
      return *this;
    }

    bool equals(const TimeSerieSlice& other) const
    {
      return std::equal(_begin, _begin + _flatSize(), other._begin);
    }

    bool gt(const TimeSerieSlice& other) const
    {
      static_assert(!compareValue, "don't know how to compare multi-dim value");
      return t() > other.t();
    }

    double t() const { return _t; }
    double& t() { return _t; }

    auto begin() const
    {
      if constexpr(NDim <= 1) { return Iterator_t(_begin, 0, _t); }
      else
      {
        return IteratorND_t<NDim - 1>(_begin, 0, _t, _element_shape(),
                                      _element_size());
      }
    }

    auto end() const
    {
      if constexpr(NDim <= 1) { return Iterator_t(_begin, _flatSize(), _t); }
      else
      {
        return IteratorND_t<NDim - 1>(_begin, _flatSize(), _t, _element_shape(),
                                      _element_size());
      }
    }

    auto operator[](const std::size_t& position)
    {
      if constexpr(NDim == 1)
        return *(_begin + (position * _element_size()));
      else
      {
        return sub_slice_t(_begin + position * _element_size(), _t,
                           _element_shape());
      }
    }

    auto shape() { return _shape; }
  };
} // namespace TimeSeries::details::iterators

#endif
