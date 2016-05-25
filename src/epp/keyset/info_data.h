/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef INFO_DATA_H_ECB1C74D6310A59EBF2086241AA09E20//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define INFO_DATA_H_ECB1C74D6310A59EBF2086241AA09E20

#include "src/epp/keyset/ds_record.h"
#include "src/epp/keyset/dns_key.h"
#include "src/fredlib/object/object_state.h"
#include "util/db/nullable.h"

#include <string>
#include <set>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

struct KeysetInfoData
{
    typedef std::set< Fred::Object_State::Enum > States;
    typedef std::set< KeySet::DsRecord > DsRecords;
    typedef std::set< KeySet::DnsKey > DnsKeys;
    typedef std::set< std::string > TechContacts;

    KeysetInfoData(
            const std::string &_handle,
            const std::string &_roid,
            const std::string &_sponsoring_registrar_handle,
            const std::string &_creating_registrar_handle,
            const Nullable< std::string > &_last_update_registrar_handle,
            const States &_states,
            const boost::posix_time::ptime &_crdate,
            const Nullable< boost::posix_time::ptime > &_last_update,
            const Nullable< boost::posix_time::ptime > &_last_transfer,
            const Nullable< std::string > &_auth_info_pw,
            const DsRecords &_ds_records,
            const DnsKeys &_dns_keys,
            const TechContacts &_tech_contacts)
    :   handle(_handle),
        roid(_roid),
        sponsoring_registrar_handle(_sponsoring_registrar_handle),
        creating_registrar_handle(_creating_registrar_handle),
        last_update_registrar_handle(_last_update_registrar_handle),
        states(_states),
        crdate(_crdate),
        last_update(_last_update),
        last_transfer(_last_transfer),
        auth_info_pw(_auth_info_pw),
        ds_records(_ds_records),
        dns_keys(_dns_keys),
        tech_contacts(_tech_contacts)
    { }

    std::string handle; ///< KeySet handle
    std::string roid; ///< KeySet identifier - repository ID
    std::string sponsoring_registrar_handle; ///< registrar identifier, which has to right for change
    std::string creating_registrar_handle; ///< Registrar identifier, which created contact
    Nullable< std::string > last_update_registrar_handle; ///< Registrar identifier, which realized changes
    States states; ///< KeySet states list
    boost::posix_time::ptime crdate; ///< Creation date and time
    Nullable< boost::posix_time::ptime > last_update; ///< Date and time of last change
    Nullable< boost::posix_time::ptime > last_transfer; ///< Date and time of last transfer
    Nullable< std::string > auth_info_pw; ///< Password for keyset transfer
    DsRecords ds_records; ///< List of ds records
    DnsKeys dns_keys; ///< List of dnskeys
    TechContacts tech_contacts; ///< List of technical contacts identifier
};

}//namespace Epp

#endif//INFO_DATA_H_ECB1C74D6310A59EBF2086241AA09E20
