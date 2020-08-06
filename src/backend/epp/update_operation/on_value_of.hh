/*
 * Copyright (C) 2018-2020  CZ.NIC, z. s. p. o.
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
#ifndef ON_VALUE_OF_HH_9C9BBA87CB6CE44C8808AB927F645F5B//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define ON_VALUE_OF_HH_9C9BBA87CB6CE44C8808AB927F645F5B

#include "src/backend/epp/update_operation/type_of.hh"

#include <boost/variant.hpp>

#include <iostream>
#include <stdexcept>
#include <utility>

namespace Epp {
namespace UpdateOperation {

struct OperationNotSpecified
{
    OperationNotSpecified() = delete;
    explicit OperationNotSpecified(int) { }
    static const OperationNotSpecified& static_instance()
    {
        static const OperationNotSpecified singleton(0);
        return singleton;
    }
};

namespace {

const OperationNotSpecified& operation_not_specified = OperationNotSpecified::static_instance();

}//namespace Epp::UpdateOperation::{anonymous}

template <typename T>
struct OnValueOf
{
    using Type = T;
    OnValueOf() = delete;
    template <Action s, Action ...o>
    class AnyOf
    {
    public:
        using Type = T;
        AnyOf() = default;
        AnyOf(const AnyOf&) = default;
        AnyOf(AnyOf&&) = default;
        AnyOf(const OperationNotSpecified&) : operation_(NotSpecified()) { }
        template <typename U>
        AnyOf(U&& src) : operation_(std::forward<U>(src)) { }
        AnyOf& operator=(const AnyOf&) = default;
        AnyOf& operator=(AnyOf&&) = default;
        template <typename U>
        AnyOf& operator=(U&& src) { operation_ = std::forward<U>(src); return *this; }
        AnyOf& operator=(const OperationNotSpecified&) { operation_ = NotSpecified(); return *this; }
        bool operator==(Action operation)const
        {
            switch (operation)
            {
                case Action::set_value:
                    return this->is<Action::set_value>();
                case Action::delete_value:
                    return this->is<Action::delete_value>();
                case Action::no_operation:
                    return this->is<Action::no_operation>();
            }
            throw std::runtime_error("unexpected operation");
        }
        bool operator!=(Action operation)const
        {
            return !this->operator==(operation);
        }
        bool operator==(const OperationNotSpecified&)const
        {
            return !this->visit(IsSpecified());
        }
        Type operator*()const
        {
            return this->visit(GetValue());
        }
    private:
        struct NotSpecified { };
        using OneOfSupported = boost::variant<NotSpecified,
                                              typename TypeOf<s>::template On<Type>,
                                              TypeOf<o>...>;
        OneOfSupported operation_;
        template <typename I>
        struct Is : boost::static_visitor<bool>
        {
            bool operator()(const I&)const
            {
                return true;
            }
            template <typename X>
            bool operator()(const X&)const
            {
                return false;
            }
            bool operator()(const NotSpecified&)const
            {
                struct OperationNotSpecifiedException : std::runtime_error
                {
                    OperationNotSpecifiedException() : std::runtime_error("operation not specified") { }
                };
                throw OperationNotSpecifiedException();
            }
        };
        struct IsSpecified : boost::static_visitor<bool>
        {
            template <typename X>
            bool operator()(const X&)const
            {
                return true;
            }
            bool operator()(const NotSpecified&)const
            {
                return false;
            }
        };
        struct GetValue : boost::static_visitor<Type>
        {
            Type operator()(const TypeOf<Action::set_value>::On<Type>& operation)const
            {
                return TypeOf<Action::set_value>::get(operation);
            }
            template <Action op, typename>
            struct NotSetValueException;
            template <typename X>
            struct NotSetValueException<Action::delete_value, X> : TypeOf<Action::delete_value>, std::runtime_error
            {
                NotSetValueException()
                    : TypeOf<Action::delete_value>(TypeOf<Action::delete_value>::make()),
                      std::runtime_error("set_value operation expected instead of delete_value")
                { }
            };
            template <typename X>
            struct NotSetValueException<Action::no_operation, X> : TypeOf<Action::no_operation>, std::runtime_error
            {
                NotSetValueException()
                    : TypeOf<Action::no_operation>(TypeOf<Action::no_operation>::make()),
                      std::runtime_error("set_value operation expected instead of no_operation")
                { }
            };
            template <Action op>
            Type operator()(const TypeOf<op>&)const
            {
                throw NotSetValueException<op, Type>();
            }
            Type operator()(const NotSpecified&)const
            {
                struct OperationNotSpecifiedException : std::runtime_error
                {
                    OperationNotSpecifiedException() : std::runtime_error("operation not specified") { }
                };
                throw OperationNotSpecifiedException();
            }
        };
        template <Action op>
        bool is()const
        {
            return this->visit(Is<typename TypeOf<op>::template On<Type>>());
        }
        template <typename V>
        typename V::result_type visit(const V& visitor)const
        {
            return boost::apply_visitor(visitor, operation_);
        }
        friend std::ostream& operator<<(std::ostream& out, const AnyOf& operation)
        {
            if (operation == operation_not_specified)
            {
                return out << "not specified";
            }
            if (operation == Action::no_operation)
            {
                return out << "no operation";
            }
            if (operation == Action::set_value)
            {
                return out << "set value: " << (*operation);
            }
            if (operation == Action::delete_value)
            {
                return out << "delete value";
            }
            throw std::runtime_error("unexpected operation");
        }
    };
};

}//namespace Epp::UpdateOperation
}//namespace Epp

#endif//ON_VALUE_OF_HH_9C9BBA87CB6CE44C8808AB927F645F5B
