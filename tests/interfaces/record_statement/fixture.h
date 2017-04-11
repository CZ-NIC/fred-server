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
 */

#ifndef TEST_INTERFACE_RECORD_STATEMENT_FIXTURE_66E900A63EDE4F14A4F8E2B8C011E7C1
#define TEST_INTERFACE_RECORD_STATEMENT_FIXTURE_66E900A63EDE4F14A4F8E2B8C011E7C1

#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/documents.h"
#include "src/corba/mailer_manager.h"

#include "util/corba_wrapper_decl.h"
#include "util/cfg/config_handler_decl.h"
#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_corbanameservice_args.h"

#include "src/record_statement/record_statement.hh"


#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/test_tools.hpp>

namespace Test {

    boost::shared_ptr<Fred::Document::Manager> make_doc_manager()
    {
        return boost::shared_ptr<Fred::Document::Manager>(Fred::Document::Manager::create(
            CfgArgs::instance()->get_handler_ptr_by_type< HandleRegistryArgs >()->docgen_path,
            CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()->docgen_template_path,
            CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()->fileclient_path,
            CfgArgs::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>()->get_nameservice_host_port()
        ).release());
    }

    boost::shared_ptr<Fred::Mailer::Manager> make_mailer_manager()
    {
        return boost::shared_ptr<Fred::Mailer::Manager> (
            new MailerManager(CorbaContainer::get_instance()->getNS()));
    }



    struct domain_fixture : virtual public Test::instantiate_db_template
    {
        Fred::OperationContextCreator ctx;

        std::string registrar_handle;
        std::string admin_contact_handle;
        std::string admin_contact1_handle;
        std::string admin_contact2_handle;
        std::string registrant_contact_handle;
        std::string test_nsset_handle;
        std::string test_keyset_handle;
        std::string test_fqdn;

        Fred::InfoDomainOutput test_info_domain_output;
        Fred::InfoRegistrarData test_info_registrar;

        Registry::RecordStatement::RecordStatementImpl rs_impl;

        domain_fixture()
        : registrar_handle("REGISTRAR1"),
          admin_contact_handle("TEST-ADMIN-CONTACT-HANDLE"),
          admin_contact1_handle("TEST-ADMIN-CONTACT2-HANDLE"),
          admin_contact2_handle("TEST-ADMIN-CONTACT3-HANDLE"),
          registrant_contact_handle("TEST-REGISTRANT-CONTACT-HANDLE"),
          test_nsset_handle("TEST-NSSET-HANDLE"),
          test_keyset_handle("TEST-KEYSET-HANDLE"),
          test_fqdn(std::string("fredinfo")+".cz"),
          rs_impl("test-printout-pdf", make_doc_manager(), make_mailer_manager(),
                  CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()->registry_timezone)
        {
            namespace ip = boost::asio::ip;

            Fred::CreateRegistrar(registrar_handle).exec(ctx);
            test_info_registrar = Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;

            Fred::Contact::PlaceAddress place;
            place.street1 = "STR1";
            place.city = "Praha";
            place.postalcode = "11150";
            place.country = "CZ";
            Fred::CreateContact(admin_contact_handle,registrar_handle)
                .set_name("TEST-ADMIN-CONTACT NAME")
                .set_disclosename(true)
                .set_email("jan.zima@nic.cz")
                .set_place(place)
                .set_discloseaddress(true)
                .exec(ctx);

            Fred::CreateContact(admin_contact1_handle,registrar_handle)
                .set_name("TEST-ADMIN-CONTACT2 NAME")
                .set_disclosename(true)
                .set_email("jan.zima@nic.cz")
                .set_place(place)
                .set_discloseaddress(true)
                .exec(ctx);

            Fred::CreateContact(admin_contact2_handle,registrar_handle)
                .set_name("TEST-ADMIN-CONTACT3 NAME")
                .set_disclosename(true)
                .set_place(place)
                .set_discloseaddress(true)
                .exec(ctx);

            Fred::CreateContact(registrant_contact_handle,registrar_handle)
                .set_name("TEST-REGISTRANT-CONTACT NAME")
                .set_ssntype("ICO").set_ssn("123456789")
                .set_email("jan.zima@nic.cz")
                .set_disclosename(true)
                .set_place(place)
                .set_discloseaddress(true)
                .exec(ctx);

            Fred::CreateNsset(test_nsset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle)(admin_contact1_handle))
                .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                ).exec(ctx);

