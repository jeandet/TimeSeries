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

template <class Iterator_t>
struct Iterator_inc_op
{
    Iterator_t &operator++()
    {
        static_cast<Iterator_t*>(this)->next(1);
        return *static_cast<Iterator_t*>(this);
    }

    Iterator_t operator++(int)const
    {
        Iterator_t result(*static_cast<const Iterator_t*>(this));
        this->operator++();
        return result;
    }

    Iterator_t &operator--()
    {
        static_cast<Iterator_t*>(this)->prev(1);
        return *static_cast<Iterator_t*>(this);
    }

    Iterator_t operator--(int)const
    {
        Iterator_t result(*static_cast<const Iterator_t*>(this));
        this->operator--();
        return result;
    }

    Iterator_t &operator+=(int offset)
    {
        static_cast<Iterator_t*>(this)->next(offset);
        return *static_cast<Iterator_t*>(this);
    }
    Iterator_t &operator-=(int offset)
    {
        return *this += -offset;
    }

    Iterator_t operator+(int offset) const
    {
        Iterator_t result(*static_cast<const Iterator_t*>(this));
        result += offset;
        return result;
    }

    Iterator_t operator-(int offset) const
    {
        Iterator_t result(*static_cast<const Iterator_t*>(this));
        result -= offset;
        return *static_cast<Iterator_t*>(&result);
    }

    int operator-(const Iterator_t &other) const
    {
        return static_cast<const Iterator_t*>(this)->distance(other);
    }
};

template <class Iterator_t>
struct Iterator_comp_op
{
    bool operator==(const Iterator_t &other) const
    {
        return static_cast<const Iterator_t*>(this)->equals(other);
    }
    bool operator!=(const Iterator_t &other) const
    {
        return !static_cast<const Iterator_t*>(this)->equals(other);
    }
    bool operator<(const Iterator_t &other) const
    {
        return !static_cast<const Iterator_t*>(this)->gt(other);
    }
    bool operator>(const Iterator_t &other) const
    {
        return !(this < other);
    }
    bool operator>=(const Iterator_t &other) const
    {
        return !(this < other);
    }
    bool operator<=(const Iterator_t &other) const
    {
        return !(this > other);
    }

};

template <typename itValue_t, class ts_t, bool isConst=false>
struct _iterator :
        public Iterator_inc_op < _iterator<itValue_t, ts_t, isConst> >,
        public Iterator_comp_op< _iterator<itValue_t, ts_t, isConst> >
{
    friend struct Iterator_inc_op<_iterator<itValue_t, ts_t, isConst>>;
    friend struct Iterator_comp_op<_iterator<itValue_t, ts_t, isConst>>;

    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename std::conditional<isConst, const itValue_t, itValue_t>::type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    explicit _iterator(ts_t* ts, std::size_t pos)
        : _ts{ts}, _CurrentValue{ts->t(pos),ts->v(pos)}, _position{pos}
    {}

    _iterator(const _iterator& other)
        : _ts{other._ts}, _CurrentValue{other._ts->t(other._position),other._ts->v(other._position)}, _position{other._position}
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

template <typename ValueType, typename TimeSerieType>
class TimeSerie : public ITimeSerie
{

protected:
    std::vector<ValueType> _data;
    std::vector<double> _t;
public:

    struct IteratorValue
    {
        using ts_type = TimeSerie<ValueType, TimeSerieType>;
        IteratorValue()=delete ;
        virtual ~IteratorValue() noexcept = default;

        IteratorValue(double& t, ValueType& v)
            :_t_{-100.},_v_{-100.},_t{std::ref(t)}, _v{std::ref(v)}
        {
        }

        IteratorValue(IteratorValue && other)
            :_t_{other.t()}, _v_{other.v()},_t{std::ref(_t_)},_v{std::ref(_v_)}
        {
        }

        IteratorValue(const IteratorValue & other)
            :_t_{other.t()}, _v_{other.v()},_t{std::ref(_t_)},_v{std::ref(_v_)}
        {

        }

        void operator()(double& t, double& v)
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

        friend bool operator< (const IteratorValue& lhs, const IteratorValue& rhs)
        { return lhs.v() < rhs.v(); }
        friend bool operator> (const IteratorValue& lhs, const IteratorValue& rhs){ return rhs < lhs; }
        friend bool operator<=(const IteratorValue& lhs, const IteratorValue& rhs){ return !(lhs > rhs); }
        friend bool operator>=(const IteratorValue& lhs, const IteratorValue& rhs){ return !(lhs < rhs); }

        friend bool operator==(const IteratorValue& lhs, const IteratorValue& rhs){ return lhs.v() == rhs.v(); }

        auto v()const{return _v;}
        auto& v(){return _v;}
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
    typedef _iterator<IteratorValue, TimeSerie<ValueType, TimeSerieType>> IteratorT;

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

    IteratorT begin(){return IteratorT(this,0);};
    IteratorT end(){return IteratorT(this,size());};

    std::size_t size()override {return _t.size();}
};


#define DECLARE_TS(name, DataType) \
    class name: public TimeSerie<DataType, name>\
{\
    public:\
    ~name() = default;\
    name(){};\
    using TimeSerie::TimeSerie; \
};


DECLARE_TS(ScalarTs, double)



struct Vector
{
    double x;
    double y;
    double z;
};
DECLARE_TS(VectorTs, Vector)

}

