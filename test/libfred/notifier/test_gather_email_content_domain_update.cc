/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
 */

#include <boost/test/unit_test.hpp>
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/libfred/notifier/util.hh"
#include "test/libfred/notifier/fixture_data.hh"

#include "libfred/notifier/gather_email_data/gather_email_content.hh"

#include <boost/algorithm/string/join.hpp>


BOOST_AUTO_TEST_SUITE(TestNotifier)
BOOST_AUTO_TEST_SUITE(GatherEmailContent)
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(Update)

struct has_domain_empty_update : has_domain {

    const unsigned long long logd_request_id;
    const unsigned long long new_historyid;

    has_domain_empty_update()
    :   logd_request_id(12345),
        new_historyid(::LibFred::UpdateDomain(dom.fqdn, dom.create_registrar_handle).set_logd_request_id(logd_request_id).exec(ctx))
    { }
};

BOOST_FIXTURE_TEST_CASE(test_minimal_update_no_data, has_domain_empty_update)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "3";
    etalon["handle"] = dom.fqdn;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "0";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(::LibFred::domain, Notification::updated),
                registrar.id,
                new_historyid,
                input_svtrid
            )
        )
    );
}

struct has_empty_domain_big_update : has_enum_domain {

    const ::LibFred::InfoContactData new_registrant;
    const ::LibFred::InfoKeysetData  new_keyset;
    const ::LibFred::InfoNssetData   new_nsset;
    const ::LibFred::InfoContactData new_admin_c1;
    const ::LibFred::InfoContactData new_admin_c2;
    const ::LibFred::InfoDomainData  new_domain_data;

    has_empty_domain_big_update()
    :   new_registrant(
            Test::exec(
                ::LibFred::CreateContact("NEWREGISTRANT", registrar.handle)
                    .set_email("new.registrant@.nic.cz")
                    .set_notifyemail("new.registrant.notify@.nic.cz"),
                ctx
            )
        ),
        new_keyset(
            Test::exec(
                ::LibFred::CreateKeyset("NEWKEYSET", registrar.handle),
                ctx
            )
        ),
        new_nsset(
            Test::exec(
                ::LibFred::CreateNsset("NEWNSSET", registrar.handle),
                ctx
            )
        ),
        new_admin_c1(
            Test::exec(
                ::LibFred::CreateContact("NEWADMINC1", registrar.handle)
                    .set_email("new.adminc1@nic.cz")
                    .set_notifyemail("new.adminc1notify@nic.cz"),
                ctx
            )
        ),
        new_admin_c2(
            Test::exec(
                ::LibFred::CreateContact("NEWADMINC2", registrar.handle)
                    .set_email("new.adminc2@nic.cz")
                    .set_notifyemail("new.adminc2notify@nic.cz"),
                ctx
            )
        ),
        new_domain_data(
            ::LibFred::InfoDomainHistoryByHistoryid(
                ::LibFred::UpdateDomain(dom.fqdn, dom.create_registrar_handle)
                    .set_authinfo("abc123zxy")
                    .set_registrant(new_registrant.handle)
                    .set_keyset(new_keyset.handle)
                    .set_nsset(new_nsset.handle)
                    .set_enum_validation_expiration( dom.enum_domain_validation.get_value().validation_expiration + boost::gregorian::months(1) )
                    .set_enum_publish_flag(true)
                    .add_admin_contact(new_admin_c1.handle)
                    .add_admin_contact(new_admin_c2.handle)
                    .exec(ctx)
            ).exec(ctx).info_domain_data
        )
    { }
};

