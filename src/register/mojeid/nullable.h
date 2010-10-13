#ifndef NULLABLE_H_
#define NULLABLE_H_

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

    operator T() const
    {
        return value_;
    }

    bool isnull() const
    {
        return isnull_;
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
};


#endif /*NULLABLE_H_*/

