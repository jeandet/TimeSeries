#ifndef TS_TIME_H
#define TS_TIME_H

namespace TimeSeries {
// Could use opaque library but here we only have one use of
// opaque/strong typedefs
class Second {
public:
  explicit Second(double val) : value_(val) {}

  explicit operator double() const noexcept { return value_; }

private:
  double value_;
};

}

#endif
