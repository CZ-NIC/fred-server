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
 *  registry record statement xml implementation
 */

#include "src/record_statement/impl/record_statement_xml.hh"
#include "src/record_statement/exceptions.hh"

#include "util/log/context.h"
#include "util/util.h"
#include "util/xmlgen.h"
#include "util/timezones.hh"
#include "util/tz/get_psql_handle_of.hh"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/zone/zone.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/object_state/get_object_states.h"

#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>


namespace Fred {
namespace RecordStatement {
namespace Impl {

std::set<std::string> make_external_states(
        unsigned long long object_id,
        Fred::OperationContext& ctx)
{
    std::set<std::string> ret;

    const std::vector<Fred::ObjectStateData> states = Fred::GetObjectStates(object_id).exec(ctx);
    for (std::vector<Fred::ObjectStateData>::const_iterator states_itr = states.begin();
         states_itr != states.end(); ++states_itr)
    {
        if (states_itr->is_external)
        {
            ret.insert(states_itr->state_name);
        }
    }

    return ret;
}

std::set<std::string> make_historic_external_states(
        const unsigned long long object_id,
        const Tz::LocalTimestamp& valid_at,
        Fred::OperationContext& ctx)
{
    const Database::Result db_res = ctx.get_conn().exec_params(
            "WITH valid_at AS ("
                "SELECT $1::TIMESTAMP-($2::INT||'MINUTES')::INTERVAL AS utc_time) "
            "SELECT DISTINCT eos.name "
            "FROM valid_at v "
            "JOIN object_state os ON os.valid_from<(v.utc_time+'1SECOND'::INTERVAL) AND "
                                    "(v.utc_time<os.valid_to OR os.valid_to IS NULL) "
            "JOIN enum_object_states eos ON eos.id=os.state_id "
            "WHERE os.object_id=$3::BIGINT AND "
                  "eos.external",
            Database::query_param_list(boost::posix_time::to_iso_extended_string(valid_at.get_local_time()))
                                      (valid_at.get_timezone_offset_in_minutes())
                                      (object_id));

    std::set<std::string> historic_external_states;
    for (unsigned long long idx = 0 ; idx < db_res.size() ; ++idx)
    {
        historic_external_states.insert(static_cast<std::string>(db_res[idx][0]));
    }

    return historic_external_states;
}

boost::optional<NssetPrintoutInputData> make_nsset_data(
        const boost::optional<std::string>& nsset_handle,
        Fred::OperationContext& ctx)
{
    if (!static_cast<bool>(nsset_handle))
    {
        return boost::optional<NssetPrintoutInputData>();
    }

    NssetPrintoutInputData retval;
    retval.info = Fred::InfoNssetByHandle(*nsset_handle).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());
    retval.tech_contact.reserve(retval.info.info_nsset_data.tech_contacts.size());

    for (std::vector<Fred::ObjectIdHandlePair>::const_iterator itr = retval.info.info_nsset_data.tech_contacts.begin();
         itr != retval.info.info_nsset_data.tech_contacts.end(); ++itr)
    {
        retval.tech_contact.push_back(Fred::InfoContactByHandle(itr->handle).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>()));
    }

    retval.sponsoring_registrar = Fred::InfoRegistrarByHandle(
            retval.info.info_nsset_data.sponsoring_registrar_handle).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());

    retval.external_states = make_external_states(retval.info.info_nsset_data.id, ctx);

    return retval;
}

boost::optional<KeysetPrintoutInputData> make_keyset_data(
        const boost::optional<std::string>& keyset_handle,
        Fred::OperationContext& ctx)
{
    if (!static_cast<bool>(keyset_handle))
    {
        return boost::optional<KeysetPrintoutInputData>();
    }

    KeysetPrintoutInputData retval;
    retval.info = Fred::InfoKeysetByHandle(*keyset_handle).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());
    retval.tech_contact.reserve(retval.info.info_keyset_data.tech_contacts.size());

    for (std::vector<Fred::ObjectIdHandlePair>::const_iterator itr = retval.info.info_keyset_data.tech_contacts.begin();
         itr != retval.info.info_keyset_data.tech_contacts.end(); ++itr)
    {
        retval.tech_contact.push_back(Fred::InfoContactByHandle(itr->handle).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>()));
    }

    retval.sponsoring_registrar = Fred::InfoRegistrarByHandle(
            retval.info.info_keyset_data.sponsoring_registrar_handle).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());

    retval.external_states = make_external_states(retval.info.info_keyset_data.id, ctx);
    return retval;
}

