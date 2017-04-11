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

#include "record_statement_xml.hh"
#include "src/record_statement/record_statement_exception.hh"

#include "util/log/context.h"
#include "util/util.h"
#include "util/xmlgen.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/zone/zone.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/object_state/get_object_states.h"

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

#include <boost/lexical_cast.hpp>


namespace Fred
{
namespace RecordStatement
{
    TimeWithOffset make_time_with_offset(const std::string& rfc3339_timestamp)
    {
        TimeWithOffset ret;
        std::string::size_type tpos = rfc3339_timestamp.find_last_of("Tt ");

        if(tpos != std::string::npos)
        {
            std::string time_with_offset = rfc3339_timestamp.substr(tpos, std::string::npos);

            std::string::size_type offset_pos = time_with_offset.find_first_of("+-Zz");

            if(offset_pos != std::string::npos)
            {
                std::string toffset = time_with_offset.substr(offset_pos, std::string::npos);
                ret.offset = time_with_offset.substr(offset_pos, std::string::npos);
                ret.time = rfc3339_timestamp.substr(0, rfc3339_timestamp.size() - ret.offset.size());
            }
            else
            {
                ret.time = rfc3339_timestamp;
            }
        }
        else
        {
            ret.time = rfc3339_timestamp;
        }

        return ret;
    }

    Database::ParamQuery make_utc_timestamp_query(const std::string& timestamp)
    {
        const TimeWithOffset object_timestamp = make_time_with_offset(timestamp);
        const Database::ReusableParameter object_time(object_timestamp.time, "timestamp");
        const Database::ReusableParameter object_time_offset(object_timestamp.offset
            + (object_timestamp.offset.find(':') == std::string::npos //if missing ':' in offset
                ? " hours" //add unit to interval
                : ""), "interval");

        Database::ParamQuery utc_timestamp_query;
        utc_timestamp_query("date_trunc('second', ((").param(object_time);//truncate to whole seconds
        if(object_timestamp.offset.size() > 1)//Z or empty offset is invalid intervall
        {
            utc_timestamp_query(" - ").param(object_time_offset);
        }
        utc_timestamp_query(") AT TIME ZONE 'UTC' AT TIME ZONE 'UTC')::timestamp)");

        return utc_timestamp_query;
    }


    std::vector<std::string> make_external_states(unsigned long long object_id, Fred::OperationContext& ctx)
    {
        std::vector<std::string> ret;

        const std::vector<Fred::ObjectStateData> states = Fred::GetObjectStates(object_id).exec(ctx);
        for(std::vector<Fred::ObjectStateData>::const_iterator ci = states.begin();
            ci != states.end(); ++ci)
        {
            if(ci->is_external)
            {
                ret.push_back(ci->state_name);
            }
        }

        return ret;
    }

    std::vector<std::string> make_historic_external_states(
            const unsigned long long object_id,
            const std::string& timestamp,
            Fred::OperationContext& ctx)
    {
        std::vector<std::string> ret;
        Database::ParamQuery timestamp_query = make_utc_timestamp_query(timestamp);
        Database::Result historic_external_states_result = ctx.get_conn().exec_params(
            Database::ParamQuery(
            "SELECT DISTINCT ON (eos.name) eos.name"
            " FROM object_state os "
                " JOIN enum_object_states eos ON eos.id = os.state_id "
                " WHERE os.object_id = ").param_bigint(object_id)(
                    " AND eos.external = TRUE "
                    " AND date_trunc('second', os.valid_from) <= ")(timestamp_query)(
                    " AND (os.valid_to IS NULL OR date_trunc('second', os.valid_to) > ")(timestamp_query)(") "
            " ORDER BY eos.name, eos.importance ")
        );

        ret.reserve(historic_external_states_result.size());
        for(unsigned long long i = 0 ; i < historic_external_states_result.size() ; ++i)
        {
            ret.push_back(static_cast<std::string>(historic_external_states_result[i][0]));
        }

        return ret;
    }

