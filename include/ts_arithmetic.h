#ifndef TS_ARITHMETIC_H
#define TS_ARITHMETIC_H

namespace TimeSeries::details::arithmetic
{
  template<class Object> struct _addable_object
  {
    Object& operator+=(const Object& other)
    {
      return static_cast<const Object*>(this)->iadd(other);
    }

    friend Object operator+(Object lhs, const Object& rhs)
    {
      lhs += rhs;
      return lhs;
    }
  };

  template<class Object> struct _incrementable_object
  {
    Object& operator++()
    {
      static_cast<Object*>(this)->next(1);
      return *static_cast<Object*>(this);
    }

    Object operator++(int)
    {
      Object result(*static_cast<const Object*>(this));
      this->operator++();
      return result;
    }

    Object& operator--()
    {
      static_cast<Object*>(this)->prev(1);
      return *static_cast<Object*>(this);
    }

    Object operator--(int)
    {
      Object result(*static_cast<const Object*>(this));
      this->operator--();
      return result;
    }

    Object& operator+=(int offset)
    {
      static_cast<Object*>(this)->next(offset);
      return *static_cast<Object*>(this);
    }
    Object& operator-=(int offset) { return *this += -offset; }

    Object operator+(int offset)
    {
      Object result(*static_cast<const Object*>(this));
      result += offset;
      return result;
    }

    friend Object operator+(int offset, Object& obj) { return obj + offset; }

    Object operator-(int offset)
    {
      Object result(*static_cast<const Object*>(this));
      result -= offset;
      return *static_cast<Object*>(&result);
    }

    int operator-(const Object& other) const
    {
      return static_cast<const Object*>(this)->distance(other);
    }
  };

  template<class Object> struct _comparable_object
  {
    bool operator==(const Object& other) const
    {
      return static_cast<const Object*>(this)->equals(other);
    }
    bool operator!=(const Object& other) const { return !(*this == other); }
    bool operator<(const Object& other) const
    {
      return !static_cast<const Object*>(this)->gt(other);
    }
    bool operator>(const Object& other) const { return !(*this < other); }
    bool operator>=(const Object& other) const { return !(*this < other); }
    bool operator<=(const Object& other) const { return !(*this > other); }
  };
} // namespace TimeSeries::details::arithmetic

#endif
