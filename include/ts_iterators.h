#ifndef TS_ITERATORS_H
#define TS_ITERATORS_H
#include <ts_arithmetic.h>
#include <iterator>

namespace TimeSeries::details::iterators {

template <typename itValue_t, class ts_t, bool isConst = false>
struct _iterator
    : public details::arithmetic::_incrementable_object<_iterator<itValue_t, ts_t, isConst>>,
      public details::arithmetic::_comparable_object<_iterator<itValue_t, ts_t, isConst>> {

  using iterator_category = std::random_access_iterator_tag;
  using value_type =
      typename std::conditional<isConst, const itValue_t, itValue_t>::type;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type *;
  using reference = value_type &;

  explicit _iterator(ts_t *ts, std::size_t pos)
      : _ts{ts}, _CurrentValue{ts, pos}, _position{pos} {}

  _iterator(const _iterator &other)
      : _ts{other._ts},
        _CurrentValue{other._ts, other._position}, _position{other._position} {}

  virtual ~_iterator() noexcept = default;

  _iterator &operator=(_iterator other) {
    std::swap(_CurrentValue, other._CurrentValue);
    return *this;
  }

  friend void swap(const _iterator &lhs, const _iterator &rhs) {
    std::swap(lhs._CurrentValue, rhs._CurrentValue);
  }

  void next(int offset) {
    this->_position += offset;
    this->_CurrentValue(_ts->t(_position), _ts->v(_position));
  }
  void prev(int offset) { next(-offset); }

  bool equals(const _iterator &other) const {
    return _position == other._position;
  }

  bool gt(const _iterator &other) const { return _position > other._position; }

  std::size_t distance(const _iterator &other) const {
    return _position - other._position;
  }

  const itValue_t *operator->() const { return &_CurrentValue; }
  const itValue_t &operator*() const { return _CurrentValue; }
  itValue_t *operator->() { return &_CurrentValue; }
  itValue_t &operator*() { return _CurrentValue; }
  itValue_t &operator[](int offset) const {
    return _CurrentValue.advance(offset);
  }

private:
  ts_t *_ts;
  itValue_t _CurrentValue;
  std::size_t _position;
};
}

#endif
