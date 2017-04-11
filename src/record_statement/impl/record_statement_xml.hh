/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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
 *  header of registry record statement xml implementation
 */

#ifndef RECORD_STATEMENT_XML_HH_A3016D5B689542E6948999BE031F3AAD
#define RECORD_STATEMENT_XML_HH_A3016D5B689542E6948999BE031F3AAD

#include <string>
#include <vector>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/registrar/info_registrar.h"

#include "src/fredlib/documents.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "src/fredlib/nsset/nsset_dns_host.h"
#include "src/fredlib/contact/place_address.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

namespace Fred
{
namespace RecordStatement
{
    struct TimeWithOffset
    {
        std::string time;
        std::string offset;
    };

    //split rfc3339_timestamp to time and offset
    TimeWithOffset make_time_with_offset(const std::string& rfc3339_timestamp);

    //make rfc3339 timestamp to UTC converting sql expression, fractional seconds ignored
    Database::ParamQuery make_utc_timestamp_query(const std::string& timestamp);

    boost::posix_time::ptime convert_utc_timestamp_to_local(
        Fred::OperationContext& ctx,
        const boost::posix_time::ptime& utc_timestamp,
        const std::string& local_timezone);

    std::vector<std::string> make_external_states(unsigned long long object_id, Fred::OperationContext& ctx);

    std::vector<std::string> make_historic_external_states(
            unsigned long long object_id,
            const std::string& timestamp,//rfc3339 timestamp
            Fred::OperationContext& ctx);

    struct NssetPrintoutInputData
    {
        Fred::InfoNssetOutput info;
        std::vector<Fred::InfoContactOutput> tech_contact;
        Fred::InfoRegistrarOutput sponsoring_registrar;
        std::vector<std::string> external_states;
    };

    boost::optional<NssetPrintoutInputData> make_nsset_data(
        boost::optional<std::string> nsset_handle, Fred::OperationContext& ctx);

    struct KeysetPrintoutInputData
    {
        Fred::InfoKeysetOutput info;
        std::vector<Fred::InfoContactOutput> tech_contact;
        Fred::InfoRegistrarOutput sponsoring_registrar;
        std::vector<std::string> external_states;
    };

    boost::optional<KeysetPrintoutInputData> make_keyset_data(
        boost::optional<std::string> keyset_handle, Fred::OperationContext& ctx);

    std::string domain_printout_xml(
        const Fred::InfoDomainOutput& info,
        const boost::posix_time::ptime& local_timestamp,//< must be info.utc_timestamp converted to local timezone
        const boost::posix_time::ptime& local_creation_time,
        const boost::optional<boost::posix_time::ptime>& local_update_time,
        bool is_private_printout,
        const Fred::InfoContactOutput& registrant_info,
        const std::vector<Fred::InfoContactOutput>& admin_contact_info,
        const Fred::InfoRegistrarOutput& sponsoring_registrar_info,
        const boost::optional<NssetPrintoutInputData>& nsset_data,
        const boost::optional<KeysetPrintoutInputData>& keyset_data,
        const std::vector<std::string>& external_states
        );

    std::string nsset_printout_xml(
        const NssetPrintoutInputData& nsset_input_data,
        const boost::posix_time::ptime& local_timestamp//< must be nsset_input_data.info.utc_timestamp converted to local timezone
        );

    std::string keyset_printout_xml(
        const KeysetPrintoutInputData& keyset_input_data,
        const boost::posix_time::ptime& local_timestamp//< must be keyset_input_data.info.utc_timestamp converted to local timezone
        );

    std::string contact_printout_xml(
        bool is_private_printout,
        const Fred::InfoContactOutput& info,
        const boost::posix_time::ptime& local_timestamp,//< must be info.utc_timestamp converted to local timezone
        const boost::posix_time::ptime& local_creation_time,
        const boost::optional<boost::posix_time::ptime>& local_update_time,
        const boost::optional<boost::posix_time::ptime>& local_transfer_time,
        const Fred::InfoRegistrarOutput& sponsoring_registrar_info,
        const std::vector<std::string>& external_states
        );

    std::string domain_printout_xml_with_data(
        const std::string& fqdn,
        const std::string& registry_timezone,
        bool is_private_printout,
        Fred::OperationContext& ctx,
        std::string* registrant_email_out = NULL,
        boost::posix_time::ptime* request_local_timestamp = NULL
        );

    std::string nsset_printout_xml_with_data(
        const std::string& handle,
        const std::string& registry_timezone,
        Fred::OperationContext& ctx,
        std::vector<std::string>* email_out = NULL,
        boost::posix_time::ptime* request_local_timestamp = NULL
        );

    std::string keyset_printout_xml_with_data(
        const std::string& handle,
        const std::string& registry_timezone,
        Fred::OperationContext& ctx,
        std::vector<std::string>* email_out = NULL,
        boost::posix_time::ptime* request_local_timestamp = NULL
        );

    std::string contact_printout_xml_with_data(
        const std::string& handle,
        const std::string& registry_timezone,
        bool is_private_printout,
        Fred::OperationContext& ctx,
        std::string* email_out = NULL,
        boost::posix_time::ptime* request_local_timestamp = NULL
        );

}//namespace RecordStatement
}//namespace Fred

#endif
