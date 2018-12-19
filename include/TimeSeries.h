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

template <typename T, bool isConst=false>
class _iterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename std::conditional<isConst, const T, T>::type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    explicit _iterator(T value) : m_CurrentValue{std::move(value)} {}

    virtual ~_iterator() noexcept = default;
    _iterator(const _iterator &) = default;
    _iterator &operator=(_iterator other)
    {
        swap(m_CurrentValue,other.m_CurrentValue);
        return *this;
    }

    _iterator &operator++()
    {
        m_CurrentValue.next();
        return *this;
    }

    _iterator &operator--()
    {
        m_CurrentValue.prev();
        return *this;
    }

    _iterator operator++(int)const
    {
        auto result = *this;
        this->operator++();
        return result;
    }
    _iterator operator--(int)const
    {
        auto result = *this;
        this->operator--();
        return result;
    }

    _iterator &operator+=(int offset)
    {
        if (offset >= 0) {
            m_CurrentValue.next(offset);
        }
        else {
            m_CurrentValue.prev(-offset);
        }

        return *this;
    }
    _iterator &operator-=(int offset) { return *this += -offset; }

    _iterator operator+(int offset) const
    {
        auto result = *this;
        result += offset;
        return result;
    }

    _iterator operator-(int offset) const
    {
        auto result = *this;
        result -= offset;
        return result;
    }

    int operator-(const _iterator &other) const
    {
        return m_CurrentValue.distance(other.m_CurrentValue);
    }

    const T *operator->() const { return &m_CurrentValue; }
    const T &operator*() const { return m_CurrentValue; }
    T *operator->() { return &m_CurrentValue; }
    T &operator*() { return m_CurrentValue; }
    T &operator[](int offset) const { return m_CurrentValue.advance(offset); }

    bool operator==(const _iterator &other) const
    {
        return m_CurrentValue.equals(other.m_CurrentValue);
    }
    bool operator!=(const _iterator &other) const { return !(*this == other); }
    bool operator>(const _iterator &other) const { return other.m_CurrentValue.lowerThan(*this); }
    bool operator<(const _iterator &other) const
    {
        return m_CurrentValue.lowerThan(other.m_CurrentValue);
    }
    bool operator>=(const _iterator &other) const { return !(*this < other); }
    bool operator<=(const _iterator &other) const { return !(*this > other); }

private:
    T m_CurrentValue;
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
    struct IteratorValue
    {
        IteratorValue(TimeSerieType* ts, std::size_t position = 0)
            :_position{position}, _ts{ts}
        {
            if(ts==nullptr)throw;
        }

        friend void swap(IteratorValue& lhs, IteratorValue& rhs)
        {
            std::swap(lhs._ts, rhs._ts);
            std::swap(lhs._position, rhs._position);
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
            (*this->_ts)[this->_position] = other.v();
            return *this;
        }

        inline auto v()const {return (*_ts)[_position];}
        inline double t()const {return _ts->t(_position);};

        void next(std::size_t offset=1){_position+=offset;}
        void prev(std::size_t offset=1){_position-=offset;}
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

protected:
    std::vector<ValueType> _data;
    std::vector<double> _t;
public:
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

DECLARE_TS(ScalarTs, double)

struct Vector
{
    double x;
    double y;
    double z;
};
DECLARE_TS(VectorTs, Vector)

}

