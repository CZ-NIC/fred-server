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
 *  registry record statement impl tests
 */

#include "tests/interfaces/record_statement/util.h"
#include "tests/interfaces/record_statement/fixture.h"

#include "src/record_statement/record_statement.hh"

#include "src/record_statement/impl/record_statement_xml.hh"
#include "src/fredlib/object_state/get_object_states.h"

#include "util/subprocess.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

#include <set>
#include <vector>
#include <fstream>
#include <sstream>



BOOST_AUTO_TEST_SUITE(TestRecordStatement)

BOOST_AUTO_TEST_CASE(historic_timestamp_conversion)
{
    using namespace Fred::RecordStatement;

    {
        Fred::OperationContextCreator ctx;
        BOOST_CHECK_THROW(ctx.get_conn().exec_params(Database::ParamQuery(
                "SELECT ")(make_utc_timestamp_query(""))),
            Database::ResultFailed);
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29T14:51:45+02:00");
        //BOOST_TEST_MESSAGE(expr.get_query().first);
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 12:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29T14:51:45+02");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 12:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29T14:51:45-01:00");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 15:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29T14:51:45-01");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 15:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29T12:51:45Z");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 12:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29T12:51:45");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 12:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29t14:51:45+02:00");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 12:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29t14:51:45+02");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 12:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29t14:51:45-01:00");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 15:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29t14:51:45-01");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 15:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29t12:51:45z");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 12:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29t12:51:45");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 12:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29T14:51:45.053106+02:00");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 12:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29T14:51:45.053106+02");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 12:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29T14:51:45.053106-01:00");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 15:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29T14:51:45.053106-01");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 15:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29T14:51:45.053106Z");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 14:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }

    try
    {
        Fred::OperationContextCreator ctx;
        Database::ParamQuery expr = make_utc_timestamp_query("2017-05-29T14:51:45.053106");
        BOOST_CHECK(static_cast<std::string>(ctx.get_conn().exec_params(
            Database::ParamQuery("SELECT ")(expr))[0][0]) == "2017-05-29 14:51:45");
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }
}

