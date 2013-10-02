/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include <boost/algorithm/string.hpp>
#include <boost/function.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

//#include <omniORB4/fixed.h>

#include "setup_server_decl.h"
#include "time_clock.h"
#include "fredlib/registrar.h"
#include "fredlib/domain/info_domain.h"
#include "fredlib/domain/update_domain.h"
#include "fredlib/nsset/update_nsset.h"
#include "fredlib/keyset/update_keyset.h"
#include "fredlib/contact/delete_contact.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/nsset/create_nsset.h"
#include "fredlib/keyset/create_keyset.h"
#include "fredlib/domain/create_domain.h"
#include "fredlib/opexception.h"
#include "util/util.h"

#include "fredlib/contact_verification/contact.h"
#include "fredlib/object_states.h"
#include "contact_verification/contact_verification_impl.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"


#include "fredlib/db_settings.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestInfoDomain)

const std::string server_name = "test-info-domain";


/**
 * test InfoDomain
 * basic InfoDomainOut comparison
  */
BOOST_AUTO_TEST_CASE(info_domain)
{
    std::string registrar_handle = "REG-FRED_A";
    Fred::OperationContext ctx;
    std::string xmark = RandomDataGenerator().xnumstring(6);

    std::string admin_contact_handle = std::string("TEST-ADMIN-CONTACT-HANDLE")+xmark;
    Fred::CreateContact(admin_contact_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string admin_contact1_handle = std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark;
    Fred::CreateContact(admin_contact1_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT2 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string admin_contact2_handle = std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark;
    Fred::CreateContact(admin_contact2_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);


    std::string registrant_contact_handle = std::string("TEST-REGISTRANT-CONTACT-HANDLE")+xmark;
    Fred::CreateContact(registrant_contact_handle,registrar_handle)
            .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

    std::string test_nsset_handle = std::string("TEST-NSSET-HANDLE")+xmark;
    Fred::CreateNsset(test_nsset_handle, registrar_handle)
        .set_dns_hosts(Util::vector_of<Fred::DnsHost>
            (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3"))) //add_dns
            (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))) //add_dns
            )
            .exec(ctx);

    std::string test_keyset_handle = std::string("TEST-KEYSET-HANDLE")+xmark;
    Fred::CreateKeyset(test_keyset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact2_handle))
            .exec(ctx);


    std::string test_domain_handle = std::string("fredinfo")+xmark+".cz";
    Fred::CreateDomain(test_domain_handle//const std::string& fqdn
                , registrar_handle//const std::string& registrar
                , registrant_contact_handle//const std::string& registrant
                , Optional<std::string>("testpasswd")//const Optional<std::string>& authinfo
                , Nullable<std::string>(test_nsset_handle)//const Optional<Nullable<std::string> >& nsset
                , Nullable<std::string>(test_keyset_handle)//const Optional<Nullable<std::string> >& keyset
                , Util::vector_of<std::string>(admin_contact2_handle)//const std::vector<std::string>& admin_contacts
                , boost::gregorian::day_clock::local_day()+boost::gregorian::months(12)//const Optional<boost::gregorian::date>& expiration_date
                , Optional<boost::gregorian::date>()
                , Optional<bool>()
                , 0//const Optional<unsigned long long> logd_request_id
                ).exec(ctx);

    Fred::InfoDomainOut info_data_1 = Fred::InfoDomain(test_domain_handle).exec(ctx);
    Fred::InfoDomainOut info_data_2 = Fred::InfoDomain(test_domain_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);


    //call update using big ctor
    Fred::UpdateDomain(test_domain_handle//fqdn
            , registrar_handle//registrar
            , Optional<std::string>()//sponsoring registrar
            , registrant_contact_handle //registrant - owner
            , std::string("testauthinfo1") //authinfo
            , Nullable<std::string>()//unset nsset - set to null
            , Optional<Nullable<std::string> >()//dont change keyset
            , Util::vector_of<std::string> (admin_contact1_handle)(registrant_contact_handle) //add admin contacts
            , Util::vector_of<std::string> (admin_contact2_handle) //remove admin contacts
            , Optional<boost::gregorian::date>()//exdate
            , Optional<boost::gregorian::date>()//valexdate
            , Optional<bool>()
            , Optional<unsigned long long>() //request_id not set
            ).exec(ctx);

    Fred::InfoDomainOut info_data_3 = Fred::InfoDomain(test_domain_handle).exec(ctx);
    BOOST_CHECK(info_data_1 != info_data_3);

}//info_domain

BOOST_AUTO_TEST_SUITE_END();//TestInfoDomain
