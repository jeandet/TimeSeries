#ifndef TS_ITERATOR_VALUE_H
#define TS_ITERATOR_VALUE_H
#include "ts_iterator_indexes.h"
#include "ts_iterators.h"
#include "ts_time.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

namespace TimeSeries::details::iterators
{
  template<typename ValueType, bool compareValue = true>
  struct IteratorValue : public details::arithmetic::_comparable_object<
                             IteratorValue<ValueType, compareValue>>,
                         public details::arithmetic::_addable_object<
                             IteratorValue<ValueType, compareValue>>
  {
    IteratorValue() noexcept = default;

    virtual ~IteratorValue() noexcept = default;

    IteratorValue(double* t, ValueType* v) : _t_ptr{t}, _v_ptr{v}, _t{0.} {}

    IteratorValue(const std::pair<double, ValueType>& value)
        : _t_ptr{nullptr}, _v_ptr{nullptr}, _t{value.first}, _v{value.second}
    {}

    /**
     * @brief IteratorValue rvalue copy ctor, this means that a value is built
     * outside of an iterator and should be detached from any container
     * @param other
     */
    IteratorValue(IteratorValue&& other)
        : _t_ptr{nullptr}, _v_ptr{nullptr}, _t{other.t()}, _v{other.v()}
    {}

    /**
     * @brief IteratorValue copy ctor, this means that a value is built
     * outside of an iterator and should be detached from any container
     * @param other
     */
    IteratorValue(const IteratorValue& other)
        : _t_ptr{nullptr}, _v_ptr{nullptr}, _t{other.t()}, _v{other.v()}
    {}

    void _update_pointers(double* t, ValueType* v)
    {
      _t_ptr = t;
      _v_ptr = v;
    }

    friend bool operator==(const IteratorValue& itVal, const ValueType& value)
    {
      return itVal.v() == value;
    }

    IteratorValue& operator=(const IteratorValue& other)
    {
      t() = other.t();
      v() = other.v();
      return *this;
    }

    IteratorValue& operator=(IteratorValue&& other)
    {
      t() = other.t();
      v() = other.v();
      return *this;
    }

    IteratorValue& operator=(const std::pair<double, ValueType>& value)
    {
      t() = value.first;
      v() = value.second;
      return *this;
    }

    IteratorValue& operator=(ValueType value)
    {
      v() = value;
      return *this;
    }

    IteratorValue& operator=(Second value)
    {
      t() = static_cast<double>(value);
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
      v() += other.v();
      return *this;
    }

    inline ValueType v() const
    {
      if(_v_ptr) { return *_v_ptr; }
      else
      {
        return _v;
      }
    }
    inline ValueType& v()
    {
      if(_v_ptr) { return *_v_ptr; }
      else
      {
        return _v;
      }
    }
    inline double t() const
    {
      if(_t_ptr) { return *_t_ptr; }
      else
      {
        return _t;
      }
    }
    inline double& t()
    {
      if(_t_ptr) { return *_t_ptr; }
      else
      {
        return _t;
      }
    }

    friend ValueType operator-(const IteratorValue& lhs,
                               const IteratorValue& rhs)
    {
      return lhs.v() - rhs.v();
    }

  private:
    double* _t_ptr    = nullptr;
    ValueType* _v_ptr = nullptr;
    double _t         = 0.;
    ValueType _v;
  };