    boost::optional<NssetPrintoutInputData> make_nsset_data(
        boost::optional<std::string> nsset_handle, Fred::OperationContext& ctx)
    {
        boost::optional<NssetPrintoutInputData> ret;

        if(nsset_handle.is_initialized())
        {
            NssetPrintoutInputData nd;
            nd.info = Fred::InfoNssetByHandle(
                    *nsset_handle).exec(ctx, "UTC");

            for(std::vector<Fred::ObjectIdHandlePair>::const_iterator
                it = nd.info.info_nsset_data.tech_contacts.begin();
                it != nd.info.info_nsset_data.tech_contacts.end(); ++it)
            {
                nd.tech_contact.push_back(Fred::InfoContactByHandle(it->handle).exec(ctx, "UTC"));
            }

            nd.sponsoring_registrar = Fred::InfoRegistrarByHandle(
                nd.info.info_nsset_data.sponsoring_registrar_handle).exec(ctx, "UTC");

            nd.external_states = make_external_states(nd.info.info_nsset_data.id, ctx);

            ret = nd;
        }

        return ret;
    }

    boost::optional<KeysetPrintoutInputData> make_keyset_data(
        boost::optional<std::string> keyset_handle, Fred::OperationContext& ctx)
    {
        boost::optional<KeysetPrintoutInputData> ret;

        if(keyset_handle.is_initialized())
        {
            KeysetPrintoutInputData kd;
            kd.info = Fred::InfoKeysetByHandle(
                    *keyset_handle).exec(ctx, "UTC");

            for(std::vector<Fred::ObjectIdHandlePair>::const_iterator
                it = kd.info.info_keyset_data.tech_contacts.begin();
                it != kd.info.info_keyset_data.tech_contacts.end(); ++it)
            {
                kd.tech_contact.push_back(Fred::InfoContactByHandle(it->handle).exec(ctx, "UTC"));
            }

            kd.sponsoring_registrar = Fred::InfoRegistrarByHandle(
                kd.info.info_keyset_data.sponsoring_registrar_handle).exec(ctx, "UTC");

            kd.external_states = make_external_states(kd.info.info_keyset_data.id, ctx);

            ret = kd;
        }

        return ret;
    }



    boost::posix_time::ptime convert_utc_timestamp_to_local(
        Fred::OperationContext& ctx,
        const boost::posix_time::ptime& utc_timestamp,
        const std::string& local_timezone)
    {
        Database::Result datetime_conversion = ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")
            .param_timestamp(utc_timestamp)
            (" AT TIME ZONE 'UTC' AT TIME ZONE ")
            .param_text(local_timezone)(" AS local_timestamp"));

        return boost::posix_time::time_from_string(
            static_cast<std::string>(datetime_conversion[0]["local_timestamp"]));
    }

    std::string ptime_to_rfc3339_datetime_string(
            const boost::posix_time::ptime& local_datetime,
            const boost::posix_time::time_duration& current_local_offset)
    {
        std::string datetime = boost::posix_time::to_iso_extended_string(
            boost::posix_time::ptime(local_datetime.date(),
                boost::posix_time::seconds(local_datetime.time_of_day().total_seconds()) //to remove fractional seconds
            )
        );//YYYY-MM-DDTHH:MM:SS part

        std::string offset = ((current_local_offset.hours() || current_local_offset.minutes())
            ? std::string(boost::str(boost::format("%1$+03d:%2$02d")
                % current_local_offset.hours()
                % boost::date_time::absolute_value(current_local_offset.minutes())))
            : std::string("Z"));//+ZZ:ZZ or "Z" part

        return datetime + offset;
    }


    std::vector<Util::XmlCallback> external_states_xml(const std::vector<std::string>& external_states)
    {
        std::vector<Util::XmlCallback> ret;
        for(std::vector<std::string>::const_iterator
            i = external_states.begin();
            i != external_states.end(); ++i)
        {
            ret.push_back(Util::XmlTagPair("state", Util::XmlEscapeTag(*i)));
        }
        return ret;
    }

    std::vector<Util::XmlCallback> nsset_xml(const boost::optional<NssetPrintoutInputData>& nsset_data)
    {
        if(!nsset_data.is_initialized())
        {
            return Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair("nsset", std::vector<Util::XmlCallback>()));
        }

        std::vector<Util::XmlCallback> nameserver_list;