BOOST_FIXTURE_TEST_CASE(test_big_update_from_empty_data, has_empty_domain_big_update)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "3";
    etalon["handle"] = dom.fqdn;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "1";

    etalon["changes.object.authinfo"]       = "1";
    etalon["changes.object.authinfo.old"]   = dom            .authinfopw;
    etalon["changes.object.authinfo.new"]   = new_domain_data.authinfopw;

    etalon["changes.domain.registrant"]     = "1";
    etalon["changes.domain.registrant.old"] = dom            .registrant.handle;
    etalon["changes.domain.registrant.new"] = new_domain_data.registrant.handle;

    etalon["changes.domain.nsset"]          = "1";
    etalon["changes.domain.nsset.old"]      = "";
    etalon["changes.domain.nsset.new"]      = new_domain_data.nsset.get_value().handle;

    etalon["changes.domain.keyset"]         = "1";
    etalon["changes.domain.keyset.old"]     = "";
    etalon["changes.domain.keyset.new"]     = new_domain_data.keyset.get_value().handle;

    std::string admin_c_list;
    {
        std::vector<std::string> admin_contact_handles;
        for (const auto& a_c : new_domain_data.admin_contacts) {
            admin_contact_handles.push_back(a_c.handle);
        }
        std::sort( admin_contact_handles.begin(), admin_contact_handles.end() );
        admin_c_list = boost::algorithm::join(admin_contact_handles, " ");
    }
    etalon["changes.domain.admin_c"]        = "1";
    etalon["changes.domain.admin_c.old"]    = "";
    etalon["changes.domain.admin_c.new"]    = admin_c_list;

    /* XXX This is sad indeed. But I am not going to use all the circus around facets to do simple output formatting! */
    struct cz_format {
        static std::string convert(const boost::gregorian::date& _date) {
            return
                boost::lexical_cast<std::string>( _date.day().as_number() )
                + "." +
                boost::lexical_cast<std::string>( _date.month().as_number() )
                + "." +
                boost::lexical_cast<std::string>( _date.year() );
        }
    };

    etalon["changes.domain.val_ex_date"]     = "1";
    etalon["changes.domain.val_ex_date.old"] = cz_format::convert(dom            .enum_domain_validation.get_value().validation_expiration);
    etalon["changes.domain.val_ex_date.new"] = cz_format::convert(new_domain_data.enum_domain_validation.get_value().validation_expiration);

    etalon["changes.domain.publish"]         = "1";
    etalon["changes.domain.publish.old"]     = "0";
    etalon["changes.domain.publish.new"]     = "1";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(::LibFred::domain, Notification::updated),
                registrar.id,
                new_domain_data.historyid,
                input_svtrid
            )
        )
    );
}

struct has_full_domain_big_update : has_empty_domain_big_update {

    const ::LibFred::InfoContactData newest_registrant;
    const ::LibFred::InfoKeysetData  newest_keyset;
    const ::LibFred::InfoNssetData   newest_nsset;
    const ::LibFred::InfoContactData newest_admin_c1;
    const ::LibFred::InfoContactData newest_admin_c2;
    const ::LibFred::InfoDomainData  newest_domain_data;

    has_full_domain_big_update()
    :   newest_registrant(
            Test::exec(
                ::LibFred::CreateContact("NEWESTREGISTRANT", registrar.handle)
                    .set_email("newest.registrant@.nic.cz")
                    .set_notifyemail("newest.registrant.notify@.nic.cz"),
                ctx
            )
        ),
        newest_keyset(
            Test::exec(
                ::LibFred::CreateKeyset("NEWESTKEYSET", registrar.handle),
                ctx
            )
        ),
        newest_nsset(
            Test::exec(
                ::LibFred::CreateNsset("NEWESTNSSET", registrar.handle),
                ctx
            )
        ),
        newest_admin_c1(
            Test::exec(
                ::LibFred::CreateContact("NEWESTADMINC1", registrar.handle)
                    .set_email("newest.adminc1@nic.cz")
                    .set_notifyemail("newest.adminc1notify@nic.cz"),
                ctx
            )
        ),
        newest_admin_c2(
            Test::exec(
                ::LibFred::CreateContact("NEWESTADMINC2", registrar.handle)
                    .set_email("newest.adminc2@nic.cz")
                    .set_notifyemail("newest.adminc2notify@nic.cz"),
                ctx
            )
        ),
        newest_domain_data(
            ::LibFred::InfoDomainHistoryByHistoryid(
                ::LibFred::UpdateDomain(dom.fqdn, dom.create_registrar_handle)
                    .set_authinfo("heslo1tajne")
                    .set_registrant(newest_registrant.handle)
                    .set_keyset(newest_keyset.handle)
                    .set_nsset(newest_nsset.handle)
                    .set_enum_validation_expiration( new_domain_data.enum_domain_validation.get_value().validation_expiration + boost::gregorian::months(1) )
                    .set_enum_publish_flag(false)
                    .rem_admin_contact(new_admin_c1.handle)
                    .rem_admin_contact(new_admin_c2.handle)
                    .add_admin_contact(newest_admin_c1.handle)
                    .add_admin_contact(newest_admin_c2.handle)
                    .exec(ctx)
            ).exec(ctx).info_domain_data
        )
    { }
};