            Fred::CreateKeyset(test_keyset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle)(admin_contact1_handle))
            .set_dns_keys(Util::vector_of<Fred::DnsKey>
                (Fred::DnsKey(257, 3, 5, "test1"))
                (Fred::DnsKey(257, 3, 5, "test2")))
            .exec(ctx);

            Fred::CreateDomain(test_fqdn
                        , registrar_handle
                        , registrant_contact_handle
                        , Optional<std::string>("testauthinfo1")
                        , Nullable<std::string>(test_nsset_handle)
                        , Nullable<std::string>(test_keyset_handle)
                        , Util::vector_of<std::string>(admin_contact1_handle)
                        , boost::gregorian::from_simple_string("2012-06-30")
                        , Optional<boost::gregorian::date>()
                        , Optional<bool>()
                        , 0
                        ).exec(ctx);

            //id query
            Database::Result id_res = ctx.get_conn().exec_params("SELECT"
            " (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='domain'::text) AND name = LOWER($1::text)) AS test_fqdn_id"
            ",  (SELECT crhistoryid FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='domain'::text) AND name = LOWER($1::text)) AS test_fqdn_crhistoryid"
            ",  (SELECT historyid FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='domain'::text) AND name = LOWER($1::text)) AS test_fqdn_historyid"
            ", (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='contact'::text) AND name = UPPER($2::text)) AS registrant_contact_handle_id"
            ", (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) AND name = UPPER($3::text)) AS test_nsset_handle_id"
            ", (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) AND name = UPPER($4::text)) AS test_keyset_handle_id"
            ", (SELECT id FROM registrar WHERE handle = UPPER($5::text)) AS registrar_handle_id"
            ", (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='contact'::text) AND name = UPPER($6::text)) AS admin_contact1_handle_id"
            , Database::query_param_list(test_fqdn)(registrant_contact_handle)(test_nsset_handle)(test_keyset_handle)(registrar_handle)(admin_contact1_handle));

            //crdate fix
            ctx.get_conn().exec_params("UPDATE object_registry SET crdate = $1::timestamp WHERE id = $2::bigint",
                Database::query_param_list("2011-06-30T23:59:59.653796")(static_cast<unsigned long long>(id_res[0]["test_fqdn_id"])));

            ctx.get_conn().exec_params("UPDATE object_registry SET crdate = $1::timestamp WHERE id = $2::bigint",
                Database::query_param_list("2010-06-30T23:59:59.653796")(static_cast<unsigned long long>(id_res[0]["registrant_contact_handle_id"])));

            test_info_domain_output = Fred::InfoDomainByHandle(test_fqdn).exec(ctx, "Europe/Prague");
        }
        ~domain_fixture()
        {}
    };


    struct domain_by_name_and_time_fixture : virtual public Test::instantiate_db_template
    {
        Fred::OperationContextCreator ctx;

        std::string registrar_handle;
        std::string admin_contact_handle;
        std::string admin_contact1_handle;
        std::string admin_contact2_handle;
        std::string registrant_contact_handle;
        std::string test_nsset_handle;
        std::string test_keyset_handle;
        std::string test_fqdn;
        boost::posix_time::time_duration offset;

        Fred::InfoDomainOutput test_info_domain_output;
        Fred::InfoRegistrarData test_info_registrar;

        std::string timestamp;
        Registry::RecordStatement::RecordStatementImpl rs_impl;

        domain_by_name_and_time_fixture()
        : registrar_handle("REGISTRAR1"),
          admin_contact_handle("TEST-ADMIN-CONTACT-HANDLE"),
          admin_contact1_handle("TEST-ADMIN-CONTACT2-HANDLE"),
          admin_contact2_handle("TEST-ADMIN-CONTACT3-HANDLE"),
          registrant_contact_handle("TEST-REGISTRANT-CONTACT-HANDLE"),
          test_nsset_handle("TEST-NSSET-HANDLE"),
          test_keyset_handle("TEST-KEYSET-HANDLE"),
          test_fqdn(std::string("fredinfo")+".cz"),
          rs_impl("test-printout-pdf", make_doc_manager(), make_mailer_manager(),
                  CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()->registry_timezone)
        {
            namespace ip = boost::asio::ip;

            Fred::CreateRegistrar(registrar_handle).exec(ctx);
            test_info_registrar = Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;

            Fred::Contact::PlaceAddress place;
            place.street1 = "STR1";
            place.city = "Praha";
            place.postalcode = "11150";
            place.country = "CZ";
            Fred::CreateContact(admin_contact_handle,registrar_handle)
                .set_name("TEST-ADMIN-CONTACT NAME")
                .set_disclosename(true)
                .set_place(place)
                .set_discloseaddress(true)
                .exec(ctx);

            Fred::CreateContact(admin_contact1_handle,registrar_handle)
                .set_name("TEST-ADMIN-CONTACT2 NAME")
                .set_disclosename(true)
                .set_place(place)
                .set_discloseaddress(true)
                .exec(ctx);

            Fred::CreateContact(admin_contact2_handle,registrar_handle)
                .set_name("TEST-ADMIN-CONTACT3 NAME")
                .set_disclosename(true)
                .set_place(place)
                .set_discloseaddress(true)
                .exec(ctx);

            Fred::CreateContact(registrant_contact_handle,registrar_handle)
                .set_name("TEST-REGISTRANT-CONTACT NAME")
                .set_ssntype("ICO").set_ssn("123456789")
                .set_disclosename(true)
                .set_place(place)
                .set_discloseaddress(true)
                .exec(ctx);

            Fred::CreateNsset(test_nsset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
                .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                ).exec(ctx);

            Fred::CreateKeyset(test_keyset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_dns_keys(Util::vector_of<Fred::DnsKey>
                (Fred::DnsKey(257, 3, 5, "test1"))
                (Fred::DnsKey(257, 3, 5, "test2")))
            .exec(ctx);

            Fred::CreateDomain(test_fqdn
                        , registrar_handle
                        , registrant_contact_handle
                        , Optional<std::string>("testauthinfo1")
                        , Nullable<std::string>(test_nsset_handle)
                        , Nullable<std::string>(test_keyset_handle)
                        , Util::vector_of<std::string>(admin_contact1_handle)
                        , boost::gregorian::from_simple_string("2012-06-30")
                        , Optional<boost::gregorian::date>()
                        , Optional<bool>()
                        , 0
                        ).exec(ctx);

            test_info_domain_output = Fred::InfoDomainByHandle(test_fqdn).exec(ctx, "Europe/Prague");
            offset = test_info_domain_output.info_domain_data.creation_time -test_info_domain_output.utc_timestamp;

            timestamp = boost::posix_time::to_iso_extended_string(
                        boost::posix_time::ptime(
                            test_info_domain_output.history_valid_from.date(),
                            boost::posix_time::seconds(test_info_domain_output.history_valid_from.time_of_day().total_seconds())
                        )
                    ) + std::string("+0") + boost::lexical_cast<std::string>(offset.hours())
                    + std::string(":00");
        }
        ~domain_by_name_and_time_fixture()
        {}
    };

}//namespace Test

#endif
