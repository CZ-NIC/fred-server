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

#ifndef FIXTURE_H_F9A390F89F6D4E078F26DB46A61B1BC1
#define FIXTURE_H_F9A390F89F6D4E078F26DB46A61B1BC1

#include "tests/interfaces/epp/util.h"
#include "tests/setup/fixtures.h"

#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/perform_object_state_request.h"

#include "src/epp/nsset/check_nsset_config_data.h"
#include "src/epp/nsset/create_nsset_config_data.h"
#include "src/epp/nsset/create_nsset_localized.h"
#include "src/epp/nsset/create_nsset_localized.h"
#include "src/epp/nsset/delete_nsset_config_data.h"
#include "src/epp/nsset/impl/nsset.h"
#include "src/epp/nsset/info_nsset_config_data.h"
#include "src/epp/nsset/transfer_nsset_config_data.h"
#include "src/epp/nsset/update_nsset_config_data.h"
#include "src/epp/nsset/update_nsset_localized.h"

namespace Test {
namespace Backend {
namespace Epp {
namespace Nsset {

struct DefaultCheckNssetConfigData : ::Epp::Nsset::CheckNssetConfigData
{
    DefaultCheckNssetConfigData()
        : CheckNssetConfigData(false)
    {
    }
};

struct DefaultInfoNssetConfigData : ::Epp::Nsset::InfoNssetConfigData
{
    DefaultInfoNssetConfigData()
        : InfoNssetConfigData(false)
    {
    }
};

struct DefaultCreateNssetConfigData : ::Epp::Nsset::CreateNssetConfigData
{
    DefaultCreateNssetConfigData()
        : CreateNssetConfigData(
                false, // rifd_EPP_operations_charging
                3,     // default_tech_check_level
                2,     // min_hosts
                10)    // max_hosts
    {
    }
};

struct DefaultUpdateNssetConfigData : ::Epp::Nsset::UpdateNssetConfigData
{
    DefaultUpdateNssetConfigData()
        : UpdateNssetConfigData(
                false, // rifd_epp_operations_charging;
                0,     // min_hosts;
                10)    // max_hosts;
    {
    }
};

struct DefaultDeleteNssetConfigData : ::Epp::Nsset::DeleteNssetConfigData
{
    DefaultDeleteNssetConfigData()
        : DeleteNssetConfigData(false)
    {
    }
};

struct DefaultTransferNssetConfigData : ::Epp::Nsset::TransferNssetConfigData
{
    DefaultTransferNssetConfigData()
        : TransferNssetConfigData(false)
    {
    }
};

struct Nsset {
    std::string handle;
    Nsset(Fred::OperationContext& _ctx, const std::string& _registrar_handle) {
        handle = "NSSET";
        Fred::CreateNsset(handle, _registrar_handle).exec(_ctx);
    }
};


// fixtures

struct HasRegistrar : virtual autorollbacking_context {
    Fred::InfoRegistrarData registrar;

    HasRegistrar()
    {
        const std::string registrar_handle = "REGISTRAR1";
        Fred::CreateRegistrar(registrar_handle).exec(ctx);
        registrar = Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;
    }
};

struct has_nsset : HasRegistrar {
    Fred::InfoNssetData nsset;

    has_nsset() {
        namespace ip = boost::asio::ip;

        Fred::Contact::PlaceAddress place;
        place.street1 = "street 1";
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";

        std::string admin_contact2_handle = "TEST-ADMIN-CONTACT2";

        Fred::CreateContact(admin_contact2_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT2 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        const std::string nsset_handle = "NSSET1";
        Fred::CreateNsset(nsset_handle, registrar.handle)
        .set_tech_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_dns_hosts(Util::vector_of<Fred::DnsHost>
            (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("11.0.0.3"))(ip::address::from_string("11.1.1.3")))) //add_dns
            (Fred::DnsHost("c.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("11.0.0.4"))(ip::address::from_string("11.1.1.4")))) //add_dns
            )
        .exec(ctx);
        nsset = Fred::InfoNssetByHandle(nsset_handle).exec(ctx).info_nsset_data;
    }
};

struct has_nsset_with_all_data_set : HasRegistrar {
    Fred::InfoNssetData nsset;
    std::string admin_contact4_handle;
    std::string admin_contact5_handle;


