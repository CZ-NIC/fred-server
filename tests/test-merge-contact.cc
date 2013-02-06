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
#include "fredlib/contact/merge_contact_selection.h"
#include "fredlib/contact/merge_contact_email_notification_data.h"
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

namespace Fred
{

const ContactSelectionFilterType MCS_FILTER_TEST1 = "mcs_filter_test1";
const ContactSelectionFilterType MCS_FILTER_TEST2 = "mcs_filter_test2";


class FilterTest1
: public ContactSelectionFilterBase
, public Util::FactoryAutoRegister<ContactSelectionFilterBase, FilterTest1>
{
public:
    std::vector<std::string> operator()(OperationContext& ctx
            , const std::vector<std::string>& contact_handle)
    {

        std::vector<std::string> filtered;
        for(std::vector<std::string>::const_iterator i = contact_handle.begin(); i != contact_handle.end() ; ++i)
        {
            Database::Result contact_check = ctx.get_conn().exec_params(
            "SELECT  $1::text", Database::query_param_list(*i));
            if((contact_check.size() == 1) && (std::string(contact_check[0][0]).compare("test1") != 0))
            {
                filtered.push_back(*i);
            }
        }
        return filtered;
    }

    static ContactSelectionFilterType registration_name()
    {
        return MCS_FILTER_TEST1;
    }

};//class FilterTest1

class FilterTest2
: public ContactSelectionFilterBase
, public Util::FactoryAutoRegister<ContactSelectionFilterBase, FilterTest2>
{
public:
    std::vector<std::string> operator()(OperationContext& ctx
            , const std::vector<std::string>& contact_handle)
    {

        std::vector<std::string> filtered;
        for(std::vector<std::string>::const_iterator i = contact_handle.begin(); i != contact_handle.end() ; ++i)
        {
            Database::Result contact_check = ctx.get_conn().exec_params(
            "SELECT  $1::text", Database::query_param_list(*i));
            if((contact_check.size() == 1) && (std::string(contact_check[0][0]).compare("test2") != 0))
            {
                filtered.push_back(*i);
            }
        }
        return filtered;
    }

    static ContactSelectionFilterType registration_name()
    {
        return MCS_FILTER_TEST2;
    }

};//class FilterTest2

}//namespace Fred


BOOST_AUTO_TEST_SUITE(TestMergeContact)

const std::string server_name = "test-merge-contact";

