#ifndef NULLABLE_H_
#define NULLABLE_H_

#include "src/fredlib/opexception.h"
#include "db/query_param.h"
#include "db/value.h"

template<typename T>
class Nullable
{
private:
    bool isnull_;
    T value_;

public:
    Nullable()
        : isnull_(true),
          value_()
    {
    }
    Nullable(const T &_value)
        : isnull_(false),
          value_(_value)
    {
    }

    Nullable<T>& operator=(const T&_value)
    {
        isnull_ = false;
        value_ = _value;
        return *this;
    }

    bool isnull() const
    {
        return isnull_;
    }

    T get_value() const
    {
        if (isnull())
        {
            BOOST_THROW_EXCEPTION(Fred::InternalError("value is null"));
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
        ss << (*this);
        return isnull() ? std::string("[NULL]") : std::string("'") + ss.str() + "'";
    }
};


#endif /*NULLABLE_H_*/