    has_nsset_with_all_data_set() {
        namespace ip = boost::asio::ip;

        Fred::Contact::PlaceAddress place;
        place.street1 = "street 1";
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";

        std::string admin_contact2_handle = "TEST-ADMIN-CONTACT2";

        Fred::CreateContact(admin_contact2_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT2 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        std::string admin_contact3_handle = "TEST-ADMIN-CONTACT3";

        Fred::CreateContact(admin_contact3_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT3 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        admin_contact4_handle = "TEST-ADMIN-CONTACT4";

        Fred::CreateContact(admin_contact4_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT4 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        admin_contact5_handle = "TEST-ADMIN-CONTACT5";

        Fred::CreateContact(admin_contact5_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT5 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        const std::string nsset_handle = "NSSET1";
        Fred::CreateNsset(nsset_handle, registrar.handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("11.0.0.3"))(ip::address::from_string("11.1.1.3")))) //add_dns
                (Fred::DnsHost("c.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("11.0.0.4"))(ip::address::from_string("11.1.1.4")))) //add_dns
                )
            .set_authinfo("abcdef1234")
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact3_handle)(admin_contact2_handle))
            .set_tech_check_level(3)
            .exec(ctx);
        nsset = Fred::InfoNssetByHandle(nsset_handle).exec(ctx).info_nsset_data;
    }
};

struct has_nsset_input_data_set : HasRegistrar
{
    ::Epp::Nsset::CreateNssetInputData create_nsset_input_data;
    ::Epp::Nsset::CreateNssetConfigData create_nsset_config_data;
    static const bool rifd_epp_operations_charging = false;
    static const unsigned int nsset_tech_check_level = 3;
    static const unsigned int default_nsset_tech_check_level = 3;
    static const unsigned int nsset_min_hosts = 2;
    static const unsigned int nsset_max_hosts = 10;

    has_nsset_input_data_set()
    : create_nsset_input_data(
            ::Epp::Nsset::CreateNssetInputData(
                    "NSSET1",
                    boost::optional<std::string>("authInfo123"),
                    Util::vector_of<std::string>
                        ("TEST-ADMIN-CONTACT2")
                        ("TEST-ADMIN-CONTACT3"),
                    Util::vector_of< ::Epp::Nsset::DnsHostInput>
                        (::Epp::Nsset::DnsHostInput("a.ns.nic.cz",
                            Util::vector_of< boost::optional<boost::asio::ip::address> >
                                (boost::asio::ip::address::from_string("11.0.0.3"))
                                (boost::asio::ip::address::from_string("11.1.1.3")))) //add_dns
                        (::Epp::Nsset::DnsHostInput("c.ns.nic.cz",
                            Util::vector_of<boost::optional<boost::asio::ip::address> >
                                (boost::asio::ip::address::from_string("11.0.0.4"))
                                (boost::asio::ip::address::from_string("11.1.1.4")))), //add_dns
                    nsset_tech_check_level)),
      create_nsset_config_data(
            ::Epp::Nsset::CreateNssetConfigData(
                    rifd_epp_operations_charging,
                    default_nsset_tech_check_level,
                    nsset_min_hosts,
                    nsset_max_hosts))
    {
        Fred::Contact::PlaceAddress place;
        place.street1 = "street 1";
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";

        std::string admin_contact2_handle = create_nsset_input_data.tech_contacts.at(1);

        Fred::CreateContact(admin_contact2_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT2 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        std::string admin_contact3_handle = create_nsset_input_data.tech_contacts.at(0);

        Fred::CreateContact(admin_contact3_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT3 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
    }
};


struct has_nsset_with_input_data_set : HasRegistrar {
    Fred::InfoNssetData nsset;
    ::Epp::Nsset::CreateNssetInputData create_nsset_input_data;
    ::Epp::Nsset::CreateNssetConfigData create_nsset_config_data;
    static const bool rifd_epp_operations_charging = false;
    static const unsigned int nsset_tech_check_level = 3;
    static const unsigned int default_nsset_tech_check_level = 3;
    static const unsigned int nsset_min_hosts = 2;
    static const unsigned int nsset_max_hosts = 10;

    has_nsset_with_input_data_set()
    : create_nsset_input_data(
            ::Epp::Nsset::CreateNssetInputData(
                    "NSSET1",
                    boost::optional<std::string>("authInfo123"),
                    Util::vector_of<std::string>
                        ("TEST-ADMIN-CONTACT2")
                        ("TEST-ADMIN-CONTACT3"),
                    Util::vector_of< ::Epp::Nsset::DnsHostInput>
                        (::Epp::Nsset::DnsHostInput("a.ns.nic.cz",
                            Util::vector_of<boost::optional<boost::asio::ip::address> >
                                (boost::asio::ip::address::from_string("11.0.0.3"))
                                (boost::asio::ip::address::from_string("11.1.1.3")))) //add_dns
                        (::Epp::Nsset::DnsHostInput("c.ns.nic.cz",
                            Util::vector_of<boost::optional<boost::asio::ip::address> >
                                (boost::asio::ip::address::from_string("11.0.0.4"))
                                (boost::asio::ip::address::from_string("11.1.1.4")))), //add_dns
                    nsset_tech_check_level)),
      create_nsset_config_data(
            ::Epp::Nsset::CreateNssetConfigData(
                    rifd_epp_operations_charging,
                    default_nsset_tech_check_level,
                    nsset_min_hosts,
                    nsset_max_hosts))
    {
        Fred::Contact::PlaceAddress place;
        place.street1 = "street 1";
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";

        std::string admin_contact2_handle = create_nsset_input_data.tech_contacts.at(0);

        Fred::CreateContact(admin_contact2_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT2 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        std::string admin_contact3_handle = create_nsset_input_data.tech_contacts.at(1);

        Fred::CreateContact(admin_contact3_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT3 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateNsset(create_nsset_input_data.handle, registrar.handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost(create_nsset_input_data.dns_hosts.at(0).fqdn, ::Epp::Nsset::make_ipaddrs(create_nsset_input_data.dns_hosts.at(0).inet_addr) )) //add_dns
                (Fred::DnsHost(create_nsset_input_data.dns_hosts.at(1).fqdn, ::Epp::Nsset::make_ipaddrs(create_nsset_input_data.dns_hosts.at(1).inet_addr) )) //add_dns
                )
            .set_authinfo(create_nsset_input_data.authinfopw ? *create_nsset_input_data.authinfopw : std::string())
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact2_handle)(admin_contact3_handle))
            .set_tech_check_level(*create_nsset_input_data.tech_check_level)
            .exec(ctx);
        nsset = Fred::InfoNssetByHandle(create_nsset_input_data.handle).exec(ctx).info_nsset_data;

    }
};


struct has_nsset_with_status_request : has_nsset {
    const std::string status;

