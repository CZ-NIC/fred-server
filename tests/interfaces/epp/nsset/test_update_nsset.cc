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

#include "tests/interfaces/epp/nsset/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/nsset/update_nsset.h"
#include "util/map_at.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include <map>
#include <vector>
#include <algorithm>
#include <iterator>

BOOST_AUTO_TEST_SUITE(Nsset)
BOOST_AUTO_TEST_SUITE(UpdateNsset)

bool update_nsset_invalid_registrar_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_nsset_invalid_registrar, has_nsset)
{
    const Epp::Nsset::UpdateNssetInputData input_data(
        nsset.handle + "*?!",
        Optional<std::string>(),
        std::vector<Epp::Nsset::DnsHostInput>(),
        std::vector<Epp::Nsset::DnsHostInput>(),
        std::vector<std::string>(),
        std::vector<std::string>(),
        boost::optional<short>()
    );
    const Epp::Nsset::UpdateNssetConfigData config_data(2, 10);

    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::update_nsset(
            ctx,
            input_data,
            config_data,
            0, // registrar id
            42 // whatever
        ),
        Epp::EppResponseFailure,
        update_nsset_invalid_registrar_exception
    );

}

bool update_fail_nonexistent_handle_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_fail_nonexistent_handle, has_nsset)
{
    const Epp::Nsset::UpdateNssetInputData input_data(
        nsset.handle + "abc",
        Optional<std::string>(),
        std::vector<Epp::Nsset::DnsHostInput>(),
        std::vector<Epp::Nsset::DnsHostInput>(),
        std::vector<std::string>(),
        std::vector<std::string>(),
        boost::optional<short>()
    );
    const Epp::Nsset::UpdateNssetConfigData config_data(2, 10);

    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::update_nsset(
            ctx,
            input_data,
            config_data,
            registrar.id,
            42 // whatever
        ),
        Epp::EppResponseFailure,
        update_fail_nonexistent_handle_exception
    );
}

bool update_fail_wrong_registrar_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authorization_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::registrar_autor);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::unauthorized_registrar);
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_fail_wrong_registrar, has_nsset_and_a_different_registrar)
{
    const Epp::Nsset::UpdateNssetInputData input_data(
        nsset.handle,
        Optional<std::string>(),
        std::vector<Epp::Nsset::DnsHostInput>(),
        std::vector<Epp::Nsset::DnsHostInput>(),
        std::vector<std::string>(),
        std::vector<std::string>(),
        boost::optional<short>()
    );
    const Epp::Nsset::UpdateNssetConfigData config_data(2, 10);

    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::update_nsset(
            ctx,
            input_data,
            config_data,
            the_different_registrar.id,
            42 // whatever
        ),
        Epp::EppResponseFailure,
        update_fail_wrong_registrar_exception
    );
}

bool update_fail_prohibiting_status1_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_fail_prohibiting_status1, has_nsset_with_server_update_prohibited)
{
    const Epp::Nsset::UpdateNssetInputData input_data(
        nsset.handle,
        Optional<std::string>(),
        std::vector<Epp::Nsset::DnsHostInput>(),
        std::vector<Epp::Nsset::DnsHostInput>(),
        std::vector<std::string>(),
        std::vector<std::string>(),
        boost::optional<short>()
    );
    const Epp::Nsset::UpdateNssetConfigData config_data(2, 10);

    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::update_nsset(
            ctx,
            input_data,
            config_data,
            registrar.id,
            42 // whatever
        ),
        Epp::EppResponseFailure,
        update_fail_prohibiting_status1_exception
    );
}

bool update_fail_prohibiting_status2_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_fail_prohibiting_status2, has_nsset_with_delete_candidate)
{
    const Epp::Nsset::UpdateNssetInputData input_data(
        nsset.handle,
        Optional<std::string>(),
        std::vector<Epp::Nsset::DnsHostInput>(),
        std::vector<Epp::Nsset::DnsHostInput>(),
        std::vector<std::string>(),
        std::vector<std::string>(),
        boost::optional<short>()
    );
    const Epp::Nsset::UpdateNssetConfigData config_data(2, 10);

    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::update_nsset(
            ctx,
            input_data,
            config_data,
            registrar.id,
            42 // whatever
        ),
        Epp::EppResponseFailure,
        update_fail_prohibiting_status2_exception
    );
}

