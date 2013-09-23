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
#include "fredlib/contact/create_contact.h"
#include "fredlib/contact/update_contact.h"
#include "fredlib/contact/delete_contact.h"
#include "fredlib/contact/check_contact.h"
#include "fredlib/nsset/create_nsset.h"
#include "fredlib/keyset/create_keyset.h"
#include "fredlib/keyset/delete_keyset.h"
#include "fredlib/keyset/check_keyset.h"
#include "fredlib/domain/create_domain.h"
#include "fredlib/domain/delete_domain.h"
#include "fredlib/domain/check_domain.h"
#include "fredlib/domain/update_domain.h"
#include "fredlib/keyset/info_keyset.h"
#include "fredlib/keyset/info_keyset_history.h"
#include "fredlib/keyset/info_keyset_compare.h"
#include "fredlib/keyset/update_keyset.h"
#include "fredlib/nsset/info_nsset.h"
#include "fredlib/nsset/info_nsset_history.h"
#include "fredlib/nsset/info_nsset_compare.h"
#include "fredlib/nsset/delete_nsset.h"
#include "fredlib/domain/info_domain.h"
#include "fredlib/domain/info_domain_history.h"
#include "fredlib/domain/info_domain_compare.h"
#include "fredlib/contact/info_contact.h"
#include "fredlib/contact/info_contact_history.h"
#include "fredlib/contact/info_contact_compare.h"


#include "util/util.h"

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

BOOST_AUTO_TEST_SUITE(TestPrintable)

const std::string server_name = "test-printable";

/**
 * test template to check that string made by to_string() and operator<< is the same
 */
template <class T> void printable_test(const T& i)
{
    BOOST_MESSAGE(i.to_string());
    std::stringstream ss;
    ss << i;
    BOOST_CHECK(i.to_string().compare(ss.str()) == 0);
}

/**
 * test CreateContact print to string
 */
BOOST_AUTO_TEST_CASE(create_contact)
{
    printable_test(
    Fred::CreateContact("TEST-CREATE-CONTACT-HANDLE", "REGISTRAR-TEST-HANDLE")
    .set_authinfo("testauthinfo").set_logd_request_id(0)
    );
}

/**
 * test DeleteContact print to string
 */
BOOST_AUTO_TEST_CASE(delete_contact)
{
    printable_test(
    Fred::DeleteContact("TEST-DELETE-CONTACT-HANDLE")
    );
}

/**
 * test CheckContact print to string
 */
BOOST_AUTO_TEST_CASE(check_contact)
{
    printable_test(
    Fred::CheckContact("TEST-CHECK-CONTACT-HANDLE")
    );
}

/**
 * test InfoContactData print to string
 */
BOOST_AUTO_TEST_CASE(info_contact_data)
{
    Fred::InfoContactData i;
    i.handle = "TEST-INFO-CONTACT-HANDLE";
    i.creation_time = boost::posix_time::microsec_clock::universal_time();
    i.delete_time = boost::posix_time::microsec_clock::universal_time();
    i.disclosename = true;
    printable_test(i);
}

/**
 * test InfoContactHistoryOutput print to string
 */
BOOST_AUTO_TEST_CASE(info_contact_history_output)
{
    Fred::InfoContactData icd;
    icd.handle = "TEST-INFO-CONTACT-HANDLE";
    icd.creation_time = boost::posix_time::microsec_clock::universal_time();
    icd.delete_time = boost::posix_time::microsec_clock::universal_time();
    icd.disclosename = true;

    Fred::InfoContactHistoryOutput i;
    i.history_valid_from = boost::posix_time::microsec_clock::universal_time();
    i.history_valid_to = boost::posix_time::microsec_clock::universal_time();
    i.info_contact_data = icd;
    i.logd_request_id = 1;
    i.next_historyid = 2;
    printable_test(i);
}

/**
 * test InfoContactHistory print to string
 */
BOOST_AUTO_TEST_CASE(info_contact_history)
{
    printable_test(
    Fred::InfoContactHistory("TEST-CONTACT-ROID", boost::posix_time::microsec_clock::universal_time())
    );
}

/**
 * test InfoContactOutput print to string
 */
BOOST_AUTO_TEST_CASE(info_contact_output)
{
    Fred::InfoContactData icd;
    icd.handle = "TEST-INFO-CONTACT-HANDLE";
    icd.creation_time = boost::posix_time::microsec_clock::universal_time();
    icd.delete_time = boost::posix_time::microsec_clock::universal_time();
    icd.disclosename = true;

    Fred::InfoContactOutput i;
    i.utc_timestamp = boost::posix_time::microsec_clock::universal_time();
    i.local_timestamp = boost::posix_time::microsec_clock::local_time();
    i.info_contact_data = icd;
    printable_test(i);
}

