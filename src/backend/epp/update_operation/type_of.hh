/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef TYPE_OF_HH_95CFEA27A7227B375AFB6EDD0E6CB3A5//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define TYPE_OF_HH_95CFEA27A7227B375AFB6EDD0E6CB3A5

namespace Epp {
/**
 * @brief All about update operations
 */
namespace UpdateOperation {

/**
 * @brief Specifies kind of update action
 */
enum class Action
{
    set_value, ///< new value must be set
    delete_value, ///< value must be deleted
    no_operation ///< do not touch value
};

template <Action o>
class TypeOf
{
public:
    static constexpr Action operation = o;
    static TypeOf make() { return TypeOf(); }
    template <typename>
    using On = TypeOf;
private:
    TypeOf() { }
};

template <>
class TypeOf<Action::set_value>
{
public:
    TypeOf() = delete;
    template <typename T>
    class On
    {
    public:
        using Base = T;
        On() = delete;
        On(const On&) = default;
        On& operator=(const On&) = default;
    private:
        explicit On(const Base& value) : value_(value) { }
        Base value_;
        friend class TypeOf;
    };
    template <typename T>
    static On<T> make(const T& value)
    {
        return On<T>(value);
    }
    template <typename T>
    static T get(const On<T>& operation)
    {
        return operation.value_;
    }
};

using SetValue = TypeOf<Action::set_value>;
using DeleteValue = TypeOf<Action::delete_value>;
using NoOperation = TypeOf<Action::no_operation>;

template <typename V>
typename SetValue::template On<V>
set_value(const V& value)
{
    return SetValue::template make(value);
}

inline DeleteValue delete_value() { return DeleteValue::make(); }

inline NoOperation no_operation() { return NoOperation::make(); }

}//namespace Epp::UpdateOperation
}//namespace Epp

#endif//TYPE_OF_HH_95CFEA27A7227B375AFB6EDD0E6CB3A5
