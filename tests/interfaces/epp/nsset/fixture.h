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

#ifndef TEST_INTERFACE_EPP_FIXTURE_797c9c7aa34e4282b6c545a702cc4c06
#define TEST_INTERFACE_EPP_FIXTURE_797c9c7aa34e4282b6c545a702cc4c06

#include "tests/setup/fixtures.h"
#include "tests/interfaces/epp/util.h"

#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object_state/get_object_states.h"

#include "src/epp/nsset/nsset_create.h"
#include "src/epp/nsset/nsset_update.h"

struct has_registrar : virtual Test::autocommitting_context {
    Fred::InfoRegistrarData registrar;

    has_registrar() {
        const std::string reg_handle = "REGISTRAR1";
        Fred::CreateRegistrar(reg_handle).exec(ctx);
        registrar = Fred::InfoRegistrarByHandle(reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_nsset : has_registrar {
    Fred::InfoNssetData nsset;

    has_nsset() {
        const std::string nsset_handle = "NSSET1";
        Fred::CreateNsset(nsset_handle, registrar.handle).exec(ctx);
        nsset = Fred::InfoNssetByHandle(nsset_handle).exec(ctx).info_nsset_data;
    }
};

struct has_nsset_with_all_data_set : has_registrar {
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
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("10.0.0.3"))(ip::address::from_string("10.1.1.3")))) //add_dns
                (Fred::DnsHost("c.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("10.0.0.4"))(ip::address::from_string("10.1.1.4")))) //add_dns
                )
            .set_authinfo("abcdef1234")
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact3_handle)(admin_contact2_handle))
            .set_tech_check_level(3)
            .exec(ctx);
        nsset = Fred::InfoNssetByHandle(nsset_handle).exec(ctx).info_nsset_data;
    }
};

struct has_nsset_input_data_set : has_registrar
{
    Epp::NssetCreateInputData nsset_input_data;

    has_nsset_input_data_set()
    : nsset_input_data(Epp::NssetCreateInputData(
            "NSSET1",
            "authInfo123",
            Util::vector_of<Epp::DNShostData>
                (Epp::DNShostData("a.ns.nic.cz",
                    Util::vector_of<boost::asio::ip::address>
                        (boost::asio::ip::address::from_string("10.0.0.3"))
                        (boost::asio::ip::address::from_string("10.1.1.3")))) //add_dns
                (Epp::DNShostData("c.ns.nic.cz",
                    Util::vector_of<boost::asio::ip::address>
                        (boost::asio::ip::address::from_string("10.0.0.4"))
                        (boost::asio::ip::address::from_string("10.1.1.4")))), //add_dns
            Util::vector_of<std::string>
                ("TEST-ADMIN-CONTACT2")
                ("TEST-ADMIN-CONTACT3"),
            3
        ))
    {
        Fred::Contact::PlaceAddress place;
        place.street1 = "street 1";
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";

        std::string admin_contact2_handle = nsset_input_data.tech_contacts.at(1);

        Fred::CreateContact(admin_contact2_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT2 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        std::string admin_contact3_handle = nsset_input_data.tech_contacts.at(0);

        Fred::CreateContact(admin_contact3_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT3 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
    }
};


struct has_nsset_with_input_data_set : has_registrar {
    Fred::InfoNssetData nsset;
    Epp::NssetCreateInputData nsset_input_data;

    has_nsset_with_input_data_set()
    : nsset_input_data(Epp::NssetCreateInputData(
            "NSSET1",
            "authInfo123",
            Util::vector_of<Epp::DNShostData>
                (Epp::DNShostData("a.ns.nic.cz",
                    Util::vector_of<boost::asio::ip::address>
                        (boost::asio::ip::address::from_string("10.0.0.3"))
                        (boost::asio::ip::address::from_string("10.1.1.3")))) //add_dns
                (Epp::DNShostData("c.ns.nic.cz",
                    Util::vector_of<boost::asio::ip::address>
                        (boost::asio::ip::address::from_string("10.0.0.4"))
                        (boost::asio::ip::address::from_string("10.1.1.4")))), //add_dns
            Util::vector_of<std::string>
                ("TEST-ADMIN-CONTACT2")
                ("TEST-ADMIN-CONTACT3"),
            3
        ))
    {
        Fred::Contact::PlaceAddress place;
        place.street1 = "street 1";
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";

        std::string admin_contact2_handle = nsset_input_data.tech_contacts.at(0);

        Fred::CreateContact(admin_contact2_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT2 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        std::string admin_contact3_handle = nsset_input_data.tech_contacts.at(1);

        Fred::CreateContact(admin_contact3_handle,registrar.handle)
            .set_name("TEST-ADMIN-CONTACT3 NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateNsset(nsset_input_data.handle, registrar.handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost(nsset_input_data.dns_hosts.at(0).fqdn, nsset_input_data.dns_hosts.at(0).inet_addr )) //add_dns
                (Fred::DnsHost(nsset_input_data.dns_hosts.at(1).fqdn, nsset_input_data.dns_hosts.at(1).inet_addr )) //add_dns
                )
            .set_authinfo(nsset_input_data.authinfo)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact2_handle)(admin_contact3_handle))
            .set_tech_check_level(nsset_input_data.tech_check_level)
            .exec(ctx);
        nsset = Fred::InfoNssetByHandle(nsset_input_data.handle).exec(ctx).info_nsset_data;

    }
};


struct has_nsset_and_a_different_registrar : has_nsset {
    Fred::InfoRegistrarData the_different_registrar;

    has_nsset_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        Fred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = Fred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
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

#endif
