/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file
 *  registry record statement impl tests
 */

#include "test/backend/record_statement/util.hh"
#include "test/backend/record_statement/fixture.hh"

#include "src/backend/record_statement/record_statement.hh"

#include "src/backend/record_statement/impl/record_statement_xml.hh"
#include "libfred/object_state/get_object_states.hh"

#include "src/util/subprocess.hh"

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

BOOST_FIXTURE_TEST_CASE(domain_printout_xml, Test::domain_fixture)
{
    try
    {
        ::LibFred::InfoDomainOutput info_domain_output =
                ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx, "UTC");
        const ::LibFred::InfoContactOutput info_registrant_output =
                ::LibFred::InfoContactByHandle(info_domain_output.info_domain_data.registrant.handle).exec(ctx, "UTC");

        std::vector<::LibFred::InfoContactOutput> info_admin_contact_output;

        const ::LibFred::InfoRegistrarOutput info_sponsoring_registrar_output = ::LibFred::InfoRegistrarByHandle(
                info_domain_output.info_domain_data.sponsoring_registrar_handle).exec(ctx, "UTC");

        for (std::vector<::LibFred::ObjectIdHandlePair>::const_iterator itr = info_domain_output.info_domain_data.admin_contacts.begin();
             itr != info_domain_output.info_domain_data.admin_contacts.end(); ++itr)
        {
            info_admin_contact_output.push_back(
                ::LibFred::InfoContactByHandle(itr->handle).exec(ctx, "UTC"));
        }

        const boost::optional<std::string> nsset_handle = info_domain_output.info_domain_data.nsset.isnull()
            ? boost::optional<std::string>()
            : info_domain_output.info_domain_data.nsset.get_value().handle;

        const boost::optional<Fred::Backend::RecordStatement::Impl::NssetPrintoutInputData> nsset_data =
                Fred::Backend::RecordStatement::Impl::make_nsset_data(nsset_handle, ctx);


        const boost::optional<std::string> keyset_handle = info_domain_output.info_domain_data.keyset.isnull()
            ? boost::optional<std::string>()
            : info_domain_output.info_domain_data.keyset.get_value().handle;

        const boost::optional<Fred::Backend::RecordStatement::Impl::KeysetPrintoutInputData> keyset_data =
                Fred::Backend::RecordStatement::Impl::make_keyset_data(keyset_handle, ctx);

        info_domain_output.utc_timestamp = boost::posix_time::time_from_string("2017-05-04 11:06:00");
        const Tz::LocalTimestamp data_valid_at =
                Tz::LocalTimestamp(boost::posix_time::time_from_string("2017-05-01 08:32:17"), 2 * 60);

        const std::string test_domain_xml = Fred::Backend::RecordStatement::Impl::domain_printout_xml<Tz::Europe::Prague>(
                ctx,
                info_domain_output,
                data_valid_at,
                Fred::Backend::RecordStatement::Purpose::public_printout,
                info_registrant_output,
                info_admin_contact_output,
                info_sponsoring_registrar_output,
                nsset_data,
                keyset_data,
                Fred::Backend::RecordStatement::Impl::make_external_states(info_domain_output.info_domain_data.id, ctx));

        //BOOST_TEST_MESSAGE(test_domain_xml);

        BOOST_CHECK_EQUAL(
                test_domain_xml,
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<record_statement>"
                    "<current_datetime>2017-05-01T08:32:17+02:00</current_datetime>"
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
                            "<disclose "
                                "name='true' "
                                "organization='true' "
                                "address='true' "
                                "telephone='false' "
                                "fax='false' "
                                "email='false' "
                                "vat='false' "
                                "ident='false' "
                                "notifyemail='false'>"
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
                "</record_statement>");
    }
    catch (const boost::exception& e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
    }
}

