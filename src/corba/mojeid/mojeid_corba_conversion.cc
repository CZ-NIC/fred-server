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

    void Wrapper_Registry_MojeIDImplData_ValidationResult_into_Registry_MojeID_ValidationResult::wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
    {
        switch (src) {
            case Registry::MojeID::OK:
            case Registry::MojeID::NOT_AVAILABLE:
            case Registry::MojeID::INVALID:
            case Registry::MojeID::REQUIRED:
                dst = src;
                return;
        }
        throw NotEnumValidationResultValue();
    }

    void Wrapper_Registry_MojeIDImplData_AddressValidationResult_into_Registry_MojeID_AddressValidationResult_var::wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
    {
        dst = new Registry::MojeID::AddressValidationResult;
        dst->street1     = wrap_into< Registry::MojeID::ValidationResult >(src.street1);
        dst->city        = wrap_into< Registry::MojeID::ValidationResult >(src.city);
        dst->postal_code = wrap_into< Registry::MojeID::ValidationResult >(src.postal_code);
        dst->country     = wrap_into< Registry::MojeID::ValidationResult >(src.country);
    }

    void Wrapper_Registry_MojeIDImplData_MandatoryAddressValidationResult_into_Registry_MojeID_MandatoryAddressValidationResult_var::wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
    {
        dst = new Registry::MojeID::MandatoryAddressValidationResult;
        dst->address_presence = wrap_into< Registry::MojeID::ValidationResult >(src.address_presence);
        dst->street1          = wrap_into< Registry::MojeID::ValidationResult >(src.street1);
        dst->city             = wrap_into< Registry::MojeID::ValidationResult >(src.city);
        dst->postal_code      = wrap_into< Registry::MojeID::ValidationResult >(src.postal_code);
        dst->country          = wrap_into< Registry::MojeID::ValidationResult >(src.country);
    }

    void Wrapper_Registry_MojeIDImplData_ShippingAddressValidationResult_into_Registry_MojeID_ShippingAddressValidationResult_var::wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
    {
        dst = new Registry::MojeID::ShippingAddressValidationResult;
        dst->street1     = wrap_into< Registry::MojeID::ValidationResult >(src.street1);
        dst->city        = wrap_into< Registry::MojeID::ValidationResult >(src.city);
        dst->postal_code = wrap_into< Registry::MojeID::ValidationResult >(src.postal_code);
        dst->country     = wrap_into< Registry::MojeID::ValidationResult >(src.country);
    }

    void Wrapper_Registry_MojeIDImplData_MessageLimitExceeded_into_Registry_MojeID_Server_MESSAGE_LIMIT_EXCEEDED::wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
    {
        try {
            dst.limit_expire_date = wrap_into< Registry::MojeID::Date_var >(src.limit_expire_date);
            CorbaConversion::wrap(src.limit_count, dst.limit_count);
            CorbaConversion::wrap(src.limit_days,  dst.limit_days);
        }
        catch(...) {
            throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
        }
    }

    void Wrapper_Registry_MojeIDImplData_RegistrationValidationResult_into_Registry_MojeID_Server_REGISTRATION_VALIDATION_ERROR::wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
    {
        try {
            dst.username     = wrap_into< Registry::MojeID::ValidationResult >(src.username);
            dst.first_name   = wrap_into< Registry::MojeID::ValidationResult >(src.first_name);
            dst.last_name    = wrap_into< Registry::MojeID::ValidationResult >(src.last_name);
            dst.birth_date   = wrap_into< Registry::MojeID::ValidationResult >(src.birth_date);
            dst.email        = wrap_into< Registry::MojeID::ValidationResult >(src.email);
            dst.notify_email = wrap_into< Registry::MojeID::ValidationResult >(src.notify_email);
            dst.phone        = wrap_into< Registry::MojeID::ValidationResult >(src.phone);
            dst.fax          = wrap_into< Registry::MojeID::ValidationResult >(src.fax);

            dst.permanent    = wrap_into< Registry::MojeID::AddressValidationResult_var >(src.permanent);
            dst.mailing      = wrap_into< Registry::MojeID::AddressValidationResult_var >(src.mailing);
            dst.billing      = wrap_into< Registry::MojeID::AddressValidationResult_var >(src.billing);

            dst.shipping     = wrap_into< Registry::MojeID::ShippingAddressValidationResult_var >(src.shipping);
            dst.shipping2    = wrap_into< Registry::MojeID::ShippingAddressValidationResult_var >(src.shipping2);
            dst.shipping3    = wrap_into< Registry::MojeID::ShippingAddressValidationResult_var >(src.shipping3);
        }
        catch(...) {
            throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
        }
    }

    void Wrapper_Registry_MojeIDImplData_UpdateContactPrepareValidationResult_into_Registry_MojeID_Server_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR::wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
    {
        try {
            dst.first_name   = wrap_into< Registry::MojeID::ValidationResult >(src.first_name);
            dst.last_name    = wrap_into< Registry::MojeID::ValidationResult >(src.last_name);
            dst.birth_date   = wrap_into< Registry::MojeID::ValidationResult >(src.birth_date);
            dst.email        = wrap_into< Registry::MojeID::ValidationResult >(src.email);
            dst.notify_email = wrap_into< Registry::MojeID::ValidationResult >(src.notify_email);
            dst.phone        = wrap_into< Registry::MojeID::ValidationResult >(src.phone);
            dst.fax          = wrap_into< Registry::MojeID::ValidationResult >(src.fax);

            dst.permanent    = wrap_into< Registry::MojeID::AddressValidationResult_var >(src.permanent);
            dst.mailing      = wrap_into< Registry::MojeID::AddressValidationResult_var >(src.mailing);
            dst.billing      = wrap_into< Registry::MojeID::AddressValidationResult_var >(src.billing);

            dst.shipping     = wrap_into< Registry::MojeID::ShippingAddressValidationResult_var >(src.shipping);
            dst.shipping2    = wrap_into< Registry::MojeID::ShippingAddressValidationResult_var >(src.shipping2);
            dst.shipping3    = wrap_into< Registry::MojeID::ShippingAddressValidationResult_var >(src.shipping3);

        }
        catch(...) {
            throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
        }
    }

    void Wrapper_Registry_MojeIDImplData_CreateValidationRequestValidationResult_into_Registry_MojeID_Server_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR::wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
    {
        try {
            dst.first_name   = wrap_into< Registry::MojeID::ValidationResult >(src.first_name);
            dst.last_name    = wrap_into< Registry::MojeID::ValidationResult >(src.last_name);

            dst.permanent    = wrap_into< Registry::MojeID::MandatoryAddressValidationResult_var >(src.permanent);

            dst.email        = wrap_into< Registry::MojeID::ValidationResult >(src.email);
            dst.phone        = wrap_into< Registry::MojeID::ValidationResult >(src.phone);
            dst.notify_email = wrap_into< Registry::MojeID::ValidationResult >(src.notify_email);
            dst.fax          = wrap_into< Registry::MojeID::ValidationResult >(src.fax);
            dst.ssn          = wrap_into< Registry::MojeID::ValidationResult >(src.ssn);
        }
        catch(...) {
            throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
        }
    }

    void Wrapper_Registry_MojeIDImplData_ProcessRegistrationValidationResult_into_Registry_MojeID_Server_PROCESS_REGISTRATION_VALIDATION_ERROR::wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
    {
        try {
            dst.email = wrap_into< Registry::MojeID::ValidationResult >(src.email);
            dst.phone = wrap_into< Registry::MojeID::ValidationResult >(src.phone);
        }
        catch (...) {
            throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
        }
    }

    void Unwrapper_Registry_MojeID_CreateContact_into_Registry_MojeIDImplData_CreateContact::unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst)
    {
        CorbaConversion::unwrap(src.username.in(),     dst.username);
        CorbaConversion::unwrap(src.first_name.in(),   dst.first_name);
        CorbaConversion::unwrap(src.last_name.in(),    dst.last_name );
        CorbaConversion::unwrap(src.organization.in(), dst.organization );
        CorbaConversion::unwrap(src.vat_reg_num.in(),  dst.vat_reg_num);
        CorbaConversion::unwrap(src.birth_date.in(),   dst.birth_date);
        CorbaConversion::unwrap(src.id_card_num.in(),  dst.id_card_num);
        CorbaConversion::unwrap(src.passport_num.in(), dst.passport_num);
        CorbaConversion::unwrap(src.ssn_id_num.in(),   dst.ssn_id_num);
        CorbaConversion::unwrap(src.vat_id_num.in(),   dst.vat_id_num);
        CorbaConversion::unwrap(src.permanent,         dst.permanent);
        CorbaConversion::unwrap(src.mailing.in(),      dst.mailing);
        CorbaConversion::unwrap(src.billing.in(),      dst.billing);
        CorbaConversion::unwrap(src.shipping.in(),     dst.shipping);
        CorbaConversion::unwrap(src.shipping2.in(),    dst.shipping2);
        CorbaConversion::unwrap(src.shipping3.in(),    dst.shipping3);
        CorbaConversion::unwrap(src.email.in(),        dst.email);
        CorbaConversion::unwrap(src.notify_email.in(), dst.notify_email);
        CorbaConversion::unwrap(src.telephone.in(),    dst.telephone);
        CorbaConversion::unwrap(src.fax.in(),          dst.fax);
    }

    void Unwrapper_Registry_MojeID_UpdateContact_into_Registry_MojeIDImplData_UpdateContact::unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst)
    {
        CorbaConversion::unwrap(src.id,                dst.id);
        CorbaConversion::unwrap(src.first_name.in(),   dst.first_name);
        CorbaConversion::unwrap(src.last_name.in(),    dst.last_name );
        CorbaConversion::unwrap(src.organization.in(), dst.organization );
        CorbaConversion::unwrap(src.vat_reg_num.in(),  dst.vat_reg_num);
        CorbaConversion::unwrap(src.birth_date.in(),   dst.birth_date);
        CorbaConversion::unwrap(src.id_card_num.in(),  dst.id_card_num);
        CorbaConversion::unwrap(src.passport_num.in(), dst.passport_num);
        CorbaConversion::unwrap(src.ssn_id_num.in(),   dst.ssn_id_num);
        CorbaConversion::unwrap(src.vat_id_num.in(),   dst.vat_id_num);
        CorbaConversion::unwrap(src.permanent,         dst.permanent);
        CorbaConversion::unwrap(src.mailing.in(),      dst.mailing);
        CorbaConversion::unwrap(src.billing.in(),      dst.billing);
        CorbaConversion::unwrap(src.shipping.in(),     dst.shipping);
        CorbaConversion::unwrap(src.shipping2.in(),    dst.shipping2);
        CorbaConversion::unwrap(src.shipping3.in(),    dst.shipping3);
        CorbaConversion::unwrap(src.email.in(),        dst.email);
        CorbaConversion::unwrap(src.notify_email.in(), dst.notify_email);
        CorbaConversion::unwrap(src.telephone.in(),    dst.telephone);
        CorbaConversion::unwrap(src.fax.in(),          dst.fax);
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

    void Wrapper_Registry_MojeIDImplData_InfoContact_into_Registry_MojeID_InfoContact_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        Registry::MojeID::InfoContact_var info = new Registry::MojeID::InfoContact;

        info->id = CorbaConversion::wrap_into<CORBA::ULongLong>(nct_in.id);
        info->first_name = CorbaConversion::wrap_into<CORBA::String_var>(nct_in.first_name);
        info->last_name = CorbaConversion::wrap_into<CORBA::String_var>(nct_in.last_name);
        info->organization = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
            nct_in.organization);
        info->vat_reg_num = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
            nct_in.vat_reg_num);
        info->birth_date = CorbaConversion::wrap_into<Registry::MojeID::NullableDate_var>(
            nct_in.birth_date);
        info->id_card_num = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
            nct_in.id_card_num);
        info->passport_num = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
            nct_in.passport_num);
        info->ssn_id_num = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
            nct_in.ssn_id_num);
        info->vat_id_num = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
            nct_in.vat_id_num);
        info->permanent = CorbaConversion::wrap_into<Registry::MojeID::Address_var>(nct_in.permanent);
        info->mailing = CorbaConversion::wrap_into<Registry::MojeID::NullableAddress_var>(
            nct_in.mailing);
        info->billing = CorbaConversion::wrap_into<Registry::MojeID::NullableAddress_var>(
            nct_in.billing);
        info->shipping = CorbaConversion::wrap_into<Registry::MojeID::NullableShippingAddress_var>(
            nct_in.shipping);
        info->shipping2 = CorbaConversion::wrap_into<Registry::MojeID::NullableShippingAddress_var>(
            nct_in.shipping2);
        info->shipping3 = CorbaConversion::wrap_into<Registry::MojeID::NullableShippingAddress_var>(
            nct_in.shipping3);
        info->email = CorbaConversion::wrap_into<CORBA::String_var>(nct_in.email);
        info->notify_email = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
            nct_in.notify_email);
        info->telephone = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
            nct_in.telephone);
        info->fax = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(nct_in.fax);

        ct_out = info._retn();
    }

    void Wrapper_Registry_MojeIDImplData_ContactStateInfo_into_Registry_MojeID_ContactStateInfo_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        Registry::MojeID::ContactStateInfo_var info = new Registry::MojeID::ContactStateInfo;

        info->contact_id = CorbaConversion::wrap_into<CORBA::ULongLong>(nct_in.contact_id);
        info->mojeid_activation_datetime = CorbaConversion::wrap_into<
            Registry::MojeID::DateTime_var>(nct_in.mojeid_activation_datetime);
        info->conditionally_identification_date = CorbaConversion::wrap_into<
            Registry::MojeID::Date_var>(nct_in.conditionally_identification_date);
        info->identification_date = CorbaConversion::wrap_into<
            Registry::MojeID::NullableDate_var>(nct_in.identification_date);
        info->validation_date = CorbaConversion::wrap_into<
            Registry::MojeID::NullableDate_var>(nct_in.validation_date);
        info->linked_date = CorbaConversion::wrap_into<
            Registry::MojeID::NullableDate_var>(nct_in.linked_date);

        ct_out = info._retn();
    }

}
