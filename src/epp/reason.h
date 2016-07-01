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
 */

#ifndef EPP_REASON_H_976354250151
#define EPP_REASON_H_976354250151

#include "src/epp/exception.h"

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
        min_two_dns_server          = 12,
        bad_period                  = 13,
        period_range                = 14,
        period_policy               = 15,
        country_notexist            = 16,
        msgid_notexist              = 17,
        valexpdate_not_used         = 18,
        valexpdate_not_valid        = 19,
        valexpdate_required         = 20,
        can_not_rem_dns             = 21,
        can_not_add_dns             = 22,
        can_not_remove_tech         = 23,
        tech_exist                  = 24,
        tech_notexist               = 25,
        admin_exist                 = 26,
        admin_notexist              = 27,
        nsset_notexist              = 28,
        registrant_notexist         = 29,
        dns_name_exist              = 30,
        dns_name_notexist           = 31,
        curexpdate_not_expdate      = 32,
        transfer_op                 = 33,
        contact_identtype_missing   = 34,
        poll_msgid_missing          = 35,
        blacklisted_domain          = 36,
        xml_validity_error          = 37,
        duplicated_contact          = 38,
        bad_format_keyset_handle    = 39,
        keyset_notexist             = 40,
        no_dsrecord                 = 41,
        can_not_rem_dsrecord        = 42,
        duplicated_dsrecord         = 43,
        dsrecord_exist              = 44,
        dsrecord_notexist           = 45,
        dsrecord_bad_digest_type    = 46,
        dsrecord_bad_digest_length  = 47,
        unauthorized_registrar      = 48,
        techadmin_limit             = 49,
        dsrecord_limit              = 50,
        dnskey_limit                = 51,
        nsset_limit                 = 52,
        no_dnskey                   = 53,
        dnskey_bad_flags            = 54,
        dnskey_bad_protocol         = 55,
        dnskey_bad_alg              = 56,
        dnskey_bad_key_len          = 57,
        dnskey_bad_key_char         = 58,
        dnskey_exist                = 59,
        dnskey_notexist             = 60,
        duplicated_dnskey           = 61,
        no_dnskey_dsrecord          = 62
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
            case min_two_dns_server:
            case bad_period:
            case period_range:
            case period_policy:
            case country_notexist:
            case msgid_notexist:
            case valexpdate_not_used:
            case valexpdate_not_valid:
            case valexpdate_required:
            case can_not_rem_dns:
            case can_not_add_dns:
            case can_not_remove_tech:
            case tech_exist:
            case tech_notexist:
            case admin_exist:
            case admin_notexist:
            case nsset_notexist:
            case registrant_notexist:
            case dns_name_exist:
            case dns_name_notexist:
            case curexpdate_not_expdate:
            case transfer_op:
            case contact_identtype_missing:
            case poll_msgid_missing:
            case blacklisted_domain:
            case xml_validity_error:
            case duplicated_contact:
            case bad_format_keyset_handle:
            case keyset_notexist:
            case no_dsrecord:
            case can_not_rem_dsrecord:
            case duplicated_dsrecord:
            case dsrecord_exist:
            case dsrecord_notexist:
            case dsrecord_bad_digest_type:
            case dsrecord_bad_digest_length:
            case unauthorized_registrar:
            case techadmin_limit:
            case dsrecord_limit:
            case dnskey_limit:
            case nsset_limit:
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

}

#endif