BOOST_FIXTURE_TEST_CASE(test_big_update_from_full_data, has_full_domain_big_update)
{
    const std::string input_svtrid = "xyz-987";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "3";
    etalon["handle"] = dom.fqdn;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "1";

    etalon["changes.object.authinfo"]       = "1";
    etalon["changes.object.authinfo.old"]   = new_domain_data   .authinfopw;
    etalon["changes.object.authinfo.new"]   = newest_domain_data.authinfopw;

    etalon["changes.domain.registrant"]     = "1";
    etalon["changes.domain.registrant.old"] = new_domain_data   .registrant.handle;
    etalon["changes.domain.registrant.new"] = newest_domain_data.registrant.handle;

    etalon["changes.domain.nsset"]          = "1";
    etalon["changes.domain.nsset.old"]      = new_domain_data   .nsset.get_value().handle;
    etalon["changes.domain.nsset.new"]      = newest_domain_data.nsset.get_value().handle;

    etalon["changes.domain.keyset"]         = "1";
    etalon["changes.domain.keyset.old"]     = new_domain_data   .keyset.get_value().handle;
    etalon["changes.domain.keyset.new"]     = newest_domain_data.keyset.get_value().handle;

    etalon["changes.domain.admin_c"]        = "1";
    {
        std::string new_admin_c_list;

        std::vector<std::string> admin_contact_handles;
        for (const auto& a_c : new_domain_data.admin_contacts) {
            admin_contact_handles.push_back(a_c.handle);
        }
        std::sort( admin_contact_handles.begin(), admin_contact_handles.end() );
        new_admin_c_list = boost::algorithm::join(admin_contact_handles, " ");

        etalon["changes.domain.admin_c.old"]    = new_admin_c_list;
    }
    {
        std::string newest_admin_c_list;

        std::vector<std::string> admin_contact_handles;
        for (const auto& a_c : newest_domain_data.admin_contacts) {
            admin_contact_handles.push_back(a_c.handle);
        }
        std::sort( admin_contact_handles.begin(), admin_contact_handles.end() );
        newest_admin_c_list = boost::algorithm::join(admin_contact_handles, " ");

        etalon["changes.domain.admin_c.new"]    = newest_admin_c_list;
    }

    /* XXX This is sad indeed. But I am not going to use all the circus around facets to do simple output formatting! */
    struct cz_format {
        static std::string convert(const boost::gregorian::date& _date) {
            return
                boost::lexical_cast<std::string>( _date.day().as_number() )
                + "." +
                boost::lexical_cast<std::string>( _date.month().as_number() )
                + "." +
                boost::lexical_cast<std::string>( _date.year() );
        }
    };

    etalon["changes.domain.val_ex_date"]     = "1";
    etalon["changes.domain.val_ex_date.old"] = cz_format::convert(new_domain_data   .enum_domain_validation.get_value().validation_expiration);
    etalon["changes.domain.val_ex_date.new"] = cz_format::convert(newest_domain_data.enum_domain_validation.get_value().validation_expiration);

    etalon["changes.domain.publish"]         = "1";
    etalon["changes.domain.publish.old"]     = "1";
    etalon["changes.domain.publish.new"]     = "0";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(::LibFred::domain, Notification::updated),
                registrar.id,
                newest_domain_data.historyid,
                input_svtrid
            )
        )
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
