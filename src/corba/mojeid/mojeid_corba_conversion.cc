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
 *  implementation for MojeID CORBA conversion
 */

#include "mojeid_corba_conversion.h"

namespace CorbaConversion
{
    void Unwrapper_Registry_MojeID_Date_ptr_into_boost_gregorian_date::unwrap( CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in == NULL)
        {
            throw PointerIsNULL();
        }

        nct_out = boost::gregorian::from_simple_string(ct_in->value.in());

        if(nct_out.is_special())
        {
            throw ArgumentIsSpecial();
        }
    }

    void Wrapper_boost_gregorian_date_into_Registry_MojeID_Date_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.is_special())
        {
            throw ArgumentIsSpecial();
        }

        Registry::MojeID::Date_var res = new Registry::MojeID::Date;
        res->value = wrap_by<Wrapper_std_string_into_String_var>(boost::gregorian::to_iso_extended_string(nct_in));
        ct_out = res._retn();
    }

    void Unwrapper_Registry_MojeID_DateTime_ptr_into_boost_posix_time_ptime::unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in == NULL)
        {
            throw PointerIsNULL();
        }

        nct_out = boost::date_time::parse_delimited_time<ptime>(ct_in->value.in(), 'T');

        if(nct_out.is_special())
        {
            throw ArgumentIsSpecial();
        }
    }

    void Wrapper_boost_posix_time_ptime_into_Registry_MojeID_DateTime_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.is_special())
        {
            throw ArgumentIsSpecial();
        }

        Registry::MojeID::DateTime_var res = new Registry::MojeID::DateTime;
        res->value = wrap_by<Wrapper_std_string_into_String_var>(boost::posix_time::to_iso_extended_string(nct_in));
        ct_out = res._retn();
    }

    void Unwrapper_Registry_MojeID_NullableDate_ptr_into_Nullable_boost_gregorian_date::unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in == NULL)
        {
            nct_out = Nullable<boost::gregorian::date>();
        }
        else
        {
            nct_out = Nullable<boost::gregorian::date>(boost::gregorian::from_simple_string(ct_in->_value().value.in()));
        }
    }

    void Wrapper_Nullable_boost_gregorian_date_into_Registry_MojeID_NullableDate_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.isnull())
        {
            ct_out = NULL;
        }
        else
        {
            Registry::MojeID::NullableDate_var valuetype_date = new Registry::MojeID::NullableDate;
            valuetype_date->value(CorbaConversion::wrap_by<Wrapper_boost_gregorian_date_into_Registry_MojeID_Date_var>(nct_in.get_value())->value.in());
            ct_out = valuetype_date._retn();
        }
    }

    void Unwrapper_Registry_MojeID_Address_var_into_Registry_MojeIDImplData_Address::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in.operator->() == NULL)
        {
            throw PointerIsNULL();
        }

        Registry::MojeIDImplData::Address res;
        res.street1 = unwrap_into<std::string>(ct_in->street1.in());
        res.street2 = unwrap_into<Nullable<std::string> >(ct_in->street2.in());
        res.street3 = unwrap_into<Nullable<std::string> >(ct_in->street3.in());
        res.city = unwrap_into<std::string>(ct_in->city.in());
        res.state = unwrap_into<Nullable<std::string> >(ct_in->state.in());
        res.postal_code = unwrap_into<std::string>(ct_in->postal_code.in());
        res.country = unwrap_into<std::string>(ct_in->country.in());
        nct_out = res;
    }

    void Wrapper_Registry_MojeIDImplData_Address_into_Registry_MojeID_Address_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        Registry::MojeID::Address_var addr = new Registry::MojeID::Address;
        addr->street1 = wrap_by<Wrapper_std_string_into_String_var>(nct_in.street1);
        addr->street2 = wrap_by<Wrapper_Nullable_std_string_into_Registry_MojeID_NullableString_var>(nct_in.street2);
        addr->street3 = wrap_by<Wrapper_Nullable_std_string_into_Registry_MojeID_NullableString_var>(nct_in.street3);
        addr->city = wrap_by<Wrapper_std_string_into_String_var>(nct_in.city);
        addr->state = wrap_by<Wrapper_Nullable_std_string_into_Registry_MojeID_NullableString_var>(nct_in.state);
        addr->postal_code = wrap_by<Wrapper_std_string_into_String_var>(nct_in.postal_code);
        addr->country = wrap_by<Wrapper_std_string_into_String_var>(nct_in.country);
        ct_out = addr._retn();
    }

}
