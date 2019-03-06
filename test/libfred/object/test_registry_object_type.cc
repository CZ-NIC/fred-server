/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include <boost/test/unit_test.hpp>

#include "libfred/object/registry_object_type.hh"

#include "libfred/opcontext.hh"

#include "test/setup/fixtures.hh"

BOOST_AUTO_TEST_SUITE(TestRegistryObjectType)

BOOST_FIXTURE_TEST_CASE(all_definitions_from_db_are_supported, Test::instantiate_db_template)
{
    ::LibFred::OperationContextCreator ctx;
    Database::Result obj_types_res = ctx.get_conn().exec("SELECT name FROM enum_object_type");
    BOOST_CHECK(obj_types_res.size() > 0);

    for(::size_t i = 0; i < obj_types_res.size(); ++i) {
        ::LibFred::object_type tmp;

        BOOST_CHECK_NO_THROW(
            tmp = ::LibFred::object_type_from_db_handle( static_cast<std::string>( obj_types_res[i]["name"] ) );
        );

        BOOST_CHECK_EQUAL(
            static_cast<std::string>( obj_types_res[i]["name"] ),
            ::LibFred::to_db_handle( tmp )
        );
    }
}

BOOST_AUTO_TEST_CASE(invalid_handle)
{
    BOOST_CHECK_THROW(
        ::LibFred::object_type_from_db_handle("pretty_much_obvious_nonsense"),
        ::LibFred::ExceptionUnknownObjectType
    );

    BOOST_CHECK_THROW(
        ::LibFred::object_type_from_db_handle(""),
        ::LibFred::ExceptionUnknownObjectType
    );
}

BOOST_AUTO_TEST_SUITE_END();

