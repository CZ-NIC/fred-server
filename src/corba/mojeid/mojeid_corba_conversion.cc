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
#include <string>
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

    void Unwrapper_Registry_MojeID_ShippingAddress_into_Registry_MojeIDImplData_ShippingAddress::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        Registry::MojeIDImplData::ShippingAddress res;
        res.company_name = unwrap_into<Nullable<std::string> >(ct_in.company_name.in());
        res.street1 = unwrap_into<std::string>(ct_in.street1.in());
        res.street2 = unwrap_into<Nullable<std::string> >(ct_in.street2.in());
        res.street3 = unwrap_into<Nullable<std::string> >(ct_in.street3.in());
        res.city = unwrap_into<std::string>(ct_in.city.in());
        res.state = unwrap_into<Nullable<std::string> >(ct_in.state.in());
        res.postal_code = unwrap_into<std::string>(ct_in.postal_code.in());
        res.country = unwrap_into<std::string>(ct_in.country.in());
        nct_out = res;
    }

    void Wrapper_Registry_MojeIDImplData_ShippingAddress_into_Registry_MojeID_ShippingAddress_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        Registry::MojeID::ShippingAddress_var addr = new Registry::MojeID::ShippingAddress;
        addr->company_name = wrap_into<Registry::MojeID::NullableString_var>(nct_in.company_name);
        addr->street1 = wrap_into<CORBA::String_var>(nct_in.street1);
        addr->street2 = wrap_into<Registry::MojeID::NullableString_var>(nct_in.street2);
        addr->street3 = wrap_into<Registry::MojeID::NullableString_var>(nct_in.street3);
        addr->city = wrap_into<CORBA::String_var>(nct_in.city);
        addr->state = wrap_into<Registry::MojeID::NullableString_var>(nct_in.state);
        addr->postal_code = wrap_into<CORBA::String_var>(nct_in.postal_code);
        addr->country = wrap_into<CORBA::String_var>(nct_in.country);
        ct_out = addr._retn();
    }

    void Unwrapper_Registry_MojeID_NullableShippingAddress_ptr_into_Nullable_Registry_MojeIDImplData_ShippingAddress::unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in == NULL)
        {
            nct_out = Nullable<Registry::MojeIDImplData::ShippingAddress>();
        }
        else
        {
            Registry::MojeIDImplData::ShippingAddress addr = CorbaConversion::unwrap_into<Registry::MojeIDImplData::ShippingAddress>(ct_in->_value());
            nct_out = Nullable<Registry::MojeIDImplData::ShippingAddress>(addr);
        }
    }

    void Wrapper_Nullable_Registry_MojeIDImplData_ShippingAddress_into_Registry_MojeID_NullableShippingAddress_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.isnull())
        {
            ct_out = NULL;
        }
        else
        {
            Registry::MojeID::NullableShippingAddress_var valuetype_addr = new Registry::MojeID::NullableShippingAddress;
            const Registry::MojeID::ShippingAddress_var addr_var = CorbaConversion::wrap_into<Registry::MojeID::ShippingAddress_var>(nct_in.get_value());
            valuetype_addr->_value(addr_var.in());
            ct_out = valuetype_addr._retn();
        }
    }

    void Unwrapper_Registry_MojeID_ValidationError_into_Registry_MojeIDImplData_ValidationError_EnumType::unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
    {
        switch (ct_in)
        {
            case Registry::MojeID::NOT_AVAILABLE:
                nct_out = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
                break;
            case Registry::MojeID::INVALID:
                nct_out = Registry::MojeIDImplData::ValidationError::INVALID;
                break;
            case Registry::MojeID::REQUIRED:
                nct_out = Registry::MojeIDImplData::ValidationError::REQUIRED;
                break;
            default:
                throw NotEnumValidationErrorValue();
        }
    }

    void Wrapper_Registry_MojeIDImplData_ValidationError_EnumType_into_Registry_MojeID_ValidationError::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        switch (nct_in)
        {
            case Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE:
                ct_out = Registry::MojeID::NOT_AVAILABLE;
                break;
            case Registry::MojeIDImplData::ValidationError::INVALID:
                ct_out = Registry::MojeID::INVALID;
                break;
            case Registry::MojeIDImplData::ValidationError::REQUIRED:
                ct_out = Registry::MojeID::REQUIRED;
                break;
            default:
                throw NotEnumValidationErrorValue();
        }
    }

    void Unwrapper_Registry_MojeID_NullableValidationError_ptr_into_Registry_MojeIDImplData_Nullable_ValidationError_EnumType::unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in == NULL)
        {
            nct_out = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>();
        }
        else
        {
            nct_out = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
                unwrap_into<Registry::MojeIDImplData::ValidationError::EnumType>(ct_in->_value()));
        }
    }

    void Wrapper_Registry_MojeIDImplData_Nullable_ValidationError_EnumType_into_Registry_MojeID_NullableValidationError_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.isnull())
        {
            ct_out = NULL;
        }
        else
        {
            Registry::MojeID::NullableValidationError_var valuetype_val_err = new Registry::MojeID::NullableValidationError;
            valuetype_val_err->_value(wrap_into<Registry::MojeID::ValidationError>(nct_in.get_value()));
            ct_out = valuetype_val_err._retn();
        }
    }

    void Unwrapper_Registry_MojeID_AddressValidationError_into_Registry_MojeIDImplData_AddressValidationError::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        Registry::MojeIDImplData::AddressValidationError res;
        res.street1 = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.street1.in());
        res.city = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.city.in());
        res.postal_code = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.postal_code.in());
        res.country = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.country.in());
        nct_out = res;
    }

    void Wrapper_Registry_MojeIDImplData_AddressValidationError_into_Registry_MojeID_AddressValidationError_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        Registry::MojeID::AddressValidationError_var addr = new Registry::MojeID::AddressValidationError;
        addr->street1 = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.street1);
        addr->city = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.city);
        addr->postal_code = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.postal_code);
        addr->country = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.country);
        ct_out = addr._retn();
    }

    void Unwrapper_Registry_MojeID_NullableAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_AddressValidationError::unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in == NULL)
        {
            nct_out = Nullable<Registry::MojeIDImplData::AddressValidationError>();
        }
        else
        {
            Registry::MojeIDImplData::AddressValidationError addr = CorbaConversion::unwrap_into<Registry::MojeIDImplData::AddressValidationError>(ct_in->_value());
            nct_out = Nullable<Registry::MojeIDImplData::AddressValidationError>(addr);
        }
    }

    void Wrapper_Nullable_Registry_MojeIDImplData_AddressValidationError_into_Registry_MojeID_NullableAddressValidationError_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.isnull())
        {
            ct_out = NULL;
        }
        else
        {
            Registry::MojeID::NullableAddressValidationError_var valuetype_addr = new Registry::MojeID::NullableAddressValidationError;
            const Registry::MojeID::AddressValidationError_var addr_var = CorbaConversion::wrap_into<Registry::MojeID::AddressValidationError_var>(nct_in.get_value());
            valuetype_addr->_value(addr_var.in());
            ct_out = valuetype_addr._retn();
        }
    }

    void Unwrapper_Registry_MojeID_MandatoryAddressValidationError_into_Registry_MojeIDImplData_MandatoryAddressValidationError::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        Registry::MojeIDImplData::MandatoryAddressValidationError res;
        res.address_presence = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.address_presence.in());
        res.street1 = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.street1.in());
        res.city = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.city.in());
        res.postal_code = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.postal_code.in());
        res.country = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.country.in());
        nct_out = res;
    }

    void Wrapper_Registry_MojeIDImplData_MandatoryAddressValidationError_into_Registry_MojeID_MandatoryAddressValidationError_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        Registry::MojeID::MandatoryAddressValidationError_var addr = new Registry::MojeID::MandatoryAddressValidationError;
        addr->address_presence = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.address_presence);
        addr->street1 = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.street1);
        addr->city = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.city);
        addr->postal_code = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.postal_code);
        addr->country = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.country);
        ct_out = addr._retn();
    }

    void Unwrapper_Registry_MojeID_NullableMandatoryAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_MandatoryAddressValidationError::unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in == NULL)
        {
            nct_out = Nullable<Registry::MojeIDImplData::MandatoryAddressValidationError>();
        }
        else
        {
            Registry::MojeIDImplData::MandatoryAddressValidationError addr = CorbaConversion::unwrap_into<Registry::MojeIDImplData::MandatoryAddressValidationError>(ct_in->_value());
            nct_out = Nullable<Registry::MojeIDImplData::MandatoryAddressValidationError>(addr);
        }
    }

    void Wrapper_Nullable_Registry_MojeIDImplData_MandatoryAddressValidationError_into_Registry_MojeID_NullableMandatoryAddressValidationError_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.isnull())
        {
            ct_out = NULL;
        }
        else
        {
            Registry::MojeID::NullableMandatoryAddressValidationError_var valuetype_addr = new Registry::MojeID::NullableMandatoryAddressValidationError;
            const Registry::MojeID::MandatoryAddressValidationError_var addr_var = CorbaConversion::wrap_into<Registry::MojeID::MandatoryAddressValidationError_var>(nct_in.get_value());
            valuetype_addr->_value(addr_var.in());
            ct_out = valuetype_addr._retn();
        }
    }

    void Unwrapper_Registry_MojeID_ShippingAddressValidationError_into_Registry_MojeIDImplData_ShippingAddressValidationError::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        Registry::MojeIDImplData::ShippingAddressValidationError res;
        res.street1 = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.street1.in());
        res.city = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.city.in());
        res.postal_code = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.postal_code.in());
        res.country = unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(ct_in.country.in());
        nct_out = res;
    }

    void Wrapper_Registry_MojeIDImplData_ShippingAddressValidationError_into_Registry_MojeID_ShippingAddressValidationError_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        Registry::MojeID::ShippingAddressValidationError_var addr = new Registry::MojeID::ShippingAddressValidationError;
        addr->street1 = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.street1);
        addr->city = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.city);
        addr->postal_code = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.postal_code);
        addr->country = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.country);
        ct_out = addr._retn();
    }

    void Unwrapper_Registry_MojeID_NullableShippingAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_ShippingAddressValidationError::unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in == NULL)
        {
            nct_out = Nullable<Registry::MojeIDImplData::ShippingAddressValidationError>();
        }
        else
        {
            Registry::MojeIDImplData::ShippingAddressValidationError addr = CorbaConversion::unwrap_into<Registry::MojeIDImplData::ShippingAddressValidationError>(ct_in->_value());
            nct_out = Nullable<Registry::MojeIDImplData::ShippingAddressValidationError>(addr);
        }
    }

    void Wrapper_Nullable_Registry_MojeIDImplData_ShippingAddressValidationError_into_Registry_MojeID_NullableShippingAddressValidationError_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.isnull())
        {
            ct_out = NULL;
        }
        else
        {
            Registry::MojeID::NullableShippingAddressValidationError_var valuetype_addr = new Registry::MojeID::NullableShippingAddressValidationError;
            const Registry::MojeID::ShippingAddressValidationError_var addr_var = CorbaConversion::wrap_into<Registry::MojeID::ShippingAddressValidationError_var>(nct_in.get_value());
            valuetype_addr->_value(addr_var.in());
            ct_out = valuetype_addr._retn();
        }
    }

    void Wrapper_Registry_MojeIDImplData_MessageLimitExceeded_into_Registry_MojeID_Server_MESSAGE_LIMIT_EXCEEDED::wrap(const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out)
    {
        try
        {
            Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED res;
            res.limit_expire_date = wrap_into<Registry::MojeID::Date_var>(nct_in.limit_expire_date);
            res.limit_count = nct_in.limit_count;
            res.limit_days = nct_in.limit_days;
            ct_out = res;
        }
        catch(...)
        {
            throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
        }
    }

    void Wrapper_Registry_MojeIDImplData_RegistrationValidationError_into_Registry_MojeID_Server_REGISTRATION_VALIDATION_ERROR::wrap(const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out)
    {
        try
        {
            Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR res;

            res.username = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.username);
            res.first_name = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.first_name);
            res.last_name = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.last_name);
            res.birth_date = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.birth_date);
            res.email = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.email);
            res.notify_email = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.notify_email);
            res.phone = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.phone);
            res.fax = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.fax);

            res.permanent = wrap_into<Registry::MojeID::NullableAddressValidationError_var>(nct_in.permanent);
            res.mailing = wrap_into<Registry::MojeID::NullableAddressValidationError_var>(nct_in.mailing);
            res.billing = wrap_into<Registry::MojeID::NullableAddressValidationError_var>(nct_in.billing);

            res.shipping = wrap_into<Registry::MojeID::NullableShippingAddressValidationError_var>(nct_in.shipping);
            res.shipping2 = wrap_into<Registry::MojeID::NullableShippingAddressValidationError_var>(nct_in.shipping2);
            res.shipping3 = wrap_into<Registry::MojeID::NullableShippingAddressValidationError_var>(nct_in.shipping3);

            ct_out = res;
        }
        catch(...)
        {
            throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
        }
    }

    void Wrapper_Registry_MojeIDImplData_UpdateContactPrepareValidationError_into_Registry_MojeID_Server_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR::wrap(const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out)
    {
        try
        {
            Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR res;

            res.first_name = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.first_name);
            res.last_name = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.last_name);
            res.birth_date = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.birth_date);
            res.email = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.email);
            res.notify_email = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.notify_email);
            res.phone = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.phone);
            res.fax = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.fax);

            res.permanent = wrap_into<Registry::MojeID::NullableAddressValidationError_var>(nct_in.permanent);
            res.mailing = wrap_into<Registry::MojeID::NullableAddressValidationError_var>(nct_in.mailing);
            res.billing = wrap_into<Registry::MojeID::NullableAddressValidationError_var>(nct_in.billing);

            res.shipping = wrap_into<Registry::MojeID::NullableShippingAddressValidationError_var>(nct_in.shipping);
            res.shipping2 = wrap_into<Registry::MojeID::NullableShippingAddressValidationError_var>(nct_in.shipping2);
            res.shipping3 = wrap_into<Registry::MojeID::NullableShippingAddressValidationError_var>(nct_in.shipping3);

            ct_out = res;
        }
        catch(...)
        {
            throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
        }
    }

    void Wrapper_Registry_MojeIDImplData_CreateValidationRequestValidationError_into_Registry_MojeID_Server_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR::wrap(const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out)
    {
        try
        {
            Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR res;

            res.first_name = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.first_name);
            res.last_name = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.last_name);

            res.permanent = wrap_into<Registry::MojeID::NullableMandatoryAddressValidationError_var>(nct_in.permanent);

            res.email = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.email);
            res.phone = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.phone);
            res.notify_email = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.notify_email);
            res.fax = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.fax);
            res.ssn = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.ssn);

            ct_out = res;
        }
        catch(...)
        {
            throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
        }
    }

    void Wrapper_Registry_MojeIDImplData_ProcessRegistrationValidationError_into_Registry_MojeID_Server_PROCESS_REGISTRATION_VALIDATION_ERROR::wrap(const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out)
    {
        try
        {
            Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR res;
            res.email = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.email);
            res.phone = wrap_into<Registry::MojeID::NullableValidationError_var>(nct_in.phone);
            ct_out = res;
        }
        catch(...)
        {
            throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
        }
    }

    void Unwrapper_Registry_MojeID_CreateContact_into_Registry_MojeIDImplData_CreateContact::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        Registry::MojeIDImplData::CreateContact res;

        CorbaConversion::unwrap(ct_in.username.in(), res.username);
        CorbaConversion::unwrap(ct_in.first_name.in(), res.first_name);
        CorbaConversion::unwrap(ct_in.last_name.in(), res.last_name );
        CorbaConversion::unwrap(ct_in.organization.in(), res.organization );
        CorbaConversion::unwrap(ct_in.vat_reg_num.in(), res.vat_reg_num);
        CorbaConversion::unwrap(ct_in.birth_date.in(), res.birth_date);
        CorbaConversion::unwrap(ct_in.id_card_num.in(), res.id_card_num);
        CorbaConversion::unwrap(ct_in.passport_num.in(), res.passport_num);
        CorbaConversion::unwrap(ct_in.ssn_id_num.in(), res.ssn_id_num);
        CorbaConversion::unwrap(ct_in.vat_id_num.in(), res.vat_id_num);
        CorbaConversion::unwrap(ct_in.permanent, res.permanent);
        CorbaConversion::unwrap(ct_in.mailing.in(), res.mailing);
        CorbaConversion::unwrap(ct_in.billing.in(), res.billing);
        CorbaConversion::unwrap(ct_in.shipping.in(), res.shipping);
        CorbaConversion::unwrap(ct_in.shipping2.in(), res.shipping2);
        CorbaConversion::unwrap(ct_in.shipping3.in(), res.shipping3);
        CorbaConversion::unwrap(ct_in.email.in(), res.email);
        CorbaConversion::unwrap(ct_in.notify_email.in(), res.notify_email);
        CorbaConversion::unwrap(ct_in.telephone.in(), res.telephone);
        CorbaConversion::unwrap(ct_in.fax.in(), res.fax);

        nct_out = res;
    }

    void Unwrapper_Registry_MojeID_UpdateContact_into_Registry_MojeIDImplData_UpdateContact::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        Registry::MojeIDImplData::UpdateContact res;

        CorbaConversion::unwrap(ct_in.id, res.id);
        CorbaConversion::unwrap(ct_in.first_name.in(), res.first_name);
        CorbaConversion::unwrap(ct_in.last_name.in(), res.last_name );
        CorbaConversion::unwrap(ct_in.organization.in(), res.organization );
        CorbaConversion::unwrap(ct_in.vat_reg_num.in(), res.vat_reg_num);
        CorbaConversion::unwrap(ct_in.birth_date.in(), res.birth_date);
        CorbaConversion::unwrap(ct_in.id_card_num.in(), res.id_card_num);
        CorbaConversion::unwrap(ct_in.passport_num.in(), res.passport_num);
        CorbaConversion::unwrap(ct_in.ssn_id_num.in(), res.ssn_id_num);
        CorbaConversion::unwrap(ct_in.vat_id_num.in(), res.vat_id_num);
        CorbaConversion::unwrap(ct_in.permanent, res.permanent);
        CorbaConversion::unwrap(ct_in.mailing.in(), res.mailing);
        CorbaConversion::unwrap(ct_in.billing.in(), res.billing);
        CorbaConversion::unwrap(ct_in.shipping.in(), res.shipping);
        CorbaConversion::unwrap(ct_in.shipping2.in(), res.shipping2);
        CorbaConversion::unwrap(ct_in.shipping3.in(), res.shipping3);
        CorbaConversion::unwrap(ct_in.email.in(), res.email);
        CorbaConversion::unwrap(ct_in.notify_email.in(), res.notify_email);
        CorbaConversion::unwrap(ct_in.telephone.in(), res.telephone);
        CorbaConversion::unwrap(ct_in.fax.in(), res.fax);

        nct_out = res;
    }

    void Unwrapper_Registry_MojeID_SetContact_into_Registry_MojeIDImplData_SetContact::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        Registry::MojeIDImplData::SetContact res;

        CorbaConversion::unwrap(ct_in.organization.in(), res.organization);
        CorbaConversion::unwrap(ct_in.vat_reg_num.in(), res.vat_reg_num);
        CorbaConversion::unwrap(ct_in.birth_date.in(), res.birth_date);
        CorbaConversion::unwrap(ct_in.vat_id_num.in(), res.vat_id_num);
        CorbaConversion::unwrap(ct_in.permanent, res.permanent);
        CorbaConversion::unwrap(ct_in.mailing.in(), res.mailing);
        CorbaConversion::unwrap(ct_in.email.in(), res.email);
        CorbaConversion::unwrap(ct_in.notify_email.in(), res.notify_email);
        CorbaConversion::unwrap(ct_in.telephone.in(), res.telephone);
        CorbaConversion::unwrap(ct_in.fax.in(), res.fax);

        nct_out = res;
    }
}