        for(std::vector<Fred::DnsHost>::const_iterator it = nsset_data.operator *().info.info_nsset_data.dns_hosts.begin();
            it != nsset_data.operator *().info.info_nsset_data.dns_hosts.end(); ++it)
        {
            std::vector<Util::XmlCallback> ip_list;

            const std::vector<boost::asio::ip::address> ns_ip_info = it->get_inet_addr();
            for(std::vector<boost::asio::ip::address>::const_iterator ip_it = ns_ip_info.begin();
                    ip_it != ns_ip_info.end(); ++ip_it)
            {
                ip_list.push_back(Util::XmlTagPair("ip", Util::XmlEscapeTag(ip_it->to_string())));
            }

            nameserver_list.push_back(
                Util::XmlTagPair("nameserver", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("fqdn", Util::XmlEscapeTag(it->get_fqdn())))
                    (Util::XmlTagPair("ip_list", ip_list))));
        }


        std::vector<Util::XmlCallback> tech_contact_list;
        for(std::vector<Fred::InfoContactOutput>::const_iterator
            i = nsset_data.operator *().tech_contact.begin();
            i != nsset_data.operator *().tech_contact.end(); ++i)
        {
            tech_contact_list.push_back(
                Util::XmlTagPair("tech_contact", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("handle", Util::XmlEscapeTag(i->info_contact_data.handle)))
                    (Util::XmlTagPair("name", Util::XmlEscapeTag(i->info_contact_data.name.get_value_or(""))))
                    (Util::XmlTagPair("organization", Util::XmlEscapeTag(i->info_contact_data.organization.get_value_or(""))))
                )
            );
        }

        return Util::vector_of<Util::XmlCallback>
        (Util::XmlTagPair("nsset", Util::vector_of<Util::XmlCallback>
            (Util::XmlTagPair("handle", Util::XmlEscapeTag(nsset_data.operator *().info.info_nsset_data.handle)))
            (Util::XmlTagPair("nameserver_list", nameserver_list))
            (Util::XmlTagPair("tech_contact_list", tech_contact_list))
            (Util::XmlTagPair("sponsoring_registrar", Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair("handle", Util::XmlEscapeTag(
                    nsset_data.operator *().sponsoring_registrar.info_registrar_data.handle)))
                (Util::XmlTagPair("name", Util::XmlEscapeTag(
                    nsset_data.operator *().sponsoring_registrar.info_registrar_data.name.get_value_or(""))))
                (Util::XmlTagPair("organization", Util::XmlEscapeTag(
                    nsset_data.operator *().sponsoring_registrar.info_registrar_data.organization.get_value_or(""))))
            ))
            (Util::XmlTagPair("external_states_list", external_states_xml(nsset_data.operator *().external_states)))
        ));
    }


    std::vector<Util::XmlCallback> keyset_xml(const boost::optional<KeysetPrintoutInputData>& keyset_data)
    {
        if(!keyset_data.is_initialized())
        {
            return Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair("keyset", std::vector<Util::XmlCallback>()));
        }

        std::vector<Util::XmlCallback> dns_key_list;

        for(std::vector<Fred::DnsKey>::const_iterator it = keyset_data.operator *().info.info_keyset_data.dns_keys.begin();
            it != keyset_data.operator *().info.info_keyset_data.dns_keys.end(); ++it)
        {
            dns_key_list.push_back(
                Util::XmlTagPair("dns_key", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("flags", Util::XmlEscapeTag(boost::lexical_cast<std::string>(it->get_flags()))))
                    (Util::XmlTagPair("protocol", Util::XmlEscapeTag(boost::lexical_cast<std::string>(it->get_protocol()))))
                    (Util::XmlTagPair("algorithm", Util::XmlEscapeTag(boost::lexical_cast<std::string>(it->get_alg()))))
                    (Util::XmlTagPair("key", Util::XmlEscapeTag(it->get_key())))
            ));
        }

        std::vector<Util::XmlCallback> tech_contact_list;
        for(std::vector<Fred::InfoContactOutput>::const_iterator
            i = keyset_data.operator *().tech_contact.begin();
            i != keyset_data.operator *().tech_contact.end(); ++i)
        {
            tech_contact_list.push_back(
                Util::XmlTagPair("tech_contact", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("handle", Util::XmlEscapeTag(i->info_contact_data.handle)))
                    (Util::XmlTagPair("name", Util::XmlEscapeTag(i->info_contact_data.name.get_value_or(""))))
                    (Util::XmlTagPair("organization", Util::XmlEscapeTag(i->info_contact_data.organization.get_value_or(""))))
                )
            );
        }

        return Util::vector_of<Util::XmlCallback>
            (Util::XmlTagPair("keyset", Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair("handle", Util::XmlEscapeTag(keyset_data.operator *().info.info_keyset_data.handle)))
                (Util::XmlTagPair("dns_key_list", dns_key_list))
                (Util::XmlTagPair("tech_contact_list", tech_contact_list))
                (Util::XmlTagPair("sponsoring_registrar", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("handle", Util::XmlEscapeTag(
                        keyset_data.operator *().sponsoring_registrar.info_registrar_data.handle)))
                    (Util::XmlTagPair("name", Util::XmlEscapeTag(
                        keyset_data.operator *().sponsoring_registrar.info_registrar_data.name.get_value_or(""))))
                    (Util::XmlTagPair("organization", Util::XmlEscapeTag(
                        keyset_data.operator *().sponsoring_registrar.info_registrar_data.organization.get_value_or(""))))
                ))
                (Util::XmlTagPair("external_states_list", external_states_xml(keyset_data.operator *().external_states)))
            ));
    }

    std::string domain_printout_xml(
        const Fred::InfoDomainOutput& info,
        const boost::posix_time::ptime& local_timestamp,
        const boost::posix_time::ptime& local_creation_time,
        const boost::optional<boost::posix_time::ptime>& local_update_time,
        const bool is_private_printout,
        const Fred::InfoContactOutput& registrant_info,
        const std::vector<Fred::InfoContactOutput>& admin_contact_info,
        const Fred::InfoRegistrarOutput& sponsoring_registrar_info,
        const boost::optional<NssetPrintoutInputData>& nsset_data,
        const boost::optional<KeysetPrintoutInputData>& keyset_data,
        const std::vector<std::string>& external_states
        )
    {
        std::vector<Util::XmlCallback> admin_contact_list;

        for(std::vector<Fred::InfoContactOutput>::const_iterator
            i = admin_contact_info.begin();
            i != admin_contact_info.end(); ++i)
        {
            admin_contact_list.push_back(
                Util::XmlTagPair("admin_contact", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("handle", Util::XmlEscapeTag(i->info_contact_data.handle)))
                    (Util::XmlTagPair("name", Util::XmlEscapeTag(i->info_contact_data.name.get_value_or(""))))
                    (Util::XmlTagPair("organization", Util::XmlEscapeTag(i->info_contact_data.organization.get_value_or(""))))
                )
            );
        }

        std::stringstream private_printout_attr;
        private_printout_attr << std::boolalpha
            << "is_private_printout='" << is_private_printout << "'";

        std::stringstream disclose_flags;
        disclose_flags << std::boolalpha
            << "name='" << registrant_info.info_contact_data.disclosename << "'"
            << " organization='" << registrant_info.info_contact_data.discloseorganization << "'"
            << " address='" << registrant_info.info_contact_data.discloseaddress << "'"
            << " telephone='" << registrant_info.info_contact_data.disclosetelephone << "'"
            << " fax='" << registrant_info.info_contact_data.disclosefax << "'"
            << " email='" << registrant_info.info_contact_data.discloseemail << "'"
            << " vat='" << registrant_info.info_contact_data.disclosevat << "'"
            << " ident='" << registrant_info.info_contact_data.discloseident << "'"
            << " notifyemail='" << registrant_info.info_contact_data.disclosenotifyemail << "'"
            ;


        std::string printout_xml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        Util::XmlTagPair("record_statement", Util::vector_of<Util::XmlCallback>
            (Util::XmlTagPair("current_datetime", Util::XmlEscapeTag(ptime_to_rfc3339_datetime_string(
                local_timestamp, local_timestamp - info.utc_timestamp))))
            (Util::XmlTagPair("domain", Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair("fqdn", Util::XmlEscapeTag(info.info_domain_data.fqdn)))
                (Util::XmlTagPair("creation_date", Util::XmlEscapeTag(
                    boost::gregorian::to_iso_extended_string(local_creation_time.date()))))
                (Util::XmlTagPair("last_update_date", Util::XmlEscapeTag(
                    local_update_time.is_initialized()
                        ? boost::gregorian::to_iso_extended_string(local_update_time.operator *().date())
                        :  std::string()
                )))
                (Util::XmlTagPair("expiration_date", Util::XmlEscapeTag(
                    boost::gregorian::to_iso_extended_string(info.info_domain_data.expiration_date))))

                (Util::XmlTagPair("holder", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("handle", Util::XmlEscapeTag(
                        registrant_info.info_contact_data.handle)))
                    (Util::XmlTagPair("name", Util::XmlEscapeTag(
                        registrant_info.info_contact_data.name.get_value_or(""))))
                    (Util::XmlTagPair("organization", Util::XmlEscapeTag(
                        registrant_info.info_contact_data.organization.get_value_or(""))))
                    (Util::XmlTagPair("id_number", Util::XmlEscapeTag(
                        registrant_info.info_contact_data.ssntype.get_value_or("") == "ICO"
                        ? registrant_info.info_contact_data.ssn.get_value_or("") : std::string())))
                    (Util::XmlTagPair("street1", Util::XmlEscapeTag(
                        registrant_info.info_contact_data.place.get_value().street1)))
                    (Util::XmlTagPair("street2", Util::XmlEscapeTag(
                        registrant_info.info_contact_data.place.get_value().street2.get_value_or(""))))
                    (Util::XmlTagPair("street3", Util::XmlEscapeTag(
                        registrant_info.info_contact_data.place.get_value().street3.get_value_or(""))))
                    (Util::XmlTagPair("city", Util::XmlEscapeTag(
                        registrant_info.info_contact_data.place.get_value().city)))
                    (Util::XmlTagPair("stateorprovince", Util::XmlEscapeTag(
                        registrant_info.info_contact_data.place.get_value().stateorprovince.get_value_or(""))))
                    (Util::XmlTagPair("postal_code", Util::XmlEscapeTag(
                        registrant_info.info_contact_data.place.get_value().postalcode)))
                    (Util::XmlTagPair("country", Util::XmlEscapeTag(
                        registrant_info.info_contact_data.place.get_value().country)))
                    (Util::XmlTagPair("disclose", "", disclose_flags.str())),
                    private_printout_attr.str()))
                (Util::XmlTagPair("admin_contact_list", admin_contact_list))

                (Util::XmlTagPair("sponsoring_registrar", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("handle", Util::XmlEscapeTag(
                        sponsoring_registrar_info.info_registrar_data.handle)))
                    (Util::XmlTagPair("name", Util::XmlEscapeTag(
                        sponsoring_registrar_info.info_registrar_data.name.get_value_or(""))))
                    (Util::XmlTagPair("organization", Util::XmlEscapeTag(
                        sponsoring_registrar_info.info_registrar_data.organization.get_value_or(""))))
                ))
                (nsset_xml(nsset_data))
                (keyset_xml(keyset_data))
                (Util::XmlTagPair("external_states_list", external_states_xml(external_states)))
        ))
        )(printout_xml);

        return printout_xml;
    }


    std::string nsset_printout_xml(
        const NssetPrintoutInputData& nsset_input_data,
        const boost::posix_time::ptime& local_timestamp)
    {
        std::string printout_xml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        Util::XmlTagPair("record_statement", Util::vector_of<Util::XmlCallback>
            (Util::XmlTagPair("current_datetime", Util::XmlEscapeTag(ptime_to_rfc3339_datetime_string(
                local_timestamp, local_timestamp -  nsset_input_data.info.utc_timestamp))))
                (nsset_xml(nsset_input_data))
        )(printout_xml);

        return printout_xml;
    }

    std::string keyset_printout_xml(
        const KeysetPrintoutInputData& keyset_input_data,
        const boost::posix_time::ptime& local_timestamp)
    {
        std::string printout_xml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        Util::XmlTagPair("record_statement", Util::vector_of<Util::XmlCallback>
            (Util::XmlTagPair("current_datetime", Util::XmlEscapeTag(ptime_to_rfc3339_datetime_string(
                local_timestamp, local_timestamp -  keyset_input_data.info.utc_timestamp))))
                (keyset_xml(keyset_input_data))
        )(printout_xml);

        return printout_xml;
    }

    std::string contact_printout_xml(
        const bool is_private_printout,
        const Fred::InfoContactOutput& info,
        const boost::posix_time::ptime& local_timestamp,
        const boost::posix_time::ptime& local_creation_time,
        const boost::optional<boost::posix_time::ptime>& local_update_time,
        const boost::optional<boost::posix_time::ptime>& local_transfer_time,
        const Fred::InfoRegistrarOutput& sponsoring_registrar_info,
        const std::vector<std::string>& external_states
        )
    {
        std::string printout_xml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");

        std::stringstream private_printout_attr;
        private_printout_attr << std::boolalpha
            << "is_private_printout='" << is_private_printout << "'";

        std::stringstream disclose_flags;
        disclose_flags << std::boolalpha
            << "name='" << info.info_contact_data.disclosename << "'"
            << " organization='" << info.info_contact_data.discloseorganization << "'"
            << " address='" << info.info_contact_data.discloseaddress << "'"
            << " telephone='" << info.info_contact_data.disclosetelephone << "'"
            << " fax='" << info.info_contact_data.disclosefax << "'"
            << " email='" << info.info_contact_data.discloseemail << "'"
            << " vat='" << info.info_contact_data.disclosevat << "'"
            << " ident='" << info.info_contact_data.discloseident << "'"
            << " notifyemail='" << info.info_contact_data.disclosenotifyemail << "'"
            ;

        Util::XmlTagPair("record_statement", Util::vector_of<Util::XmlCallback>
            (Util::XmlTagPair("current_datetime", Util::XmlEscapeTag(ptime_to_rfc3339_datetime_string(
                local_timestamp, local_timestamp - info.utc_timestamp))))

            (Util::XmlTagPair("contact", Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair("handle", Util::XmlEscapeTag(
                    info.info_contact_data.handle)))
                (Util::XmlTagPair("name", Util::XmlEscapeTag(
                    info.info_contact_data.name.get_value_or(""))))
                (Util::XmlTagPair("organization", Util::XmlEscapeTag(
                    info.info_contact_data.organization.get_value_or(""))))
                (Util::XmlTagPair("taxpayer_id_number", Util::XmlEscapeTag(
                    info.info_contact_data.vat.get_value_or(""))))
                (Util::XmlTagPair("id_type", Util::XmlEscapeTag(
                    info.info_contact_data.ssntype.get_value_or(""))))
                (Util::XmlTagPair("id_value", Util::XmlEscapeTag(
                    info.info_contact_data.ssn.get_value_or(""))))
                (Util::XmlTagPair("email", Util::XmlEscapeTag(
                    info.info_contact_data.email.get_value_or(""))))
                (Util::XmlTagPair("notification_email", Util::XmlEscapeTag(
                    info.info_contact_data.notifyemail.get_value_or(""))))
                (Util::XmlTagPair("phone", Util::XmlEscapeTag(
                    info.info_contact_data.telephone.get_value_or(""))))
                (Util::XmlTagPair("fax", Util::XmlEscapeTag(
                    info.info_contact_data.fax.get_value_or(""))))

                (Util::XmlTagPair("creation_date", Util::XmlEscapeTag(
                    boost::gregorian::to_iso_extended_string(local_creation_time.date())
                )))
                (Util::XmlTagPair("last_update_date", Util::XmlEscapeTag(
                    local_update_time.is_initialized()
                        ? boost::gregorian::to_iso_extended_string(local_update_time.operator *().date())
                        :  std::string()
                )))
                (Util::XmlTagPair("last_transfer_date", Util::XmlEscapeTag(
                    local_transfer_time.is_initialized()
                        ? boost::gregorian::to_iso_extended_string(local_transfer_time.operator *().date())
                        :  std::string()
                )))
                (Util::XmlTagPair("address", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("street1", Util::XmlEscapeTag(
                        info.info_contact_data.place.get_value().street1)))
                    (Util::XmlTagPair("street2", Util::XmlEscapeTag(
                        info.info_contact_data.place.get_value().street2.get_value_or(""))))
                    (Util::XmlTagPair("street3", Util::XmlEscapeTag(
                        info.info_contact_data.place.get_value().street3.get_value_or(""))))
                    (Util::XmlTagPair("city", Util::XmlEscapeTag(
                        info.info_contact_data.place.get_value().city)))
                    (Util::XmlTagPair("stateorprovince", Util::XmlEscapeTag(
                        info.info_contact_data.place.get_value().stateorprovince.get_value_or(""))))
                    (Util::XmlTagPair("postal_code", Util::XmlEscapeTag(
                        info.info_contact_data.place.get_value().postalcode)))
                    (Util::XmlTagPair("country", Util::XmlEscapeTag(
                        info.info_contact_data.place.get_value().country)))
                ))
                (Util::XmlTagPair("sponsoring_registrar", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("handle", Util::XmlEscapeTag(
                        sponsoring_registrar_info.info_registrar_data.handle)))
                    (Util::XmlTagPair("name", Util::XmlEscapeTag(
                        sponsoring_registrar_info.info_registrar_data.name.get_value_or(""))))
                    (Util::XmlTagPair("organization", Util::XmlEscapeTag(
                        sponsoring_registrar_info.info_registrar_data.organization.get_value_or(""))))
                ))
                (Util::XmlTagPair("disclose", "", disclose_flags.str()))
                (Util::XmlTagPair("external_states_list", external_states_xml(external_states))),
            private_printout_attr.str()))
        )(printout_xml);
        return printout_xml;
    }

    std::string domain_printout_xml_with_data(
        const std::string& fqdn,
        const std::string& registry_timezone,
        bool is_private_printout,
        Fred::OperationContext& ctx,
        std::string* registrant_email_out,
        boost::posix_time::ptime* request_local_timestamp
        )
    {
        Fred::InfoDomainOutput info_domain_output;
        try
        {
            info_domain_output = Fred::InfoDomainByHandle(Fred::Zone::rem_trailing_dot(fqdn)).exec(ctx, "UTC");
        }
        catch (const Fred::InfoDomainByHandle::Exception& e)
        {
            if (e.is_set_unknown_fqdn())
            {
                throw Registry::RecordStatement::ObjectNotFound();
            }

            //other error
            throw;
        }

        Fred::InfoContactOutput info_registrant_output = Fred::InfoContactByHandle(
                info_domain_output.info_domain_data.registrant.handle).exec(ctx, "UTC");

        if(registrant_email_out != NULL)
        {
            *registrant_email_out = info_registrant_output.info_contact_data.email.get_value_or("");
        }

        Fred::InfoRegistrarOutput info_sponsoring_registrar_output = Fred::InfoRegistrarByHandle(
                info_domain_output.info_domain_data.sponsoring_registrar_handle).exec(ctx, "UTC");

        std::vector<Fred::InfoContactOutput> info_admin_contact_output;

        for(std::vector<Fred::ObjectIdHandlePair>::const_iterator
            i = info_domain_output.info_domain_data.admin_contacts.begin();
            i != info_domain_output.info_domain_data.admin_contacts.end(); ++i)
        {
            info_admin_contact_output.push_back(
                Fred::InfoContactByHandle(i->handle).exec(ctx, "UTC"));
        }

        boost::optional<std::string> nsset_handle = info_domain_output.info_domain_data.nsset.isnull()
            ? boost::optional<std::string>()
            : boost::optional<std::string>(info_domain_output.info_domain_data.nsset.get_value().handle);

        boost::optional<NssetPrintoutInputData> nsset_data = make_nsset_data(nsset_handle, ctx);

        boost::optional<std::string> keyset_handle = info_domain_output.info_domain_data.keyset.isnull()
            ? boost::optional<std::string>()
            : boost::optional<std::string>(info_domain_output.info_domain_data.keyset.get_value().handle);

        boost::optional<KeysetPrintoutInputData> keyset_data = make_keyset_data(keyset_handle, ctx);

        const boost::posix_time::ptime local_timestamp = convert_utc_timestamp_to_local(ctx, info_domain_output.utc_timestamp, registry_timezone);

        if(request_local_timestamp != NULL)
        {
            *request_local_timestamp = local_timestamp;
        }

        const std::string xml_document = domain_printout_xml(
            info_domain_output,
            local_timestamp,
            convert_utc_timestamp_to_local(ctx, info_domain_output.info_domain_data.creation_time, registry_timezone),
            (info_domain_output.info_domain_data.update_time.isnull()
                ? boost::optional<boost::posix_time::ptime>()
                : boost::optional<boost::posix_time::ptime>(
                    convert_utc_timestamp_to_local(
                        ctx, info_domain_output.info_domain_data.update_time.get_value(), registry_timezone))),
            is_private_printout,
            info_registrant_output,
            info_admin_contact_output,
            info_sponsoring_registrar_output,
            nsset_data,
            keyset_data,
            make_external_states(info_domain_output.info_domain_data.id, ctx)
        );

        ctx.get_log().debug(xml_document);
        return xml_document;
    }


    std::string nsset_printout_xml_with_data(
        const std::string& handle,
        const std::string& registry_timezone,
        Fred::OperationContext& ctx,
        std::vector<std::string>* email_out,
        boost::posix_time::ptime* request_local_timestamp)
    {
        NssetPrintoutInputData nsset_data;

        try
        {
            nsset_data = *make_nsset_data(handle, ctx);
        }
        catch (const Fred::InfoNssetByHandle::Exception& e)
        {
            if (e.is_set_unknown_handle())
            {
                throw Registry::RecordStatement::ObjectNotFound();
            }

            //other error
            throw;
        }

        if(email_out != NULL)
        {
            email_out->reserve(nsset_data.tech_contact.size());
            for(std::vector<Fred::InfoContactOutput>::const_iterator ci = nsset_data.tech_contact.begin();
                    ci != nsset_data.tech_contact.end(); ++ci)
            {
                if(!ci->info_contact_data.email.isnull())
                {
                    email_out->push_back(ci->info_contact_data.email.get_value());
                }
            }
        }

        const boost::posix_time::ptime local_timestamp = convert_utc_timestamp_to_local(ctx, nsset_data.info.utc_timestamp, registry_timezone);

        if(request_local_timestamp != NULL)
        {
            *request_local_timestamp = local_timestamp;
        }

        const std::string xml_document = nsset_printout_xml(nsset_data, local_timestamp);

        ctx.get_log().debug(xml_document);
        return xml_document;
    }


    std::string keyset_printout_xml_with_data(
        const std::string& handle,
        const std::string& registry_timezone,
        Fred::OperationContext& ctx,
        std::vector<std::string>* email_out,
        boost::posix_time::ptime* request_local_timestamp)
    {
        KeysetPrintoutInputData keyset_data;

        try
        {
            keyset_data = *make_keyset_data(handle, ctx);
        }
        catch (const Fred::InfoKeysetByHandle::Exception& e)
        {
            if (e.is_set_unknown_handle())
            {
                throw Registry::RecordStatement::ObjectNotFound();
            }

            //other error
            throw;
        }

        if(email_out != NULL)
        {
            email_out->reserve(keyset_data.tech_contact.size());
            for(std::vector<Fred::InfoContactOutput>::const_iterator ci = keyset_data.tech_contact.begin();
                    ci != keyset_data.tech_contact.end(); ++ci)
            {
                if(!ci->info_contact_data.email.isnull())
                {
                    email_out->push_back(ci->info_contact_data.email.get_value());
                }
            }
        }

        const boost::posix_time::ptime local_timestamp = convert_utc_timestamp_to_local(ctx, keyset_data.info.utc_timestamp, registry_timezone);

        if(request_local_timestamp != NULL)
        {
            *request_local_timestamp = local_timestamp;
        }

        const std::string xml_document = keyset_printout_xml(keyset_data, local_timestamp);

        ctx.get_log().debug(xml_document);
        return xml_document;
    }

    std::string contact_printout_xml_with_data(
        const std::string& handle,
        const std::string& registry_timezone,
        bool is_private_printout,
        Fred::OperationContext& ctx,
        std::string* email_out,
        boost::posix_time::ptime* request_local_timestamp
        )
    {
        Fred::InfoContactOutput info_contact_output;

        try
        {
            info_contact_output = Fred::InfoContactByHandle(handle).exec(ctx, "UTC");
        }
        catch (const Fred::InfoContactByHandle::Exception& e)
        {
            if (e.is_set_unknown_contact_handle())
            {
                throw Registry::RecordStatement::ObjectNotFound();
            }

            //other error
            throw;
        }

        if(email_out != NULL)
        {
            *email_out = info_contact_output.info_contact_data.email.get_value_or("");
        }

        Fred::InfoRegistrarOutput info_sponsoring_registrar_output = Fred::InfoRegistrarByHandle(
            info_contact_output.info_contact_data.sponsoring_registrar_handle).exec(ctx, "UTC");

        const boost::posix_time::ptime local_timestamp = convert_utc_timestamp_to_local(
                ctx, info_contact_output.utc_timestamp, registry_timezone);

        if(request_local_timestamp != NULL)
        {
            *request_local_timestamp = local_timestamp;
        }

         const std::string xml_document = contact_printout_xml(is_private_printout, info_contact_output,
             local_timestamp,
            convert_utc_timestamp_to_local(ctx, info_contact_output.info_contact_data.creation_time, registry_timezone),
            (info_contact_output.info_contact_data.update_time.isnull()
                ? boost::optional<boost::posix_time::ptime>()
                : boost::optional<boost::posix_time::ptime>(
                    convert_utc_timestamp_to_local(
                        ctx, info_contact_output.info_contact_data.update_time.get_value(), registry_timezone))),
            (info_contact_output.info_contact_data.transfer_time.isnull()
                ? boost::optional<boost::posix_time::ptime>()
                : boost::optional<boost::posix_time::ptime>(
                    convert_utc_timestamp_to_local(
                        ctx, info_contact_output.info_contact_data.transfer_time.get_value(), registry_timezone))),
            info_sponsoring_registrar_output,
                make_external_states(info_contact_output.info_contact_data.id, ctx)
        );

        ctx.get_log().debug(xml_document);
        return xml_document;
    }


}//namespace RecordStatement
}//namespace Registry