BOOST_FIXTURE_TEST_CASE(nsset_printout_xml, Test::domain_fixture)
{
    try
    {
        boost::optional<std::string> nsset_handle(test_nsset_handle);

        boost::optional<Fred::Backend::RecordStatement::Impl::NssetPrintoutInputData> nsset_data
            = Fred::Backend::RecordStatement::Impl::make_nsset_data(nsset_handle, ctx);

        nsset_data->info.utc_timestamp = boost::posix_time::time_from_string("2017-05-04 11:06:00");
        const Tz::LocalTimestamp data_valid_at =
                Tz::LocalTimestamp(boost::posix_time::time_from_string("2017-05-04 13:06:00"), 2 * 60);

        const std::string test_nsset_xml = Fred::Backend::RecordStatement::Impl::nsset_printout_xml(
                *nsset_data,
                data_valid_at);

        //BOOST_TEST_MESSAGE(test_nsset_xml);

        BOOST_CHECK_EQUAL(
                test_nsset_xml,
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
                    "</record_statement>");
    }
    catch (const boost::exception& e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
    }
}

BOOST_FIXTURE_TEST_CASE(keyset_printout_xml, Test::domain_fixture)
{
    try
    {
        boost::optional<std::string> keyset_handle(test_keyset_handle);

        boost::optional<Fred::Backend::RecordStatement::Impl::KeysetPrintoutInputData> keyset_data
            = Fred::Backend::RecordStatement::Impl::make_keyset_data(keyset_handle, ctx);

        keyset_data->info.utc_timestamp = boost::posix_time::time_from_string("2017-05-04 11:06:00");
        const Tz::LocalTimestamp data_valid_at =
                Tz::LocalTimestamp(boost::posix_time::time_from_string("2017-05-04 13:06:00"), 2 * 60);

        const std::string test_keyset_xml = Fred::Backend::RecordStatement::Impl::keyset_printout_xml(
                *keyset_data,
                data_valid_at);

        //BOOST_TEST_MESSAGE(test_keyset_xml);

        BOOST_CHECK_EQUAL(
                test_keyset_xml,
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
                "</record_statement>");
    }
    catch (const boost::exception& e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
    }
}

BOOST_FIXTURE_TEST_CASE(contact_printout_xml, Test::domain_fixture)
{
    try
    {
        ::LibFred::InfoContactOutput info_contact_output = ::LibFred::InfoContactByHandle(registrant_contact_handle).exec(ctx, "UTC");
        info_contact_output.utc_timestamp = boost::posix_time::time_from_string("2017-05-04 11:06:00");

        const ::LibFred::InfoRegistrarOutput info_sponsoring_registrar_output = ::LibFred::InfoRegistrarByHandle(
                info_contact_output.info_contact_data.sponsoring_registrar_handle).exec(ctx, "UTC");
        const Tz::LocalTimestamp data_valid_at =
                Tz::LocalTimestamp(boost::posix_time::time_from_string("2017-05-04 13:06:00"), 2 * 60);

        const std::string test_contact_xml = Fred::Backend::RecordStatement::Impl::contact_printout_xml<Tz::Europe::Prague>(
              ctx,
              info_contact_output,
              data_valid_at,
              Fred::Backend::RecordStatement::Purpose::private_printout,
              info_sponsoring_registrar_output,
              Fred::Backend::RecordStatement::Impl::make_external_states(info_contact_output.info_contact_data.id, ctx));

        //BOOST_TEST_MESSAGE(test_contact_xml);

        BOOST_CHECK_EQUAL(
                test_contact_xml,
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
                "</record_statement>");
    }
    catch (const boost::exception& e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
    }
}

