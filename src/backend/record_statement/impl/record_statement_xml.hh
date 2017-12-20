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

#include "src/libfred/documents.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/object/object_type.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/contact/place_address.hh"
#include "src/libfred/registrable_object/domain/enum_validation_extension.hh"
#include "src/libfred/registrable_object/domain/info_domain.hh"
#include "src/libfred/registrable_object/nsset/nsset_dns_host.hh"
#include "src/libfred/registrable_object/nsset/info_nsset.hh"
#include "src/libfred/registrable_object/keyset/info_keyset.hh"
#include "src/libfred/registrar/info_registrar.hh"

#include "src/backend/record_statement/record_statement.hh"

#include "src/util/xmlgen.hh"
#include "src/util/db/nullable.hh"
#include "src/util/optional_value.hh"
#include "src/util/tz/local_timestamp.hh"
#include "src/util/tz/get_psql_handle_of.hh"
#include "src/util/tz/utc.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <string>
#include <set>
#include <vector>

namespace LibFred {
namespace RecordStatement {
namespace Impl {

struct NssetPrintoutInputData
{
    LibFred::InfoNssetOutput info;
    std::vector<LibFred::InfoContactOutput> tech_contact;
    LibFred::InfoRegistrarOutput sponsoring_registrar;
    std::set<std::string> external_states;
};

boost::optional<NssetPrintoutInputData> make_nsset_data(
        const boost::optional<std::string>& nsset_handle,
        LibFred::OperationContext& ctx);

struct KeysetPrintoutInputData
{
    LibFred::InfoKeysetOutput info;
    std::vector<LibFred::InfoContactOutput> tech_contact;
    LibFred::InfoRegistrarOutput sponsoring_registrar;
    std::set<std::string> external_states;
};

boost::optional<KeysetPrintoutInputData> make_keyset_data(
        const boost::optional<std::string>& keyset_handle,
        LibFred::OperationContext& ctx);

template <typename REGISTRY_TIMEZONE>
std::string domain_printout_xml(
        LibFred::OperationContext& ctx,
        const LibFred::InfoDomainOutput& info,
        const Tz::LocalTimestamp& valid_at,
        Registry::RecordStatement::Purpose::Enum purpose,
        const LibFred::InfoContactOutput& registrant_info,
        const std::vector<LibFred::InfoContactOutput>& admin_contact_info,
        const LibFred::InfoRegistrarOutput& sponsoring_registrar_info,
        const boost::optional<NssetPrintoutInputData>& nsset_data,
        const boost::optional<KeysetPrintoutInputData>& keyset_data,
        const std::set<std::string>& external_states);

std::string nsset_printout_xml(
        const NssetPrintoutInputData& nsset_input_data,
        const Tz::LocalTimestamp& valid_at);

std::string keyset_printout_xml(
        const KeysetPrintoutInputData& keyset_input_data,
        const Tz::LocalTimestamp& valid_at);

template <typename REGISTRY_TIMEZONE>
std::string contact_printout_xml(
        LibFred::OperationContext& ctx,
        const LibFred::InfoContactOutput& info,
        const Tz::LocalTimestamp& valid_at,
        Registry::RecordStatement::Purpose::Enum purpose,
        const LibFred::InfoRegistrarOutput& sponsoring_registrar_info,
        const std::set<std::string>& external_states);

struct XmlWithData
{
    XmlWithData();
    std::string xml;
    std::set<std::string> email_out;
    Tz::LocalTimestamp request_local_timestamp;
};

template <typename REGISTRY_TIMEZONE>
XmlWithData domain_printout_xml_with_data(
        LibFred::OperationContext& ctx,
        const std::string& fqdn,
        Registry::RecordStatement::Purpose::Enum purpose);

template <typename REGISTRY_TIMEZONE>
XmlWithData nsset_printout_xml_with_data(
        LibFred::OperationContext& ctx,
        const std::string& handle);

template <typename REGISTRY_TIMEZONE>
XmlWithData keyset_printout_xml_with_data(
        LibFred::OperationContext& ctx,
        const std::string& handle);

template <typename REGISTRY_TIMEZONE>
XmlWithData contact_printout_xml_with_data(
        LibFred::OperationContext& ctx,
        const std::string& handle,
        Registry::RecordStatement::Purpose::Enum purpose);

std::set<std::string> make_external_states(
        unsigned long long object_id,
        LibFred::OperationContext& ctx);

std::set<std::string> make_historic_external_states(
        unsigned long long object_id,
        const Tz::LocalTimestamp& valid_at,
        LibFred::OperationContext& ctx);

template <typename LOCAL_TIMEZONE>
Tz::LocalTimestamp convert_utc_timestamp_to_local(
        LibFred::OperationContext& ctx,
        const boost::posix_time::ptime& utc_timestamp);

template <>
Tz::LocalTimestamp convert_utc_timestamp_to_local<Tz::UTC>(
        LibFred::OperationContext&,
        const boost::posix_time::ptime& utc_timestamp);

std::vector<Util::XmlCallback> external_states_xml(
        const std::set<std::string>& external_states);

std::vector<Util::XmlCallback> nsset_xml(
        const boost::optional<NssetPrintoutInputData>& nsset_data);

std::vector<Util::XmlCallback> keyset_xml(
        const boost::optional<KeysetPrintoutInputData>& keyset_data);

class DbDateTimeArithmetic
{
public:
    DbDateTimeArithmetic(LibFred::OperationContext& _ctx);
    template <typename SRC_TIMEZONE>
    Tz::LocalTimestamp within(const boost::posix_time::ptime& _local_time)const
    {
        return this->append_offset(_local_time, Tz::get_psql_handle_of<SRC_TIMEZONE>());
    }
    template <typename DST_TIMEZONE>
    Tz::LocalTimestamp into(
            const boost::posix_time::ptime& _src_local_time,
            ::int16_t _src_timezone_offset_in_minutes)const
    {
        return this->convert_into_other_timezone(
                _src_local_time,
                _src_timezone_offset_in_minutes,
                Tz::get_psql_handle_of<DST_TIMEZONE>());
    }
private:
    Tz::LocalTimestamp append_offset(
            const boost::posix_time::ptime& _local_time,
            const std::string& _timezone_handle)const;
    Tz::LocalTimestamp convert_into_other_timezone(
            const boost::posix_time::ptime& _src_local_time,
            ::int16_t _src_timezone_offset_in_minutes,
            const std::string& _dst_timezone_handle)const;
    LibFred::OperationContext& ctx_;
};

template <typename T>
class ImplementationWithin:public Registry::RecordStatement::RecordStatementImpl::WithExternalContext
{
public:
    typedef T RegistryTimeZone;
    ImplementationWithin(
            const boost::shared_ptr<LibFred::Document::Manager>& _doc_manager,
            const boost::shared_ptr<LibFred::Mailer::Manager>& _mailer_manager);
    ~ImplementationWithin();

