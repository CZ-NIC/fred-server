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
#include "src/fredlib/registrar.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/contact/delete_contact.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/keyset/delete_keyset.h"
#include "src/fredlib/keyset/check_keyset.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/domain/delete_domain.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/domain/update_domain.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/keyset/update_keyset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/delete_nsset.h"
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/nsset/update_nsset.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/merge_contact.h"
#include "src/fredlib/contact/merge_contact_email_notification_data.h"
#include "src/fredlib/contact/merge_contact_selection.h"
#include "src/fredlib/poll/create_update_object_poll_message.h"
#include "src/fredlib/contact/info_contact_diff.h"
#include "src/fredlib/domain/info_domain_diff.h"
#include "src/fredlib/keyset/info_keyset_diff.h"
#include "src/fredlib/nsset/info_nsset_diff.h"
#include "src/fredlib/object/object_id_handle_pair.h"

#include "util/util.h"
#include "util/printable.h"

#include "random_data_generator.h"
#include "concurrent_queue.h"

#include "src/fredlib/db_settings.h"

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
    BOOST_TEST_MESSAGE(i.to_string());
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
    Fred::DeleteContactByHandle("TEST-DELETE-CONTACT-HANDLE")
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
    i.history_valid_from = boost::posix_time::microsec_clock::universal_time();
    i.history_valid_to = boost::posix_time::microsec_clock::universal_time();
    i.info_contact_data = icd;
    i.logd_request_id = 1;
    i.next_historyid = 2;
    printable_test(i);
}

/**
 * test InfoContactHistoryByRoid print to string
 */
BOOST_AUTO_TEST_CASE(info_contact_history)
{
    printable_test(
    Fred::InfoContactHistoryByRoid("TEST-CONTACT-ROID")
    );
}

/**
 * test InfoContactByHandle print to string
 */
BOOST_AUTO_TEST_CASE(info_contact_by_handle)
{
    printable_test(
    Fred::InfoContactByHandle("TEST-CONTACT-HANDLE")
    );
}

/**
 * test InfoContactById print to string
 */
BOOST_AUTO_TEST_CASE(info_contact_by_id)
{
    printable_test(
    Fred::InfoContactById(1)
    );
}

/**
 * test InfoContactHistoryById print to string
 */
BOOST_AUTO_TEST_CASE(history_info_contact_by_id)
{
    printable_test(
    Fred::InfoContactHistoryById(1)
    );
}

/**
 * test InfoContactHistoryByHistoryid print to string
 */
