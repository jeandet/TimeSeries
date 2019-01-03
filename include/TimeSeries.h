#ifndef TIMESERIES_H
#define TIMESERIES_H
#include <iostream>
#include <memory>
#include <vector>

namespace TimeSeries
{
// Could use opaque library but here we only have one use of
// opaque/strong typedefs
class Second
{
public:
    explicit Second(double val)
    : value_(val) {}

    explicit operator double() const noexcept
    {
        return value_;
    }

private:
    double value_;
};

template <class Object>
struct _addable_object
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

template <class Object>
struct _incrementable_object
{
    Object &operator++()
    {
        static_cast<Object*>(this)->next(1);
        return *static_cast<Object*>(this);
    }

    Object operator++(int)const
    {
        Object result(*static_cast<const Object*>(this));
        this->operator++();
        return result;
    }

    Object &operator--()
    {
        static_cast<Object*>(this)->prev(1);
        return *static_cast<Object*>(this);
    }

    Object operator--(int)const
    {
        Object result(*static_cast<const Object*>(this));
        this->operator--();
        return result;
    }

    Object &operator+=(int offset)
    {
        static_cast<Object*>(this)->next(offset);
        return *static_cast<Object*>(this);
    }
    Object &operator-=(int offset)
    {
        return *this += -offset;
    }

    Object operator+(int offset) const
    {
        Object result(*static_cast<const Object*>(this));
        result += offset;
        return result;
    }

    Object operator-(int offset) const
    {
        Object result(*static_cast<const Object*>(this));
        result -= offset;
        return *static_cast<Object*>(&result);
    }

    int operator-(const Object &other) const
    {
        return static_cast<const Object*>(this)->distance(other);
    }
};

template <class Object>
struct _comparable_object
{
    bool operator==(const Object &other) const
    {
        return static_cast<const Object*>(this)->equals(other);
    }
    bool operator!=(const Object &other) const
    {
        return !(*this==other);
    }
    bool operator<(const Object &other) const
    {
        return !static_cast<const Object*>(this)->gt(other);
    }
    bool operator>(const Object &other) const
    {
        return !(*this < other);
    }
    bool operator>=(const Object &other) const
    {
        return !(*this < other);
    }
    bool operator<=(const Object &other) const
    {
        return !(*this > other);
    }

};

template <typename itValue_t, class ts_t, bool isConst=false>
struct _iterator :
        public _incrementable_object < _iterator<itValue_t, ts_t, isConst> >,
        public _comparable_object< _iterator<itValue_t, ts_t, isConst> >
{

    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename std::conditional<isConst, const itValue_t, itValue_t>::type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    explicit _iterator(ts_t* ts, std::size_t pos)
        : _ts{ts}, _CurrentValue{ts, pos}, _position{pos}
    {}

    _iterator(const _iterator& other)
        : _ts{other._ts}, _CurrentValue{other._ts,other._position}, _position{other._position}
    {}

    virtual ~_iterator() noexcept = default;


    _iterator &operator=(_iterator other)
    {
        std::swap(_CurrentValue,other._CurrentValue);
        return *this;
    }

    friend void swap(const _iterator &lhs, const _iterator &rhs)
    {
        std::swap(lhs._CurrentValue, rhs._CurrentValue);
    }

    void next(int offset)
    {
        this->_position+=offset;
        this->_CurrentValue(_ts->t(_position), _ts->v(_position));
    }
    void prev(int offset)
    {
        next(-offset);
    }

    bool equals(const _iterator& other) const
    {
        return _position == other._position;
    }

    bool gt(const _iterator& other) const
    {
        return _position > other._position;
    }

    std::size_t distance(const _iterator& other) const
    {
        return _position - other._position;
    }

    const itValue_t *operator->() const { return &_CurrentValue; }
    const itValue_t &operator*() const { return _CurrentValue; }
    itValue_t *operator->() { return &_CurrentValue; }
    itValue_t &operator*() { return _CurrentValue; }
    itValue_t &operator[](int offset) const { return _CurrentValue.advance(offset); }

private:
    ts_t* _ts;
    itValue_t _CurrentValue;
    std::size_t _position;
};

class ITimeSerie
{
public:
    virtual std::size_t size()=0;
};

template <typename T>
struct TimeSerieView
{
    T _begin;
    T _end;
    inline T begin(){return _begin;}
    inline T end(){return _end;}
    TimeSerieView(T begin, T end)
        :_begin(begin), _end(end)
    {}
};

template <typename ValueType, typename TimeSerieType, int NDim=1, template<typename val_t, typename...> class container_t = std::vector>
class TimeSerie : public ITimeSerie
{

protected:
    container_t<ValueType> _data;
    container_t<double> _t;
public:

