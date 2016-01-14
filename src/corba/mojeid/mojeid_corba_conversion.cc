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
#include "src/corba/mojeid/mojeid_corba_conversion.h"
#include <string>

namespace CorbaConversion
{
    namespace Internal
    {
        void into_safe_storage< std::string, char >::wrap(const std::string &src, char *&safe_dst)
        {
            //some string holders are initialized to the special value
            if (safe_dst != _CORBA_String_helper::empty_string) {
                check_empty_storage(safe_dst);
            }
            safe_dst = CORBA::string_dup(src.c_str());
        }
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::Date,
                            boost::gregorian::date >::unwrap(const CORBA_TYPE &src,
                                                             NON_CORBA_TYPE &dst)
    {
        std::string value;
        unwrap_holder(src.value, value);
        dst = boost::gregorian::from_simple_string(value);

        if (dst.is_special()) {
            throw ArgumentIsSpecial();
        }
    }

    void DEFAULT_WRAPPER< boost::gregorian::date,
                          Registry::MojeID::Date >::wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
    {
        if (src.is_special()) {
            throw ArgumentIsSpecial();
        }

        wrap_into_holder(boost::gregorian::to_iso_extended_string(src), dst.value);
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::DateTime,
                            boost::posix_time::ptime >::unwrap(const CORBA_TYPE &src,
                                                               NON_CORBA_TYPE &dst)
    {
        std::string value;
        unwrap_holder(src.value, value);
        dst = boost::date_time::parse_delimited_time< ptime >(value, 'T');

        if (dst.is_special()) {
            throw ArgumentIsSpecial();
        }
    }

    void DEFAULT_WRAPPER< boost::posix_time::ptime,
                          Registry::MojeID::DateTime >::wrap(const NON_CORBA_TYPE &src,
                                                             CORBA_TYPE &dst)
    {
        if (src.is_special()) {
            throw ArgumentIsSpecial();
        }

        wrap_into_holder(boost::posix_time::to_iso_extended_string(src), dst.value);
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::Address,
                            Registry::MojeIDImplData::Address >::unwrap(const CORBA_TYPE &src,
                                                                        NON_CORBA_TYPE &dst)
    {
        unwrap_holder(src.street1,     dst.street1);
        unwrap_holder(src.street2,     dst.street2);
        unwrap_holder(src.street3,     dst.street3);
        unwrap_holder(src.city,        dst.city);
        unwrap_holder(src.state,       dst.state);
        unwrap_holder(src.postal_code, dst.postal_code);
        unwrap_holder(src.country,     dst.country);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::Address,
                          Registry::MojeID::Address >::wrap(const NON_CORBA_TYPE &src,
                                                            CORBA_TYPE &dst)
    {
        wrap_into_holder         (src.street1,     dst.street1);
        wrap_nullable_into_holder(src.street2,     dst.street2);
        wrap_nullable_into_holder(src.street3,     dst.street3);
        wrap_into_holder         (src.city,        dst.city);
        wrap_nullable_into_holder(src.state,       dst.state);
        wrap_into_holder         (src.postal_code, dst.postal_code);
        wrap_into_holder         (src.country,     dst.country);
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::ShippingAddress,
                            Registry::MojeIDImplData::ShippingAddress >::unwrap(const CORBA_TYPE &src,
                                                                                NON_CORBA_TYPE &dst)
    {
        unwrap_holder(src.company_name, dst.company_name);
        unwrap_holder(src.street1,      dst.street1);
        unwrap_holder(src.street2,      dst.street2);
        unwrap_holder(src.street3,      dst.street3);
        unwrap_holder(src.city,         dst.city);
        unwrap_holder(src.state,        dst.state);
        unwrap_holder(src.postal_code,  dst.postal_code);
        unwrap_holder(src.country,      dst.country);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::ShippingAddress,
                          Registry::MojeID::ShippingAddress >::wrap(const NON_CORBA_TYPE &src,
                                                                    CORBA_TYPE &dst)
    {
        wrap_nullable_into_holder(src.company_name, dst.company_name);
        wrap_into_holder         (src.street1,      dst.street1);
        wrap_nullable_into_holder(src.street2,      dst.street2);
        wrap_nullable_into_holder(src.street3,      dst.street3);
        wrap_into_holder         (src.city,         dst.city);
        wrap_nullable_into_holder(src.state,        dst.state);
        wrap_into_holder         (src.postal_code,  dst.postal_code);
        wrap_into_holder         (src.country,      dst.country);
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
                          Registry::MojeID::AddressValidationResult >::wrap(const NON_CORBA_TYPE &src,
                                                                            CORBA_TYPE &dst)
    {
        CorbaConversion::wrap(src.street1,     dst.street1);
        CorbaConversion::wrap(src.city,        dst.city);
        CorbaConversion::wrap(src.postal_code, dst.postal_code);
        CorbaConversion::wrap(src.country,     dst.country);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::MandatoryAddressValidationResult,
                          Registry::MojeID::MandatoryAddressValidationResult >::wrap(const NON_CORBA_TYPE &src,
                                                                                     CORBA_TYPE &dst)
    {
        CorbaConversion::wrap(src.address_presence, dst.address_presence);
        CorbaConversion::wrap(src.street1,          dst.street1);
        CorbaConversion::wrap(src.city,             dst.city);
        CorbaConversion::wrap(src.postal_code,      dst.postal_code);
        CorbaConversion::wrap(src.country,          dst.country);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::ShippingAddressValidationResult,
                          Registry::MojeID::ShippingAddressValidationResult >::wrap(const NON_CORBA_TYPE &src,
                                                                                    CORBA_TYPE &dst)
    {
        CorbaConversion::wrap(src.street1,     dst.street1);
        CorbaConversion::wrap(src.city,        dst.city);
        CorbaConversion::wrap(src.postal_code, dst.postal_code);
        CorbaConversion::wrap(src.country,     dst.country);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::MessageLimitExceeded,
                          Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED >::wrap(const NON_CORBA_TYPE &src,
                                                                                   CORBA_TYPE &dst)
    {
        CorbaConversion::wrap(src.limit_expire_date, dst.limit_expire_date);
        CorbaConversion::wrap(src.limit_count,       dst.limit_count);
        CorbaConversion::wrap(src.limit_days,        dst.limit_days);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::RegistrationValidationResult,
                          Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR >::wrap(const NON_CORBA_TYPE &src,
                                                                                          CORBA_TYPE &dst)
    {
        CorbaConversion::wrap(src.username,     dst.username);
        CorbaConversion::wrap(src.first_name,   dst.first_name);
        CorbaConversion::wrap(src.last_name,    dst.last_name);
        CorbaConversion::wrap(src.birth_date,   dst.birth_date);
        CorbaConversion::wrap(src.email,        dst.email);
        CorbaConversion::wrap(src.notify_email, dst.notify_email);
        CorbaConversion::wrap(src.phone,        dst.phone);
        CorbaConversion::wrap(src.fax,          dst.fax);

        CorbaConversion::wrap(src.permanent,    dst.permanent);
        CorbaConversion::wrap(src.mailing,      dst.mailing);
        CorbaConversion::wrap(src.billing,      dst.billing);

        CorbaConversion::wrap(src.shipping,     dst.shipping);
        CorbaConversion::wrap(src.shipping2,    dst.shipping2);
        CorbaConversion::wrap(src.shipping3,    dst.shipping3);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::UpdateContactPrepareValidationResult,
                          Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR >::wrap(const NON_CORBA_TYPE &src,
                                                                                                    CORBA_TYPE &dst)
    {
        CorbaConversion::wrap(src.first_name,   dst.first_name);
        CorbaConversion::wrap(src.last_name,    dst.last_name);
        CorbaConversion::wrap(src.birth_date,   dst.birth_date);
        CorbaConversion::wrap(src.email,        dst.email);
        CorbaConversion::wrap(src.notify_email, dst.notify_email);
        CorbaConversion::wrap(src.phone,        dst.phone);
        CorbaConversion::wrap(src.fax,          dst.fax);

        CorbaConversion::wrap(src.permanent,    dst.permanent);
        CorbaConversion::wrap(src.mailing,      dst.mailing);
        CorbaConversion::wrap(src.billing,      dst.billing);

        CorbaConversion::wrap(src.shipping,     dst.shipping);
        CorbaConversion::wrap(src.shipping2,    dst.shipping2);
        CorbaConversion::wrap(src.shipping3,    dst.shipping3);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::CreateValidationRequestValidationResult,
                          Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR >::wrap(const NON_CORBA_TYPE &src,
                                                                                                       CORBA_TYPE &dst)
    {
        CorbaConversion::wrap(src.first_name,   dst.first_name);
        CorbaConversion::wrap(src.last_name,    dst.last_name);

        CorbaConversion::wrap(src.permanent,    dst.permanent);

        CorbaConversion::wrap(src.email,        dst.email);
        CorbaConversion::wrap(src.phone,        dst.phone);
        CorbaConversion::wrap(src.notify_email, dst.notify_email);
        CorbaConversion::wrap(src.fax,          dst.fax);
        CorbaConversion::wrap(src.ssn,          dst.ssn);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::ProcessRegistrationValidationResult,
                          Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR >::wrap(const NON_CORBA_TYPE &src,
                                                                                                  CORBA_TYPE &dst)
    {
        CorbaConversion::wrap(src.email, dst.email);
        CorbaConversion::wrap(src.phone, dst.phone);
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::CreateContact,
                            Registry::MojeIDImplData::CreateContact >::unwrap(const CORBA_TYPE &src,
                                                                              NON_CORBA_TYPE &dst)
    {
        unwrap_holder          (src.username,     dst.username);
        unwrap_holder          (src.first_name,   dst.first_name);
        unwrap_holder          (src.last_name,    dst.last_name);
        unwrap_holder          (src.organization, dst.organization);
        unwrap_holder          (src.vat_reg_num,  dst.vat_reg_num);
        unwrap_holder          (src.birth_date,   dst.birth_date);
        unwrap_holder          (src.id_card_num,  dst.id_card_num);
        unwrap_holder          (src.passport_num, dst.passport_num);
        unwrap_holder          (src.ssn_id_num,   dst.ssn_id_num);
        unwrap_holder          (src.vat_id_num,   dst.vat_id_num);
        CorbaConversion::unwrap(src.permanent,    dst.permanent);
        unwrap_holder          (src.mailing,      dst.mailing);
        unwrap_holder          (src.billing,      dst.billing);
        unwrap_holder          (src.shipping,     dst.shipping);
        unwrap_holder          (src.shipping2,    dst.shipping2);
        unwrap_holder          (src.shipping3,    dst.shipping3);
        unwrap_holder          (src.email,        dst.email);
        unwrap_holder          (src.notify_email, dst.notify_email);
        unwrap_holder          (src.telephone,    dst.telephone);
        unwrap_holder          (src.fax,          dst.fax);
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::UpdateContact,
                            Registry::MojeIDImplData::UpdateContact >::unwrap(const CORBA_TYPE &src,
                                                                              NON_CORBA_TYPE &dst)
    {
        CorbaConversion::unwrap(src.id,           dst.id);
        unwrap_holder          (src.first_name,   dst.first_name);
        unwrap_holder          (src.last_name,    dst.last_name);
        unwrap_holder          (src.organization, dst.organization);
        unwrap_holder          (src.vat_reg_num,  dst.vat_reg_num);
        unwrap_holder          (src.birth_date,   dst.birth_date);
        unwrap_holder          (src.id_card_num,  dst.id_card_num);
        unwrap_holder          (src.passport_num, dst.passport_num);
        unwrap_holder          (src.ssn_id_num,   dst.ssn_id_num);
        unwrap_holder          (src.vat_id_num,   dst.vat_id_num);
        CorbaConversion::unwrap(src.permanent,    dst.permanent);
        unwrap_holder          (src.mailing,      dst.mailing);
        unwrap_holder          (src.billing,      dst.billing);
        unwrap_holder          (src.shipping,     dst.shipping);
        unwrap_holder          (src.shipping2,    dst.shipping2);
        unwrap_holder          (src.shipping3,    dst.shipping3);
        unwrap_holder          (src.email,        dst.email);
        unwrap_holder          (src.notify_email, dst.notify_email);
        unwrap_holder          (src.telephone,    dst.telephone);
        unwrap_holder          (src.fax,          dst.fax);
    }

    void DEFAULT_UNWRAPPER< Registry::MojeID::SetContact,
                            Registry::MojeIDImplData::SetContact >::unwrap(const CORBA_TYPE &src,
                                                                           NON_CORBA_TYPE &dst)
    {
        unwrap_holder          (src.organization, dst.organization);
        unwrap_holder          (src.vat_reg_num,  dst.vat_reg_num);
        unwrap_holder          (src.birth_date,   dst.birth_date);
        unwrap_holder          (src.vat_id_num,   dst.vat_id_num);
        CorbaConversion::unwrap(src.permanent,    dst.permanent);
        unwrap_holder          (src.mailing,      dst.mailing);
        unwrap_holder          (src.email,        dst.email);
        unwrap_holder          (src.notify_email, dst.notify_email);
        unwrap_holder          (src.telephone,    dst.telephone);
        unwrap_holder          (src.fax,          dst.fax);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::InfoContact,
                          Registry::MojeID::InfoContact >::wrap(const NON_CORBA_TYPE &src,
                                                                CORBA_TYPE &dst)
    {
        CorbaConversion::wrap    (src.id,           dst.id);
        wrap_into_holder         (src.first_name,   dst.first_name);
        wrap_into_holder         (src.last_name,    dst.last_name);
        wrap_nullable_into_holder(src.organization, dst.organization);
        wrap_nullable_into_holder(src.vat_reg_num,  dst.vat_reg_num);
        wrap_nullable_into_holder(src.birth_date,   dst.birth_date);
        wrap_nullable_into_holder(src.id_card_num,  dst.id_card_num);
        wrap_nullable_into_holder(src.passport_num, dst.passport_num);
        wrap_nullable_into_holder(src.ssn_id_num,   dst.ssn_id_num);
        wrap_nullable_into_holder(src.vat_id_num,   dst.vat_id_num);
        CorbaConversion::wrap    (src.permanent,    dst.permanent);
        wrap_nullable_into_holder(src.mailing,      dst.mailing);
        wrap_nullable_into_holder(src.billing,      dst.billing);
        wrap_nullable_into_holder(src.shipping,     dst.shipping);
        wrap_nullable_into_holder(src.shipping2,    dst.shipping2);
        wrap_nullable_into_holder(src.shipping3,    dst.shipping3);
        wrap_into_holder         (src.email,        dst.email);
        wrap_nullable_into_holder(src.notify_email, dst.notify_email);
        wrap_nullable_into_holder(src.telephone,    dst.telephone);
        wrap_nullable_into_holder(src.fax,          dst.fax);
    }

    void DEFAULT_WRAPPER< Registry::MojeIDImplData::ContactStateInfo,
                          Registry::MojeID::ContactStateInfo >::wrap(const NON_CORBA_TYPE &src,
                                                                     CORBA_TYPE &dst)
    {
        CorbaConversion::wrap    (src.contact_id,                        dst.contact_id);
        CorbaConversion::wrap    (src.mojeid_activation_datetime,        dst.mojeid_activation_datetime);
        CorbaConversion::wrap    (src.conditionally_identification_date, dst.conditionally_identification_date);
        wrap_nullable_into_holder(src.identification_date,               dst.identification_date);
        wrap_nullable_into_holder(src.validation_date,                   dst.validation_date);
        wrap_nullable_into_holder(src.linked_date,                       dst.linked_date);
    }

}