DbDateTimeArithmetic::DbDateTimeArithmetic(Fred::OperationContext& _ctx)
    : ctx_(_ctx)
{ }

Tz::LocalTimestamp DbDateTimeArithmetic::append_offset(
        const boost::posix_time::ptime& _local_time,
        const std::string& _timezone_handle)const
{
    const Database::Result dbres = ctx_.get_conn().exec_params(
            "WITH src AS (SELECT $1::TIMESTAMP AS t) "
            "SELECT EXTRACT(EPOCH FROM t-(t AT TIME ZONE $2::TEXT AT TIME ZONE $3::TEXT)) FROM src",
            Database::query_param_list(boost::posix_time::to_simple_string(_local_time))
                                      (_timezone_handle)
                                      (Tz::get_psql_handle_of<Tz::UTC>()));
    const int offset_in_seconds = static_cast<int>(dbres[0][0]);
    return Tz::LocalTimestamp(_local_time, offset_in_seconds / 60);
}

Tz::LocalTimestamp DbDateTimeArithmetic::convert_into_other_timezone(
        const boost::posix_time::ptime& _src_local_time,
        ::int16_t _src_timezone_offset_in_minutes,
        const std::string& _dst_timezone_handle)const
{
    const Database::Result dbres = ctx_.get_conn().exec_params(
            "WITH src AS (SELECT $1::TIMESTAMP AS t,($2::INT||'MINUTES')::INTERVAL AS o),"
                 "dst AS (SELECT (t-o) AT TIME ZONE $3::TEXT AT TIME ZONE $4::TEXT AS t FROM src) "
            "SELECT EXTRACT(EPOCH FROM src.t-dst.t),"//difference between both local times
                   "EXTRACT(EPOCH FROM dst.t-(src.t-src.o)) "//difference of dst local time from UTC time
            "FROM src,dst",
            Database::query_param_list(boost::posix_time::to_simple_string(_src_local_time))
                                      (_src_timezone_offset_in_minutes)
                                      (Tz::get_psql_handle_of<Tz::UTC>())
                                      (_dst_timezone_handle));
    const int diff_src_from_dst_in_seconds = static_cast<int>(dbres[0][0]);
    const int diff_dst_from_utc_in_seconds = static_cast<int>(dbres[0][1]);
    const boost::posix_time::ptime dst_local_time =
            _src_local_time - boost::posix_time::seconds(diff_src_from_dst_in_seconds);
    return Tz::LocalTimestamp(dst_local_time, diff_dst_from_utc_in_seconds / 60);
}

template <>
Tz::LocalTimestamp convert_utc_timestamp_to_local<Tz::UTC>(
        Fred::OperationContext&,
        const boost::posix_time::ptime& utc_timestamp)
{
    return Tz::LocalTimestamp::within_utc(utc_timestamp);
}

std::vector<Util::XmlCallback> external_states_xml(
        const std::set<std::string>& external_states)
{
    std::vector<Util::XmlCallback> ret;
    ret.reserve(external_states.size());
    for (std::set<std::string>::const_iterator itr = external_states.begin(); itr != external_states.end(); ++itr)
    {
        ret.push_back(Util::XmlTagPair("state", Util::XmlEscapeTag(*itr)));
    }
    return ret;
}

