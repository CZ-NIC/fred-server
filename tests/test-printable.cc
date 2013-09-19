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
#include "fredlib/domain/create_domain.h"
#include "fredlib/keyset/info_keyset.h"
#include "fredlib/keyset/info_keyset_history.h"
#include "fredlib/keyset/info_keyset_compare.h"
#include "fredlib/nsset/info_nsset.h"
#include "fredlib/nsset/info_nsset_history.h"
#include "fredlib/nsset/info_nsset_compare.h"
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
    Fred::CreateDomain("TEST-CREATE-CONTACT-HANDLE", "REGISTRAR-TEST-HANDLE", "REGISTRANT-TEST-HANDLE")
    .set_admin_contacts(Util::vector_of<std::string>("admin1")("admin2")("admin3"))
    .set_authinfo("testauthinfo").set_logd_request_id(0)
    );
}


BOOST_AUTO_TEST_SUITE_END();//TestPrintable