bool update_fail_prohibiting_status_request_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_fail_prohibiting_status_request, has_nsset_with_delete_candidate_request)
{
    const Epp::Nsset::UpdateNssetInputData input_data(
        nsset.handle,
        Optional<std::string>(),
        std::vector<Epp::Nsset::DnsHostInput>(),
        std::vector<Epp::Nsset::DnsHostInput>(),
        std::vector<std::string>(),
        std::vector<std::string>(),
        boost::optional<short>()
    );
    const Epp::Nsset::UpdateNssetConfigData config_data(2, 10);

    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::update_nsset(
            ctx,
            input_data,
            config_data,
            registrar.id,
            42 // whatever
        ),
        Epp::EppResponseFailure,
        update_fail_prohibiting_status_request_exception
    );

    /* now object has the state deleteCandidate itself */
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(nsset.id).exec(ctx) ) {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
            std::find( object_states_after.begin(), object_states_after.end(), status )
            !=
            object_states_after.end()
        );
    }
}


void check_after_update_data(const Epp::Nsset::UpdateNssetInputData& update_data,
        const Fred::InfoNssetData& info_data //info after update
        )
{

    BOOST_CHECK_EQUAL( boost::to_upper_copy( update_data.handle ), info_data.handle );

    if(update_data.authinfopw.isset())
    {
        BOOST_CHECK_EQUAL( update_data.authinfopw.get_value(), info_data.authinfopw );
    }

    //tech contacts to add are added before removal of technical contacts to remove
    {
        //added tech contacts sorted
        std::vector<std::string> update_tech_contacts_add = update_data.tech_contacts_add;
        std::sort(update_tech_contacts_add.begin(), update_tech_contacts_add.end());

        //removed tech contacts sorted
        std::vector<std::string> update_tech_contacts_rem = update_data.tech_contacts_rem;
        std::sort(update_tech_contacts_rem.begin(), update_tech_contacts_rem.end());

        //tech contacts after update converted sorted
        std::vector<std::string> info_tech_contacts;
        info_tech_contacts.reserve(info_data.tech_contacts.size());
        BOOST_FOREACH(const Fred::ObjectIdHandlePair& tc, info_data.tech_contacts)
        {
            info_tech_contacts.push_back(tc.handle);
        }
        std::sort(info_tech_contacts.begin(), info_tech_contacts.end());

        //tech_contacts_add - tech_contacts_rem
        std::vector<std::string> tech_contacts_add_diff_rem;
        std::set_difference(update_tech_contacts_add.begin(), update_tech_contacts_add.end(),
            update_tech_contacts_rem.begin(), update_tech_contacts_rem.end(),
            std::inserter(tech_contacts_add_diff_rem, tech_contacts_add_diff_rem.begin()));

        //tech contacts intersection of info with positive change
        std::vector<std::string> tech_contacts_ax;
        std::set_intersection(info_tech_contacts.begin(), info_tech_contacts.end(),
            tech_contacts_add_diff_rem.begin(), tech_contacts_add_diff_rem.end(),
                std::back_inserter(tech_contacts_ax));

        //intersection of tech contacts info with positive change == positive change
        BOOST_CHECK(tech_contacts_ax.size() == tech_contacts_add_diff_rem.size());
        BOOST_CHECK(std::equal(tech_contacts_ax.begin(), tech_contacts_ax.end(), tech_contacts_add_diff_rem.begin()));

        //tech contacts intersection of info with negative change
        std::vector<std::string> tech_contacts_rx;
        std::set_intersection(info_tech_contacts.begin(), info_tech_contacts.end(),
            update_tech_contacts_rem.begin(), update_tech_contacts_rem.end(),
                std::back_inserter(tech_contacts_rx));

        //intersection of tech contacts info with negative change have to be empty
        BOOST_CHECK(tech_contacts_rx.empty());

    }

    //DNS hosts to remove are removed before addition of DNS hosts to add
    {
        //removed DNS names converted and sorted
        std::vector<std::string> update_dns_host_names_rem;
        BOOST_FOREACH(const Epp::Nsset::DnsHostInput& dnshost, update_data.dns_hosts_rem)
        {
            update_dns_host_names_rem.push_back(dnshost.fqdn);
        }
        std::sort(update_dns_host_names_rem.begin(), update_dns_host_names_rem.end());

        //added DNS host names converted and sorted
        std::vector<std::string> update_dns_host_names_add;
        BOOST_FOREACH(const Epp::Nsset::DnsHostInput& dnshost, update_data.dns_hosts_add)
        {
            update_dns_host_names_add.push_back(dnshost.fqdn);
        }
        std::sort(update_dns_host_names_add.begin(), update_dns_host_names_add.end());

        //DNS host names after update converted sorted
        std::vector<std::string> info_dns_host_names;
        info_dns_host_names.reserve(info_data.dns_hosts.size());
        BOOST_FOREACH(const Fred::DnsHost& dnshost, info_data.dns_hosts)
        {
            info_dns_host_names.push_back(dnshost.get_fqdn());
        }
        std::sort(info_dns_host_names.begin(), info_dns_host_names.end());

        //DNS host names intersection of info with positive change
        std::vector<std::string> dns_host_names_ax;
        std::set_intersection(info_dns_host_names.begin(), info_dns_host_names.end(),
            update_dns_host_names_add.begin(), update_dns_host_names_add.end(),
                std::back_inserter(dns_host_names_ax));

        //intersection of DNS host names info with positive change == positive change
        BOOST_CHECK_EQUAL(dns_host_names_ax.size(), update_dns_host_names_add.size());
        BOOST_CHECK(std::equal(dns_host_names_ax.begin(), dns_host_names_ax.end(), update_dns_host_names_add.begin()));

        //update_dns_host_names_rem - update_dns_host_names_add
        std::vector<std::string> update_dns_host_names_rem_diff_add;
        std::set_difference(update_dns_host_names_rem.begin(), update_dns_host_names_rem.end(),
            update_dns_host_names_add.begin(), update_dns_host_names_add.end(),
            std::inserter(update_dns_host_names_rem_diff_add, update_dns_host_names_rem_diff_add.begin()));

        //DNS host names intersection of info with negative change
        std::vector<std::string> dns_host_names_rx;
        std::set_intersection(info_dns_host_names.begin(), info_dns_host_names.end(),
                update_dns_host_names_rem_diff_add.begin(), update_dns_host_names_rem_diff_add.end(),
                std::back_inserter(dns_host_names_rx));

        //intersection of DNS host names info with negative change have to be empty
        BOOST_CHECK(dns_host_names_rx.empty());

        std::map<std::string, std::set<boost::asio::ip::address> > info_dns_host_ip_set_by_fqdn;
        BOOST_FOREACH(const Fred::DnsHost& info_dnshost, info_data.dns_hosts)
        {
            std::vector<boost::asio::ip::address> tmp_ips = info_dnshost.get_inet_addr();
            std::set<boost::asio::ip::address> ipaddrs(tmp_ips.begin(), tmp_ips.end());
            info_dns_host_ip_set_by_fqdn[info_dnshost.get_fqdn()] = ipaddrs;
        }
        //check that all added ips are in the info data
        BOOST_FOREACH(const Epp::Nsset::DnsHostInput& added_dnshost, update_data.dns_hosts_add)
        {
            std::set<boost::asio::ip::address> info_ipaddrs = map_at(info_dns_host_ip_set_by_fqdn, added_dnshost.fqdn);
            BOOST_FOREACH(const boost::optional<boost::asio::ip::address>& ipaddr, added_dnshost.inet_addr)
            {
                BOOST_CHECK(ipaddr.is_initialized() && info_ipaddrs.find(ipaddr.get()) != info_ipaddrs.end());
            }
        }
    }

    if(update_data.tech_check_level)
    {
        BOOST_CHECK_EQUAL( *update_data.tech_check_level, info_data.tech_check_level.get_value_or_default());
    }

}


