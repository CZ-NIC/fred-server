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
    void DEFAULT_UNWRAPPER< Registry::MojeID::Date,
                            boost::gregorian::date >::unwrap(const CORBA_TYPE &src,
                                                             NON_CORBA_TYPE &dst)
    {
        const std::string value = unwrap_into< std::string >(src.value.in());
        dst = boost::gregorian::from_simple_string(value);

        if (dst.is_special()) {
            throw ArgumentIsSpecial();
        }
    }

    void DEFAULT_WRAPPER< boost::gregorian::date,
                          Registry::MojeID::Date_var >::wrap(const NON_CORBA_TYPE &src,
                                                             CORBA_TYPE &dst)
    {
        if (src.is_special()) {
            throw ArgumentIsSpecial();
        }

        dst = new Registry::MojeID::Date;
        dst->value = wrap_into< CORBA::String_var >(boost::gregorian::to_iso_extended_string(src));
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::DateTime,
                            boost::posix_time::ptime >::unwrap(const CORBA_TYPE &src,
                                                               NON_CORBA_TYPE &dst)
    {
        const std::string value = unwrap_into< std::string >(src.value.in());
        dst = boost::date_time::parse_delimited_time< ptime >(value, 'T');

        if (dst.is_special()) {
            throw ArgumentIsSpecial();
        }
    }

    void DEFAULT_WRAPPER< boost::posix_time::ptime,
                          Registry::MojeID::DateTime_var >::wrap(const NON_CORBA_TYPE &src,
                                                                 CORBA_TYPE &dst)
    {
        if (src.is_special()) {
            throw ArgumentIsSpecial();
        }

        dst = new Registry::MojeID::DateTime;
        dst->value = wrap_into< CORBA::String_var >(boost::posix_time::to_iso_extended_string(src));
    }

    template < class CORBA_TYPE, class NON_CORBA_TYPE >
    void unwrap_nullable(CORBA_TYPE *src_ptr, Nullable< NON_CORBA_TYPE > &dst)
    {
        if (src_ptr == NULL) {
            dst = Nullable< NON_CORBA_TYPE >();
        }
        else {
            dst = unwrap_into< NON_CORBA_TYPE >(src_ptr->_value());
        }
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::NullableDate*,
                            Nullable< boost::gregorian::date > >::unwrap(CORBA_TYPE src,
                                                                         NON_CORBA_TYPE &dst)
    {
        unwrap_nullable(src, dst);
    }

    template < class CORBA_VAR_TYPE, class CORBA_NULLABLE_TYPE,
               class NON_CORBA_TYPE, class CORBA_NULLABLE_VAR_TYPE >
    void wrap_nullable(const Nullable< NON_CORBA_TYPE > &src, CORBA_NULLABLE_VAR_TYPE &dst)
    {
        dst = src.isnull() ? NULL
                           : new CORBA_NULLABLE_TYPE(wrap_into< CORBA_VAR_TYPE >(src.get_value()).in());
    }

    void DEFAULT_WRAPPER< Nullable< boost::gregorian::date >,
                          Registry::MojeID::NullableDate_var >::wrap(const NON_CORBA_TYPE &src,
                                                                     CORBA_TYPE &dst)
    {
        wrap_nullable< Registry::MojeID::Date_var, Registry::MojeID::NullableDate >(src, dst);
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::NullableBoolean*,
                            Nullable< bool > >::unwrap(CORBA_TYPE src,
                                                       NON_CORBA_TYPE &dst)
    {
        unwrap_nullable(src, dst);
    }

    void DEFAULT_WRAPPER< Nullable< bool >,
                          Registry::MojeID::NullableBoolean_var >::wrap(const NON_CORBA_TYPE &src,
                                                                        CORBA_TYPE &dst)
    {
        dst = src.isnull() ? NULL
                           : new Registry::MojeID::NullableBoolean(wrap_into< bool >(src.get_value()));
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::Address,
                            Registry::MojeIDImplData::Address >::unwrap(const CORBA_TYPE &src,
                                                                        NON_CORBA_TYPE &dst)
    {
        CorbaConversion::unwrap(src.street1.in(),     dst.street1);
        CorbaConversion::unwrap(src.street2.in(),     dst.street2);
        CorbaConversion::unwrap(src.street3.in(),     dst.street3);
        CorbaConversion::unwrap(src.city.in(),        dst.city);
        CorbaConversion::unwrap(src.state.in(),       dst.state);
        CorbaConversion::unwrap(src.postal_code.in(), dst.postal_code);
        CorbaConversion::unwrap(src.country.in(),     dst.country);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::Address,
                          Registry::MojeID::Address_var >::wrap(const NON_CORBA_TYPE &src,
                                                                CORBA_TYPE &dst)
    {
        dst = new Registry::MojeID::Address;
        dst->street1     = wrap_into< CORBA::String_var >(src.street1);
        dst->street2     = wrap_into< Registry::MojeID::NullableString_var >(src.street2);
        dst->street3     = wrap_into< Registry::MojeID::NullableString_var >(src.street3);
        dst->city        = wrap_into< CORBA::String_var >(src.city);
        dst->state       = wrap_into< Registry::MojeID::NullableString_var >(src.state);
        dst->postal_code = wrap_into< CORBA::String_var >(src.postal_code);
        dst->country     = wrap_into< CORBA::String_var >(src.country);
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::NullableAddress*,
                            Nullable< Registry::MojeIDImplData::Address > >::unwrap(CORBA_TYPE src,
                                                                                    NON_CORBA_TYPE &dst)
    {
        unwrap_nullable(src, dst);
    }

    void DEFAULT_WRAPPER< Nullable< Registry::MojeIDImplData::Address >,
                          Registry::MojeID::NullableAddress_var >::wrap(const NON_CORBA_TYPE &src,
                                                                        CORBA_TYPE &dst)
    {
        wrap_nullable< Registry::MojeID::Address_var, Registry::MojeID::NullableAddress >(src, dst);
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::ShippingAddress,
                            Registry::MojeIDImplData::ShippingAddress >::unwrap(const CORBA_TYPE &src,
                                                                                NON_CORBA_TYPE &dst)
    {
        CorbaConversion::unwrap(src.company_name.in(), dst.company_name);
        CorbaConversion::unwrap(src.street1.in(),      dst.street1);
        CorbaConversion::unwrap(src.street2.in(),      dst.street2);
        CorbaConversion::unwrap(src.street3.in(),      dst.street3);
        CorbaConversion::unwrap(src.city.in(),         dst.city);
        CorbaConversion::unwrap(src.state.in(),        dst.state);
        CorbaConversion::unwrap(src.postal_code.in(),  dst.postal_code);
        CorbaConversion::unwrap(src.country.in(),      dst.country);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::ShippingAddress,
                          Registry::MojeID::ShippingAddress_var >::wrap(const NON_CORBA_TYPE &src,
                                                                        CORBA_TYPE &dst)
    {
        dst = new Registry::MojeID::ShippingAddress;
        dst->company_name = wrap_into< Registry::MojeID::NullableString_var >(src.company_name);
        dst->street1      = wrap_into< CORBA::String_var >(src.street1);
        dst->street2      = wrap_into< Registry::MojeID::NullableString_var >(src.street2);
        dst->street3      = wrap_into< Registry::MojeID::NullableString_var >(src.street3);
        dst->city         = wrap_into< CORBA::String_var >(src.city);
        dst->state        = wrap_into< Registry::MojeID::NullableString_var >(src.state);
        dst->postal_code  = wrap_into< CORBA::String_var >(src.postal_code);
        dst->country      = wrap_into< CORBA::String_var >(src.country);
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::NullableShippingAddress*,
                            Nullable< Registry::MojeIDImplData::ShippingAddress > >::unwrap(CORBA_TYPE src,
                                                                                            NON_CORBA_TYPE &dst)
    {
        unwrap_nullable(src, dst);
    }

    void DEFAULT_WRAPPER< Nullable< Registry::MojeIDImplData::ShippingAddress >,
                          Registry::MojeID::NullableShippingAddress_var >::wrap(const NON_CORBA_TYPE &src,
                                                                                CORBA_TYPE &dst)
    {
        wrap_nullable< Registry::MojeID::ShippingAddress_var, Registry::MojeID::NullableShippingAddress >(src, dst);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::ValidationResult,
                          Registry::MojeID::ValidationResult >::wrap(const NON_CORBA_TYPE &src,
                                                                     CORBA_TYPE &dst)
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

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::AddressValidationResult,
                          Registry::MojeID::AddressValidationResult_var >::wrap(const NON_CORBA_TYPE &src,
                                                                                CORBA_TYPE &dst)
    {
        dst = new Registry::MojeID::AddressValidationResult;
        dst->street1     = wrap_into< Registry::MojeID::ValidationResult >(src.street1);
        dst->city        = wrap_into< Registry::MojeID::ValidationResult >(src.city);
        dst->postal_code = wrap_into< Registry::MojeID::ValidationResult >(src.postal_code);
        dst->country     = wrap_into< Registry::MojeID::ValidationResult >(src.country);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::MandatoryAddressValidationResult,
                          Registry::MojeID::MandatoryAddressValidationResult_var >::wrap(const NON_CORBA_TYPE &src,
                                                                                         CORBA_TYPE &dst)
    {
        dst = new Registry::MojeID::MandatoryAddressValidationResult;
        dst->address_presence = wrap_into< Registry::MojeID::ValidationResult >(src.address_presence);
        dst->street1          = wrap_into< Registry::MojeID::ValidationResult >(src.street1);
        dst->city             = wrap_into< Registry::MojeID::ValidationResult >(src.city);
        dst->postal_code      = wrap_into< Registry::MojeID::ValidationResult >(src.postal_code);
        dst->country          = wrap_into< Registry::MojeID::ValidationResult >(src.country);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::ShippingAddressValidationResult,
                          Registry::MojeID::ShippingAddressValidationResult_var >::wrap(const NON_CORBA_TYPE &src,
                                                                                        CORBA_TYPE &dst)
    {
        dst = new Registry::MojeID::ShippingAddressValidationResult;
        dst->street1     = wrap_into< Registry::MojeID::ValidationResult >(src.street1);
        dst->city        = wrap_into< Registry::MojeID::ValidationResult >(src.city);
        dst->postal_code = wrap_into< Registry::MojeID::ValidationResult >(src.postal_code);
        dst->country     = wrap_into< Registry::MojeID::ValidationResult >(src.country);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::MessageLimitExceeded,
                          Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED >::wrap(const NON_CORBA_TYPE &src,
                                                                                   CORBA_TYPE &dst)
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

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::RegistrationValidationResult,
                          Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR >::wrap(const NON_CORBA_TYPE &src,
                                                                                          CORBA_TYPE &dst)
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

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::UpdateContactPrepareValidationResult,
                          Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR >::wrap(const NON_CORBA_TYPE &src,
                                                                                                    CORBA_TYPE &dst)
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

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::CreateValidationRequestValidationResult,
                          Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR >::wrap(const NON_CORBA_TYPE &src,
                                                                                                       CORBA_TYPE &dst)
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

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::ProcessRegistrationValidationResult,
                          Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR >::wrap(const NON_CORBA_TYPE &src,
                                                                                                  CORBA_TYPE &dst)
    {
        try {
            dst.email = wrap_into< Registry::MojeID::ValidationResult >(src.email);
            dst.phone = wrap_into< Registry::MojeID::ValidationResult >(src.phone);
        }
        catch (...) {
            throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
        }
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::CreateContact,
                            Registry::MojeIDImplData::CreateContact >::unwrap(const CORBA_TYPE &src,
                                                                              NON_CORBA_TYPE &dst)
    {
        CorbaConversion::unwrap(src.username.in(),     dst.username);
        CorbaConversion::unwrap(src.first_name.in(),   dst.first_name);
        CorbaConversion::unwrap(src.last_name.in(),    dst.last_name);
        CorbaConversion::unwrap(src.organization.in(), dst.organization);
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

    void DEFAULT_UNWRAPPER< Registry::MojeID::UpdateContact,
                            Registry::MojeIDImplData::UpdateContact >::unwrap(const CORBA_TYPE &src,
                                                                              NON_CORBA_TYPE &dst)
    {
        CorbaConversion::unwrap(src.id,                dst.id);
        CorbaConversion::unwrap(src.first_name.in(),   dst.first_name);
        CorbaConversion::unwrap(src.last_name.in(),    dst.last_name);
        CorbaConversion::unwrap(src.organization.in(), dst.organization);
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

    void DEFAULT_UNWRAPPER< Registry::MojeID::SetContact,
                            Registry::MojeIDImplData::SetContact >::unwrap(const CORBA_TYPE &src,
                                                                           NON_CORBA_TYPE &dst)
    {
        CorbaConversion::unwrap(src.organization.in(), dst.organization);
        CorbaConversion::unwrap(src.vat_reg_num.in(),  dst.vat_reg_num);
        CorbaConversion::unwrap(src.birth_date.in(),   dst.birth_date);
        CorbaConversion::unwrap(src.vat_id_num.in(),   dst.vat_id_num);
        CorbaConversion::unwrap(src.permanent,         dst.permanent);
        CorbaConversion::unwrap(src.mailing.in(),      dst.mailing);
        CorbaConversion::unwrap(src.email.in(),        dst.email);
        CorbaConversion::unwrap(src.notify_email.in(), dst.notify_email);
        CorbaConversion::unwrap(src.telephone.in(),    dst.telephone);
        CorbaConversion::unwrap(src.fax.in(),          dst.fax);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::InfoContact,
                          Registry::MojeID::InfoContact_var >::wrap(const NON_CORBA_TYPE &src,
                                                                    CORBA_TYPE &dst)
    {
        dst = new Registry::MojeID::InfoContact;

        dst->id           = wrap_into< CORBA::ULongLong >(src.id);
        dst->first_name   = wrap_into< CORBA::String_var >(src.first_name);
        dst->last_name    = wrap_into< CORBA::String_var >(src.last_name);
        dst->organization = wrap_into< Registry::MojeID::NullableString_var >(src.organization);
        dst->vat_reg_num  = wrap_into< Registry::MojeID::NullableString_var >(src.vat_reg_num);
        dst->birth_date   = wrap_into< Registry::MojeID::NullableDate_var >(src.birth_date);
        dst->id_card_num  = wrap_into< Registry::MojeID::NullableString_var >(src.id_card_num);
        dst->passport_num = wrap_into< Registry::MojeID::NullableString_var >(src.passport_num);
        dst->ssn_id_num   = wrap_into< Registry::MojeID::NullableString_var >(src.ssn_id_num);
        dst->vat_id_num   = wrap_into< Registry::MojeID::NullableString_var >(src.vat_id_num);
        dst->permanent    = wrap_into< Registry::MojeID::Address_var >(src.permanent);
        dst->mailing      = wrap_into< Registry::MojeID::NullableAddress_var >(src.mailing);
        dst->billing      = wrap_into< Registry::MojeID::NullableAddress_var >(src.billing);
        dst->shipping     = wrap_into< Registry::MojeID::NullableShippingAddress_var >(src.shipping);
        dst->shipping2    = wrap_into< Registry::MojeID::NullableShippingAddress_var >(src.shipping2);
        dst->shipping3    = wrap_into< Registry::MojeID::NullableShippingAddress_var >(src.shipping3);
        dst->email        = wrap_into< CORBA::String_var >(src.email);
        dst->notify_email = wrap_into< Registry::MojeID::NullableString_var >(src.notify_email);
        dst->telephone    = wrap_into< Registry::MojeID::NullableString_var >(src.telephone);
        dst->fax          = wrap_into< Registry::MojeID::NullableString_var >(src.fax);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::ContactStateInfo,
                          Registry::MojeID::ContactStateInfo_var >::wrap(const NON_CORBA_TYPE &src,
                                                                         CORBA_TYPE &dst)
    {
        dst = new Registry::MojeID::ContactStateInfo;

        dst->contact_id                        = wrap_into< CORBA::ULongLong >(src
            .contact_id);
        dst->mojeid_activation_datetime        = wrap_into< Registry::MojeID::DateTime_var >(src
            .mojeid_activation_datetime);
        dst->conditionally_identification_date = wrap_into< Registry::MojeID::Date_var >(src
            .conditionally_identification_date);
        dst->identification_date               = wrap_into< Registry::MojeID::NullableDate_var >(src
            .identification_date);
        dst->validation_date                   = wrap_into< Registry::MojeID::NullableDate_var >(src
            .validation_date);
        dst->linked_date                       = wrap_into< Registry::MojeID::NullableDate_var >(src
            .linked_date);
    }

}