    has_nsset_with_status_request(const std::string& _status)
    :   status(_status)
    {
        ctx.get_conn().exec_params(
            "UPDATE enum_object_states SET manual = 'true'::bool WHERE name = $1::text",
            Database::query_param_list(_status)
        );

        const std::set<std::string> statuses = boost::assign::list_of(_status);

        Fred::CreateObjectStateRequestId(nsset.id, statuses).exec(ctx);

        // ensure object has only request, not the state itself
        {
            std::vector<std::string> object_states_before;
            {
                BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(nsset.id).exec(ctx) ) {
                    object_states_before.push_back(state.state_name);
                }
            }

            BOOST_CHECK(
                std::find( object_states_before.begin(), object_states_before.end(), _status )
                ==
                object_states_before.end()
            );
        }
    }
};

struct has_nsset_with_status : has_nsset_with_status_request {
    has_nsset_with_status(const std::string& _status)
    :   has_nsset_with_status_request(_status)
    {
        Fred::PerformObjectStateRequest(nsset.id).exec(ctx);
    }
};

struct has_nsset_with_server_update_prohibited : has_nsset_with_status {
    has_nsset_with_server_update_prohibited()
    :   has_nsset_with_status("serverUpdateProhibited")
    { }
};

struct has_nsset_with_server_transfer_prohibited : has_nsset_with_status {
    has_nsset_with_server_transfer_prohibited()
    :   has_nsset_with_status("serverTransferProhibited")
    { }
};

struct has_nsset_with_delete_candidate : has_nsset_with_status {
    has_nsset_with_delete_candidate()
    :   has_nsset_with_status("deleteCandidate")
    { }
};

struct has_nsset_with_delete_candidate_request : has_nsset_with_status_request {
    has_nsset_with_delete_candidate_request()
    :   has_nsset_with_status_request("deleteCandidate")
    { }
};

struct has_nsset_with_server_transfer_prohibited_request : has_nsset_with_status_request {
    has_nsset_with_server_transfer_prohibited_request()
    :   has_nsset_with_status_request("serverTransferProhibited")
    { }
};

struct has_nsset_with_server_update_prohibited_request : has_nsset_with_status_request {
    has_nsset_with_server_update_prohibited_request()
    :   has_nsset_with_status_request("serverUpdateProhibited")
    { }
};

} // namespace Test::Backend::Epp::Nsset
} // namespace Test::Backend::Epp
} // namespace Test::Backend
} // namespace Test

#endif