BOOST_AUTO_TEST_CASE(history_info_contact_by_historyid)
{
    printable_test(
    Fred::InfoContactHistoryByHistoryid(1)
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
    Fred::DeleteDomainByHandle("TEST-DELETE-CONTACT-HANDLE")
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
    i.admin_contacts = Util::vector_of<Fred::ObjectIdHandlePair>
        (Fred::ObjectIdHandlePair(1, "admin1"))
        (Fred::ObjectIdHandlePair(2, "admin2"))
        (Fred::ObjectIdHandlePair(3, "admin3"));

    i.expiration_date = boost::gregorian::day_clock::local_day();
    printable_test(i);
}

/**
 * test InfoDomainOutput print to string
 */
BOOST_AUTO_TEST_CASE(info_domain_history_output)
{
    Fred::InfoDomainData id;
    id.fqdn = "test-fred.cz";
    id.creation_time = boost::posix_time::microsec_clock::universal_time();
    id.delete_time = boost::posix_time::microsec_clock::universal_time();
    id.keyset=Nullable<Fred::ObjectIdHandlePair>(Fred::ObjectIdHandlePair(1,"TEST-KEYSET"));

    Fred::InfoDomainOutput ih;
    ih.history_valid_from = boost::posix_time::microsec_clock::universal_time();
    ih.history_valid_to = boost::posix_time::microsec_clock::universal_time();
    ih.info_domain_data = id;
    ih.logd_request_id = 1;
    ih.next_historyid = 2;
    printable_test(ih);
}

/**
 * test InfoDomainHistoryByRoid print to string
 */
BOOST_AUTO_TEST_CASE(info_domain_history)
{
    printable_test(
    Fred::InfoDomainHistoryByRoid("test-fred.cz")
    );
}

/**
 * test InfoDomainByHandle print to string
 */
BOOST_AUTO_TEST_CASE(info_domain)
{
    printable_test(
    Fred::InfoDomainByHandle("fred.cz")
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
    .set_authinfo("testauthinfo")
    );
}

/**
 * test DeleteKeyset print to string
 */
BOOST_AUTO_TEST_CASE(delete_keyset)
{
    printable_test(
    Fred::DeleteKeysetByHandle("TEST-DEL-KEYSET-HANDLE")
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
    i.tech_contacts = Util::vector_of<Fred::ObjectIdHandlePair>(Fred::ObjectIdHandlePair(1, "tech1"))
        (Fred::ObjectIdHandlePair(2, "tech2"))
        (Fred::ObjectIdHandlePair(3, "tech3"));
    i.dns_keys = Util::vector_of<Fred::DnsKey>
    (Fred::DnsKey(257, 3, 5, "test1"))
    (Fred::DnsKey(257, 3, 5, "test2"));
    printable_test(i);
}


/**
 * test InfoKeysetOutput print to string
 */
BOOST_AUTO_TEST_CASE(info_keyset_output)
{
    Fred::InfoKeysetData d;
    d.id = 1;
    d.handle = "TEST-INFO-KEYSET-HANDLE";
    d.creation_time = boost::posix_time::microsec_clock::universal_time();
    d.delete_time = boost::posix_time::microsec_clock::universal_time();
    d.tech_contacts = Util::vector_of<Fred::ObjectIdHandlePair>(Fred::ObjectIdHandlePair(1, "tech1"))
        (Fred::ObjectIdHandlePair(2, "tech2"))
        (Fred::ObjectIdHandlePair(3, "tech3"));
    d.dns_keys = Util::vector_of<Fred::DnsKey>
    (Fred::DnsKey(257, 3, 5, "test1"))
    (Fred::DnsKey(257, 3, 5, "test2"));

    Fred::InfoKeysetOutput i;
    i.history_valid_from = boost::posix_time::microsec_clock::universal_time();
    i.history_valid_to = boost::posix_time::microsec_clock::universal_time();
    i.utc_timestamp = boost::posix_time::microsec_clock::universal_time();
    i.info_keyset_data = d;
    i.logd_request_id = 1;
    i.next_historyid = 2;
    printable_test(i);
}

/**
 * test InfoKeysetHistoryByRoid print to string
 */
BOOST_AUTO_TEST_CASE(info_keyset_history)
{
    printable_test(
    Fred::InfoKeysetHistoryByRoid("TEST-KEYSET-ROID")
    );
}

/**
 * test InfoKeysetByHandle print to string
 */
BOOST_AUTO_TEST_CASE(info_keyset_by_handle)
{
    printable_test(
    Fred::InfoKeysetByHandle("TEST-KEYSET-HANDLE")
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
    namespace ip = boost::asio::ip;

    printable_test(
    Fred::CreateNsset("TEST-CREATE-NSSET-HANDLE", "REGISTRAR-TEST-HANDLE")
    .set_tech_contacts(Util::vector_of<std::string>("admin1")("admin2")("admin3"))
    .set_dns_hosts(Util::vector_of<Fred::DnsHost>
        (Fred::DnsHost("test1dns.cz", Util::vector_of<ip::address>(ip::address::from_string("6.6.6.6"))(ip::address::from_string("7.7.7.7"))))
        (Fred::DnsHost("test2dns.cz", Util::vector_of<ip::address>(ip::address::from_string("6.6.6.6"))(ip::address::from_string("7.7.7.7"))))
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
    Fred::DeleteNssetByHandle("TEST-DEL-NSSET-HANDLE")
    );
}


/**
 * test InfoNssetData print to string
 */
BOOST_AUTO_TEST_CASE(info_nsset_data)
{
    namespace ip = boost::asio::ip;
    Fred::InfoNssetData i;
    i.handle = "TEST-INFO-NSSET-HANDLE";
    i.creation_time = boost::posix_time::microsec_clock::universal_time();
    i.delete_time = boost::posix_time::microsec_clock::universal_time();
    i.dns_hosts = Util::vector_of<Fred::DnsHost>
    (Fred::DnsHost("test1dns.cz", Util::vector_of<ip::address>(ip::address::from_string("6.6.6.6"))(ip::address::from_string("7.7.7.7"))))
    (Fred::DnsHost("test2dns.cz", Util::vector_of<ip::address>(ip::address::from_string("6.6.6.6"))(ip::address::from_string("7.7.7.7"))));
    printable_test(i);
}

/**
 * test InfoNssetOutput print to string
 */
BOOST_AUTO_TEST_CASE(info_nsset_output)
{
    namespace ip = boost::asio::ip;
    Fred::InfoNssetData d;
    d.handle = "TEST-INFO-NSSET-HANDLE";
    d.creation_time = boost::posix_time::microsec_clock::universal_time();
    d.delete_time = boost::posix_time::microsec_clock::universal_time();
    d.tech_contacts = Util::vector_of<Fred::ObjectIdHandlePair>(Fred::ObjectIdHandlePair(1, "tech1"))
        (Fred::ObjectIdHandlePair(2, "tech2"))
        (Fred::ObjectIdHandlePair(3, "tech3"));
    d.dns_hosts = Util::vector_of<Fred::DnsHost>
        (Fred::DnsHost("test1dns.cz", Util::vector_of<ip::address>(ip::address::from_string("6.6.6.6"))(ip::address::from_string("7.7.7.7"))))
        (Fred::DnsHost("test2dns.cz", Util::vector_of<ip::address>(ip::address::from_string("6.6.6.6"))(ip::address::from_string("7.7.7.7"))));
    d.tech_check_level = 2;

    Fred::InfoNssetOutput i;
    i.history_valid_from = boost::posix_time::microsec_clock::universal_time();
    i.history_valid_to = boost::posix_time::microsec_clock::universal_time();
    i.info_nsset_data = d;
    i.logd_request_id = 1;
    i.next_historyid = 2;
    printable_test(i);
}


/**
 * test InfoNssetHistoryByRoid print to string
 */
BOOST_AUTO_TEST_CASE(info_nsset_history)
{
    printable_test(
    Fred::InfoNssetHistoryByRoid("TEST-NSSET-ROID")
    );
}

/**
 * test InfoNssetByHandle print to string
 */
BOOST_AUTO_TEST_CASE(info_nsset)
{
    printable_test(
    Fred::InfoNssetByHandle("TEST-NSSET-HANDLE")
    );
}

/**
 * test UpdateNsset print to string
 */
BOOST_AUTO_TEST_CASE(update_nsset)
{
    namespace ip = boost::asio::ip;
    printable_test(
    Fred::UpdateNsset("TEST-NSSET-HANDLE", "REGISTRAR-TEST-HANDLE")
    .add_tech_contact("tech1")
    .add_tech_contact("tech2")
    .add_tech_contact("tech3")
    .add_dns(Fred::DnsHost("test1dns.cz", Util::vector_of<ip::address>(ip::address::from_string("6.6.6.6"))(ip::address::from_string("7.7.7.7"))))
    .add_dns(Fred::DnsHost("test2dns.cz", Util::vector_of<ip::address>(ip::address::from_string("6.6.6.6"))(ip::address::from_string("7.7.7.7"))))
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
 * test UpdateContactById print to string
 */

BOOST_AUTO_TEST_CASE(update_contact_by_id)
{
    Fred::Contact::PlaceAddress place;
    place.street3 = std::string("test street 3");
    printable_test(Fred::UpdateContactById(5,"REGISTRAR-TEST").set_place(place));
}

/**
 * test InfoContactDiff print to string
 */

BOOST_AUTO_TEST_CASE(info_contact_diff)
{
    Fred::InfoContactData d;
    d.authinfopw ="test";
    Fred::InfoContactDiff diff = Fred::diff_contact_data(Fred::InfoContactData(),d);
    printable_test(diff);
}

/**
 * test InfoDOmainDiff print to string
 */

BOOST_AUTO_TEST_CASE(info_domain_diff)
{
    Fred::InfoDomainData d;
    d.authinfopw ="test";
    Fred::InfoDomainDiff diff = Fred::diff_domain_data(Fred::InfoDomainData(),d);
    printable_test(diff);
}

/**
 * test InfoNssetDiff print to string
 */

BOOST_AUTO_TEST_CASE(info_nsset_diff)
{
    Fred::InfoNssetData d;
    d.authinfopw ="test";
    Fred::InfoNssetDiff diff = Fred::diff_nsset_data(Fred::InfoNssetData(),d);
    printable_test(diff);
}

/**
 * test InfoKeysetDiff print to string
 */

BOOST_AUTO_TEST_CASE(info_keyset_diff)
{
    Fred::InfoKeysetData d;
    d.authinfopw ="test";
    Fred::InfoKeysetDiff diff = Fred::diff_keyset_data(Fred::InfoKeysetData(),d);
    printable_test(diff);
}

/**
 * test ObjectIdHandlePair print to string
 */

BOOST_AUTO_TEST_CASE(object_id_handle_pair)
{
    Fred::ObjectIdHandlePair i(1,"test");
    printable_test(i);
    BOOST_CHECK(i.id == 1);
    BOOST_CHECK(i.handle == "test");
}

BOOST_AUTO_TEST_SUITE_END();//TestPrintable
