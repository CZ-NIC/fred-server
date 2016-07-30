/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file check_keyset.cc
 *  keyset check
 */

#include "src/fredlib/keyset/check_keyset.h"
#include "src/fredlib/object/check_handle.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"

#include <string>

#include <boost/regex.hpp>
#include <boost/mpl/assert.hpp>


namespace Fred
{

template < Object_Type::Enum OBJECT_TYPE >
TestHandleOf< OBJECT_TYPE >::TestHandleOf(const std::string &_handle)
:   handle_(_handle)
{}

template < Object_Type::Enum OBJECT_TYPE >
bool TestHandleOf< OBJECT_TYPE >::is_invalid_handle()const
{
    BOOST_MPL_ASSERT_MSG(OBJECT_TYPE != Object_Type::domain, deprecated_function_for_domain, (Object_Type::Enum));
    static const std::size_t max_length_of_handle = 30;
    if (max_length_of_handle < handle_.length())
    {
        return true;
    }
    const boost::regex handle_syntax_rule("[a-zA-Z0-9](-?[a-zA-Z0-9])*");
    return !boost::regex_match(handle_, handle_syntax_rule);
}


template < Object_Type::Enum OBJECT_TYPE >
bool TestHandleOf< OBJECT_TYPE >::is_protected(OperationContext &_ctx)const
{
    BOOST_MPL_ASSERT_MSG(OBJECT_TYPE != Object_Type::domain, deprecated_function_for_domain, (Object_Type::Enum));
    static const char *const parameter_name = "handle_registration_protection_period";
    const std::string object_type_name = Conversion::Enums::to_db_handle(OBJECT_TYPE);
    const Database::Result db_res = _ctx.get_conn().exec_params(
        "WITH obj AS "
            "(SELECT obr.id,"
                    "CURRENT_TIMESTAMP<(obr.erdate+(ep.val||'MONTH')::INTERVAL) AS protected "
             "FROM enum_parameters ep,"
                  "object_registry obr "
             "WHERE ep.name=$1::TEXT AND "
                   "obr.type=get_object_type_id($2::TEXT) AND "
                   "UPPER(obr.name)=UPPER($3::TEXT)) "
        "SELECT protected FROM obj "
        "ORDER BY id DESC LIMIT 1",
        Database::query_param_list(parameter_name)
                                  (object_type_name)
                                  (handle_));

    return (db_res.size() == 1) && !db_res[0][0].isnull() && static_cast< bool >(db_res[0][0]);
}


template < Object_Type::Enum OBJECT_TYPE >
bool TestHandleOf< OBJECT_TYPE >::is_registered(OperationContext &_ctx)const
{
    BOOST_MPL_ASSERT_MSG(OBJECT_TYPE != Object_Type::domain, deprecated_function_for_domain, (Object_Type::Enum));
    const std::string object_type_name = Conversion::Enums::to_db_handle(OBJECT_TYPE);
    const Database::Result db_res = _ctx.get_conn().exec_params(
        "SELECT 1 "
        "FROM object_registry "
        "WHERE erdate IS NULL AND "
              "type=get_object_type_id($1::TEXT) AND "
              "name=UPPER($2::TEXT)",
        Database::query_param_list(object_type_name)(handle_));

    return 0 < db_res.size();
}

template class TestHandleOf< Object_Type::contact >;
template class TestHandleOf< Object_Type::nsset >;
template class TestHandleOf< Object_Type::keyset >;

}//namespace Fred