BOOST_FIXTURE_TEST_CASE(nsset_update_ok_full_data, has_nsset_with_all_data_set)
{
    Epp::Nsset::UpdateNssetInputData input_data(
            nsset.handle,
            "authInfo1234",
            Util::vector_of<Epp::Nsset::DnsHostInput>
                (Epp::Nsset::DnsHostInput("a.ns.nic.cz",
                    Util::vector_of<boost::optional<boost::asio::ip::address> >
                        (boost::asio::ip::address::from_string("11.0.0.3"))
                        (boost::asio::ip::address::from_string("11.1.1.3")))) //add_dns
                (Epp::Nsset::DnsHostInput("b.ns.nic.cz",
                    Util::vector_of<boost::optional<boost::asio::ip::address> >
                        (boost::asio::ip::address::from_string("11.2.0.4"))
                        (boost::asio::ip::address::from_string("11.3.1.4")))), //add_dns
            Util::vector_of<Epp::Nsset::DnsHostInput>
                (Epp::Nsset::DnsHostInput("a.ns.nic.cz",
                    std::vector<boost::optional<boost::asio::ip::address> >())), //rem_dns
            Util::vector_of<std::string>
                ("TEST-ADMIN-CONTACT4")
                ("TEST-ADMIN-CONTACT5"),//tech_contacts_add
            Util::vector_of<std::string>
                ("TEST-ADMIN-CONTACT2")
                ("TEST-ADMIN-CONTACT3"),//tech_contacts_rem
            3
        );
    const Epp::Nsset::UpdateNssetConfigData config_data(2, 10);

    try {
        Epp::Nsset::update_nsset(
            ctx,
            input_data,
            config_data,
            registrar.id,
            42 // whatever
        );
        check_after_update_data(input_data, Fred::InfoNssetByHandle(nsset.handle).exec(ctx).info_nsset_data);
    }
    catch(const Epp::EppResponseFailure& e)
    {
        if(e.epp_result().extended_errors()) {
            BOOST_ERROR(e.what());
            BOOST_FOREACH(const Epp::EppExtendedError& epp_extended_error, e.epp_result().extended_errors().get())
            {
                BOOST_ERROR(std::string("epp_extended_error: param: ") << epp_extended_error.param()
                    << std::string(" position: ") << epp_extended_error.position()
                    << std::string(" reason: ") << epp_extended_error.reason());
            }
        }
    }
    catch(const std::exception& e)
    {
        BOOST_ERROR(e.what());
    }

}

BOOST_FIXTURE_TEST_CASE(update_ok_states_are_upgraded, has_nsset_with_server_transfer_prohibited_request)
{
    Epp::Nsset::UpdateNssetInputData input_data(
            nsset.handle,
            "authInfo1234",
            std::vector<Epp::Nsset::DnsHostInput>(), //add_dns
            std::vector<Epp::Nsset::DnsHostInput>(), //rem_dns
            std::vector<std::string>(),//0
            std::vector<std::string>(),
            3
        );
    const Epp::Nsset::UpdateNssetConfigData config_data(2, 10);

    Epp::Nsset::update_nsset(
        ctx,
        input_data,
        config_data,
        registrar.id,
        42 // whatever
    );

    check_after_update_data(input_data, Fred::InfoNssetByHandle(nsset.handle).exec(ctx).info_nsset_data);

    /* now object has the state server_transfer_prohibited itself */
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(nsset.id).exec(ctx) ) {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
            std::find( object_states_after.begin(), object_states_after.end(), status )
            !=
            object_states_after.end()
        );
    }

}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
