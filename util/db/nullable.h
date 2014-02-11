#ifndef NULLABLE_H_
#define NULLABLE_H_

#include <stdexcept>
#include "db/query_param.h"
#include "db/value.h"

template<typename T>
class Nullable
{
public:
    typedef T Type;
private:
    bool isnull_;
    Type value_;
    template < typename Tc >
    friend class Nullable; // private members of Nullable< X > are accessible in Nullable< T >
public:
    // default ctor => value is NULL
    Nullable()
        : isnull_(true), // value is NULL
          value_()       // default value
    {}//default

    // init ctor; conversion Tc -> T must be possible
    template < typename Tc >
    Nullable(const Tc &_value)
        : isnull_(false), // value isn't NULL
          value_(_value)  // conversion Tc -> T exists
    {}//init

    // copy ctor; an Nullable can be constructed from another Nullable of a different but convertible type
    template < typename Tc >
    Nullable(const Nullable< Tc > &_rhs)
    : isnull_(_rhs.isnull_)
    , value_(_rhs.value_)
    {}//copy

    // assignment; an Nullable can be assigned value of a different but convertible type
    template < typename Tc >
    Nullable& operator=(const Tc &_value)
    {
        value_ = _value;
        isnull_ = false;
        return *this;
    }//assignment

    // assignment; an Nullable can be assigned another Nullable of a different but convertible type
    template < typename Tc >
    Nullable& operator=(const Nullable< Tc > &_rhs)
    {
        value_ = _rhs.value_;
        isnull_ = _rhs.isnull_;
        return *this;
    }//assignment

    bool isnull() const
    {
        return isnull_;
    }

    T get_value() const
    {
        if (isnull())
        {
            throw std::logic_error("value is null");
        }

        return value_;
    }

    T get_value_or_default() const
    {
        return value_;
    }

    operator Database::QueryParam()
    {
        if (isnull())
            return ::Database::QueryParam();
        else
            return ::Database::QueryParam(value_);
    }

    Nullable<T>& operator=(const Database::Value &_v)
    {
        if (_v.isnull()) {
            isnull_ = true;
        }
        else {
            isnull_ = false;
            value_ = static_cast<T>(_v);
        }
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const Nullable<T>& v)
    {
        if (v.isnull())
            return os << "[NULL]";
        else
            return os << v.get_value();
    }

    std::string print_quoted() const
    {
        std::stringstream ss;
        if(!isnull()) ss << (*this);
        return isnull() ? std::string("[NULL]") : std::string("'") + ss.str() + "'";
    }
};

// comparison of equality
template < typename T >
bool operator==(const Nullable< T > &_a, const Nullable< T > &_b)
{
    return (!_a.isnull() && !_b.isnull() && (_a.get_value_or_default() == _b.get_value_or_default())) ||
           (_a.isnull() && _b.isnull());
}

// comparison of inequality
template < typename T >
bool operator!=(const Nullable< T > &_a, const Nullable< T > &_b)
{
    return !(_a == _b);
}

// never use Nullable< T* >; isnull() has two meanings
template < typename T > class Nullable< T* >;

#endif /*NULLABLE_H_*/