std::vector<Util::XmlCallback> nsset_xml(const boost::optional<NssetPrintoutInputData>& nsset_data)
{
    if (!static_cast<bool>(nsset_data))
    {
        return Util::vector_of<Util::XmlCallback>(Util::XmlTagPair("nsset", std::vector<Util::XmlCallback>()));
    }

    std::vector<Util::XmlCallback> nameserver_list;
    nameserver_list.reserve(nsset_data->info.info_nsset_data.dns_hosts.size());

    for (std::vector<Fred::DnsHost>::const_iterator itr = nsset_data->info.info_nsset_data.dns_hosts.begin();
         itr != nsset_data->info.info_nsset_data.dns_hosts.end(); ++itr)
    {
        std::vector<Util::XmlCallback> ip_list;
        const std::vector<boost::asio::ip::address> ns_ip_info = itr->get_inet_addr();
        ip_list.reserve(ns_ip_info.size());

        for (std::vector<boost::asio::ip::address>::const_iterator ip_itr = ns_ip_info.begin();
             ip_itr != ns_ip_info.end(); ++ip_itr)
        {
            ip_list.push_back(Util::XmlTagPair("ip", Util::XmlEscapeTag(ip_itr->to_string())));
        }

        nameserver_list.push_back(
                Util::XmlTagPair(
                        "nameserver",
                        Util::vector_of<Util::XmlCallback>
                            (Util::XmlTagPair("fqdn", Util::XmlEscapeTag(itr->get_fqdn())))
                            (Util::XmlTagPair("ip_list", ip_list))));
    }

    std::vector<Util::XmlCallback> tech_contact_list;
    tech_contact_list.reserve(nsset_data->tech_contact.size());

    for (std::vector<Fred::InfoContactOutput>::const_iterator itr = nsset_data->tech_contact.begin();
         itr != nsset_data->tech_contact.end(); ++itr)
    {
        tech_contact_list.push_back(
                Util::XmlTagPair(
                        "tech_contact",
                        Util::vector_of<Util::XmlCallback>
                            (Util::XmlTagPair("handle", Util::XmlEscapeTag(itr->info_contact_data.handle)))
                            (Util::XmlTagPair("name", Util::XmlEscapeTag(itr->info_contact_data.name.get_value_or(""))))
                            (Util::XmlTagPair("organization",
                                              Util::XmlEscapeTag(itr->info_contact_data.organization.get_value_or(""))))));
    }

    return Util::vector_of<Util::XmlCallback>
        (Util::XmlTagPair(
                "nsset",
                Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair(
                            "handle",
                            Util::XmlEscapeTag(nsset_data->info.info_nsset_data.handle)))
                    (Util::XmlTagPair(
                            "nameserver_list",
                            nameserver_list))
                    (Util::XmlTagPair(
                            "tech_contact_list",
                            tech_contact_list))
                    (Util::XmlTagPair(
                            "sponsoring_registrar",
                            Util::vector_of<Util::XmlCallback>
                                (Util::XmlTagPair(
                                        "handle",
                                        Util::XmlEscapeTag(
                                                nsset_data->sponsoring_registrar.info_registrar_data.handle)))
                                (Util::XmlTagPair(
                                        "name",
                                        Util::XmlEscapeTag(
                                                nsset_data->sponsoring_registrar.info_registrar_data.name.get_value_or(""))))
                                (Util::XmlTagPair(
                                        "organization",
                                        Util::XmlEscapeTag(
                                                nsset_data->sponsoring_registrar.info_registrar_data.organization.get_value_or(""))))))
                    (Util::XmlTagPair("external_states_list", external_states_xml(nsset_data->external_states)))));
}

