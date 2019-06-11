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
  constexpr auto compare_value = true;
  constexpr auto compare_time  = false;

  template<typename ValueType, bool compareValue = true>
  struct IteratorValue : public details::arithmetic::_comparable_object<
                             IteratorValue<ValueType, compareValue>>,
                         public details::arithmetic::_addable_object<
                             IteratorValue<ValueType, compareValue>>
  {
    using raw_value_type     = ValueType;
    using raw_value_ptr_type = raw_value_type*;
    using time_ptr_t         = double*;

    IteratorValue() noexcept = default;

    virtual ~IteratorValue() noexcept = default;

    IteratorValue(time_ptr_t t, raw_value_ptr_type v)
        : _t_ptr{t}, _v_ptr{v}, _t{0.}
    {}

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

    IteratorValue(const IteratorValue& other, bool built_from_iterator)
        : _t_ptr{other._t_ptr}, _v_ptr{other._v_ptr}
    {
      if(!built_from_iterator) throw;
    }

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
    time_ptr_t _t_ptr         = nullptr;
    raw_value_ptr_type _v_ptr = nullptr;
    double _t                 = 0.;
    ValueType _v;
  };

  template<typename ValueType,
           template<typename val_t, typename...> class container_t =
               std::vector,
           int NDim = 1, bool compareValue = false>
  struct TimeSerieSlice
      : public details::arithmetic::_comparable_object<
            TimeSerieSlice<ValueType, container_t, NDim, compareValue>>
  {
  private:
    using iterator_value_1d =
        details::iterators::IteratorValue<ValueType, true>;
    using sub_slice_t =
        TimeSerieSlice<ValueType, container_t, NDim - 1, compareValue>;
    using iterator_t = typename std::conditional_t<
        NDim == 1,
        details::iterators::_iterator<iterator_value_1d, 0, dont_iter_time>,
        details::iterators::_iterator<sub_slice_t, NDim - 1, dont_iter_time>>;

    using raw_value_ptr_type = ValueType*;
    using time_ptr_t         = double*;

    double _t;
    time_ptr_t _t_ptr          = nullptr;
    std::vector<ValueType>* _v = nullptr;
    const int _NDim            = NDim;
    std::array<std::size_t, NDim> _shape;
    raw_value_ptr_type _begin = nullptr;

    std::size_t _flatSize() const
    {
      return std::accumulate(_shape.begin(), _shape.end(), 1,
                             std::multiplies<std::size_t>());
    }

    template<int dim, typename T> auto _container_begin(T& t)
    {
      return std::begin(_begin);
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
    using raw_value_type = ValueType;
    TimeSerieSlice() : _t_ptr{nullptr}, _t{0.}, _v{new container_t<ValueType>}
    {
      _begin = _v->data();
    }

    TimeSerieSlice(time_ptr_t t, raw_value_ptr_type begin,
                   const std::array<std::size_t, NDim>& shape)
        : _t_ptr{t}, _v{nullptr}, _shape{shape}, _begin{begin}
    {}

    TimeSerieSlice(time_ptr_t t, raw_value_ptr_type begin,
                   const std::vector<std::size_t>& shape)
        : _t_ptr{t}, _v{nullptr}, _begin{begin}
    {
      std::copy(std::cbegin(shape), std::cend(shape), std::begin(_shape));
    }

    TimeSerieSlice(const TimeSerieSlice& other)
        : _t{other.t()}, _shape{other._shape}
    {
      _v     = new container_t<ValueType>(other._flatSize());
      _begin = _v->data();
      std::copy(other._begin, other._begin + _v->size(), std::begin(*_v));
    }

    TimeSerieSlice(const TimeSerieSlice& other, bool built_from_iterator)
        : _t_ptr{other._t_ptr}, _NDim{other._NDim}, _shape{other._shape},
          _begin{other._begin}
    {
      if(!built_from_iterator) throw;
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
      std::copy(other._begin, other._begin + other._flatSize(), this->_begin);
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
        std::copy(other._begin, other._begin + other._flatSize(), this->_begin);
      }
      t() = other.t();
      return *this;
    }

    void reshape(const std::array<std::size_t, NDim>& shape) { _shape = shape; }

    void _update_pointers(time_ptr_t t, raw_value_ptr_type begin)
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
      return std::equal(_begin, _begin + _flatSize(), other._begin);
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

    auto begin()
    {
      if constexpr(NDim == 1)
      {
        if(_v)
          return iterator_t(&_t, _begin);
        else
        {
          return iterator_t(_t_ptr, _begin);
        }
      }
      else
      {
        if(_v)
          return iterator_t(&_t, _begin, _element_shape());
        else
          return iterator_t(_t_ptr, _begin, _element_shape());
      }
    }

    auto end()
    {
      if constexpr(NDim == 1)
      {
        if(_v)
          return iterator_t(&_t, _begin + _flatSize());
        else
        {
          return iterator_t(_t_ptr, _begin + _flatSize());
        }
      }
      else
      {
        if(_v)
          return iterator_t(&_t, _begin + _flatSize(), _element_shape(),
                            _element_size());
        else
        {
          return iterator_t(_t_ptr, _begin + _flatSize(), _element_shape(),
                            _element_size());
        }
      }
    }

    auto flat_begin() { return _begin; }
    auto flat_begin() const { return _begin; }

    auto flat_end() { return _begin + _flatSize(); }
    auto flat_end() const { return _begin + _flatSize(); }

    typename std::conditional_t<NDim == 1, ValueType&, sub_slice_t>
    operator[](const std::size_t& position)
    {
      if constexpr(NDim == 1)
        return *(_begin + (position * _element_size()));
      else
      {
        if(_v)
          return sub_slice_t(&_t, _begin + position * _element_size(),
                             _element_shape());
        else
        {
          return sub_slice_t(_t_ptr, _begin + position * _element_size(),
                             _element_shape());
        }
      }
    }

    typename std::conditional_t<NDim == 1, const ValueType&, const sub_slice_t>
    operator[](const std::size_t& position) const
    {
      if constexpr(NDim == 1)
        return *(_begin + (position * _element_size()));
      else
      {
        if(_v)
          return sub_slice_t(&_t, _begin + position * _element_size(),
                             _element_shape());
        else
        {
          return sub_slice_t(_t_ptr, _begin + position * _element_size(),
                             _element_shape());
        }
      }
    }

    const auto shape() const { return _shape; }
  };
} // namespace TimeSeries::details::iterators

#endif
