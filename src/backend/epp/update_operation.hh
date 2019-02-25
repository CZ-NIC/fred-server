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
#ifndef UPDATE_OPERATION_HH_6406A4633F19C685B37A838D3CD3B5B1//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define UPDATE_OPERATION_HH_6406A4633F19C685B37A838D3CD3B5B1

/**
 * How to use `Deletable` and `Updateable` templates
 * @code{.cpp}
try
{
    Epp::Deletable<std::string> name;
    name = Epp::UpdateOperation::set_value(std::string("Alice"));
    //     Epp::UpdateOperation::delete_value();
    //     Epp::UpdateOperation::no_operation();
    if (name == Epp::UpdateOperation::Action::set_value)
    {
        set_new_name(*name);
    }
    else if (name == Epp::UpdateOperation::Action::delete_value)
    {
        delete_name();
    }
    else if (name == Epp::UpdateOperation::Action::no_operation)
    {
        leave_name_unchanged();
    }

    Epp::Updateable<bool> flag;
    flag = Epp::UpdateOperation::set_value(true);
    //     Epp::UpdateOperation::no_operation();
    if (flag == Epp::UpdateOperation::Action::set_value)
    {
        set_new_flag(*flag);
    }
    else if (flag == Epp::UpdateOperation::Action::no_operation)
    {
        leave_flag_unchanged();
    }
}
catch (const std::exception& e)
{
    std::cerr << "exception caught: " << e.what() << std::endl;
}
 * @endcode
 */

#include "src/backend/epp/update_operation/on_value_of.hh"

namespace Epp {

/**
 * @brief Update operations `set_value`, `delete_value` and `no_operation` supported
 *
 * Intuitive replacement for cryptic `boost::optional<Nullable<T>>`
 */
template <typename T>
using Deletable = typename UpdateOperation::template OnValueOf<T>::template AnyOf<UpdateOperation::Action::set_value,
                                                                                  UpdateOperation::Action::no_operation,
                                                                                  UpdateOperation::Action::delete_value>;

/**
 * @brief Update operations `set_value` and `no_operation` supported
 *
 * Intuitive replacement for cryptic `boost::optional<T>`
 */
template <typename T>
using Updateable = typename UpdateOperation::template OnValueOf<T>::template AnyOf<UpdateOperation::Action::set_value,
                                                                                   UpdateOperation::Action::no_operation>;

}//namespace Epp

#endif//UPDATE_OPERATION_HH_6406A4633F19C685B37A838D3CD3B5B1