std::vector<Util::XmlCallback> keyset_xml(const boost::optional<KeysetPrintoutInputData>& keyset_data)
{
    if (!static_cast<bool>(keyset_data))
    {
        return Util::vector_of<Util::XmlCallback>(Util::XmlTagPair("keyset", std::vector<Util::XmlCallback>()));
    }

    std::vector<Util::XmlCallback> dns_key_list;
    dns_key_list.reserve(keyset_data->info.info_keyset_data.dns_keys.size());

    for (std::vector<Fred::DnsKey>::const_iterator itr = keyset_data->info.info_keyset_data.dns_keys.begin();
         itr != keyset_data->info.info_keyset_data.dns_keys.end(); ++itr)
    {
        dns_key_list.push_back(
                Util::XmlTagPair("dns_key", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("flags", Util::XmlEscapeTag(boost::lexical_cast<std::string>(itr->get_flags()))))
                    (Util::XmlTagPair("protocol", Util::XmlEscapeTag(boost::lexical_cast<std::string>(itr->get_protocol()))))
                    (Util::XmlTagPair("algorithm", Util::XmlEscapeTag(boost::lexical_cast<std::string>(itr->get_alg()))))
                    (Util::XmlTagPair("key", Util::XmlEscapeTag(itr->get_key())))));
    }

    std::vector<Util::XmlCallback> tech_contact_list;
    tech_contact_list.reserve(keyset_data->tech_contact.size());

    for (std::vector<Fred::InfoContactOutput>::const_iterator itr = keyset_data->tech_contact.begin();
         itr != keyset_data->tech_contact.end(); ++itr)
    {
        tech_contact_list.push_back(
                Util::XmlTagPair(
                        "tech_contact",
                        Util::vector_of<Util::XmlCallback>
                            (Util::XmlTagPair("handle", Util::XmlEscapeTag(itr->info_contact_data.handle)))
                            (Util::XmlTagPair("name", Util::XmlEscapeTag(itr->info_contact_data.name.get_value_or(""))))
                            (Util::XmlTagPair("organization",
                                              Util::XmlEscapeTag(itr->info_contact_data.organization.get_value_or(""))))));
    }

    return Util::vector_of<Util::XmlCallback>
        (Util::XmlTagPair(
                "keyset",
                Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair(
                            "handle",
                            Util::XmlEscapeTag(keyset_data->info.info_keyset_data.handle)))
                    (Util::XmlTagPair(
                            "dns_key_list",
                            dns_key_list))
                    (Util::XmlTagPair(
                            "tech_contact_list",
                            tech_contact_list))
                    (Util::XmlTagPair(
                            "sponsoring_registrar",
                            Util::vector_of<Util::XmlCallback>
                                (Util::XmlTagPair(
                                        "handle",
                                        Util::XmlEscapeTag(
                                                keyset_data->sponsoring_registrar.info_registrar_data.handle)))
                                (Util::XmlTagPair(
                                        "name",
                                        Util::XmlEscapeTag(
                                                keyset_data->sponsoring_registrar.info_registrar_data.name.get_value_or(""))))
                                (Util::XmlTagPair(
                                        "organization",
                                        Util::XmlEscapeTag(
                                                keyset_data->sponsoring_registrar.info_registrar_data.organization.get_value_or(""))))))
                    (Util::XmlTagPair("external_states_list", external_states_xml(keyset_data->external_states)))));
}

std::string nsset_printout_xml(
        const NssetPrintoutInputData& nsset_input_data,
        const Tz::LocalTimestamp& valid_at)
{
    std::string printout_xml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    Util::XmlTagPair(
            "record_statement",
            Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair(
                        "current_datetime",
                        Util::XmlEscapeTag(valid_at.get_rfc3339_formated_string())))
                (nsset_xml(nsset_input_data)))
    (printout_xml);

    return printout_xml;
}

std::string keyset_printout_xml(
        const KeysetPrintoutInputData& keyset_input_data,
        const Tz::LocalTimestamp& valid_at)
{
    std::string printout_xml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    Util::XmlTagPair(
            "record_statement",
            Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair(
                        "current_datetime",
                        Util::XmlEscapeTag(valid_at.get_rfc3339_formated_string())))
                (keyset_xml(keyset_input_data)))
    (printout_xml);

    return printout_xml;
}

XmlWithData::XmlWithData()
    : xml(),
      email_out(),
      request_local_timestamp(Tz::LocalTimestamp::within_utc(boost::posix_time::ptime()))
{ }