  template<typename ValueType, typename ts_type, int NDim = 1,
           bool compareValue = false>
  struct TimeSerieSlice
      : public details::arithmetic::_comparable_object<
            TimeSerieSlice<ValueType, ts_type, NDim, compareValue>>
  {
  private:
    using Iterator_t = details::iterators::_iterator<
        details::iterators::IteratorValue<ValueType>, ts_type, 0, false>;

    template<int _NDim>
    using IteratorND_t = details::iterators::_iterator<
        details::iterators::TimeSerieSlice<ValueType, ts_type, _NDim, false>,
        ts_type, _NDim, false>;

    template<typename T, typename... args>
    using container_t = typename ts_type::template container_type<T, args...>;
    using container_it_t =
        decltype(std::declval<container_t<ValueType>>().begin());

    using sub_slice_t = TimeSerieSlice<ValueType, ts_type, NDim - 1>;

    using IteratorValue = details::iterators::IteratorValue<ValueType>;

    double _t;
    double* _t_ptr;
    std::vector<ValueType>* _v = nullptr;
    const int _NDim            = NDim;
    std::array<std::size_t, NDim> _shape;
    ValueType* _begin = nullptr;

    std::size_t _flatSize() const
    {
      return std::accumulate(_shape.begin(), _shape.end(), 1,
                             std::multiplies<std::size_t>());
    }

    template<int dim, typename T> auto _container_begin(T& t)
    {
      return std::begin(_begin);
      //      if constexpr(dim == 1)
      //        return t.begin();
      //      else
      //        return _container_begin<dim - 1>(*t.begin());
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
    //        : _t_{0.}, _v{new container_t<ValueType>}, _t{std::ref(_t_)}
    //    {
    //      _begin = std::begin(*_v);
    //    }

    TimeSerieSlice(double* t, ValueType* begin,
                   const std::array<std::size_t, NDim>& shape)
        : _t_ptr{t}, _v{nullptr}, _shape{shape}, _begin{begin}
    {}

    //    TimeSerieSlice(const TimeSerieSlice& other, bool do_not_detach)
    //        : _begin{other._begin}, _v{nullptr}, _t_{other._t_}, _t{other._t},
    //          _shape{other._shape}
    //    {
    //      assert(do_not_detach == true);
    //    }

    TimeSerieSlice(const TimeSerieSlice& other)
        : _t{other.t()}, _shape{other._shape}
    {
      _v     = new container_t<ValueType>(other._flatSize());
      _begin = _v->data();
      std::copy(other._begin, other._begin + _v->size(), std::begin(*_v));
    }

    TimeSerieSlice(TimeSerieSlice&& other) : _t{other.t()}, _shape{other._shape}
    {
      if(other._v) { std::swap(other._v, _v); }
      else
      {
        _v = new container_t<ValueType>(other._flatSize());
        std::copy(other._begin, other._begin + _v->size(), std::begin(*_v));
      }
      _begin = _v->data();
    }

    ~TimeSerieSlice()
    {
      if(_v) delete _v;
    }

    TimeSerieSlice& operator=(const TimeSerieSlice& other)
    {
      assert(_NDim == other._NDim);
      if(_v)
      {
        _v->resize(other._flatSize());
        _shape       = other._shape;
        this->_begin = _v->data();
      }
      else
      {
        assert(_flatSize() == other._flatSize());
      }
      std::copy(std::cbegin(other._begin),
                std::cbegin(other._begin) + other._flatSize(),
                std::begin(this->_begin));
      t() = other.t();
      return *this;
    }

    TimeSerieSlice& operator=(TimeSerieSlice&& other)
    {
      assert(_NDim == other._NDim);
      assert(_flatSize() == other._flatSize());
      if(other._v && _v)
      {
        std::swap(_v, other._v);
        _begin = _v->data();
      }
      else
      {
        std::copy(std::cbegin(other._begin),
                  std::cbegin(other._begin) + other._flatSize(),
                  std::begin(this->_begin));
      }
      t() = other.t();
      return *this;
    }

    void _update(double* t, ValueType* begin)
    {
#if __cplusplus > 201703L
      [[unlikely]]
#endif
      if(_v)
      {
        delete _v;
        _v = nullptr;
      }
      _begin = begin;
      _t_ptr = t;
    }

    TimeSerieSlice& operator=(const container_t<ValueType>& value)
    {
      std::copy(value.cbegin(), value.cend(), _container_begin<NDim>(*this));
      return *this;
    }

    bool equals(const TimeSerieSlice& other) const
    {
      return std::equal(std::cbegin(_begin), std::cbegin(_begin) + _flatSize(),
                        std::cbegin(other._begin));
    }

    bool gt(const TimeSerieSlice& other) const
    {
      static_assert(!compareValue, "don't know how to compare multi-dim value");
      return t() > other.t();
    }

    double t() const
    {
      if(_v)
        return _t;
      else
        return *_t_ptr;
    }
    double& t()
    {
      if(_v)
        return _t;
      else
        return *_t_ptr;
    }

    auto begin() const
    {
      if constexpr(NDim == 1) { return Iterator_t(_t, _begin); }
      else
      {
        return IteratorND_t<NDim - 1>(_t, _begin, _element_shape());
      }
    }

    auto end() const
    {
      if constexpr(NDim == 1) { return Iterator_t(_t, _begin + _flatSize()); }
      else
      {
        return IteratorND_t<NDim - 1>(_t, _begin + _flatSize(),
                                      _element_shape(), _element_size());
      }
    }

    auto flat_begin() { return std::begin(_begin); }
    auto flat_begin() const { return std::begin(_begin); }

    auto flat_end() { return std::begin(_begin) + _flatSize(); }
    auto flat_end() const { return std::begin(_begin) + _flatSize(); }

    typename std::conditional_t<NDim == 1, ValueType&, sub_slice_t>
    operator[](const std::size_t& position)
    {
      if constexpr(NDim == 1)
        return *(_begin + (position * _element_size()));
      else
      {
        return sub_slice_t(_t, _begin + position * _element_size(),
                           _element_shape());
      }
    }

    typename std::conditional_t<NDim == 1, const ValueType&, const sub_slice_t>
    operator[](const std::size_t& position) const
    {
      if constexpr(NDim == 1)
        return *(_begin + (position * _element_size()));
      else
      {
        return sub_slice_t(_t, _begin + position * _element_size(),
                           _element_shape());
      }
    }

    const auto shape() const { return _shape; }
  };
} // namespace TimeSeries::details::iterators

#endif