    struct IteratorValue:
            public _comparable_object<IteratorValue>,
            public _addable_object<IteratorValue>
    {
        using ts_type = TimeSerie<ValueType, TimeSerieType, NDim, container_t>;
        IteratorValue()=delete ;
        virtual ~IteratorValue() noexcept = default;

        explicit IteratorValue(double& t, ValueType& v)
            :_t{std::ref(t)}, _v{std::ref(v)}
        {}

        explicit IteratorValue(TimeSerie* ts, std::size_t position)
            :_t(ts->t(position)), _v(ts->v(position))
        {}

        IteratorValue(IteratorValue && other)
            :_t_{other.t()}, _v_{other.v()},_t{std::ref(_t_)},_v{std::ref(_v_)}
        {}

        IteratorValue(const IteratorValue & other)
            :_t_{other.t()}, _v_{other.v()},_t{std::ref(_t_)},_v{std::ref(_v_)}
        {}

        void operator()(double& t, ValueType& v)
        {
            _t = std::ref(t);
            _v = std::ref(v);
        }

        IteratorValue& operator=(const IteratorValue& other)
        {
            _v.get() = other.v();
            _t.get() = other.t();
            return *this;
        }

        IteratorValue& operator=(const std::pair<double,ValueType>& value)
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
            return v()==other.v();
        }

        bool gt(const IteratorValue& other) const
        {
            return v()>other.v();
        }

        IteratorValue iadd(IteratorValue &other)
        {
            _t.get() += other.v();
            return *this;
        }

        ValueType v()const{return _v.get();}
        ValueType& v(){return _v.get();}
        double t()const{return _t;}
        double& t(){return _t;}

        template<typename T>
        constexpr auto wrap(T pos, T size){return (size+pos)%size;}

        friend ValueType operator-(const IteratorValue& lhs, const IteratorValue& rhs)
        {
            return lhs.v() - rhs.v();
        }

    private:
        double _t_;
        ValueType _v_;
        std::reference_wrapper<double> _t;
        std::reference_wrapper<ValueType> _v;
    };

    typedef ValueType value_type;
    typedef _iterator<IteratorValue, TimeSerie<ValueType, TimeSerieType, NDim, container_t>> IteratorT;

    TimeSerie()=default;
    TimeSerie(std::size_t size)
        :_data(size),_t(size)
    {}
    TimeSerie(container_t<double>&& t, container_t<ValueType>&& data)
        :_data{data},_t{t}
    {}
    TimeSerie(const IteratorT& begin , const IteratorT& end)
    {
        this->resize(std::distance(begin,end));
        std::copy(begin, end, this->begin());
    }

    ValueType& operator[](const std::size_t& position){return _data[position];}
    TimeSerieView<IteratorT> operator[](const std::pair<int,int>& range)
    {
        auto b = begin();
        auto e = begin();
        if(range.first<0)
            b = end()+range.first;
        else
            b = begin()+range.first;

        if(range.second<0)
            e = end()+range.second;
        else
            e = begin()+range.second;

        return TimeSerieView(b, e);
    }

    void resize(std::size_t newSize)
    {
        _t.resize(newSize);
        _data.resize(newSize);
    }

    double t(const std::size_t& position)const {return _t[position];}
    double& t(const std::size_t& position) {return _t[position];}
    ValueType v(const std::size_t& position)const {return _data[position];}
    ValueType& v(const std::size_t& position) {return _data[position];}

    auto begin()
    {
        if constexpr (NDim==1)
            return IteratorT(this,0);
        else
            return -1;

    }
    auto end()
    {
        if constexpr (NDim==1)
                return IteratorT(this,size());
        else
            return -1;

    }

    std::size_t size()override {return _t.size();}
};

#define _DECLARE_TS(name, DataType) \
    class name: public TimeSerie<DataType, name>\
{\
    public:\
    ~name() = default;\
    name(){}\
    using TimeSerie::TimeSerie; \
};


_DECLARE_TS(ScalarTs, double)



struct Vector
{
    double x;
    double y;
    double z;
};

_DECLARE_TS(VectorTs, Vector)

}

#define USER_DECLARE_TS(name, DataType) \
    class name: public TimeSeries::TimeSerie<DataType, name>\
{\
    public:\
    ~name() = default;\
    name(){}\
    using TimeSerie::TimeSerie; \
};

#endif //TIMESERIES_H