/**
 * test InfoContact print to string
 */
BOOST_AUTO_TEST_CASE(info_contact)
{
    printable_test(
    Fred::InfoContact("TEST-CONTACT-HANDLE")
    );
}

/**
 * test UpdateContact print to string
 */
BOOST_AUTO_TEST_CASE(update_contact)
{
    printable_test(
    Fred::UpdateContact("TEST-UPDATE-CONTACT-HANDLE", "REGISTRAR-UP-TEST-HANDLE")
    .set_sponsoring_registrar("REGISTRAR-SP-TEST-HANDLE")
    .set_authinfo("testauthinfo").set_logd_request_id(0)
    );
}

/**
 * test CreateDomain print to string
 */
BOOST_AUTO_TEST_CASE(create_domain)
{
    printable_test(
    Fred::CreateDomain("test-fred.cz", "REGISTRAR-TEST-HANDLE", "REGISTRANT-TEST-HANDLE")
    .set_admin_contacts(Util::vector_of<std::string>("admin1")("admin2")("admin3"))
    .set_authinfo("testauthinfo").set_logd_request_id(0)
    );
}

/**
 * test DeleteDomain print to string
 */
BOOST_AUTO_TEST_CASE(delete_domain)
{
    printable_test(
    Fred::DeleteDomain("TEST-DELETE-CONTACT-HANDLE")
    );
}

/**
 * test CheckDomain print to string
 */
BOOST_AUTO_TEST_CASE(check_domain)
{
    printable_test(
    Fred::CheckDomain("test-fred.cz")
    );
}

/**
 * test InfoDomainData print to string
 */
BOOST_AUTO_TEST_CASE(info_domain_data)
{
    Fred::InfoDomainData i;
    i.admin_contacts = Util::vector_of<std::string>("admin1")("admin2")("admin3");
    i.expiration_date = boost::gregorian::day_clock::local_day();
    printable_test(i);
}

/**
 * test InfoDomainHistoryOutput print to string
 */
BOOST_AUTO_TEST_CASE(info_domain_history_output)
{
    Fred::InfoDomainData id;
    id.fqdn = "test-fred.cz";
    id.creation_time = boost::posix_time::microsec_clock::universal_time();
    id.delete_time = boost::posix_time::microsec_clock::universal_time();
    id.keyset_handle=Nullable<std::string>("TEST-KEYSET");

    Fred::InfoDomainHistoryOutput ih;
    ih.history_valid_from = boost::posix_time::microsec_clock::universal_time();
    ih.history_valid_to = boost::posix_time::microsec_clock::universal_time();
    ih.info_domain_data = id;
    ih.logd_request_id = 1;
    ih.next_historyid = 2;
    printable_test(ih);
}

/**
 * test InfoDomainHistory print to string
 */
BOOST_AUTO_TEST_CASE(info_domain_history)
{
    printable_test(
    Fred::InfoDomainHistory("test-fred.cz", boost::posix_time::microsec_clock::universal_time())
    );
}


/**
 * test InfoDomainOutput print to string
 */
BOOST_AUTO_TEST_CASE(info_domain_output)
{
    Fred::InfoDomainData id;
    id.fqdn = "test-fred.cz";
    id.creation_time = boost::posix_time::microsec_clock::universal_time();
    id.delete_time = boost::posix_time::microsec_clock::universal_time();
    id.keyset_handle=Nullable<std::string>("TEST-KEYSET");

    Fred::InfoDomainOutput i;
    i.utc_timestamp = boost::posix_time::microsec_clock::universal_time();
    i.local_timestamp = boost::posix_time::microsec_clock::local_time();
    i.info_domain_data = id;
    printable_test(i);
}

/**
 * test InfoDomain print to string
 */
BOOST_AUTO_TEST_CASE(info_domain)
{
    printable_test(
    Fred::InfoDomain("fred.cz")
    );
}

/**
 * test UpdateDomain print to string
 */
BOOST_AUTO_TEST_CASE(update_domain)
{
    printable_test(
    Fred::UpdateDomain("test-fred.cz", "REGISTRAR-TEST-HANDLE")
    .add_admin_contact("admin1")
    .add_admin_contact("admin2")
    .add_admin_contact("admin3")
    .set_authinfo("testauthinfo").set_logd_request_id(0)
    );
}

/**
 * test CreateKeyset print to string
 */
BOOST_AUTO_TEST_CASE(create_keyset)
{
    printable_test(
    Fred::CreateKeyset("TEST-CREATE-KEYSET-HANDLE", "REGISTRAR-TEST-HANDLE")
    .set_tech_contacts(Util::vector_of<std::string>("admin1")("admin2")("admin3"))
    .set_dns_keys(Util::vector_of<Fred::DnsKey>
        (Fred::DnsKey(257, 3, 5, "test1"))
        (Fred::DnsKey(257, 3, 5, "test2")))
    .set_authinfo("testauthinfo").set_logd_request_id(0)
    );
}

