/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  @file
 *  declaration for MojeID CORBA conversion
 */

#ifndef MOJEID_CORBA_CONVERSION_H_e5b26622ca884604abf9cf49892b20d7
#define MOJEID_CORBA_CONVERSION_H_e5b26622ca884604abf9cf49892b20d7

#include "util/corba_conversion.h"
#include "src/corba/MojeID2.hh"

namespace CorbaConversion
{

    template <> struct DEFAULT_UNWRAPPER<Registry::MojeID::NullableString_var, Nullable<std::string> >
    {
        typedef Unwrapper_NullableString_var_into_Nullable_std_string<Registry::MojeID::NullableString_var> type;
    };

    template <> struct DEFAULT_WRAPPER<Nullable<std::string>, Registry::MojeID::NullableString_var>
    {
        typedef Wrapper_Nullable_std_string_into_NullableString_var<Registry::MojeID::NullableString, Registry::MojeID::NullableString_var> type;
    };

    /**
     * Exception if argument is special
     */
    class ArgumentIsSpecial : public std::invalid_argument
    {
    public:
        ArgumentIsSpecial() : std::invalid_argument("argument is special") {}
        virtual ~ArgumentIsSpecial() throw() {}
    };

    //Registry::MojeID::Date_var
    struct Unwrapper_Registry_MojeID_Date_var_into_boost_date
    {
        typedef Registry::MojeID::Date_var CORBA_TYPE;
        typedef boost::gregorian::date NON_CORBA_TYPE;
        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
        Unwrapper_Registry_MojeID_Date_var_into_boost_date::CORBA_TYPE,
        Unwrapper_Registry_MojeID_Date_var_into_boost_date::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_Date_var_into_boost_date type;
    };

    struct Wrapper_boost_date_into_Registry_MojeID_Date_var
    {
        typedef Registry::MojeID::Date_var CORBA_TYPE;
        typedef boost::gregorian::date NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
        Wrapper_boost_date_into_Registry_MojeID_Date_var::NON_CORBA_TYPE,
        Wrapper_boost_date_into_Registry_MojeID_Date_var::CORBA_TYPE>
    {
        typedef Wrapper_boost_date_into_Registry_MojeID_Date_var type;
    };

    //Registry::MojeID::DateTime_var
    struct Unwrapper_Registry_MojeID_DateTime_var_into_boost_ptime
    {
        typedef Registry::MojeID::DateTime_var CORBA_TYPE;
        typedef boost::posix_time::ptime NON_CORBA_TYPE;
        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
        Unwrapper_Registry_MojeID_DateTime_var_into_boost_ptime::CORBA_TYPE,
        Unwrapper_Registry_MojeID_DateTime_var_into_boost_ptime::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_DateTime_var_into_boost_ptime type;
    };

    struct Wrapper_boost_ptime_into_Registry_MojeID_DateTime_var
    {
        typedef Registry::MojeID::DateTime_var CORBA_TYPE;
        typedef boost::posix_time::ptime NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
        Wrapper_boost_ptime_into_Registry_MojeID_DateTime_var::NON_CORBA_TYPE,
        Wrapper_boost_ptime_into_Registry_MojeID_DateTime_var::CORBA_TYPE>
    {
        typedef Wrapper_boost_ptime_into_Registry_MojeID_DateTime_var type;
    };

    //Registry::MojeID::NullableDate_var
    struct Unwrapper_Registry_MojeID_NullableDate_var_into_Nullable_boost_date
    {
        typedef Registry::MojeID::NullableDate_var CORBA_TYPE;
        typedef Nullable<boost::gregorian::date> NON_CORBA_TYPE;
        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_NullableDate_var_into_Nullable_boost_date::CORBA_TYPE,
    Unwrapper_Registry_MojeID_NullableDate_var_into_Nullable_boost_date::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_NullableDate_var_into_Nullable_boost_date type;
    };

    struct Wrapper_Nullable_boost_date_into_Registry_MojeID_NullableDate_var
    {
        typedef Registry::MojeID::NullableDate_var CORBA_TYPE;
        typedef Nullable<boost::gregorian::date> NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Nullable_boost_date_into_Registry_MojeID_NullableDate_var::NON_CORBA_TYPE,
    Wrapper_Nullable_boost_date_into_Registry_MojeID_NullableDate_var::CORBA_TYPE>
    {
        typedef Wrapper_Nullable_boost_date_into_Registry_MojeID_NullableDate_var type;
    };


}
#endif