template <Fred::Object_Type::Enum object_type>
unsigned long long get_history_id_of(
        const std::string& handle,
        const Tz::LocalTimestamp& valid_at,
        Fred::OperationContext& ctx)
{
    const Database::Result db_res = ctx.get_conn().exec_params(
            "WITH "
                "valid_at AS ("
                    "SELECT $1::TIMESTAMP-($2::INT||'MINUTES')::INTERVAL AS utc_time),"
                "obr AS ("
                    "SELECT MAX(obr.id) AS id "
                    "FROM object_registry obr "
                    "JOIN valid_at v ON obr.crdate<(v.utc_time+'1SECOND'::INTERVAL) AND "
                                       "(v.utc_time<obr.erdate OR obr.erdate IS NULL) "
                    "WHERE obr.type=get_object_type_id($3::TEXT) AND "
                          "UPPER(obr.name)=UPPER($4::TEXT)) "
            "SELECT h.id "
            "FROM obr "
            "JOIN object_history oh ON oh.id=obr.id "
            "JOIN history h ON h.id=oh.historyid "
            "JOIN valid_at v ON h.valid_from<(v.utc_time+'1SECOND'::INTERVAL) AND "
                               "(v.utc_time<h.valid_to OR h.valid_to IS NULL) "
            "ORDER BY h.id DESC "
            "LIMIT 1",
            Database::query_param_list(boost::posix_time::to_iso_extended_string(valid_at.get_local_time()))
                                      (valid_at.get_timezone_offset_in_minutes())
                                      (Conversion::Enums::to_db_handle(object_type))
                                      (handle));

    if (db_res.size() == 0)
    {
        throw Registry::RecordStatement::ObjectNotFound();
    }

    return static_cast<unsigned long long>(db_res[0][0]);
}

template unsigned long long get_history_id_of<Fred::Object_Type::contact>(
        const std::string&, const Tz::LocalTimestamp&, Fred::OperationContext&);
template unsigned long long get_history_id_of<Fred::Object_Type::keyset>(
        const std::string&, const Tz::LocalTimestamp&, Fred::OperationContext&);
template unsigned long long get_history_id_of<Fred::Object_Type::nsset>(
        const std::string&, const Tz::LocalTimestamp&, Fred::OperationContext&);

template <>
unsigned long long get_history_id_of<Fred::Object_Type::domain>(
        const std::string& fqdn,
        const Tz::LocalTimestamp& valid_at,
        Fred::OperationContext& ctx)
{
    const Database::Result db_res = ctx.get_conn().exec_params(
            "WITH "
                "valid_at AS ("
                    "SELECT $1::TIMESTAMP-($2::INT||'MINUTES')::INTERVAL AS utc_time),"
                "obr AS ("
                    "SELECT MAX(obr.id) AS id "
                    "FROM object_registry obr "
                    "JOIN valid_at v ON obr.crdate<(v.utc_time+'1SECOND'::INTERVAL) AND "
                                       "(v.utc_time<obr.erdate OR obr.erdate IS NULL) "
                    "WHERE obr.type=get_object_type_id($3::TEXT) AND "
                          "obr.name=LOWER($4::TEXT)) "
            "SELECT h.id "
            "FROM obr "
            "JOIN object_history oh ON oh.id=obr.id "
            "JOIN history h ON h.id=oh.historyid "
            "JOIN valid_at v ON h.valid_from<(v.utc_time+'1SECOND'::INTERVAL) AND "
                               "(v.utc_time<h.valid_to OR h.valid_to IS NULL) "
            "ORDER BY h.id DESC "
            "LIMIT 1",
            Database::query_param_list(boost::posix_time::to_iso_extended_string(valid_at.get_local_time()))
                                      (valid_at.get_timezone_offset_in_minutes())
                                      (Conversion::Enums::to_db_handle(Fred::Object_Type::domain))
                                      (fqdn));

    if (db_res.size() == 0)
    {
        throw Registry::RecordStatement::ObjectNotFound();
    }

    return static_cast<unsigned long long>(db_res[0][0]);
}