/**
 * test DeleteKeyset print to string
 */
BOOST_AUTO_TEST_CASE(delete_keyset)
{
    printable_test(
    Fred::DeleteKeyset("TEST-DEL-KEYSET-HANDLE")
    );
}

/**
 * test CheckKeyset print to string
 */
BOOST_AUTO_TEST_CASE(check_keyset)
{
    printable_test(
    Fred::CheckKeyset("TEST-CHECK-KEYSET-HANDLE")
    );
}

/**
 * test InfoKeysetData print to string
 */
BOOST_AUTO_TEST_CASE(info_keyset_data)
{
    Fred::InfoKeysetData i;
    i.handle = "TEST-INFO-KEYSET-HANDLE";
    i.creation_time = boost::posix_time::microsec_clock::universal_time();
    i.delete_time = boost::posix_time::microsec_clock::universal_time();
    printable_test(i);
}


/**
 * test InfoKeysetHistoryOutput print to string
 */
BOOST_AUTO_TEST_CASE(info_keyset_history_output)
{
    Fred::InfoKeysetData d;
    d.handle = "TEST-INFO-KEYSET-HANDLE";
    d.creation_time = boost::posix_time::microsec_clock::universal_time();
    d.delete_time = boost::posix_time::microsec_clock::universal_time();
    d.tech_contacts = Util::vector_of<std::string>("tech1")("tech2")("tech3");

    Fred::InfoKeysetHistoryOutput i;
    i.history_valid_from = boost::posix_time::microsec_clock::universal_time();
    i.history_valid_to = boost::posix_time::microsec_clock::universal_time();
    i.info_keyset_data = d;
    i.logd_request_id = 1;
    i.next_historyid = 2;
    printable_test(i);
}

/**
 * test InfoKeysetHistory print to string
 */
BOOST_AUTO_TEST_CASE(info_keyset_history)
{
    printable_test(
    Fred::InfoKeysetHistory("TEST-KEYSET-ROID", boost::posix_time::microsec_clock::universal_time())
    );
}

/**
 * test InfoKeysetOutput print to string
 */
BOOST_AUTO_TEST_CASE(info_keyset_output)
{
    Fred::InfoKeysetData d;
    d.handle = "TEST-INFO-KEYSET-HANDLE";
    d.creation_time = boost::posix_time::microsec_clock::universal_time();
    d.delete_time = boost::posix_time::microsec_clock::universal_time();
    d.tech_contacts = Util::vector_of<std::string>("tech1")("tech2")("tech3");

    Fred::InfoKeysetOutput i;
    i.utc_timestamp = boost::posix_time::microsec_clock::universal_time();
    i.local_timestamp = boost::posix_time::microsec_clock::local_time();
    i.info_keyset_data = d;
    printable_test(i);
}

/**
 * test InfoKeyset print to string
 */
BOOST_AUTO_TEST_CASE(info_keyset)
{
    printable_test(
    Fred::InfoKeyset("TEST-KEYSET-HANDLE")
    );
}

/**
 * test UpdateKeyset print to string
 */
BOOST_AUTO_TEST_CASE(update_keyset)
{
    printable_test(
    Fred::UpdateKeyset("TEST-KEYSET-HANDLE", "REGISTRAR-TEST-HANDLE")
    .add_tech_contact("admin1")
    .add_tech_contact("admin2")
    .add_tech_contact("admin3")
    .set_authinfo("testauthinfo").set_logd_request_id(0)
    );
}

/**
 * test CreateNsset print to string
 */
BOOST_AUTO_TEST_CASE(create_nsset)
{
    printable_test(
    Fred::CreateNsset("TEST-CREATE-NSSET-HANDLE", "REGISTRAR-TEST-HANDLE")
    .set_tech_contacts(Util::vector_of<std::string>("admin1")("admin2")("admin3"))
    .set_dns_hosts(Util::vector_of<Fred::DnsHost>
        (Fred::DnsHost("test1dns.cz", Util::vector_of<std::string>("6.6.6.6")("7.7.7.7")))
        (Fred::DnsHost("test2dns.cz", Util::vector_of<std::string>("6.6.6.6")("7.7.7.7")))
        )
    .set_authinfo("testauthinfo").set_logd_request_id(0)
    );
}

/**
 * test DeleteNsset print to string
 */
BOOST_AUTO_TEST_CASE(delete_nsset)
{
    printable_test(
    Fred::DeleteNsset("TEST-DEL-NSSET-HANDLE")
    );
}


BOOST_AUTO_TEST_SUITE_END();//TestPrintable

