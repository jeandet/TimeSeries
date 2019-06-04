#ifndef TS_ITERATORS_INDEXES_H
#define TS_ITERATORS_INDEXES_H
#include <functional>
#include <iostream>
#include <ts_arithmetic.h>
namespace TimeSeries::details::iterators
{
  template<typename ts_t, bool iterTime = true>
  struct _iterator_indexes : public details::arithmetic::_incrementable_object<
                                 _iterator_indexes<ts_t, iterTime>>,
                             public details::arithmetic::_comparable_object<
                                 _iterator_indexes<ts_t, iterTime>>
  {
    ts_t* ts;
    std::size_t position;
    std::size_t time_position;
    std::size_t increment = 1;

    _iterator_indexes(ts_t* ts, std::size_t position, std::size_t time_position,
                      std::size_t increment = 1)
        : ts{ts}, position{position},
          time_position{time_position}, increment{increment}
    {}

    friend void swap(_iterator_indexes& lhs, _iterator_indexes& rhs)
    {
      std::swap(lhs.ts, rhs.ts);
      std::swap(lhs.position, rhs.position);
      std::swap(lhs.position, rhs.time_position);
      std::swap(lhs.increment, rhs.increment);
    }

    void next(int offset)
    {
      this->position += offset;
      if constexpr(iterTime) { this->time_position += offset; }
    }

    void prev(int offset) { next(-offset); }

    bool equals(const _iterator_indexes& other) const
    {
      return ts == other.ts and position == other.position and
             time_position == other.time_position and
             increment == other.increment;
    }

    bool gt(const _iterator_indexes& other) const
    {
      return position > other.position;
    }

    std::size_t distance(const _iterator_indexes& other) const
    {
      return position- other.position ;
    }

    inline auto v() const { return ts->_data[position]; }
    inline auto t() const { return ts->_axes[0][time_position]; }
    inline auto& v() { return ts->_data[position]; }
    inline auto& t() { return ts->_axes[0][time_position]; }

    inline auto v_ptr() const { return ts->_data.data()+position; }
    inline auto t_ptr() const { return ts->_axes[0].data()+time_position; }
  };
} // namespace TimeSeries::details::iterators
#endif
