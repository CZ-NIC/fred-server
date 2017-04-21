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

#ifndef INFO_KEYSET_OUTPUT_DATA_H_95C2E2D9FC0045CA9DFC4AF376229616
#define INFO_KEYSET_OUTPUT_DATA_H_95C2E2D9FC0045CA9DFC4AF376229616

#include "src/epp/keyset/dns_key.h"
#include "src/epp/keyset/ds_record.h"
#include "src/epp/keyset/status_value.h"
#include "util/db/nullable.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <set>
#include <string>

namespace Epp {
namespace Keyset {

struct InfoKeysetOutputData
{

    typedef std::set<StatusValue::Enum> States;
    typedef std::set<Keyset::DsRecord> DsRecords;
    typedef std::set<Keyset::DnsKey> DnsKeys;
    typedef std::set<std::string> TechContacts;

    std::string handle; ///< Keyset handle
    std::string roid; ///< Keyset identifier - repository ID
    std::string sponsoring_registrar_handle; ///< registrar identifier, which has to right for change
    std::string creating_registrar_handle; ///< Registrar identifier, which created contact
    Nullable<std::string> last_update_registrar_handle; ///< Registrar identifier, which realized changes
    States states; ///< Keyset states list
    boost::posix_time::ptime crdate; ///< Creation date and time
    Nullable<boost::posix_time::ptime> last_update; ///< Date and time of last change
    Nullable<boost::posix_time::ptime> last_transfer; ///< Date and time of last transfer
    boost::optional<std::string> authinfopw; ///< Password for keyset transfer
    DsRecords ds_records; ///< List of ds records
    DnsKeys dns_keys; ///< List of dnskeys
    TechContacts tech_contacts; ///< List of technical contacts identifier

};

} // namespace Epp::Keyset
} // namespace Epp

#endif
