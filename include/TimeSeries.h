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

template <class Iterator>
struct Iterator_inc_op
{
    Iterator &operator++()
    {
        static_cast<Iterator*>(this)->_CurrentValue.next();
        return *static_cast<Iterator*>(this);
    }

    Iterator operator++(int)const
    {
        Iterator result(*static_cast<const Iterator*>(this));
        this->operator++();
        return result;
    }

    Iterator &operator--()
    {
        static_cast<Iterator*>(this)->_CurrentValue.prev();
        return *static_cast<Iterator*>(this);
    }

    Iterator operator--(int)const
    {
        Iterator result(*static_cast<const Iterator*>(this));
        this->operator--();
        return result;
    }

    Iterator &operator+=(int offset)
    {
        static_cast<Iterator*>(this)->_CurrentValue.next(offset);
        return *static_cast<Iterator*>(this);
    }
    Iterator &operator-=(int offset)
    {
        return *this += -offset;
    }

    Iterator operator+(int offset) const
    {
        Iterator result(*static_cast<const Iterator*>(this));
        result += offset;
        return result;
    }

    Iterator operator-(int offset) const
    {
        Iterator result(*static_cast<const Iterator*>(this));
        result -= offset;
        return *static_cast<Iterator*>(&result);
    }

    int operator-(const Iterator &other) const
    {
        return static_cast<const Iterator*>(this)->_CurrentValue.distance(other._CurrentValue);
    }
};

template <class Iterator>
struct Iterator_comp_op
{
    bool operator==(const Iterator &other) const
    {
        return static_cast<const Iterator*>(this)->_CurrentValue.equals(other._CurrentValue);
    }
    bool operator!=(const Iterator &other) const
    {
        return !static_cast<const Iterator*>(this)->_CurrentValue.equals(other._CurrentValue);
    }
    bool operator<(const Iterator &other) const
    {
        return static_cast<const Iterator*>(this)->_CurrentValue < other._CurrentValue;
    }
    bool operator>(const Iterator &other) const
    {
        return !(this < other);
    }
    bool operator>=(const Iterator &other) const
    {
        return !(this < other);
    }
    bool operator<=(const Iterator &other) const
    {
        return !(this > other);
    }

};

template <typename T, bool isConst=false>
class _iterator : public
        Iterator_inc_op < _iterator<T, isConst> >,
        public Iterator_comp_op< _iterator<T, isConst> >
{
public:
    friend struct Iterator_inc_op<_iterator<T, isConst>>;
    friend struct Iterator_comp_op<_iterator<T, isConst>>;

    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename std::conditional<isConst, const T, T>::type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    explicit _iterator(T value) : _CurrentValue{std::move(value)} {}

    virtual ~_iterator() noexcept = default;
    _iterator(const _iterator &) = default;
    _iterator &operator=(_iterator other)
    {
        swap(_CurrentValue,other._CurrentValue);
        return *this;
    }

    friend void swap(const _iterator &lhs, const _iterator &rhs)
    {
        swap(lhs._CurrentValue, rhs._CurrentValue);
    }

    const T *operator->() const { return &_CurrentValue; }
    const T &operator*() const { return _CurrentValue; }
    T *operator->() { return &_CurrentValue; }
    T &operator*() { return _CurrentValue; }
    T &operator[](int offset) const { return _CurrentValue.advance(offset); }

private:
    T _CurrentValue;
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

template <typename ValueType, typename TimeSerieType>
class TimeSerie : public ITimeSerie
{

protected:
    std::vector<ValueType> _data;
    std::vector<double> _t;
public:

    struct IteratorValue
    {
        IteratorValue(TimeSerieType* ts, std::size_t position = 0)
            :_position{position}, _ts{ts}
        {
            if(ts==nullptr)throw;
        }

        virtual ~IteratorValue() noexcept = default;

        void swap(IteratorValue& other)
        {
            auto v = this->v();
            *this = other.v();
            other = v;
            Second t{this->t()};
            *this = Second{other.t()};
            other = t;
            std::swap(this->_position, other._position);
        }

        IteratorValue& operator=(ValueType value)
        {
            (*_ts)[_position] = value;
            return *this;
        }

        IteratorValue& operator=(Second value)
        {
            _ts->t(_position) = static_cast<double>(value);
            return *this;
        }

        IteratorValue& operator=(const IteratorValue& other)
        {
            *this = other.v();
            *this = Second{other.t()};
            return *this;
        }

        friend bool operator< (const IteratorValue& lhs, const IteratorValue& rhs)
        { return lhs.v() < rhs.v(); }
        friend bool operator> (const IteratorValue& lhs, const IteratorValue& rhs){ return rhs < lhs; }
        friend bool operator<=(const IteratorValue& lhs, const IteratorValue& rhs){ return !(lhs > rhs); }
        friend bool operator>=(const IteratorValue& lhs, const IteratorValue& rhs){ return !(lhs < rhs); }


        auto v()const {return (*_ts)[_position];}
        double t()const {return _ts->t(_position);};

        template<typename T>
        constexpr auto wrap(T pos, T size){return (size+pos)%size;}

        void next(std::size_t offset=1)
        {
            _position+=offset;
            [[unlikely]]
            if(_position<0)
                _position = wrap(_position, _ts->size());
        }
        void prev(std::size_t offset=1)
        {
            _position-=offset;
            [[unlikely]]
            if(_position<0)
                _position = wrap(_position, _ts->size());
        }

        int distance(const IteratorValue& other)const
        {
            return _position - other._position;
        }

        bool equals(const IteratorValue& other)const
        {
            return this->_position == other._position;
        }

        friend ValueType operator-(const IteratorValue& lhs, const IteratorValue& rhs)
        {
            return lhs.v() - rhs.v();
        }
    private:
        std::size_t _position;
        TimeSerieType* _ts;
    };

    typedef ValueType value_type;
    typedef _iterator<IteratorValue> IteratorT;

    TimeSerie()=default;
    TimeSerie(std::size_t size)
        :_data(size),_t(size)
    {}
    TimeSerie(std::vector<double>&& t, std::vector<ValueType>&& data)
        :_data{data},_t{t}
    {}
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

    double t(const std::size_t& position)const {return _t[position];}
    double& t(const std::size_t& position) {return _t[position];}
    ValueType v(const std::size_t& position)const {return _data[position];}
    ValueType& v(const std::size_t& position) {return _data[position];}

    IteratorT begin(){return IteratorT(IteratorValue(static_cast<TimeSerieType*>(this),0));};
    IteratorT end(){return IteratorT(IteratorValue(static_cast<TimeSerieType*>(this),size()));};

    std::size_t size()override {return _t.size();}
};


#define DECLARE_TS(name, DataType) \
    class name: public TimeSerie<DataType, name>\
{\
    public:\
    ~name() = default;\
    name(){};\
    using TimeSerie::TimeSerie; \
};\
void swap(typename name::IteratorValue& lhs, typename name::IteratorValue& rhs)\
{\
    lhs.swap(rhs);\
}\

DECLARE_TS(ScalarTs, double)



struct Vector
{
    double x;
    double y;
    double z;
};
DECLARE_TS(VectorTs, Vector)

}