namespace {

void save_pdf(const std::string& file_name, const Fred::Backend::Buffer& pdf_buffer)
{
    std::ofstream pdf_file(file_name.c_str(), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
    pdf_file.write(pdf_buffer.data.c_str(), pdf_buffer.data.size());
    pdf_file.close();
}

} // namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(domain_printout_pdf, Test::domain_fixture)
{
    try
    {
        const Fred::Backend::Buffer pdf_buffer =
                rs_impl->domain_printout(ctx, test_fqdn, Fred::Backend::RecordStatement::Purpose::private_printout);

        BOOST_CHECK(!pdf_buffer.data.empty());

        //debug file output
        save_pdf("domain_printout.pdf", pdf_buffer);
    }
    catch (const boost::exception& e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
    }
    catch (const std::exception& e)
    {
        BOOST_ERROR(e.what());
    }
}

BOOST_FIXTURE_TEST_CASE(domain_printout_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(
        rs_impl->domain_printout(ctx, test_fqdn + "bad", Fred::Backend::RecordStatement::Purpose::private_printout),
        Fred::Backend::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(nsset_printout_pdf, Test::domain_fixture)
{
    try
    {
        const Fred::Backend::Buffer pdf_buffer = rs_impl->nsset_printout(ctx, test_nsset_handle);

        BOOST_CHECK(!pdf_buffer.data.empty());

        //debug file output
        save_pdf("nsset_printout.pdf", pdf_buffer);
    }
    catch (const boost::exception& e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
    }
    catch (const std::exception& e)
    {
        BOOST_ERROR(e.what());
    }
}

BOOST_FIXTURE_TEST_CASE(nsset_printout_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl->nsset_printout(ctx, test_nsset_handle + "bad"),
        Fred::Backend::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(keyset_printout_pdf, Test::domain_fixture)
{
    try
    {
        const Fred::Backend::Buffer pdf_buffer = rs_impl->keyset_printout(ctx, test_keyset_handle);

        BOOST_CHECK(!pdf_buffer.data.empty());

        //debug file output
        save_pdf("keyset_printout.pdf", pdf_buffer);
    }
    catch (const boost::exception& e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
    }
    catch (const std::exception& e)
    {
        BOOST_ERROR(e.what());
    }
}

BOOST_FIXTURE_TEST_CASE(keyset_printout_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl->keyset_printout(ctx, test_keyset_handle + "bad"),
        Fred::Backend::RecordStatement::ObjectNotFound);
}


BOOST_FIXTURE_TEST_CASE(contact_printout_pdf, Test::domain_fixture)
{
    try
    {
        const Fred::Backend::Buffer pdf_buffer =
                rs_impl->contact_printout(ctx, registrant_contact_handle, Fred::Backend::RecordStatement::Purpose::private_printout);

        BOOST_CHECK(!pdf_buffer.data.empty());

        //debug file output
        save_pdf("contact_printout.pdf", pdf_buffer);
    }
    catch (const boost::exception& e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
    }
    catch (const std::exception& e)
    {
        BOOST_ERROR(e.what());
    }
}

BOOST_FIXTURE_TEST_CASE(contact_printout_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl->contact_printout(ctx, registrant_contact_handle + "bad", Fred::Backend::RecordStatement::Purpose::private_printout),
        Fred::Backend::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(historic_domain_printout_pdf, Test::domain_by_name_and_time_fixture)
{
    try
    {
        const Fred::Backend::Buffer pdf_buffer =
                rs_impl->historic_domain_printout(ctx, test_fqdn, timestamp);

        BOOST_CHECK(!pdf_buffer.data.empty());

        //debug file output
        save_pdf("historic_domain_printout.pdf", pdf_buffer);

    }
    catch (const boost::exception& e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
    }
    catch (const std::exception& e)
    {
        BOOST_ERROR(e.what());
    }
}

BOOST_FIXTURE_TEST_CASE(historic_domain_bad_timestamp, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(Tz::LocalTimestamp::from_rfc3339_formated_string("1970-01-01X00:00:00Z"),
                      Tz::LocalTimestamp::NotRfc3339Compliant);
}

BOOST_FIXTURE_TEST_CASE(historic_domain_not_found, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(rs_impl->historic_domain_printout(ctx, "bad" + test_fqdn, timestamp),
                      Fred::Backend::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(historic_nsset_printout_pdf, Test::domain_by_name_and_time_fixture)
{
    try
    {
        const Fred::Backend::Buffer pdf_buffer =
                rs_impl->historic_nsset_printout(ctx, test_nsset_handle, timestamp);

        BOOST_CHECK(!pdf_buffer.data.empty());

        //debug file output
        save_pdf("historic_nsset_printout.pdf", pdf_buffer);
    }
    catch (const boost::exception& e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
    }
    catch (const std::exception& e)
    {
        BOOST_ERROR(e.what());
    }
}

BOOST_FIXTURE_TEST_CASE(historic_nsset_bad_timestamp, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(Tz::LocalTimestamp::from_rfc3339_formated_string("1970-01-01T00:00:00+13:00"),
                      Tz::LocalTimestamp::NotRfc3339Compliant);
}

BOOST_FIXTURE_TEST_CASE(historic_nsset_not_found, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(rs_impl->historic_nsset_printout(ctx, "bad" + test_nsset_handle, timestamp),
                      Fred::Backend::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(historic_keyset_printout_pdf, Test::domain_by_name_and_time_fixture)
{
    try
    {
        const Fred::Backend::Buffer pdf_buffer =
                rs_impl->historic_keyset_printout(ctx, test_keyset_handle, timestamp);

        BOOST_CHECK(!pdf_buffer.data.empty());

        //debug file output
        save_pdf("historic_keyset_printout.pdf", pdf_buffer);
    }
    catch (const boost::exception& e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
    }
    catch (const std::exception& e)
    {
        BOOST_ERROR(e.what());
    }
}

BOOST_FIXTURE_TEST_CASE(historic_keyset_bad_timestamp, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(Tz::LocalTimestamp::from_rfc3339_formated_string("1971-02-29T00:00:00-10:00"),
                      Tz::LocalTimestamp::NotRfc3339Compliant);
}

BOOST_FIXTURE_TEST_CASE(historic_keyset_not_found, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(rs_impl->historic_keyset_printout(ctx, "bad" + test_keyset_handle, timestamp),
                      Fred::Backend::RecordStatement::ObjectNotFound);
}


BOOST_FIXTURE_TEST_CASE(historic_contact_printout_pdf, Test::domain_by_name_and_time_fixture)
{
    try
    {
        const Fred::Backend::Buffer pdf_buffer =
                rs_impl->historic_contact_printout(ctx, registrant_contact_handle, timestamp);

        BOOST_CHECK(!pdf_buffer.data.empty());

        //debug file output
        save_pdf("historic_contact_printout.pdf", pdf_buffer);
    }
    catch (const boost::exception& e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
    }
    catch (const std::exception& e)
    {
        BOOST_ERROR(e.what());
    }
}

BOOST_FIXTURE_TEST_CASE(historic_contact_bad_timestamp, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(Tz::LocalTimestamp::from_rfc3339_formated_string("1970-01-01T24:00:00+02:00"),
                      Tz::LocalTimestamp::NotRfc3339Compliant);
}

BOOST_FIXTURE_TEST_CASE(historic_contact_not_found, Test::domain_by_name_and_time_fixture)
{
    BOOST_CHECK_THROW(rs_impl->historic_contact_printout(ctx, "bad" + registrant_contact_handle, timestamp),
                      Fred::Backend::RecordStatement::ObjectNotFound);
}


BOOST_FIXTURE_TEST_CASE(email_domain_printout_pdf, Test::domain_fixture)
{
    BOOST_CHECK_NO_THROW(rs_impl->send_domain_printout(ctx, test_fqdn, Fred::Backend::RecordStatement::Purpose::private_printout));
}

BOOST_FIXTURE_TEST_CASE(email_domain_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl->send_domain_printout(ctx, "bad" + test_fqdn, Fred::Backend::RecordStatement::Purpose::private_printout),
                      Fred::Backend::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(email_nsset_printout_pdf, Test::domain_fixture)
{
    BOOST_CHECK_NO_THROW(rs_impl->send_nsset_printout(ctx, test_nsset_handle));
}

BOOST_FIXTURE_TEST_CASE(email_nsset_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl->send_nsset_printout(ctx, "bad" + test_nsset_handle),
                      Fred::Backend::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(email_keyset_printout_pdf, Test::domain_fixture)
{
    BOOST_CHECK_NO_THROW(rs_impl->send_keyset_printout(ctx, test_keyset_handle));
}

BOOST_FIXTURE_TEST_CASE(email_keyset_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl->send_keyset_printout(ctx, "bad" + test_keyset_handle),
                      Fred::Backend::RecordStatement::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(email_contact_printout_pdf, Test::domain_fixture)
{
    BOOST_CHECK_NO_THROW(rs_impl->send_contact_printout(ctx, admin_contact_handle, Fred::Backend::RecordStatement::Purpose::private_printout));
}

BOOST_FIXTURE_TEST_CASE(email_contact_not_found, Test::domain_fixture)
{
    BOOST_CHECK_THROW(rs_impl->send_contact_printout(ctx, "bad" + admin_contact_handle, Fred::Backend::RecordStatement::Purpose::private_printout),
                      Fred::Backend::RecordStatement::ObjectNotFound);
}

BOOST_AUTO_TEST_SUITE_END()//TestRecordStatement