BOOST_FIXTURE_TEST_CASE(domain_printout_xml, Test::domain_fixture)
{

    try {
        Fred::InfoDomainOutput info_domain_output = Fred::InfoDomainByHandle(
            test_fqdn).exec(ctx, "UTC");
        Fred::InfoContactOutput info_registrant_output = Fred::InfoContactByHandle(
            info_domain_output.info_domain_data.registrant.handle).exec(ctx, "UTC");

        std::vector<Fred::InfoContactOutput> info_admin_contact_output;

        Fred::InfoRegistrarOutput info_sponsoring_registrar_output = Fred::InfoRegistrarByHandle(
                info_domain_output.info_domain_data.sponsoring_registrar_handle).exec(ctx, "UTC");

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

        boost::optional<Fred::RecordStatement::NssetPrintoutInputData> nsset_data
            = Fred::RecordStatement::make_nsset_data(nsset_handle, ctx);


        boost::optional<std::string> keyset_handle = info_domain_output.info_domain_data.keyset.isnull()
            ? boost::optional<std::string>()
            : boost::optional<std::string>(info_domain_output.info_domain_data.keyset.get_value().handle);

        boost::optional<Fred::RecordStatement::KeysetPrintoutInputData> keyset_data
            = Fred::RecordStatement::make_keyset_data(keyset_handle, ctx);

        boost::posix_time::ptime test_timestamp_utc = boost::posix_time::time_from_string("2017-05-04 11:06:00");//"2017-05-04T13:06:00+02:00"
        info_domain_output.utc_timestamp = test_timestamp_utc;

        std::string test_domain_xml = Fred::RecordStatement::domain_printout_xml(info_domain_output,
            Fred::RecordStatement::convert_utc_timestamp_to_local(ctx, test_timestamp_utc, "Europe/Prague"),
            Fred::RecordStatement::convert_utc_timestamp_to_local(ctx, info_domain_output.info_domain_data.creation_time, "Europe/Prague"),
            (info_domain_output.info_domain_data.update_time.isnull()
                ? boost::optional<boost::posix_time::ptime>()
                : boost::optional<boost::posix_time::ptime>(
                        Fred::RecordStatement::convert_utc_timestamp_to_local(
                        ctx, info_domain_output.info_domain_data.update_time.get_value(), "Europe/Prague"))),
                false, //is_private_printout
                info_registrant_output,
                info_admin_contact_output,
                info_sponsoring_registrar_output,
                nsset_data,
                keyset_data,
                Fred::RecordStatement::make_external_states(info_domain_output.info_domain_data.id, ctx)
        );

        //BOOST_TEST_MESSAGE(test_domain_xml);

        BOOST_CHECK(test_domain_xml == std::string(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<record_statement>"
                "<current_datetime>2017-05-04T13:06:00+02:00</current_datetime>"
                "<domain>"
                    "<fqdn>fredinfo.cz</fqdn>"
                    "<creation_date>2011-07-01</creation_date>"
                    "<last_update_date></last_update_date>"
                    "<expiration_date>2012-06-30</expiration_date>"
                    "<holder is_private_printout='false'>"
                        "<handle>TEST-REGISTRANT-CONTACT-HANDLE</handle>"
                        "<name>TEST-REGISTRANT-CONTACT NAME</name>"
                        "<organization></organization>"
                        "<id_number>123456789</id_number>"
                        "<street1>STR1</street1>"
                        "<street2></street2>"
                        "<street3></street3>"
                        "<city>Praha</city>"
                        "<stateorprovince></stateorprovince>"
                        "<postal_code>11150</postal_code>"
                        "<country>CZ</country>"
                        "<disclose"
                            " name='true'"
                            " organization='true'"
                            " address='true'"
                            " telephone='false'"
                            " fax='false'"
                            " email='false'"
                            " vat='false'"
                            " ident='false'"
                            " notifyemail='false'>"
                        "</disclose>"
                    "</holder>"
                    "<admin_contact_list>"
                        "<admin_contact>"
                            "<handle>TEST-ADMIN-CONTACT2-HANDLE</handle>"
                            "<name>TEST-ADMIN-CONTACT2 NAME</name>"
                            "<organization></organization>"
                        "</admin_contact>"
                    "</admin_contact_list>"
                    "<sponsoring_registrar>"
                        "<handle>REGISTRAR1</handle>"
                        "<name></name>"
                        "<organization></organization>"
                    "</sponsoring_registrar>"
                    "<nsset>"
                        "<handle>TEST-NSSET-HANDLE</handle>"
                        "<nameserver_list>"
                            "<nameserver>"
                                "<fqdn>a.ns.nic.cz</fqdn>"
                                "<ip_list>"
                                    "<ip>127.0.0.3</ip>"
                                    "<ip>127.1.1.3</ip>"
                                "</ip_list>"
                            "</nameserver>"
                            "<nameserver>"
                                "<fqdn>b.ns.nic.cz</fqdn>"
                                "<ip_list>"
                                    "<ip>127.0.0.4</ip>"
                                    "<ip>127.1.1.4</ip>"
                                "</ip_list>"
                            "</nameserver>"
                        "</nameserver_list>"
                        "<tech_contact_list>"
                            "<tech_contact>"
                                "<handle>TEST-ADMIN-CONTACT-HANDLE</handle>"
                                "<name>TEST-ADMIN-CONTACT NAME</name>"
                                "<organization></organization>"
                            "</tech_contact>"
                            "<tech_contact>"
                                "<handle>TEST-ADMIN-CONTACT2-HANDLE</handle>"
                                "<name>TEST-ADMIN-CONTACT2 NAME</name>"
                                "<organization></organization>"
                            "</tech_contact>"
                        "</tech_contact_list>"
                        "<sponsoring_registrar>"
                            "<handle>REGISTRAR1</handle>"
                            "<name></name>"
                            "<organization></organization>"
                        "</sponsoring_registrar>"
                        "<external_states_list>"
                            "<state>linked</state>"
                        "</external_states_list>"
                    "</nsset>"
                    "<keyset>"
                        "<handle>TEST-KEYSET-HANDLE</handle>"
                        "<dns_key_list>"
                            "<dns_key>"
                                "<flags>257</flags>"
                                "<protocol>3</protocol>"
                                "<algorithm>5</algorithm>"
                                "<key>test1</key>"
                            "</dns_key>"
                            "<dns_key>"
                                "<flags>257</flags>"
                                "<protocol>3</protocol>"
                                "<algorithm>5</algorithm>"
                                "<key>test2</key>"
                            "</dns_key>"
                        "</dns_key_list>"
                        "<tech_contact_list>"
                            "<tech_contact>"
                                "<handle>TEST-ADMIN-CONTACT-HANDLE</handle>"
                                "<name>TEST-ADMIN-CONTACT NAME</name>"
                                "<organization></organization>"
                            "</tech_contact>"
                            "<tech_contact>"
                                "<handle>TEST-ADMIN-CONTACT2-HANDLE</handle>"
                                "<name>TEST-ADMIN-CONTACT2 NAME</name>"
                                "<organization></organization>"
                            "</tech_contact>"
                        "</tech_contact_list>"
                        "<sponsoring_registrar>"
                            "<handle>REGISTRAR1</handle>"
                            "<name></name>"
                            "<organization></organization>"
                        "</sponsoring_registrar>"
                        "<external_states_list>"
                            "<state>linked</state>"
                        "</external_states_list>"
                    "</keyset>"
                    "<external_states_list>"
                    "</external_states_list>"
                "</domain>"
            "</record_statement>")
        );

    } catch(boost::exception& ex)
    {
        BOOST_ERROR( boost::diagnostic_information(ex));
    }

}

