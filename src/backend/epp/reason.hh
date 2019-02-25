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
#ifndef REASON_HH_F09B8C7F15B642FE8AB9F7B63FDCE2E4
#define REASON_HH_F09B8C7F15B642FE8AB9F7B63FDCE2E4

#include "src/backend/epp/exception.hh"

namespace Epp {

struct Reason
{
    enum Enum
    {
        bad_format_contact_handle   = 1,
        bad_format_nsset_handle     = 2,
        bad_format_fqdn             = 3,
        not_applicable_domain       = 4,
        invalid_handle              = 5,
        existing                    = 6,
        protected_period            = 7,
        bad_ip_address              = 8,
        bad_dns_name                = 9,
        duplicated_dns_address      = 10,
        ip_glue_not_allowed         = 11,
        period_range                = 14,
        period_policy               = 15,
        country_notexist            = 16,
        msgid_notexist              = 17,
        valexpdate_not_used         = 18,
        valexpdate_not_valid        = 19,
        can_not_remove_tech         = 23,
        technical_contact_already_assigned = 24,
        technical_contact_not_registered = 25,
        admin_exist                 = 26,
        admin_notexist              = 27,
        nsset_notexist              = 28,
        registrant_notexist         = 29,
        dns_name_exist              = 30,
        dns_name_notexist           = 31,
        curexpdate_not_expdate      = 32,
        blacklisted_domain          = 36,
        xml_validity_error          = 37,
        duplicated_contact          = 38,
        bad_format_keyset_handle    = 39,
        keyset_notexist             = 40,
        unauthorized_registrar      = 48,
        techadmin_limit             = 49,
        dsrecord_limit              = 50,
        dnskey_limit                = 51,
        no_dnskey                   = 53,
        dnskey_bad_flags            = 54,
        dnskey_bad_protocol         = 55,
        dnskey_bad_alg              = 56,
        dnskey_bad_key_len          = 57,
        dnskey_bad_key_char         = 58,
        dnskey_exist                = 59,
        dnskey_notexist             = 60,
        duplicated_dnskey           = 61,
        no_dnskey_dsrecord          = 62,
        duplicated_dns_name         = 63,
        admin_not_assigned          = 64,
        tmpcontacts_obsolete        = 65,
        period_too_short            = 66
    };
    static bool is_valid(Enum value)
    {
        switch (value)
        {
            case bad_format_contact_handle:
            case bad_format_nsset_handle:
            case bad_format_fqdn:
            case not_applicable_domain:
            case invalid_handle:
            case existing:
            case protected_period:
            case bad_ip_address:
            case bad_dns_name:
            case duplicated_dns_address:
            case ip_glue_not_allowed:
            case period_range:
            case period_policy:
            case country_notexist:
            case msgid_notexist:
            case valexpdate_not_used:
            case valexpdate_not_valid:
            case can_not_remove_tech:
            case technical_contact_already_assigned:
            case technical_contact_not_registered:
            case admin_exist:
            case admin_notexist:
            case nsset_notexist:
            case registrant_notexist:
            case dns_name_exist:
            case dns_name_notexist:
            case curexpdate_not_expdate:
            case blacklisted_domain:
            case xml_validity_error:
            case duplicated_contact:
            case bad_format_keyset_handle:
            case keyset_notexist:
            case unauthorized_registrar:
            case techadmin_limit:
            case dsrecord_limit:
            case dnskey_limit:
            case no_dnskey:
            case dnskey_bad_flags:
            case dnskey_bad_protocol:
            case dnskey_bad_alg:
            case dnskey_bad_key_len:
            case dnskey_bad_key_char:
            case dnskey_exist:
            case dnskey_notexist:
            case duplicated_dnskey:
            case no_dnskey_dsrecord:
            case duplicated_dns_name:
            case admin_not_assigned:
            case tmpcontacts_obsolete:
            case period_too_short:
                return true;
        }
        return false;
    }
};

inline unsigned to_description_db_id(const Reason::Enum value)
{
    return Reason::is_valid(value) ? static_cast< unsigned >(value)
                                   : throw InvalidReasonValue();
}

template < class HOST >
typename HOST::Enum from_description_db_id(unsigned);

template < >
inline Reason::Enum from_description_db_id< Reason >(unsigned description_db_id)
{
    const Reason::Enum result = static_cast< Reason::Enum >(description_db_id);
    return Reason::is_valid(result) ? result
                                    : throw UnknownLocalizedDescriptionId();
}

}

#endif