    Registry::RecordStatement::PdfBufferImpl domain_printout(
            LibFred::OperationContext& _ctx,
            const std::string& _fqdn,
            Registry::RecordStatement::Purpose::Enum _purpose)const;

    Registry::RecordStatement::PdfBufferImpl nsset_printout(
            LibFred::OperationContext& _ctx,
            const std::string& _handle)const;

    Registry::RecordStatement::PdfBufferImpl keyset_printout(
            LibFred::OperationContext& _ctx,
            const std::string& _handle)const;

    Registry::RecordStatement::PdfBufferImpl contact_printout(
            LibFred::OperationContext& _ctx,
            const std::string& _handle,
            Registry::RecordStatement::Purpose::Enum _purpose)const;

    Registry::RecordStatement::PdfBufferImpl historic_domain_printout(
            LibFred::OperationContext& _ctx,
            const std::string& _fqdn,
            const Tz::LocalTimestamp& _valid_at)const;

    Registry::RecordStatement::PdfBufferImpl historic_nsset_printout(
            LibFred::OperationContext& _ctx,
            const std::string& _handle,
            const Tz::LocalTimestamp& _valid_at)const;

    Registry::RecordStatement::PdfBufferImpl historic_keyset_printout(
            LibFred::OperationContext& _ctx,
            const std::string& _handle,
            const Tz::LocalTimestamp& _valid_at)const;

    Registry::RecordStatement::PdfBufferImpl historic_contact_printout(
            LibFred::OperationContext& _ctx,
            const std::string& _handle,
            const Tz::LocalTimestamp& _valid_at)const;

    void send_domain_printout(
            LibFred::OperationContext& _ctx,
            const std::string& _fqdn,
            Registry::RecordStatement::Purpose::Enum _purpose)const;

    void send_nsset_printout(
            LibFred::OperationContext& _ctx,
            const std::string& _handle)const;

    void send_keyset_printout(
            LibFred::OperationContext& _ctx,
            const std::string& _handle)const;

    void send_contact_printout(
            LibFred::OperationContext& _ctx,
            const std::string& _handle,
            Registry::RecordStatement::Purpose::Enum _purpose)const;
private:
    boost::shared_ptr<LibFred::Document::Manager> doc_manager_;
    boost::shared_ptr<LibFred::Mailer::Manager> mailer_manager_;
};

template <LibFred::Object_Type::Enum object_type>
unsigned long long get_history_id_of(
        const std::string& handle,
        const Tz::LocalTimestamp& valid_at,
        LibFred::OperationContext& ctx);

template <>
unsigned long long get_history_id_of<LibFred::Object_Type::domain>(
        const std::string& fqdn,
        const Tz::LocalTimestamp& valid_at,
        LibFred::OperationContext& ctx);

template <LibFred::Object_Type::Enum object_type>
unsigned long long get_history_id_internal_of(
        const std::string& object_name,
        const Tz::LocalTimestamp& valid_at,
        LibFred::OperationContext& ctx);

boost::optional<NssetPrintoutInputData> make_historic_nsset_data(
        const boost::optional<unsigned long long>& nsset_historyid,
        const Tz::LocalTimestamp& timestamp, //for nsset tech contact and nsset states history
        LibFred::OperationContext& ctx);

boost::optional<KeysetPrintoutInputData> make_historic_keyset_data(
        const boost::optional<unsigned long long>& keyset_historyid,
        const Tz::LocalTimestamp& timestamp, //for keyset tech contact and keyset states history
        LibFred::OperationContext& ctx);

} // namespace LibFred::RecordStatement::Impl
} // namespace LibFred::RecordStatement
} // namespace LibFred

#endif