template <Fred::Object_Type::Enum object_type>
unsigned long long get_history_id_internal_of(
        const std::string& object_name,
        const Tz::LocalTimestamp& valid_at,
        Fred::OperationContext& ctx)
{
    try
    {
        return get_history_id_of<object_type>(object_name, valid_at, ctx);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(e.what());
    }
}

template unsigned long long get_history_id_internal_of<Fred::Object_Type::domain>(
        const std::string&, const Tz::LocalTimestamp&, Fred::OperationContext&);
template unsigned long long get_history_id_internal_of<Fred::Object_Type::contact>(
        const std::string&, const Tz::LocalTimestamp&, Fred::OperationContext&);
template unsigned long long get_history_id_internal_of<Fred::Object_Type::keyset>(
        const std::string&, const Tz::LocalTimestamp&, Fred::OperationContext&);
template unsigned long long get_history_id_internal_of<Fred::Object_Type::nsset>(
        const std::string&, const Tz::LocalTimestamp&, Fred::OperationContext&);

boost::optional<NssetPrintoutInputData> make_historic_nsset_data(
        const boost::optional<unsigned long long>& nsset_historyid,
        const Tz::LocalTimestamp& timestamp,
        Fred::OperationContext& ctx)
{
    if (!static_cast<bool>(nsset_historyid))
    {
        return boost::optional<NssetPrintoutInputData>();
    }
    NssetPrintoutInputData retval;
    retval.info = Fred::InfoNssetHistoryByHistoryid(*nsset_historyid).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());

    for (std::vector<Fred::ObjectIdHandlePair>::const_iterator itr = retval.info.info_nsset_data.tech_contacts.begin();
         itr != retval.info.info_nsset_data.tech_contacts.end(); ++itr)
    {
        retval.tech_contact.push_back(
                Fred::InfoContactHistoryByHistoryid(
                        get_history_id_internal_of<Fred::Object_Type::contact>(itr->handle, timestamp, ctx))
                .exec(ctx, Tz::get_psql_handle_of<Tz::UTC>()));
    }

    retval.sponsoring_registrar =
            Fred::InfoRegistrarByHandle(retval.info.info_nsset_data.sponsoring_registrar_handle)
            .exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());

    retval.external_states =
            Fred::RecordStatement::Impl::make_historic_external_states(retval.info.info_nsset_data.id, timestamp, ctx);

    return retval;
}

boost::optional<KeysetPrintoutInputData> make_historic_keyset_data(
        const boost::optional<unsigned long long>& keyset_historyid,
        const Tz::LocalTimestamp& timestamp,
        Fred::OperationContext& ctx)
{
    if (!static_cast<bool>(keyset_historyid))
    {
        return boost::optional<KeysetPrintoutInputData>();
    }
    KeysetPrintoutInputData retval;
    retval.info = Fred::InfoKeysetHistoryByHistoryid(*keyset_historyid).exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());

    for (std::vector<Fred::ObjectIdHandlePair>::const_iterator itr = retval.info.info_keyset_data.tech_contacts.begin();
         itr != retval.info.info_keyset_data.tech_contacts.end(); ++itr)
    {
        retval.tech_contact.push_back(
                Fred::InfoContactHistoryByHistoryid(
                        get_history_id_internal_of<Fred::Object_Type::contact>(itr->handle, timestamp, ctx))
                .exec(ctx, Tz::get_psql_handle_of<Tz::UTC>()));
    }

    retval.sponsoring_registrar =
            Fred::InfoRegistrarByHandle(retval.info.info_keyset_data.sponsoring_registrar_handle)
            .exec(ctx, Tz::get_psql_handle_of<Tz::UTC>());

    retval.external_states =
            make_historic_external_states(retval.info.info_keyset_data.id, timestamp, ctx);

    return retval;
}

}//namespace Fred::RecordStatement::Impl
}//namespace Fred::RecordStatement
}//namespace Fred
