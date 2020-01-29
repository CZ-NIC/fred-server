/*
 * Copyright (C) 2016-2020  CZ.NIC, z. s. p. o.
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
/**
*  @file
*  header of enum to database handle conversions
*/
#ifndef ENUM_TO_DB_HANDLE_CONVERSION_HH_1FD9D18DF2704A3492EFC3A25953078B
#define ENUM_TO_DB_HANDLE_CONVERSION_HH_1FD9D18DF2704A3492EFC3A25953078B

#include "test/libfred/util.hh"

#include <map>
#include <boost/test/unit_test.hpp>

template < typename ENUM_HOST_TYPE, ::size_t EXPECTED_NUMBER_OF_DB_HANDLES >
void enum_to_db_handle_conversion_test(::LibFred::OperationContext &ctx, const char *sql_get_all_items)
{
    typedef typename ENUM_HOST_TYPE::Enum Enum;
    typedef std::set< std::string > DbHandles;
    DbHandles db_handles;
    DbHandles incorrect_db_handles;
    incorrect_db_handles.insert("");
    incorrect_db_handles.insert(" ");
    incorrect_db_handles.insert("blaBLAbla");

    {
        const Database::Result res = ctx.get_conn().exec(sql_get_all_items);
        BOOST_CHECK(res.size() == EXPECTED_NUMBER_OF_DB_HANDLES);
        for (::size_t idx = 0; idx < res.size(); ++idx) {
            const std::string correct_handle = static_cast< std::string >(res[idx][0]);
            db_handles.insert(correct_handle);
            incorrect_db_handles.insert(" " + correct_handle);
            incorrect_db_handles.insert("_" + correct_handle);
            incorrect_db_handles.insert(correct_handle + " ");
            incorrect_db_handles.insert(correct_handle + "_");
            incorrect_db_handles.insert(" " + correct_handle + " ");
            incorrect_db_handles.insert("_" + correct_handle + "_");
        }
        BOOST_CHECK(db_handles.size() == res.size());
    }

    long long min_enum_value = std::numeric_limits< long long >::max();
    long long max_enum_value = std::numeric_limits< long long >::min();
    for (DbHandles::const_iterator db_handle_ptr = db_handles.begin(); db_handle_ptr != db_handles.end(); ++db_handle_ptr) {
        const Enum enum_value = Conversion::Enums::from_db_handle< ENUM_HOST_TYPE >(*db_handle_ptr);
        BOOST_CHECK(Conversion::Enums::to_db_handle(enum_value) == *db_handle_ptr);
        if (static_cast< long long >(enum_value) < min_enum_value) {
            min_enum_value = static_cast< long long >(enum_value);
        }
        if (max_enum_value < static_cast< long long >(enum_value)) {
            max_enum_value = static_cast< long long >(enum_value);
        }
    }

    for (DbHandles::const_iterator incorrect_db_handle_ptr = incorrect_db_handles.begin();
         incorrect_db_handle_ptr != incorrect_db_handles.end(); ++incorrect_db_handle_ptr)
    {
        BOOST_CHECK_EXCEPTION(
        try {
            BOOST_CHECK(db_handles.count(*incorrect_db_handle_ptr) == 0);
            Conversion::Enums::from_db_handle< ENUM_HOST_TYPE >(*incorrect_db_handle_ptr);
        }
        catch (const std::invalid_argument &e) {
            BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
            throw;
        }
        catch (const std::exception &e) {
            BOOST_ERROR(boost::diagnostic_information(e));
            throw;
        }
        catch (...) {
            BOOST_ERROR("unexpected exception occurs");
            throw;
        },
        std::invalid_argument,
        check_std_exception);
    }

    BOOST_CHECK_EXCEPTION(
        try {
            Conversion::Enums::to_db_handle(Enum(min_enum_value - 1));
        }
        catch (const std::invalid_argument &e) {
            BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
            throw;
        }
        catch (const std::exception &e) {
            BOOST_ERROR(boost::diagnostic_information(e));
            throw;
        }
        catch (...) {
            BOOST_ERROR("unexpected exception occurs");
            throw;
        },
        std::invalid_argument,
        check_std_exception);

    BOOST_CHECK_EXCEPTION(
        try {
            Conversion::Enums::to_db_handle(Enum(max_enum_value + 1));
        }
        catch (const std::invalid_argument &e) {
            BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
            throw;
        }
        catch (const std::exception &e) {
            BOOST_ERROR(boost::diagnostic_information(e));
            throw;
        }
        catch (...) {
            BOOST_ERROR("unexpected exception occurs");
            throw;
        },
        std::invalid_argument,
        check_std_exception);
}

#endif
