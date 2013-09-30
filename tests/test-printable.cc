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
#include "fredlib/nsset/check_nsset.h"
#include "fredlib/nsset/update_nsset.h"
#include "fredlib/domain/info_domain.h"
#include "fredlib/domain/info_domain_history.h"
#include "fredlib/domain/info_domain_compare.h"
#include "fredlib/contact/info_contact.h"
#include "fredlib/contact/info_contact_history.h"
#include "fredlib/contact/info_contact_compare.h"
#include "fredlib/contact/merge_contact.h"
#include "fredlib/contact/merge_contact_email_notification_data.h"
#include "fredlib/contact/merge_contact_selection.h"
#include "fredlib/poll/create_delete_contact_poll_message.h"
#include "fredlib/poll/create_poll_message.h"
#include "fredlib/poll/create_update_object_poll_message.h"


#include "util/util.h"
#include "util/printable.h"

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
 * test InfoContactOut print to string
 */
BOOST_AUTO_TEST_CASE(info_contact_output)
{
    Fred::InfoContactData icd;
    icd.handle = "TEST-INFO-CONTACT-HANDLE";
    icd.creation_time = boost::posix_time::microsec_clock::universal_time();
    icd.delete_time = boost::posix_time::microsec_clock::universal_time();
    icd.disclosename = true;

    Fred::InfoContactOut i;
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
    i.tech_contacts = Util::vector_of<std::string>("tech1")("tech2")("tech3");
    i.dns_keys = Util::vector_of<Fred::DnsKey>
    (Fred::DnsKey(257, 3, 5, "test1"))
    (Fred::DnsKey(257, 3, 5, "test2"));
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
    d.dns_keys = Util::vector_of<Fred::DnsKey>
    (Fred::DnsKey(257, 3, 5, "test1"))
    (Fred::DnsKey(257, 3, 5, "test2"));

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

/**
 * test CheckNsset print to string
 */
BOOST_AUTO_TEST_CASE(check_nsset)
{
    printable_test(
    Fred::CheckNsset("TEST-CHECK-NSSET-HANDLE")
    );
}

/**
 * test InfoNssetData print to string
 */
BOOST_AUTO_TEST_CASE(info_nsset_data)
{
    Fred::InfoNssetData i;
    i.handle = "TEST-INFO-NSSET-HANDLE";
    i.creation_time = boost::posix_time::microsec_clock::universal_time();
    i.delete_time = boost::posix_time::microsec_clock::universal_time();
    i.dns_hosts = Util::vector_of<Fred::DnsHost>
    (Fred::DnsHost("test1dns.cz", Util::vector_of<std::string>("6.6.6.6")("7.7.7.7")))
    (Fred::DnsHost("test2dns.cz", Util::vector_of<std::string>("6.6.6.6")("7.7.7.7")));
    printable_test(i);
}

/**
 * test InfoKeysetHistoryOutput print to string
 */
BOOST_AUTO_TEST_CASE(info_nsset_history_output)
{
    Fred::InfoNssetData d;
    d.handle = "TEST-INFO-NSSET-HANDLE";
    d.creation_time = boost::posix_time::microsec_clock::universal_time();
    d.delete_time = boost::posix_time::microsec_clock::universal_time();
    d.tech_contacts = Util::vector_of<std::string>("tech1")("tech2")("tech3");
    d.dns_hosts = Util::vector_of<Fred::DnsHost>
        (Fred::DnsHost("test1dns.cz", Util::vector_of<std::string>("6.6.6.6")("7.7.7.7")))
        (Fred::DnsHost("test2dns.cz", Util::vector_of<std::string>("6.6.6.6")("7.7.7.7")));
    d.tech_check_level = 2;

    Fred::InfoNssetHistoryOutput i;
    i.history_valid_from = boost::posix_time::microsec_clock::universal_time();
    i.history_valid_to = boost::posix_time::microsec_clock::universal_time();
    i.info_nsset_data = d;
    i.logd_request_id = 1;
    i.next_historyid = 2;
    printable_test(i);
}


/**
 * test InfoNssetHistory print to string
 */
BOOST_AUTO_TEST_CASE(info_nsset_history)
{
    printable_test(
    Fred::InfoNssetHistory("TEST-NSSET-ROID", boost::posix_time::microsec_clock::universal_time())
    );
}

/**
 * test InfoNssetOutput print to string
 */
BOOST_AUTO_TEST_CASE(info_nsset_output)
{
    Fred::InfoNssetData d;
    d.handle = "TEST-INFO-NSSET-HANDLE";
    d.creation_time = boost::posix_time::microsec_clock::universal_time();
    d.delete_time = boost::posix_time::microsec_clock::universal_time();
    d.tech_contacts = Util::vector_of<std::string>("tech1")("tech2")("tech3");

    Fred::InfoNssetOutput i;
    i.utc_timestamp = boost::posix_time::microsec_clock::universal_time();
    i.local_timestamp = boost::posix_time::microsec_clock::local_time();
    i.info_nsset_data = d;
    printable_test(i);
}

/**
 * test InfoNsset print to string
 */
BOOST_AUTO_TEST_CASE(info_nsset)
{
    printable_test(
    Fred::InfoNsset("TEST-NSSET-HANDLE")
    );
}

/**
 * test UpdateNsset print to string
 */
BOOST_AUTO_TEST_CASE(update_nsset)
{
    printable_test(
    Fred::UpdateNsset("TEST-NSSET-HANDLE", "REGISTRAR-TEST-HANDLE")
    .add_tech_contact("tech1")
    .add_tech_contact("tech2")
    .add_tech_contact("tech3")
    .add_dns(Fred::DnsHost("test1dns.cz", Util::vector_of<std::string>("6.6.6.6")("7.7.7.7")))
    .add_dns(Fred::DnsHost("test2dns.cz", Util::vector_of<std::string>("6.6.6.6")("7.7.7.7")))
    .rem_dns("test1dns.cz")
    .set_tech_check_level(2)
    .set_authinfo("testauthinfo").set_logd_request_id(0)
    );
}

struct Empty{};
struct HavingToString{
    std::string to_string() const {return "test";}
};
struct HavingToStringDev1{
    std::string to_string(int) const {return "test";}
};
struct HavingToStringDev2{
    void to_string() const {}
};
struct HavingToStringDev3{
    std::string to_string() {return "test";}
};

struct HavingStreamOp{
    friend std::ostream& operator<<(std::ostream& os, const HavingStreamOp& i)
    {return os << "test";}
};
struct HavingConversionToString{
    operator std::string() const {return "test";}
};

struct HavingConversionToStringDev1{
    operator std::string() {return "test";}
};

/**
 * test possible conversions to string with to_string exact signature match
 */
BOOST_AUTO_TEST_CASE(test_string_conversion)
{
    //check basic use cases
    BOOST_CHECK(Util::ConversionToString<HavingToString>::result == Util::TypeOfConversionToString::METHOD_TO_STRING);
    BOOST_CHECK(Util::ConversionToString<Empty>::result != Util::TypeOfConversionToString::METHOD_TO_STRING);
    BOOST_CHECK(Util::ConversionToString<HavingConversionToString>::result != Util::TypeOfConversionToString::METHOD_TO_STRING);

    //check conversion to string
    BOOST_CHECK(Util::ConversionToString<HavingToString>::result != Util::TypeOfConversionToString::CONVERTIBLE_TO_STRING);
    BOOST_CHECK(Util::ConversionToString<Empty>::result != Util::TypeOfConversionToString::CONVERTIBLE_TO_STRING);
    BOOST_CHECK(Util::ConversionToString<HavingConversionToString>::result == Util::TypeOfConversionToString::CONVERTIBLE_TO_STRING);
    BOOST_CHECK(Util::ConversionToString<HavingConversionToStringDev1>::result == Util::TypeOfConversionToString::CONVERTIBLE_TO_STRING);

    //check string is recognized as convertible to string
    BOOST_CHECK(Util::ConversionToString<std::string>::result != Util::TypeOfConversionToString::NONE);
    BOOST_CHECK(Util::ConversionToString<std::string>::result != Util::TypeOfConversionToString::METHOD_TO_STRING);
    BOOST_CHECK(Util::ConversionToString<std::string>::result == Util::TypeOfConversionToString::CONVERTIBLE_TO_STRING);

    //check signature detection
    BOOST_CHECK(Util::ConversionToString<HavingToStringDev1>::result != Util::TypeOfConversionToString::METHOD_TO_STRING);
    BOOST_CHECK(Util::ConversionToString<HavingToStringDev2>::result != Util::TypeOfConversionToString::METHOD_TO_STRING);
    BOOST_CHECK(Util::ConversionToString<HavingToStringDev3>::result != Util::TypeOfConversionToString::METHOD_TO_STRING);


}



/**
 * test MergeContact print to string
 */
BOOST_AUTO_TEST_CASE(merge_contact)
{
    printable_test(
    Fred::MergeContact("TEST-SRC-CONTACT", "TEST-DST-CONTACT", "REGISTRAR-HANDLE").set_logd_request_id(1)
    );
}

/**
 * test MergeContactOutput print to string
 */
BOOST_AUTO_TEST_CASE(merge_contact_output)
{
    printable_test(
    Fred::MergeContactOutput
              (
                  Fred::MergeContactLockedContactId(
                          0 //unsigned long long _src_contact_id
                          , 0 // unsigned long long _src_contact_historyid
                          , "TEST_CONTACT1" // const std::string& _src_contact_roid
                          , "REG-SPONSORING"
                          , 0 // unsigned long long _dst_contact_id
                          , 0 // unsigned long long _dst_contact_historyid
                          , "TEST_CONTACT5" //const std::string& _dst_contact_roid
                          , "REG-SPONSORING"
                  )
                  , Util::vector_of<Fred::MergeContactUpdateDomainRegistrant>
                      (Fred::MergeContactUpdateDomainRegistrant("domain.cz", 0, "REG-SPONSORING"
                          , "REGISTRANT_CONTACT", Optional<unsigned long long>()))

                  , Util::vector_of<Fred::MergeContactUpdateDomainAdminContact>
                      (Fred::MergeContactUpdateDomainAdminContact("domain.cz", 0, "REG-SPONSORING"
                          , "REM_ADMIN_CONTACT", "ADD_ADMIN_CONTACT", Optional<unsigned long long>()))

                  , Util::vector_of<Fred::MergeContactUpdateNssetTechContact>
                      (Fred::MergeContactUpdateNssetTechContact("NSSET_HANDLE", 0, "REG-SPONSORING"
                          , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

                  , Util::vector_of<Fred::MergeContactUpdateKeysetTechContact>
                      (Fred::MergeContactUpdateKeysetTechContact("KEYSET_HANDLE", 0, "REG-SPONSORING"
                          , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

              )//MergeContactOutput
    );
}

/**
 * test MergeContactEmailNotificationData print to string
 */
BOOST_AUTO_TEST_CASE(merge_contact_email_notification_data)
{
    printable_test(
            Fred::MergeContactEmailNotificationData(Util::vector_of<Fred::MergeContactEmailNotificationInput>
                    (Fred::MergeContactEmailNotificationInput("TEST_CONTACT1", "TEST_CONTACT2", Fred::MergeContactOutput
                        (
                            Fred::MergeContactLockedContactId(
                                0 //unsigned long long _src_contact_id
                                , 0 // unsigned long long _src_contact_historyid
                                , "TEST_CONTACT1" // const std::string& _src_contact_roid
                                , "REG-SPONSORING"
                                , 0 // unsigned long long _dst_contact_id
                                , 0 // unsigned long long _dst_contact_historyid
                                , "TEST_CONTACT2" //const std::string& _dst_contact_roid
                                , "REG-SPONSORING"
                            )
                            , Util::vector_of<Fred::MergeContactUpdateDomainRegistrant>
                                (Fred::MergeContactUpdateDomainRegistrant("domain.cz", 0, "REG-SPONSORING"
                                    , "REGISTRANT_CONTACT", Optional<unsigned long long>()))

                            , Util::vector_of<Fred::MergeContactUpdateDomainAdminContact>
                                (Fred::MergeContactUpdateDomainAdminContact("domain.cz", 0, "REG-SPONSORING"
                                    , "REM_ADMIN_CONTACT", "ADD_ADMIN_CONTACT", Optional<unsigned long long>()))

                            , Util::vector_of<Fred::MergeContactUpdateNssetTechContact>
                                (Fred::MergeContactUpdateNssetTechContact("NSSET_HANDLE", 0, "REG-SPONSORING"
                                    , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

                            , Util::vector_of<Fred::MergeContactUpdateKeysetTechContact>
                                (Fred::MergeContactUpdateKeysetTechContact("KEYSET_HANDLE", 0, "REG-SPONSORING"
                                    , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

                        )//MergeContactOutput
                    )//MergeContactEmailNotificationInput
                  )//vector_of
                  (Fred::MergeContactEmailNotificationInput("TEST_CONTACT3", "TEST_CONTACT2", Fred::MergeContactOutput
                      (
                          Fred::MergeContactLockedContactId(
                                  0 //unsigned long long _src_contact_id
                                  , 0 // unsigned long long _src_contact_historyid
                                  , "TEST_CONTACT3" // const std::string& _src_contact_roid
                                  , "REG-SPONSORING"
                                  , 0 // unsigned long long _dst_contact_id
                                  , 0 // unsigned long long _dst_contact_historyid
                                  , "TEST_CONTACT2" //const std::string& _dst_contact_roid
                                  , "REG-SPONSORING"
                          )
                          , Util::vector_of<Fred::MergeContactUpdateDomainRegistrant>
                              (Fred::MergeContactUpdateDomainRegistrant("domain.cz", 0, "REG-SPONSORING"
                                  , "REGISTRANT_CONTACT", Optional<unsigned long long>()))

                          , Util::vector_of<Fred::MergeContactUpdateDomainAdminContact>
                              (Fred::MergeContactUpdateDomainAdminContact("domain.cz", 0, "REG-SPONSORING"
                                  , "REM_ADMIN_CONTACT", "ADD_ADMIN_CONTACT", Optional<unsigned long long>()))

                          , Util::vector_of<Fred::MergeContactUpdateNssetTechContact>
                              (Fred::MergeContactUpdateNssetTechContact("NSSET_HANDLE", 0, "REG-SPONSORING"
                                  , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

                          , Util::vector_of<Fred::MergeContactUpdateKeysetTechContact>
                              (Fred::MergeContactUpdateKeysetTechContact("KEYSET_HANDLE", 0, "REG-SPONSORING"
                                  , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

                      )//MergeContactOutput
                    )//MergeContactEmailNotificationInput
                  )//vector_of
                  (Fred::MergeContactEmailNotificationInput("TEST_CONTACT2", "TEST_CONTACT4", Fred::MergeContactOutput
                      (
                          Fred::MergeContactLockedContactId(
                                  0 //unsigned long long _src_contact_id
                                  , 0 // unsigned long long _src_contact_historyid
                                  , "TEST_CONTACT2" // const std::string& _src_contact_roid
                                  , "REG-SPONSORING"
                                  , 0 // unsigned long long _dst_contact_id
                                  , 0 // unsigned long long _dst_contact_historyid
                                  , "TEST_CONTACT4" //const std::string& _dst_contact_roid
                                  , "REG-SPONSORING"
                          )
                          , Util::vector_of<Fred::MergeContactUpdateDomainRegistrant>
                              (Fred::MergeContactUpdateDomainRegistrant("domain.cz", 0, "REG-SPONSORING"
                                , "REGISTRANT_CONTACT", Optional<unsigned long long>()))

                          , Util::vector_of<Fred::MergeContactUpdateDomainAdminContact>
                              (Fred::MergeContactUpdateDomainAdminContact("domain.cz", 0, "REG-SPONSORING"
                                , "REM_ADMIN_CONTACT", "ADD_ADMIN_CONTACT", Optional<unsigned long long>()))

                          , Util::vector_of<Fred::MergeContactUpdateNssetTechContact>
                              (Fred::MergeContactUpdateNssetTechContact("NSSET_HANDLE", 0, "REG-SPONSORING"
                                  , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

                          , Util::vector_of<Fred::MergeContactUpdateKeysetTechContact>
                              (Fred::MergeContactUpdateKeysetTechContact("KEYSET_HANDLE", 0, "REG-SPONSORING"
                                  , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

                        )//MergeContactOutput
                    )//MergeContactEmailNotificationInput
                  )//vector_of


                  (Fred::MergeContactEmailNotificationInput("TEST_CONTACT1", "TEST_CONTACT5", Fred::MergeContactOutput
                      (
                          Fred::MergeContactLockedContactId(
                                  0 //unsigned long long _src_contact_id
                                  , 0 // unsigned long long _src_contact_historyid
                                  , "TEST_CONTACT1" // const std::string& _src_contact_roid
                                  , "REG-SPONSORING"
                                  , 0 // unsigned long long _dst_contact_id
                                  , 0 // unsigned long long _dst_contact_historyid
                                  , "TEST_CONTACT5" //const std::string& _dst_contact_roid
                                  , "REG-SPONSORING"
                          )
                          , Util::vector_of<Fred::MergeContactUpdateDomainRegistrant>
                              (Fred::MergeContactUpdateDomainRegistrant("domain.cz", 0, "REG-SPONSORING"
                                  , "REGISTRANT_CONTACT", Optional<unsigned long long>()))

                          , Util::vector_of<Fred::MergeContactUpdateDomainAdminContact>
                              (Fred::MergeContactUpdateDomainAdminContact("domain.cz", 0, "REG-SPONSORING"
                                  , "REM_ADMIN_CONTACT", "ADD_ADMIN_CONTACT", Optional<unsigned long long>()))

                          , Util::vector_of<Fred::MergeContactUpdateNssetTechContact>
                              (Fred::MergeContactUpdateNssetTechContact("NSSET_HANDLE", 0, "REG-SPONSORING"
                                  , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

                          , Util::vector_of<Fred::MergeContactUpdateKeysetTechContact>
                              (Fred::MergeContactUpdateKeysetTechContact("KEYSET_HANDLE", 0, "REG-SPONSORING"
                                  , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

                      )//MergeContactOutput
                    )//MergeContactEmailNotificationInput
                  )//vector_of

                  (Fred::MergeContactEmailNotificationInput("TEST_CONTACT5", "TEST_CONTACT4", Fred::MergeContactOutput
                      (
                          Fred::MergeContactLockedContactId(
                              0 //unsigned long long _src_contact_id
                              , 0 // unsigned long long _src_contact_historyid
                              , "TEST_CONTACT5" // const std::string& _src_contact_roid
                              , "REG-SPONSORING"
                              , 0 // unsigned long long _dst_contact_id
                              , 0 // unsigned long long _dst_contact_historyid
                              , "TEST_CONTACT4" //const std::string& _dst_contact_roid
                              , "REG-SPONSORING"
                          )
                          , Util::vector_of<Fred::MergeContactUpdateDomainRegistrant>
                              (Fred::MergeContactUpdateDomainRegistrant("domain.cz", 0, "REG-SPONSORING"
                                  , "REGISTRANT_CONTACT", Optional<unsigned long long>()))

                          , Util::vector_of<Fred::MergeContactUpdateDomainAdminContact>
                              (Fred::MergeContactUpdateDomainAdminContact("domain.cz", 0, "REG-SPONSORING"
                                  , "REM_ADMIN_CONTACT", "ADD_ADMIN_CONTACT", Optional<unsigned long long>()))

                          , Util::vector_of<Fred::MergeContactUpdateNssetTechContact>
                              (Fred::MergeContactUpdateNssetTechContact("NSSET_HANDLE", 0, "REG-SPONSORING"
                                  , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

                          , Util::vector_of<Fred::MergeContactUpdateKeysetTechContact>
                              (Fred::MergeContactUpdateKeysetTechContact("KEYSET_HANDLE", 0, "REG-SPONSORING"
                                  , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

                      )//MergeContactOutput
                    )//MergeContactEmailNotificationInput
                  )//vector_of
                )//MergeContactEmailNotificationData
    );
}

/**
 * test MergeContactNotificationEmailAddr  print to string
 */
BOOST_AUTO_TEST_CASE(merge_contact_email_notification_email_addr)
{
    Fred::MergeContactNotificationEmail ed1;
    ed1.dst_contact_handle = "DST-CONTACT-HANDLE";
    ed1.dst_contact_roid = "DST-CONTACT-ROID";
    ed1.keyset_tech_list = Util::vector_of<std::string>("Keyset1")("Keyset2");

    Fred::MergeContactNotificationEmail ed2 = ed1;

    printable_test(
            Fred::MergeContactNotificationEmailAddr(
                Util::vector_of<Fred::MergeContactNotificationEmail>
                (ed1)(ed2)
        )//MergeContactNotificationEmailAddr
    );
}

/**
 * test MergeContactSelectionprint to string
 */

BOOST_AUTO_TEST_CASE(merge_contact_selection)
{
    printable_test(
    Fred::MergeContactSelection(
        Util::vector_of<std::string>("CONTACT1")("CONTACT2")
        , Util::vector_of<Fred::ContactSelectionFilterType>
        ("mcs_filter_identified_contact")("mcs_filter_not_regcznic"))
    );
}

/**
 * test MergeContactSelectionOutput print to string
 */
BOOST_AUTO_TEST_CASE(merge_contact_selection_output)
{
    printable_test(
    Fred::MergeContactSelectionOutput("HANDLE", "FILTER")
    );
}

/**
 * test CreateDeleteContactPollMessage print to string
 */

BOOST_AUTO_TEST_CASE(create_delete_contact_poll_message)
{
    printable_test(Fred::Poll::CreateDeleteContactPollMessage(0));
}

/**
 * test CreatePollMessage print to string
 */

BOOST_AUTO_TEST_CASE(create_poll_message)
{
    printable_test(Fred::Poll::CreatePollMessage("REGISTRAR","MSG-TYPE"));
}

/**
 * test CreateUpdateObjectPollMessage print to string
 */

BOOST_AUTO_TEST_CASE(create_update_object_poll_message)
{
    printable_test(Fred::Poll::CreateUpdateObjectPollMessage(0));
}


BOOST_AUTO_TEST_SUITE_END();//TestPrintable