BOOST_AUTO_TEST_CASE(test_merge_contact_selection)
{
    // factory_check - required keys are in factory
    FactoryHaveSupersetOfKeysChecker<Fred::ContactSelectionFilterFactory>
    ::KeyVector required_keys = boost::assign::list_of
     (Fred::MCS_FILTER_TEST1)
     (Fred::MCS_FILTER_TEST2)
    ;

    FactoryHaveSupersetOfKeysChecker<Fred::ContactSelectionFilterFactory>
        (required_keys).check();

    Fred::OperationContext ctx;
    std::vector<std::string> contact_handles = Util::vector_of<std::string>
        ("test1")("test2")("test3")("test1")("test2");
    std::string result = Fred::MergeContactSelection(
            contact_handles
            , Util::vector_of<Fred::ContactSelectionFilterType>
                (Fred::MCS_FILTER_TEST1)
                (Fred::MCS_FILTER_TEST2)
                //(Fred::MCS_FILTER_IDENTIFIED_CONTACT)
                //(Fred::MCS_FILTER_CONDITIONALLY_IDENTIFIED_CONTACT)
    ).exec(ctx);
    BOOST_CHECK(result.compare("test3") == 0);
}
BOOST_AUTO_TEST_CASE(merge_contact_email_notification_data)
{
    Fred::OperationContext ctx;
    std::vector<Fred::MergeContactNotificationEmail> notif_emails
      = Fred::MergeContactEmailNotificationData(Util::vector_of<Fred::MergeContactEmailNotificationInput>
        (Fred::MergeContactEmailNotificationInput("TEST_CONTACT1", "TEST_CONTACT2", Fred::MergeContactOutput
            (
                Util::vector_of<Fred::MergeContactUpdateDomainRegistrant>
                    (Fred::MergeContactUpdateDomainRegistrant("fqdn.cz", "REG-SPONSORING"
                        , "REGISTRANT_CONTACT", Optional<unsigned long long>()))

                , Util::vector_of<Fred::MergeContactUpdateDomainAdminContact>
                    (Fred::MergeContactUpdateDomainAdminContact("fqdn.cz", "REG-SPONSORING"
                        , "REM_ADMIN_CONTACT", "ADD_ADMIN_CONTACT", Optional<unsigned long long>()))

                , Util::vector_of<Fred::MergeContactUpdateNssetTechContact>
                    (Fred::MergeContactUpdateNssetTechContact("NSSET_HANDLE", "REG-SPONSORING"
                        , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

                , Util::vector_of<Fred::MergeContactUpdateKeysetTechContact>
                    (Fred::MergeContactUpdateKeysetTechContact("KEYSET_HANDLE", "REG-SPONSORING"
                        , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

            )//MergeContactOutput
        )//MergeContactEmailNotificationInput
      )//vector_of
      (Fred::MergeContactEmailNotificationInput("TEST_CONTACT3", "TEST_CONTACT2", Fred::MergeContactOutput
          (
              Util::vector_of<Fred::MergeContactUpdateDomainRegistrant>
                  (Fred::MergeContactUpdateDomainRegistrant("fqdn.cz", "REG-SPONSORING"
                      , "REGISTRANT_CONTACT", Optional<unsigned long long>()))

              , Util::vector_of<Fred::MergeContactUpdateDomainAdminContact>
                  (Fred::MergeContactUpdateDomainAdminContact("fqdn.cz", "REG-SPONSORING"
                      , "REM_ADMIN_CONTACT", "ADD_ADMIN_CONTACT", Optional<unsigned long long>()))

              , Util::vector_of<Fred::MergeContactUpdateNssetTechContact>
                  (Fred::MergeContactUpdateNssetTechContact("NSSET_HANDLE", "REG-SPONSORING"
                      , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

              , Util::vector_of<Fred::MergeContactUpdateKeysetTechContact>
                  (Fred::MergeContactUpdateKeysetTechContact("KEYSET_HANDLE", "REG-SPONSORING"
                      , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

          )//MergeContactOutput
        )//MergeContactEmailNotificationInput
      )//vector_of
      (Fred::MergeContactEmailNotificationInput("TEST_CONTACT2", "TEST_CONTACT4", Fred::MergeContactOutput
          (
            Util::vector_of<Fred::MergeContactUpdateDomainRegistrant>
                (Fred::MergeContactUpdateDomainRegistrant("fqdn.cz", "REG-SPONSORING"
                    , "REGISTRANT_CONTACT", Optional<unsigned long long>()))

            , Util::vector_of<Fred::MergeContactUpdateDomainAdminContact>
                (Fred::MergeContactUpdateDomainAdminContact("fqdn.cz", "REG-SPONSORING"
                    , "REM_ADMIN_CONTACT", "ADD_ADMIN_CONTACT", Optional<unsigned long long>()))

            , Util::vector_of<Fred::MergeContactUpdateNssetTechContact>
                (Fred::MergeContactUpdateNssetTechContact("NSSET_HANDLE", "REG-SPONSORING"
                    , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

            , Util::vector_of<Fred::MergeContactUpdateKeysetTechContact>
                (Fred::MergeContactUpdateKeysetTechContact("KEYSET_HANDLE", "REG-SPONSORING"
                    , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

            )//MergeContactOutput
        )//MergeContactEmailNotificationInput
      )//vector_of


      (Fred::MergeContactEmailNotificationInput("TEST_CONTACT1", "TEST_CONTACT5", Fred::MergeContactOutput
          (
              Util::vector_of<Fred::MergeContactUpdateDomainRegistrant>
                  (Fred::MergeContactUpdateDomainRegistrant("fqdn.cz", "REG-SPONSORING"
                      , "REGISTRANT_CONTACT", Optional<unsigned long long>()))

              , Util::vector_of<Fred::MergeContactUpdateDomainAdminContact>
                  (Fred::MergeContactUpdateDomainAdminContact("fqdn.cz", "REG-SPONSORING"
                      , "REM_ADMIN_CONTACT", "ADD_ADMIN_CONTACT", Optional<unsigned long long>()))

              , Util::vector_of<Fred::MergeContactUpdateNssetTechContact>
                  (Fred::MergeContactUpdateNssetTechContact("NSSET_HANDLE", "REG-SPONSORING"
                      , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

              , Util::vector_of<Fred::MergeContactUpdateKeysetTechContact>
                  (Fred::MergeContactUpdateKeysetTechContact("KEYSET_HANDLE", "REG-SPONSORING"
                      , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

          )//MergeContactOutput
        )//MergeContactEmailNotificationInput
      )//vector_of

      (Fred::MergeContactEmailNotificationInput("TEST_CONTACT5", "TEST_CONTACT4", Fred::MergeContactOutput
          (
              Util::vector_of<Fred::MergeContactUpdateDomainRegistrant>
                  (Fred::MergeContactUpdateDomainRegistrant("fqdn.cz", "REG-SPONSORING"
                      , "REGISTRANT_CONTACT", Optional<unsigned long long>()))

              , Util::vector_of<Fred::MergeContactUpdateDomainAdminContact>
                  (Fred::MergeContactUpdateDomainAdminContact("fqdn.cz", "REG-SPONSORING"
                      , "REM_ADMIN_CONTACT", "ADD_ADMIN_CONTACT", Optional<unsigned long long>()))

              , Util::vector_of<Fred::MergeContactUpdateNssetTechContact>
                  (Fred::MergeContactUpdateNssetTechContact("NSSET_HANDLE", "REG-SPONSORING"
                      , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

              , Util::vector_of<Fred::MergeContactUpdateKeysetTechContact>
                  (Fred::MergeContactUpdateKeysetTechContact("KEYSET_HANDLE", "REG-SPONSORING"
                      , "REM_TECH_CONTACT", "ADD_TECH_CONTACT", Optional<unsigned long long>()))

          )//MergeContactOutput
        )//MergeContactEmailNotificationInput
      )//vector_of
    )//MergeContactEmailNotificationData
    .exec(ctx);
}
BOOST_AUTO_TEST_SUITE_END();//TestMergeContact

