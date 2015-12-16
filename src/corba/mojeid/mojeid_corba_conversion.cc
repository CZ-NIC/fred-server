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
    void Unwrapper_Registry_MojeID_Date_into_boost_gregorian_date::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        nct_out = boost::gregorian::from_simple_string(ct_in.value.in());

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
        res->value = wrap_into<CORBA::String_var>(boost::gregorian::to_iso_extended_string(nct_in));
        ct_out = res._retn();
    }

    void Unwrapper_Registry_MojeID_DateTime_into_boost_posix_time_ptime::unwrap(const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {

        nct_out = boost::date_time::parse_delimited_time<ptime>(ct_in.value.in(), 'T');

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
        res->value = wrap_into<CORBA::String_var>(boost::posix_time::to_iso_extended_string(nct_in));
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
            nct_out = Nullable<boost::gregorian::date>(
                CorbaConversion::unwrap_into<boost::gregorian::date>(ct_in->_value()));
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
            valuetype_date->_value(CorbaConversion::wrap_into<Registry::MojeID::Date_var>(nct_in.get_value()).in());
            ct_out = valuetype_date._retn();
        }
    }

    void Unwrapper_Registry_MojeID_NullableBoolean_ptr_into_Nullable_bool::unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in == NULL)
        {
            nct_out = Nullable<bool>();
        }
        else
        {
            nct_out = Nullable<bool>(ct_in->_value());
        }
    }

    void Wrapper_Nullable_bool_into_Registry_MojeID_NullableBoolean_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.isnull())
        {
            ct_out = NULL;
        }
        else
        {
            Registry::MojeID::NullableBoolean_var valuetype_bool = new Registry::MojeID::NullableBoolean;
            valuetype_bool->_value(nct_in.get_value());
            ct_out = valuetype_bool._retn();
        }
    }

    void Unwrapper_Registry_MojeID_Address_into_Registry_MojeIDImplData_Address::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        Registry::MojeIDImplData::Address res;
        res.street1 = unwrap_into<std::string>(ct_in.street1.in());
        res.street2 = unwrap_into<Nullable<std::string> >(ct_in.street2.in());
        res.street3 = unwrap_into<Nullable<std::string> >(ct_in.street3.in());
        res.city = unwrap_into<std::string>(ct_in.city.in());
        res.state = unwrap_into<Nullable<std::string> >(ct_in.state.in());
        res.postal_code = unwrap_into<std::string>(ct_in.postal_code.in());
        res.country = unwrap_into<std::string>(ct_in.country.in());
        nct_out = res;
    }

    void Wrapper_Registry_MojeIDImplData_Address_into_Registry_MojeID_Address_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        Registry::MojeID::Address_var addr = new Registry::MojeID::Address;
        addr->street1 = wrap_into<CORBA::String_var>(nct_in.street1);
        addr->street2 = wrap_into<Registry::MojeID::NullableString_var>(nct_in.street2);
        addr->street3 = wrap_into<Registry::MojeID::NullableString_var>(nct_in.street3);
        addr->city = wrap_into<CORBA::String_var>(nct_in.city);
        addr->state = wrap_into<Registry::MojeID::NullableString_var>(nct_in.state);
        addr->postal_code = wrap_into<CORBA::String_var>(nct_in.postal_code);
        addr->country = wrap_into<CORBA::String_var>(nct_in.country);
        ct_out = addr._retn();
    }

    void Unwrapper_Registry_MojeID_NullableAddress_ptr_into_Nullable_Registry_MojeIDImplData_Address::unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in == NULL)
        {
            nct_out = Nullable<Registry::MojeIDImplData::Address>();
        }
        else
        {
            Registry::MojeIDImplData::Address addr = CorbaConversion::unwrap_into<Registry::MojeIDImplData::Address>(ct_in->_value());
            nct_out = Nullable<Registry::MojeIDImplData::Address>(addr);
        }
    }

    void Wrapper_Nullable_Registry_MojeIDImplData_Address_into_Registry_MojeID_NullableAddress_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.isnull())
        {
            ct_out = NULL;
        }
        else
        {
            Registry::MojeID::NullableAddress_var valuetype_addr = new Registry::MojeID::NullableAddress;
            const Registry::MojeID::Address_var addr_var = CorbaConversion::wrap_into<Registry::MojeID::Address_var>(nct_in.get_value());
            valuetype_addr->_value(addr_var.in());
            ct_out = valuetype_addr._retn();
        }
    }


}
