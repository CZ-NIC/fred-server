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

#include "src/fredlib/documents.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/place_address.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/nsset/nsset_dns_host.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/registrar/info_registrar.h"

#include "src/record_statement/record_statement.hh"

#include "util/xmlgen.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"
#include "util/tz/local_timestamp.hh"
#include "util/tz/get_psql_handle_of.hh"
#include "util/tz/utc.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <string>
#include <set>
#include <vector>

namespace Fred {
namespace RecordStatement {
namespace Impl {

struct NssetPrintoutInputData
{
    Fred::InfoNssetOutput info;
    std::vector<Fred::InfoContactOutput> tech_contact;
    Fred::InfoRegistrarOutput sponsoring_registrar;
    std::set<std::string> external_states;
};

boost::optional<NssetPrintoutInputData> make_nsset_data(
        const boost::optional<std::string>& nsset_handle,
        Fred::OperationContext& ctx);

struct KeysetPrintoutInputData
{
    Fred::InfoKeysetOutput info;
    std::vector<Fred::InfoContactOutput> tech_contact;
    Fred::InfoRegistrarOutput sponsoring_registrar;
    std::set<std::string> external_states;
};

boost::optional<KeysetPrintoutInputData> make_keyset_data(
        const boost::optional<std::string>& keyset_handle,
        Fred::OperationContext& ctx);

template <typename REGISTRY_TIMEZONE>
std::string domain_printout_xml(
        Fred::OperationContext& ctx,
        const Fred::InfoDomainOutput& info,
        const Tz::LocalTimestamp& valid_at,
        Registry::RecordStatement::Purpose::Enum purpose,
        const Fred::InfoContactOutput& registrant_info,
        const std::vector<Fred::InfoContactOutput>& admin_contact_info,
        const Fred::InfoRegistrarOutput& sponsoring_registrar_info,
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
        Fred::OperationContext& ctx,
        const Fred::InfoContactOutput& info,
        const Tz::LocalTimestamp& valid_at,
        Registry::RecordStatement::Purpose::Enum purpose,
        const Fred::InfoRegistrarOutput& sponsoring_registrar_info,
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
        Fred::OperationContext& ctx,
        const std::string& fqdn,
        Registry::RecordStatement::Purpose::Enum purpose);

template <typename REGISTRY_TIMEZONE>
XmlWithData nsset_printout_xml_with_data(
        Fred::OperationContext& ctx,
        const std::string& handle);

template <typename REGISTRY_TIMEZONE>
XmlWithData keyset_printout_xml_with_data(
        Fred::OperationContext& ctx,
        const std::string& handle);

template <typename REGISTRY_TIMEZONE>
XmlWithData contact_printout_xml_with_data(
        Fred::OperationContext& ctx,
        const std::string& handle,
        Registry::RecordStatement::Purpose::Enum purpose);

std::set<std::string> make_external_states(
        unsigned long long object_id,
        Fred::OperationContext& ctx);

std::set<std::string> make_historic_external_states(
        unsigned long long object_id,
        const Tz::LocalTimestamp& valid_at,
        Fred::OperationContext& ctx);

template <typename LOCAL_TIMEZONE>
Tz::LocalTimestamp convert_utc_timestamp_to_local(
        Fred::OperationContext& ctx,
        const boost::posix_time::ptime& utc_timestamp);

template <>
Tz::LocalTimestamp convert_utc_timestamp_to_local<Tz::UTC>(
        Fred::OperationContext&,
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
    DbDateTimeArithmetic(Fred::OperationContext& _ctx);
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
    Fred::OperationContext& ctx_;
};

template <typename T>
class ImplementationWithin:public Registry::RecordStatement::RecordStatementImpl::WithExternalContext
{
public:
    typedef T RegistryTimeZone;
    ImplementationWithin(
            const boost::shared_ptr<Fred::Document::Manager>& _doc_manager,
            const boost::shared_ptr<Fred::Mailer::Manager>& _mailer_manager);
    ~ImplementationWithin();

    Registry::RecordStatement::PdfBufferImpl domain_printout(
            Fred::OperationContext& _ctx,
            const std::string& _fqdn,
            Registry::RecordStatement::Purpose::Enum _purpose)const;

    Registry::RecordStatement::PdfBufferImpl nsset_printout(
            Fred::OperationContext& _ctx,
            const std::string& _handle)const;

    Registry::RecordStatement::PdfBufferImpl keyset_printout(
            Fred::OperationContext& _ctx,
            const std::string& _handle)const;

    Registry::RecordStatement::PdfBufferImpl contact_printout(
            Fred::OperationContext& _ctx,
            const std::string& _handle,
            Registry::RecordStatement::Purpose::Enum _purpose)const;

    Registry::RecordStatement::PdfBufferImpl historic_domain_printout(
            Fred::OperationContext& _ctx,
            const std::string& _fqdn,
            const Tz::LocalTimestamp& _valid_at)const;

    Registry::RecordStatement::PdfBufferImpl historic_nsset_printout(
            Fred::OperationContext& _ctx,
            const std::string& _handle,
            const Tz::LocalTimestamp& _valid_at)const;

    Registry::RecordStatement::PdfBufferImpl historic_keyset_printout(
            Fred::OperationContext& _ctx,
            const std::string& _handle,
            const Tz::LocalTimestamp& _valid_at)const;

    Registry::RecordStatement::PdfBufferImpl historic_contact_printout(
            Fred::OperationContext& _ctx,
            const std::string& _handle,
            const Tz::LocalTimestamp& _valid_at)const;

    void send_domain_printout(
            Fred::OperationContext& _ctx,
            const std::string& _fqdn,
            Registry::RecordStatement::Purpose::Enum _purpose)const;

    void send_nsset_printout(
            Fred::OperationContext& _ctx,
            const std::string& _handle)const;

    void send_keyset_printout(
            Fred::OperationContext& _ctx,
            const std::string& _handle)const;

    void send_contact_printout(
            Fred::OperationContext& _ctx,
            const std::string& _handle,
            Registry::RecordStatement::Purpose::Enum _purpose)const;
private:
    boost::shared_ptr<Fred::Document::Manager> doc_manager_;
    boost::shared_ptr<Fred::Mailer::Manager> mailer_manager_;
};

template <Fred::Object_Type::Enum object_type>
unsigned long long get_history_id_of(
        const std::string& handle,
        const Tz::LocalTimestamp& valid_at,
        Fred::OperationContext& ctx);

template <>
unsigned long long get_history_id_of<Fred::Object_Type::domain>(
        const std::string& fqdn,
        const Tz::LocalTimestamp& valid_at,
        Fred::OperationContext& ctx);

template <Fred::Object_Type::Enum object_type>
unsigned long long get_history_id_internal_of(
        const std::string& object_name,
        const Tz::LocalTimestamp& valid_at,
        Fred::OperationContext& ctx);

boost::optional<NssetPrintoutInputData> make_historic_nsset_data(
        const boost::optional<unsigned long long>& nsset_historyid,
        const Tz::LocalTimestamp& timestamp, //for nsset tech contact and nsset states history
        Fred::OperationContext& ctx);

boost::optional<KeysetPrintoutInputData> make_historic_keyset_data(
        const boost::optional<unsigned long long>& keyset_historyid,
        const Tz::LocalTimestamp& timestamp, //for keyset tech contact and keyset states history
        Fred::OperationContext& ctx);

}//namespace Fred::RecordStatement::Impl
}//namespace Fred::RecordStatement
}//namespace Fred

#endif