BOOST_FIXTURE_TEST_CASE(nsset_printout_xml, Test::domain_fixture)
{
    try
    {
        boost::optional<std::string> nsset_handle(test_nsset_handle);

        boost::optional<Fred::RecordStatement::NssetPrintoutInputData> nsset_data
            = Fred::RecordStatement::make_nsset_data(nsset_handle, ctx);

        boost::posix_time::ptime test_timestamp_utc = boost::posix_time::time_from_string("2017-05-04 11:06:00");//"2017-05-04T13:06:00+02:00"
        nsset_data.operator *().info.utc_timestamp = test_timestamp_utc;

        std::string test_nsset_xml = Fred::RecordStatement::nsset_printout_xml(*nsset_data,
            Fred::RecordStatement::convert_utc_timestamp_to_local(ctx, test_timestamp_utc, "Europe/Prague"));

        //BOOST_TEST_MESSAGE(test_nsset_xml);

        BOOST_CHECK(test_nsset_xml == std::string(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<record_statement>"
            "<current_datetime>2017-05-04T13:06:00+02:00</current_datetime>"
            "<nsset>"
                "<handle>TEST-NSSET-HANDLE</handle>"
                "<nameserver_list>"
                    "<nameserver>"
                        "<fqdn>a.ns.nic.cz</fqdn>"
                        "<ip_list>"
                            "<ip>127.0.0.3</ip>"
                            "<ip>127.1.1.3</ip>"
                        "</ip_list>"
                    "</nameserver>"
                    "<nameserver>"
                        "<fqdn>b.ns.nic.cz</fqdn>"
                        "<ip_list>"
                            "<ip>127.0.0.4</ip>"
                            "<ip>127.1.1.4</ip>"
                        "</ip_list>"
                    "</nameserver>"
                "</nameserver_list>"
                "<tech_contact_list>"
                    "<tech_contact>"
                        "<handle>TEST-ADMIN-CONTACT-HANDLE</handle>"
                        "<name>TEST-ADMIN-CONTACT NAME</name>"
                        "<organization></organization>"
                    "</tech_contact>"
                    "<tech_contact>"
                        "<handle>TEST-ADMIN-CONTACT2-HANDLE</handle>"
                        "<name>TEST-ADMIN-CONTACT2 NAME</name>"
                        "<organization></organization>"
                    "</tech_contact>"
                "</tech_contact_list>"
                "<sponsoring_registrar>"
                    "<handle>REGISTRAR1</handle>"
                    "<name></name>"
                    "<organization></organization>"
                "</sponsoring_registrar>"
                "<external_states_list>"
                    "<state>linked</state>"
                "</external_states_list>"
            "</nsset>"
            "</record_statement>"));

    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR( boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(keyset_printout_xml, Test::domain_fixture)
{
    try
    {
        boost::optional<std::string> keyset_handle(test_keyset_handle);

        boost::optional<Fred::RecordStatement::KeysetPrintoutInputData> keyset_data
            = Fred::RecordStatement::make_keyset_data(keyset_handle, ctx);

        boost::posix_time::ptime test_timestamp_utc = boost::posix_time::time_from_string("2017-05-04 11:06:00");//"2017-05-04T13:06:00+02:00"
        keyset_data.operator *().info.utc_timestamp = test_timestamp_utc;

        std::string test_keyset_xml = Fred::RecordStatement::keyset_printout_xml(*keyset_data,
            Fred::RecordStatement::convert_utc_timestamp_to_local(ctx, test_timestamp_utc, "Europe/Prague"));

        //BOOST_TEST_MESSAGE(test_keyset_xml);

        BOOST_CHECK(test_keyset_xml == std::string(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<record_statement>"
            "<current_datetime>2017-05-04T13:06:00+02:00</current_datetime>"
            "<keyset>"
                "<handle>TEST-KEYSET-HANDLE</handle>"
                "<dns_key_list>"
                    "<dns_key>"
                        "<flags>257</flags>"
                        "<protocol>3</protocol>"
                        "<algorithm>5</algorithm>"
                        "<key>test1</key>"
                    "</dns_key>"
                    "<dns_key>"
                        "<flags>257</flags>"
                        "<protocol>3</protocol>"
                        "<algorithm>5</algorithm>"
                        "<key>test2</key>"
                    "</dns_key>"
                "</dns_key_list>"
                "<tech_contact_list>"
                    "<tech_contact>"
                        "<handle>TEST-ADMIN-CONTACT-HANDLE</handle>"
                        "<name>TEST-ADMIN-CONTACT NAME</name>"
                        "<organization></organization>"
                    "</tech_contact>"
                    "<tech_contact>"
                        "<handle>TEST-ADMIN-CONTACT2-HANDLE</handle>"
                        "<name>TEST-ADMIN-CONTACT2 NAME</name>"
                        "<organization></organization>"
                    "</tech_contact>"
                "</tech_contact_list>"
                "<sponsoring_registrar>"
                    "<handle>REGISTRAR1</handle>"
                    "<name></name>"
                    "<organization></organization>"
                "</sponsoring_registrar>"
                "<external_states_list>"
                    "<state>linked</state>"
                "</external_states_list>"
            "</keyset>"
        "</record_statement>"
        ));

    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR( boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(contact_printout_xml, Test::domain_fixture)
{
    try
    {
        boost::posix_time::ptime test_timestamp_utc = boost::posix_time::time_from_string("2017-05-04 11:06:00");//"2017-05-04T13:06:00+02:00"

        Fred::InfoContactOutput info_contact_output = Fred::InfoContactByHandle(registrant_contact_handle).exec(ctx, "UTC");
        info_contact_output.utc_timestamp = test_timestamp_utc;

        Fred::InfoRegistrarOutput info_sponsoring_registrar_output = Fred::InfoRegistrarByHandle(
                info_contact_output.info_contact_data.sponsoring_registrar_handle).exec(ctx, "UTC");

        std::string test_contact_xml = Fred::RecordStatement::contact_printout_xml(
            true, //is_private_printout
            info_contact_output,
            Fred::RecordStatement::convert_utc_timestamp_to_local(ctx, test_timestamp_utc, "Europe/Prague"),
            Fred::RecordStatement::convert_utc_timestamp_to_local(ctx, info_contact_output.info_contact_data.creation_time, "Europe/Prague"),
            (info_contact_output.info_contact_data.update_time.isnull()
                ? boost::optional<boost::posix_time::ptime>()
                : boost::optional<boost::posix_time::ptime>(
                    Fred::RecordStatement::convert_utc_timestamp_to_local(
                        ctx, info_contact_output.info_contact_data.update_time.get_value(), "Europe/Prague"))),
            (info_contact_output.info_contact_data.transfer_time.isnull()
                ? boost::optional<boost::posix_time::ptime>()
                : boost::optional<boost::posix_time::ptime>(
                    Fred::RecordStatement::convert_utc_timestamp_to_local(
                        ctx, info_contact_output.info_contact_data.transfer_time.get_value(), "Europe/Prague"))),
            info_sponsoring_registrar_output,
            Fred::RecordStatement::make_external_states(info_contact_output.info_contact_data.id, ctx)
        );

        //BOOST_TEST_MESSAGE(test_contact_xml);

        BOOST_CHECK(test_contact_xml == std::string(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<record_statement>"
                "<current_datetime>2017-05-04T13:06:00+02:00</current_datetime>"
                "<contact is_private_printout='true'>"
                    "<handle>TEST-REGISTRANT-CONTACT-HANDLE</handle>"
                    "<name>TEST-REGISTRANT-CONTACT NAME</name>"
                    "<organization></organization>"
                    "<taxpayer_id_number></taxpayer_id_number>"
                    "<id_type>ICO</id_type>"
                    "<id_value>123456789</id_value>"
                    "<email>jan.zima@nic.cz</email>"
                    "<notification_email></notification_email>"
                    "<phone></phone>"
                    "<fax></fax>"
                    "<creation_date>2010-07-01</creation_date>"
                    "<last_update_date></last_update_date>"
                    "<last_transfer_date></last_transfer_date>"
                    "<address>"
                        "<street1>STR1</street1>"
                        "<street2></street2>"
                        "<street3></street3>"
                        "<city>Praha</city>"
                        "<stateorprovince></stateorprovince>"
                        "<postal_code>11150</postal_code>"
                        "<country>CZ</country>"
                    "</address>"
                    "<sponsoring_registrar>"
                        "<handle>REGISTRAR1</handle>"
                        "<name></name>"
                        "<organization></organization>"
                    "</sponsoring_registrar>"
                    "<disclose"
                        " name='true'"
                        " organization='true'"
                        " address='true'"
                        " telephone='false'"
                        " fax='false'"
                        " email='false'"
                        " vat='false'"
                        " ident='false'"
                        " notifyemail='false'>"
                    "</disclose>"
                    "<external_states_list>"
                        "<state>linked</state>"
                    "</external_states_list>"
                "</contact>"
            "</record_statement>"
            ));

    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR( boost::diagnostic_information(ex));
    }
}

void save_pdf(const std::string& file_name, const Registry::RecordStatement::PdfBufferImpl& pdf_buff)
{
    std::ofstream pdf_file(file_name.c_str(), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
    pdf_file.write(pdf_buff.value.c_str(), pdf_buff.value.size());
    pdf_file.close();
}

BOOST_FIXTURE_TEST_CASE(domain_printout_pdf, Test::domain_fixture)
{
    try
    {
        Registry::RecordStatement::PdfBufferImpl pdf_buff = rs_impl.domain_printout(test_fqdn, true, &ctx);

        BOOST_CHECK(pdf_buff.value.size() > 0);

        //debug file output
        save_pdf("domain_printout.pdf",pdf_buff);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR( boost::diagnostic_information(ex));
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR( ex.what());
    }
}

BOOST_FIXTURE_TEST_CASE(domain_printout_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(
        rs_impl.domain_printout(test_fqdn+"bad", true, &ctx),
        Registry::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(nsset_printout_pdf, Test::domain_fixture)
{
    try
    {
        Registry::RecordStatement::PdfBufferImpl pdf_buff = rs_impl.nsset_printout(test_nsset_handle, &ctx);

        BOOST_CHECK(pdf_buff.value.size() > 0);

        //debug file output
        save_pdf("nsset_printout.pdf",pdf_buff);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR( boost::diagnostic_information(ex));
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR( ex.what());
    }
}

BOOST_FIXTURE_TEST_CASE(nsset_printout_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl.nsset_printout(test_nsset_handle+"bad", &ctx),
        Registry::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(keyset_printout_pdf, Test::domain_fixture)
{
    try
    {
        Registry::RecordStatement::PdfBufferImpl pdf_buff = rs_impl.keyset_printout(test_keyset_handle, &ctx);

        BOOST_CHECK(pdf_buff.value.size() > 0);

        //debug file output
        save_pdf("keyset_printout.pdf",pdf_buff);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR( boost::diagnostic_information(ex));
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR( ex.what());
    }
}

BOOST_FIXTURE_TEST_CASE(keyset_printout_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl.keyset_printout(test_keyset_handle+"bad", &ctx),
        Registry::RecordStatement::ObjectNotFound);
}


BOOST_FIXTURE_TEST_CASE(contact_printout_pdf, Test::domain_fixture)
{
    try
    {
        Registry::RecordStatement::PdfBufferImpl pdf_buff = rs_impl.contact_printout(registrant_contact_handle, true, &ctx);

        BOOST_CHECK(pdf_buff.value.size() > 0);

        //debug file output
        save_pdf("contact_printout.pdf",pdf_buff);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR( boost::diagnostic_information(ex));
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR(ex.what());
    }
}

BOOST_FIXTURE_TEST_CASE(contact_printout_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl.contact_printout(registrant_contact_handle+"bad", true, &ctx),
        Registry::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(historic_domain_printout_pdf, Test::domain_by_name_and_time_fixture)
{
    try
    {
        Registry::RecordStatement::PdfBufferImpl pdf_buff = rs_impl.historic_domain_printout(
            test_fqdn,
            timestamp,
            &ctx);

        BOOST_CHECK(pdf_buff.value.size() > 0);

        //debug file output
        save_pdf("historic_domain_printout.pdf",pdf_buff);

    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR( boost::diagnostic_information(ex));
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR( ex.what());
    }
}

BOOST_FIXTURE_TEST_CASE(historic_domain_bad_timestamp, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(rs_impl.historic_domain_printout(
        test_fqdn,
        "bad"+timestamp,
        &ctx),
        Registry::RecordStatement::InvalidTimestamp);
}

BOOST_FIXTURE_TEST_CASE(historic_domain_not_found, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(rs_impl.historic_domain_printout(
        "bad"+test_fqdn,
        timestamp,
        &ctx),
        Registry::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(historic_nsset_printout_pdf, Test::domain_by_name_and_time_fixture)
{
    try
    {
        Registry::RecordStatement::PdfBufferImpl pdf_buff = rs_impl.historic_nsset_printout(test_nsset_handle, timestamp, &ctx);

        BOOST_CHECK(pdf_buff.value.size() > 0);

        //debug file output
        save_pdf("historic_nsset_printout.pdf",pdf_buff);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR( boost::diagnostic_information(ex));
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR( ex.what());
    }
}

BOOST_FIXTURE_TEST_CASE(historic_nsset_bad_timestamp, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(rs_impl.historic_nsset_printout(
        test_nsset_handle,
        "bad"+timestamp,
        &ctx),
        Registry::RecordStatement::InvalidTimestamp);
}

BOOST_FIXTURE_TEST_CASE(historic_nsset_not_found, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(rs_impl.historic_nsset_printout(
        "bad"+test_nsset_handle,
        timestamp,
        &ctx),
        Registry::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(historic_keyset_printout_pdf, Test::domain_by_name_and_time_fixture)
{
    try
    {
        Registry::RecordStatement::PdfBufferImpl pdf_buff = rs_impl.historic_keyset_printout(test_keyset_handle, timestamp, &ctx);

        BOOST_CHECK(pdf_buff.value.size() > 0);

        //debug file output
        save_pdf("historic_keyset_printout.pdf",pdf_buff);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR( boost::diagnostic_information(ex));
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR( ex.what());
    }
}

BOOST_FIXTURE_TEST_CASE(historic_keyset_bad_timestamp, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(rs_impl.historic_keyset_printout(
        test_keyset_handle,
        "bad"+timestamp,
        &ctx),
        Registry::RecordStatement::InvalidTimestamp);
}

BOOST_FIXTURE_TEST_CASE(historic_keyset_not_found, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(rs_impl.historic_keyset_printout(
        "bad"+test_keyset_handle,
        timestamp,
        &ctx),
        Registry::RecordStatement::ObjectNotFound);
}


BOOST_FIXTURE_TEST_CASE(historic_contact_printout_pdf, Test::domain_by_name_and_time_fixture)
{
    try
    {
        Registry::RecordStatement::PdfBufferImpl pdf_buff = rs_impl.historic_contact_printout(registrant_contact_handle, timestamp, &ctx);

        BOOST_CHECK(pdf_buff.value.size() > 0);

        //debug file output
        save_pdf("historic_contact_printout.pdf",pdf_buff);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR( boost::diagnostic_information(ex));
    }
    catch(std::exception& ex)
    {
        BOOST_ERROR( ex.what());
    }
}

BOOST_FIXTURE_TEST_CASE(historic_contact_bad_timestamp, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(rs_impl.historic_contact_printout(
        registrant_contact_handle,
        "bad"+timestamp,
        &ctx),
        Registry::RecordStatement::InvalidTimestamp);
}

BOOST_FIXTURE_TEST_CASE(historic_contact_not_found, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(rs_impl.historic_contact_printout(
        "bad"+registrant_contact_handle,
        timestamp,
        &ctx),
        Registry::RecordStatement::ObjectNotFound);
}


BOOST_FIXTURE_TEST_CASE(email_domain_printout_pdf, Test::domain_fixture)
{
    BOOST_CHECK_NO_THROW(rs_impl.send_domain_printout(test_fqdn, true, &ctx));
}

BOOST_FIXTURE_TEST_CASE(email_domain_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl.send_domain_printout("bad"+test_fqdn, true, &ctx),
        Registry::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(email_nsset_printout_pdf, Test::domain_fixture)
{
    BOOST_CHECK_NO_THROW(rs_impl.send_nsset_printout(test_nsset_handle, &ctx));
}

BOOST_FIXTURE_TEST_CASE(email_nsset_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl.send_nsset_printout("bad"+test_nsset_handle, &ctx),
        Registry::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(email_keyset_printout_pdf, Test::domain_fixture)
{
    BOOST_CHECK_NO_THROW(rs_impl.send_keyset_printout(test_keyset_handle, &ctx));
}

BOOST_FIXTURE_TEST_CASE(email_keyset_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl.send_keyset_printout("bad"+test_keyset_handle, &ctx),
        Registry::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(email_contact_printout_pdf, Test::domain_fixture)
{
    BOOST_CHECK_NO_THROW(rs_impl.send_contact_printout(admin_contact_handle, true, &ctx));
}

BOOST_FIXTURE_TEST_CASE(email_contact_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl.send_contact_printout("bad"+admin_contact_handle, true, &ctx),
        Registry::RecordStatement::ObjectNotFound);
}



BOOST_AUTO_TEST_CASE(historic_timestamp)
{
    using namespace Fred::RecordStatement;

    {
        TimeWithOffset time_off = make_time_with_offset("");
        BOOST_CHECK(time_off.time.empty());
        BOOST_CHECK(time_off.offset.empty());
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29T14:51:45+02:00");
        BOOST_CHECK(time_off.time == "2017-05-29T14:51:45");
        BOOST_CHECK(time_off.offset == "+02:00");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29T14:51:45+02");
        BOOST_CHECK(time_off.time == "2017-05-29T14:51:45");
        BOOST_CHECK(time_off.offset == "+02");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29T14:51:45-01:00");
        BOOST_CHECK(time_off.time == "2017-05-29T14:51:45");
        BOOST_CHECK(time_off.offset == "-01:00");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29T14:51:45-01");
        BOOST_CHECK(time_off.time == "2017-05-29T14:51:45");
        BOOST_CHECK(time_off.offset == "-01");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29T12:51:45Z");
        BOOST_CHECK(time_off.time == "2017-05-29T12:51:45");
        BOOST_CHECK(time_off.offset == "Z");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29T12:51:45");
        BOOST_CHECK(time_off.time == "2017-05-29T12:51:45");
        BOOST_CHECK(time_off.offset == "");
    }



    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29t14:51:45+02:00");
        BOOST_CHECK(time_off.time == "2017-05-29t14:51:45");
        BOOST_CHECK(time_off.offset == "+02:00");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29t14:51:45+02");
        BOOST_CHECK(time_off.time == "2017-05-29t14:51:45");
        BOOST_CHECK(time_off.offset == "+02");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29t14:51:45-01:00");
        BOOST_CHECK(time_off.time == "2017-05-29t14:51:45");
        BOOST_CHECK(time_off.offset == "-01:00");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29t14:51:45-01");
        BOOST_CHECK(time_off.time == "2017-05-29t14:51:45");
        BOOST_CHECK(time_off.offset == "-01");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29t12:51:45z");
        BOOST_CHECK(time_off.time == "2017-05-29t12:51:45");
        BOOST_CHECK(time_off.offset == "z");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29t12:51:45");
        BOOST_CHECK(time_off.time == "2017-05-29t12:51:45");
        BOOST_CHECK(time_off.offset == "");
    }



    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29T14:51:45.053106+02:00");
        BOOST_CHECK(time_off.time == "2017-05-29T14:51:45.053106");
        BOOST_CHECK(time_off.offset == "+02:00");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29T14:51:45.053106+02");
        BOOST_CHECK(time_off.time == "2017-05-29T14:51:45.053106");
        BOOST_CHECK(time_off.offset == "+02");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29T14:51:45.053106-01:00");
        BOOST_CHECK(time_off.time == "2017-05-29T14:51:45.053106");
        BOOST_CHECK(time_off.offset == "-01:00");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29T14:51:45.053106-01");
        BOOST_CHECK(time_off.time == "2017-05-29T14:51:45.053106");
        BOOST_CHECK(time_off.offset == "-01");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29T12:51:45.053106Z");
        BOOST_CHECK(time_off.time == "2017-05-29T12:51:45.053106");
        BOOST_CHECK(time_off.offset == "Z");
    }

    {
        TimeWithOffset time_off = make_time_with_offset("2017-05-29T12:51:45.053106");
        BOOST_CHECK(time_off.time == "2017-05-29T12:51:45.053106");
        BOOST_CHECK(time_off.offset == "");
    }

}

BOOST_AUTO_TEST_SUITE_END();


