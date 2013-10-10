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
#include "fredlib/contact/merge_contact.h"
#include "fredlib/contact/merge_contact_selection.h"
#include "fredlib/contact/merge_contact_email_notification_data.h"
#include "fredlib/contact/create_contact.h"

#include "fredlib/contact/create_contact.h"
#include "fredlib/nsset/create_nsset.h"
#include "fredlib/keyset/create_keyset.h"
#include "fredlib/domain/create_domain.h"
#include "fredlib/keyset/info_keyset.h"
#include "fredlib/nsset/info_nsset.h"
#include "fredlib/domain/info_domain.h"
#include "fredlib/contact/info_contact.h"

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

static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}

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

struct merge_contact_contacts_fixture
{
    Fred::OperationContext ctx;
    std::string registrar_handle;
    std::string xmark;
    std::string common_contact_handle;
    std::string src_contact_handle;
    std::string dst_contact_handle;


    merge_contact_contacts_fixture()
    : registrar_handle (static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]))
    , xmark(RandomDataGenerator().xnumstring(6))
    , common_contact_handle(std::string("TEST-MC-COMMON-CONTACT")+xmark)
    , src_contact_handle(std::string("TEST-MC-SRC-CONTACT")+xmark)
    , dst_contact_handle(std::string("TEST-MC-DST-CONTACT")+xmark)
    {

        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(common_contact_handle,registrar_handle)
            .set_name(common_contact_handle+" NAME")
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(src_contact_handle,registrar_handle)
            .set_name("COMMON NAME")
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);
        BOOST_TEST_MESSAGE(std::string("merge_contact_contacts_fixture src_contact_handle: ") + src_contact_handle);

        Fred::InfoContactOutput ic = Fred::InfoContactByHandle(src_contact_handle).exec(ctx);

        BOOST_TEST_MESSAGE(std::string("merge_contact_contacts_fixture src_contact_handle roid: ") + ic.info_contact_data.roid);

        Fred::CreateContact(dst_contact_handle,registrar_handle)
            .set_name("COMMON NAME")
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);
    }

    ~merge_contact_contacts_fixture(){}
};

struct merge_contact_domain_fixture
    : virtual merge_contact_contacts_fixture
{
    std::string test_nsset_handle;
    std::string test_keyset_handle;
    std::string test_domain_owner_handle;
    std::string test_domain_admin_handle;

    merge_contact_domain_fixture()
    : test_nsset_handle(std::string("TEST-MC-NSSET-HANDLE")+xmark)
    , test_keyset_handle (std::string("TEST-MC-KEYSET-HANDLE")+xmark)
    , test_domain_owner_handle (std::string("mergecontactowner")+xmark+".cz")
    , test_domain_admin_handle (std::string("mergecontactadmin")+xmark+".cz")
    {
        Fred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3"))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))) //add_dns
                )
                .set_tech_contacts(Util::vector_of<std::string>(src_contact_handle))
                .exec(ctx);

        Fred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(src_contact_handle))
                .exec(ctx);

        Fred::CreateDomain(
                test_domain_owner_handle //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , src_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(common_contact_handle))
        .exec(ctx);

        Fred::CreateDomain(
                test_domain_admin_handle //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , common_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(src_contact_handle))
        .exec(ctx);
    }

    ~merge_contact_domain_fixture(){}
};


struct merge_contact_n_nsset_fixture
    : virtual merge_contact_contacts_fixture
{
    int nsset_count;
    std::string test_nsset_handle;

    std::string get_handle(int i) const
    {
        std::stringstream test_nsset_handle_n;
        test_nsset_handle_n << test_nsset_handle << i;
        return test_nsset_handle_n.str();
    }

    merge_contact_n_nsset_fixture(int n)
    : nsset_count(n)
    , test_nsset_handle(std::string("TEST-MC-NSSET-HANDLE")+xmark+"_")
    {
        for(int i = 0 ; i < nsset_count; ++i)
        {
            Fred::CreateNsset(get_handle(i), registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3"))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))) //add_dns
                )
                .set_tech_contacts(Util::vector_of<std::string>(src_contact_handle))
                .exec(ctx);
        }//for nsset_count
    }

    ~merge_contact_n_nsset_fixture(){}
};

struct merge_contact_r_nsset_fixture
    : virtual merge_contact_contacts_fixture
{
    int nsset_count;
    std::string test_nsset_handle;

    std::string get_handle(int i) const
    {
        std::stringstream test_nsset_handle_n;
        test_nsset_handle_n << test_nsset_handle << i;
        return test_nsset_handle_n.str();
    }

    merge_contact_r_nsset_fixture(int n)
    : nsset_count(n)
    , test_nsset_handle(std::string("TEST-MC-R-NSSET-HANDLE")+xmark+"_")
    {
        for(int i = 0 ; i < nsset_count; ++i)
        {
            Fred::CreateNsset(get_handle(i), registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3"))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))) //add_dns
                )
                .set_tech_contacts(Util::vector_of<std::string>(dst_contact_handle))
                .exec(ctx);
        }//for nsset_count
    }

    ~merge_contact_r_nsset_fixture(){}
};


struct merge_contact_n_keyset_fixture
    : virtual merge_contact_contacts_fixture
{
    int keyset_count;
    std::string test_keyset_handle;

    std::string get_handle(int i) const
    {
        std::stringstream test_keyset_handle_n;
        test_keyset_handle_n << test_keyset_handle << i;
        return test_keyset_handle_n.str();
    }

    merge_contact_n_keyset_fixture(int n)
    : keyset_count(n)
    , test_keyset_handle(std::string("TEST-MC-KEYSET-HANDLE")+xmark+"_")
    {
        for(int i = 0 ; i < keyset_count; ++i)
        {
            Fred::CreateKeyset(get_handle(i), registrar_handle)
                    .set_tech_contacts(Util::vector_of<std::string>(src_contact_handle))
                    .exec(ctx);
        }//for keyset_count
    }

    ~merge_contact_n_keyset_fixture(){}
};

struct merge_contact_r_keyset_fixture
    : virtual merge_contact_contacts_fixture
{
    int keyset_count;
    std::string test_keyset_handle;

    std::string get_handle(int i) const
    {
        std::stringstream test_keyset_handle_n;
        test_keyset_handle_n << test_keyset_handle << i;
        return test_keyset_handle_n.str();
    }

    merge_contact_r_keyset_fixture(int n)
    : keyset_count(n)
    , test_keyset_handle(std::string("TEST-MC-R-KEYSET-HANDLE")+xmark+"_")
    {
        for(int i = 0 ; i < keyset_count; ++i)
        {
            Fred::CreateKeyset(get_handle(i), registrar_handle)
                    .set_tech_contacts(Util::vector_of<std::string>(dst_contact_handle))
                    .exec(ctx);
        }//for keyset_count
    }

    ~merge_contact_r_keyset_fixture(){}
};


struct merge_contact_n_domain_owner_fixture
    : virtual merge_contact_contacts_fixture
{
    int domain_owner_count;
    std::string test_domain_owner_handle;

    std::string get_handle(int i) const
    {
        std::stringstream test_domain_owner_handle_n;
        test_domain_owner_handle_n << test_domain_owner_handle << i << ".cz";
        return test_domain_owner_handle_n.str();
    }


    merge_contact_n_domain_owner_fixture(int n)
    : domain_owner_count(n)
    , test_domain_owner_handle(std::string("nmergecontactowner")+xmark+"-")
    {
        for(int i = 0 ; i < domain_owner_count; ++i)
        {
            std::stringstream test_domain_owner_handle_n;
            test_domain_owner_handle_n << test_domain_owner_handle << i << ".cz";

            Fred::CreateDomain(
                    get_handle(i) //const std::string& fqdn
                    , registrar_handle //const std::string& registrar
                    , src_contact_handle //registrant
                    )
            .set_admin_contacts(Util::vector_of<std::string>(common_contact_handle))
            .exec(ctx);
        }//for domain_owner_count
    }

    ~merge_contact_n_domain_owner_fixture(){}
};

struct merge_contact_r_domain_owner_fixture
    : virtual merge_contact_contacts_fixture
{
    int domain_owner_count;
    std::string test_domain_owner_handle;

    std::string get_handle(int i) const
    {
        std::stringstream test_domain_owner_handle_n;
        test_domain_owner_handle_n << test_domain_owner_handle << i << ".cz";
        return test_domain_owner_handle_n.str();
    }


    merge_contact_r_domain_owner_fixture(int n)
    : domain_owner_count(n)
    , test_domain_owner_handle(std::string("rmergecontactowner")+xmark+"-")
    {
        for(int i = 0 ; i < domain_owner_count; ++i)
        {
            std::stringstream test_domain_owner_handle_n;
            test_domain_owner_handle_n << test_domain_owner_handle << i << ".cz";

            Fred::CreateDomain(
                    get_handle(i) //const std::string& fqdn
                    , registrar_handle //const std::string& registrar
                    , dst_contact_handle //registrant
                    )
            .set_admin_contacts(Util::vector_of<std::string>(common_contact_handle))
            .exec(ctx);
        }//for domain_owner_count
    }

    ~merge_contact_r_domain_owner_fixture(){}
};


struct merge_contact_n_domain_admin_fixture
    : virtual merge_contact_contacts_fixture
{
    int domain_admin_count;
    std::string test_domain_admin_handle;

    std::string get_handle(int i) const
    {
        std::stringstream test_domain_admin_handle_n;
        test_domain_admin_handle_n << test_domain_admin_handle << i << ".cz";
        return test_domain_admin_handle_n.str();
    }

    merge_contact_n_domain_admin_fixture(int n)
    : domain_admin_count(n)
    , test_domain_admin_handle(std::string("nmergecontactadmin")+xmark+"-")
    {
        for(int i = 0 ; i < domain_admin_count; ++i)
        {
            std::stringstream test_domain_admin_handle_n;
            test_domain_admin_handle_n << test_domain_admin_handle << i << ".cz";

            Fred::CreateDomain(
                    get_handle(i) //const std::string& fqdn
                    , registrar_handle //const std::string& registrar
                    , common_contact_handle //registrant
                    )
            .set_admin_contacts(Util::vector_of<std::string>(src_contact_handle))
            .exec(ctx);
        }//for domain_admin_count
    }

    ~merge_contact_n_domain_admin_fixture(){}
};


struct merge_contact_r_domain_admin_fixture
    : virtual merge_contact_contacts_fixture
{
    int domain_admin_count;
    std::string test_domain_admin_handle;

    std::string get_handle(int i) const
    {
        std::stringstream test_domain_admin_handle_n;
        test_domain_admin_handle_n << test_domain_admin_handle << i << ".cz";
        return test_domain_admin_handle_n.str();
    }

    merge_contact_r_domain_admin_fixture(int n)
    : domain_admin_count(n)
    , test_domain_admin_handle(std::string("rmergecontactadmin")+xmark+"-")
    {
        for(int i = 0 ; i < domain_admin_count; ++i)
        {
            std::stringstream test_domain_admin_handle_n;
            test_domain_admin_handle_n << test_domain_admin_handle << i << ".cz";

            Fred::CreateDomain(
                    get_handle(i) //const std::string& fqdn
                    , registrar_handle //const std::string& registrar
                    , common_contact_handle //registrant
                    )
            .set_admin_contacts(Util::vector_of<std::string>(dst_contact_handle))
            .exec(ctx);
        }//for domain_admin_count
    }

    ~merge_contact_r_domain_admin_fixture(){}
};



struct merge_contact_n_fixture
    : virtual merge_contact_n_nsset_fixture
    , virtual merge_contact_n_keyset_fixture
    , virtual merge_contact_n_domain_owner_fixture
    , virtual merge_contact_n_domain_admin_fixture
{
    merge_contact_n_fixture(int nssets, int keysets, int domainowners, int domainadmins )
        : merge_contact_n_nsset_fixture(nssets)
        , merge_contact_n_keyset_fixture(keysets)
        , merge_contact_n_domain_owner_fixture(domainowners)
        , merge_contact_n_domain_admin_fixture(domainadmins)
        {}
    ~merge_contact_n_fixture(){}

    void test_impl()
    {
        //info before merge
        std::vector<Fred::InfoNssetOutput> info_nsset_1;
        info_nsset_1.reserve(nsset_count);
        for(int i = 0; i < nsset_count; ++i)
        {
            info_nsset_1.push_back(Fred::InfoNssetByHandle(merge_contact_n_nsset_fixture::get_handle(i)).exec(ctx));
        }

        std::vector<Fred::InfoKeysetOutput> info_keyset_1;
        info_keyset_1.reserve(keyset_count);
        for(int i = 0; i < keyset_count; ++i)
        {
            info_keyset_1.push_back(Fred::InfoKeysetByHandle(merge_contact_n_keyset_fixture::get_handle(i)).exec(ctx));
        }

        std::vector<Fred::InfoDomainOutput> info_domain_owner_1;
        info_domain_owner_1.reserve(domain_owner_count);
        for(int i = 0; i < domain_owner_count; ++i)
        {
            info_domain_owner_1.push_back(Fred::InfoDomainByHandle(merge_contact_n_domain_owner_fixture::get_handle(i)).exec(ctx));
        }

        std::vector<Fred::InfoDomainOutput> info_domain_admin_1;
        info_domain_admin_1.reserve(domain_admin_count);
        for(int i = 0; i < domain_admin_count; ++i)
        {
            info_domain_admin_1.push_back(Fred::InfoDomainByHandle(merge_contact_n_domain_admin_fixture::get_handle(i)).exec(ctx));
        }

        Fred::InfoContactOutput info_src_contact_1 = Fred::InfoContactByHandle(src_contact_handle).exec(ctx);
        std::vector<Fred::InfoContactOutput> info_src_contact_history_1 = Fred::InfoContactHistory(
                info_src_contact_1.info_contact_data.roid).exec(ctx);
        BOOST_CHECK(info_src_contact_history_1.at(0).info_contact_data.delete_time.isnull());//check src contact is not deleted

        //merge
        Fred::MergeContactOutput merge_data = Fred::MergeContact(src_contact_handle, dst_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();

        //info after merge
        std::vector<Fred::InfoNssetOutput> info_nsset_2;
        info_nsset_2.reserve(nsset_count);
        for(int i = 0; i < nsset_count; ++i)
        {
            info_nsset_2.push_back(Fred::InfoNssetByHandle(merge_contact_n_nsset_fixture::get_handle(i)).exec(ctx));
        }

        std::vector<Fred::InfoKeysetOutput> info_keyset_2;
        info_keyset_2.reserve(keyset_count);
        for(int i = 0; i < keyset_count; ++i)
        {
            info_keyset_2.push_back(Fred::InfoKeysetByHandle(merge_contact_n_keyset_fixture::get_handle(i)).exec(ctx));
        }

        std::vector<Fred::InfoDomainOutput> info_domain_owner_2;
        info_domain_owner_2.reserve(domain_owner_count);
        for(int i = 0; i < domain_owner_count; ++i)
        {
            info_domain_owner_2.push_back(Fred::InfoDomainByHandle(merge_contact_n_domain_owner_fixture::get_handle(i)).exec(ctx));
        }

        std::vector<Fred::InfoDomainOutput> info_domain_admin_2;
        info_domain_admin_2.reserve(domain_admin_count);
        for(int i = 0; i < domain_admin_count; ++i)
        {
            info_domain_admin_2.push_back(Fred::InfoDomainByHandle(merge_contact_n_domain_admin_fixture::get_handle(i)).exec(ctx));
        }

        //compare state before merge with state after
        for(int i = 0; i < nsset_count; ++i)
        {
            Fred::InfoNssetOutput info_nsset_with_change = info_nsset_1.at(i);
            info_nsset_with_change.info_nsset_data.tech_contacts = Util::vector_of<std::string>(dst_contact_handle);
            info_nsset_with_change.info_nsset_data.historyid = info_nsset_2.at(i).info_nsset_data.historyid;
            info_nsset_with_change.info_nsset_data.update_registrar_handle = registrar_handle;
            info_nsset_with_change.info_nsset_data.update_time = info_nsset_2.at(i).info_nsset_data.update_time;
            BOOST_CHECK(info_nsset_with_change == info_nsset_2.at(i));
        }

        for(int i = 0; i < keyset_count; ++i)
        {
            Fred::InfoKeysetOutput info_keyset_with_change = info_keyset_1.at(i);
            info_keyset_with_change.info_keyset_data.tech_contacts = Util::vector_of<std::string>(dst_contact_handle);
            info_keyset_with_change.info_keyset_data.historyid = info_keyset_2.at(i).info_keyset_data.historyid;
            info_keyset_with_change.info_keyset_data.update_registrar_handle = registrar_handle;
            info_keyset_with_change.info_keyset_data.update_time = info_keyset_2.at(i).info_keyset_data.update_time;
            BOOST_CHECK(info_keyset_with_change == info_keyset_2.at(i));
        }

        for(int i = 0; i < domain_owner_count; ++i)
        {
            Fred::InfoDomainOutput info_domain_owner_with_change = info_domain_owner_1.at(i);
            info_domain_owner_with_change.info_domain_data.registrant_handle = dst_contact_handle;
            info_domain_owner_with_change.info_domain_data.historyid = info_domain_owner_2.at(i).info_domain_data.historyid;
            info_domain_owner_with_change.info_domain_data.update_registrar_handle = registrar_handle;
            info_domain_owner_with_change.info_domain_data.update_time = info_domain_owner_2.at(i).info_domain_data.update_time;
            BOOST_CHECK(info_domain_owner_with_change == info_domain_owner_2.at(i));
        }

        for(int i = 0; i < domain_admin_count; ++i)
        {
            Fred::InfoDomainOutput info_domain_admin_with_change = info_domain_admin_1.at(i);
            info_domain_admin_with_change.info_domain_data.admin_contacts = Util::vector_of<std::string>(dst_contact_handle);
            info_domain_admin_with_change.info_domain_data.historyid = info_domain_admin_2.at(i).info_domain_data.historyid;
            info_domain_admin_with_change.info_domain_data.update_registrar_handle = registrar_handle;
            info_domain_admin_with_change.info_domain_data.update_time = info_domain_admin_2.at(i).info_domain_data.update_time;
            BOOST_CHECK(info_domain_admin_with_change == info_domain_admin_2.at(i));
        }

        std::vector<Fred::InfoContactOutput> info_src_contact_history_2 = Fred::InfoContactHistory(
            info_src_contact_1.info_contact_data.roid).exec(ctx);
        BOOST_CHECK(!info_src_contact_history_2.at(0).info_contact_data.delete_time.isnull());//check src contact is deleted

    }
};

/**
 * test MergeContact with variable numbers of objects
 * compare state before merge with state after
 */
BOOST_AUTO_TEST_CASE(merge_contact_n0_k0_do0_da0){merge_contact_n_fixture(0,0,0,0).test_impl();}

BOOST_AUTO_TEST_CASE(merge_contact_n1_k0_do0_da0){merge_contact_n_fixture(1,0,0,0).test_impl();}
BOOST_AUTO_TEST_CASE(merge_contact_n0_k1_do0_da0){merge_contact_n_fixture(0,1,0,0).test_impl();}
BOOST_AUTO_TEST_CASE(merge_contact_n0_k0_do1_da0){merge_contact_n_fixture(0,0,1,0).test_impl();}
BOOST_AUTO_TEST_CASE(merge_contact_n0_k0_do0_da1){merge_contact_n_fixture(0,0,0,1).test_impl();}

BOOST_AUTO_TEST_CASE(merge_contact_n10_k0_do0_da0){merge_contact_n_fixture(10,0,0,0).test_impl();}
BOOST_AUTO_TEST_CASE(merge_contact_n0_k10_do0_da0){merge_contact_n_fixture(0,10,0,0).test_impl();}
BOOST_AUTO_TEST_CASE(merge_contact_n0_k0_do10_da0){merge_contact_n_fixture(0,0,10,0).test_impl();}
BOOST_AUTO_TEST_CASE(merge_contact_n0_k0_do0_da10){merge_contact_n_fixture(0,0,0,10).test_impl();}

BOOST_AUTO_TEST_CASE(merge_contact_n1_k1_do1_da1){merge_contact_n_fixture(1,1,1,1).test_impl();}
BOOST_AUTO_TEST_CASE(merge_contact_n1_k1_do2_da2){merge_contact_n_fixture(1,1,2,2).test_impl();}

BOOST_AUTO_TEST_CASE(merge_contact_n10_k10_do10_da10){merge_contact_n_fixture(10,10,10,10).test_impl();}
BOOST_AUTO_TEST_CASE(merge_contact_n10_k10_do20_da20){merge_contact_n_fixture(10,10,20,20).test_impl();}


/**
 * test MergeContact
 * compare state before merge with state after
 */
BOOST_FIXTURE_TEST_CASE(merge_contact, merge_contact_domain_fixture)
{
    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    Fred::InfoContactOutput info_src_contact_1 = Fred::InfoContactByHandle(src_contact_handle).exec(ctx);
    std::vector<Fred::InfoContactOutput> info_src_contact_history_1 = Fred::InfoContactHistory(
            info_src_contact_1.info_contact_data.roid).exec(ctx);
    BOOST_CHECK(info_src_contact_history_1.at(0).info_contact_data.delete_time.isnull());//check src contact is not deleted

    //merge
    Fred::MergeContactOutput merge_data = Fred::MergeContact(src_contact_handle, dst_contact_handle, registrar_handle).exec(ctx);
    ctx.commit_transaction();

    //info after merge
    Fred::InfoDomainOutput info_domain_owner_2 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_2 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_2 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_2 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    //compare state before merge with state after
    Fred::InfoDomainOutput info_domain_owner_with_change = info_domain_owner_1;
    info_domain_owner_with_change.info_domain_data.registrant_handle = dst_contact_handle;
    info_domain_owner_with_change.info_domain_data.historyid = info_domain_owner_2.info_domain_data.historyid;
    info_domain_owner_with_change.info_domain_data.update_registrar_handle = registrar_handle;
    info_domain_owner_with_change.info_domain_data.update_time = info_domain_owner_2.info_domain_data.update_time;
    BOOST_CHECK(info_domain_owner_with_change == info_domain_owner_2);

    Fred::InfoDomainOutput info_domain_admin_with_change = info_domain_admin_1;
    info_domain_admin_with_change.info_domain_data.admin_contacts = Util::vector_of<std::string>(dst_contact_handle);
    info_domain_admin_with_change.info_domain_data.historyid = info_domain_admin_2.info_domain_data.historyid;
    info_domain_admin_with_change.info_domain_data.update_registrar_handle = registrar_handle;
    info_domain_admin_with_change.info_domain_data.update_time = info_domain_admin_2.info_domain_data.update_time;
    BOOST_CHECK(info_domain_admin_with_change == info_domain_admin_2);

    Fred::InfoKeysetOutput info_keyset_with_change = info_keyset_1;
    info_keyset_with_change.info_keyset_data.tech_contacts = Util::vector_of<std::string>(dst_contact_handle);
    info_keyset_with_change.info_keyset_data.historyid = info_keyset_2.info_keyset_data.historyid;
    info_keyset_with_change.info_keyset_data.update_registrar_handle = registrar_handle;
    info_keyset_with_change.info_keyset_data.update_time = info_keyset_2.info_keyset_data.update_time;
    BOOST_CHECK(info_keyset_with_change == info_keyset_2);

    Fred::InfoNssetOutput info_nsset_with_change = info_nsset_1;
    info_nsset_with_change.info_nsset_data.tech_contacts = Util::vector_of<std::string>(dst_contact_handle);
    info_nsset_with_change.info_nsset_data.historyid = info_nsset_2.info_nsset_data.historyid;
    info_nsset_with_change.info_nsset_data.update_registrar_handle = registrar_handle;
    info_nsset_with_change.info_nsset_data.update_time = info_nsset_2.info_nsset_data.update_time;
    BOOST_CHECK(info_nsset_with_change == info_nsset_2);

    std::vector<Fred::InfoContactOutput> info_src_contact_history_2 = Fred::InfoContactHistory(
        info_src_contact_1.info_contact_data.roid).exec(ctx);
    BOOST_CHECK(!info_src_contact_history_2.at(0).info_contact_data.delete_time.isnull());//check src contact is deleted
}

/**
 * test MergeContact with non-existing src contact
 */
BOOST_FIXTURE_TEST_CASE(merge_contact_with_bad_src_contact, merge_contact_domain_fixture)
{
    std::string bad_src_contact_handle = src_contact_handle+"_bad";

    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    try
    {
        //merge
        Fred::MergeContactOutput merge_data = Fred::MergeContact(bad_src_contact_handle, dst_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_source_contact_handle());
        BOOST_CHECK(ex.get_unknown_source_contact_handle().compare(bad_src_contact_handle) == 0);
    }

    //info after merge
    Fred::InfoDomainOutput info_domain_owner_2 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_2 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_2 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_2 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    //compare state before merge with state after
    BOOST_CHECK(info_domain_owner_1 == info_domain_owner_2);
    BOOST_CHECK(info_domain_admin_1 == info_domain_admin_2);
    BOOST_CHECK(info_keyset_1 == info_keyset_2);
    BOOST_CHECK(info_nsset_1 == info_nsset_2);
}

/**
 * test MergeContact with non-existing dst contact
 */
BOOST_FIXTURE_TEST_CASE(merge_contact_with_bad_dst_contact, merge_contact_domain_fixture)
{
    std::string bad_dst_contact_handle = dst_contact_handle+"_bad";

    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    try
    {
        //merge
        Fred::MergeContactOutput merge_data = Fred::MergeContact(src_contact_handle, bad_dst_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_destination_contact_handle());
        BOOST_CHECK(ex.get_unknown_destination_contact_handle().compare(bad_dst_contact_handle) == 0);
    }

    //info after merge
    Fred::InfoDomainOutput info_domain_owner_2 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_2 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_2 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_2 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    //compare state before merge with state after
    BOOST_CHECK(info_domain_owner_1 == info_domain_owner_2);
    BOOST_CHECK(info_domain_admin_1 == info_domain_admin_2);
    BOOST_CHECK(info_keyset_1 == info_keyset_2);
    BOOST_CHECK(info_nsset_1 == info_nsset_2);
}

/**
 * test MergeContact with different src contact
 */
BOOST_FIXTURE_TEST_CASE(merge_contact_with_different_src_contact, merge_contact_domain_fixture)
{
    std::string different_src_contact_handle = src_contact_handle+"_different";

    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    Fred::CreateContact(different_src_contact_handle,registrar_handle)
                .set_name("COMMON NAME")
                .set_disclosename(true)
                .set_street1(std::string("DIFFERENT STR1")+xmark)
                .set_city("Praha").set_postalcode("11150").set_country("CZ")
                .set_discloseaddress(true)
                .exec(ctx);
    try
    {
        //merge
        Fred::MergeContactOutput merge_data = Fred::MergeContact(different_src_contact_handle, dst_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_contacts_differ());
        BOOST_CHECK(ex.get_contacts_differ().source_handle.compare(different_src_contact_handle) == 0);
        BOOST_CHECK(ex.get_contacts_differ().destination_handle.compare(dst_contact_handle) == 0);
    }

    //info after merge
    Fred::InfoDomainOutput info_domain_owner_2 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_2 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_2 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_2 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    //compare state before merge with state after
    BOOST_CHECK(info_domain_owner_1 == info_domain_owner_2);
    BOOST_CHECK(info_domain_admin_1 == info_domain_admin_2);
    BOOST_CHECK(info_keyset_1 == info_keyset_2);
    BOOST_CHECK(info_nsset_1 == info_nsset_2);
}

/**
 * test MergeContact with different dst contact
 */
BOOST_FIXTURE_TEST_CASE(merge_contact_with_different_dst_contact, merge_contact_domain_fixture)
{
    std::string different_dst_contact_handle = dst_contact_handle+"_different";

    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    Fred::CreateContact(different_dst_contact_handle,registrar_handle)
                .set_name("COMMON NAME")
                .set_disclosename(true)
                .set_street1(std::string("DIFFERENT STR1")+xmark)
                .set_city("Praha").set_postalcode("11150").set_country("CZ")
                .set_discloseaddress(true)
                .exec(ctx);
    try
    {
        //merge
        Fred::MergeContactOutput merge_data = Fred::MergeContact(src_contact_handle, different_dst_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_contacts_differ());
        BOOST_CHECK(ex.get_contacts_differ().source_handle.compare(src_contact_handle) == 0);
        BOOST_CHECK(ex.get_contacts_differ().destination_handle.compare(different_dst_contact_handle) == 0);
    }

    //info after merge
    Fred::InfoDomainOutput info_domain_owner_2 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_2 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_2 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_2 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    //compare state before merge with state after
    BOOST_CHECK(info_domain_owner_1 == info_domain_owner_2);
    BOOST_CHECK(info_domain_admin_1 == info_domain_admin_2);
    BOOST_CHECK(info_keyset_1 == info_keyset_2);
    BOOST_CHECK(info_nsset_1 == info_nsset_2);
}

/**
 * test MergeContact with the same src and dst contact
 */
BOOST_FIXTURE_TEST_CASE(merge_contact_with_same_src_and_dst_contact, merge_contact_domain_fixture)
{
    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    try
    {
        //new db connection, there should be completed updates rolled back by exception from delete src contact
        Fred::OperationContext ctx;
        //merge
        Fred::MergeContactOutput merge_data = Fred::MergeContact(src_contact_handle, src_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_identical_contacts_handle());
        BOOST_CHECK(ex.get_identical_contacts_handle().compare(src_contact_handle) == 0);
    }

    //info after merge
    Fred::InfoDomainOutput info_domain_owner_2 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_2 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_2 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_2 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    //compare state before merge with state after
    BOOST_CHECK(info_domain_owner_1 == info_domain_owner_2);
    BOOST_CHECK(info_domain_admin_1 == info_domain_admin_2);
    BOOST_CHECK(info_keyset_1 == info_keyset_2);
    BOOST_CHECK(info_nsset_1 == info_nsset_2);
}


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
    Fred::MergeContactSelectionOutput result = Fred::MergeContactSelection(
            contact_handles
            , Util::vector_of<Fred::ContactSelectionFilterType>
                (Fred::MCS_FILTER_TEST1)
                (Fred::MCS_FILTER_TEST2)
                //(Fred::MCS_FILTER_IDENTIFIED_CONTACT)
                //(Fred::MCS_FILTER_CONDITIONALLY_IDENTIFIED_CONTACT)
    ).exec(ctx);
    BOOST_CHECK(result.handle.compare("test3") == 0);
}

BOOST_AUTO_TEST_CASE(merge_contact_email_notification_data)
{
    Fred::OperationContext ctx;
    std::vector<Fred::MergeContactNotificationEmail> notif_emails
      = Fred::MergeContactEmailNotificationData(Util::vector_of<Fred::MergeContactEmailNotificationInput>
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
    .exec(ctx);

    BOOST_CHECK(notif_emails.size() == 1);
    BOOST_CHECK(notif_emails.at(0).dst_contact_handle == "TEST_CONTACT4");

    BOOST_CHECK(notif_emails.at(0).domain_registrant_list.size() == 1);
    BOOST_CHECK(notif_emails.at(0).domain_registrant_list.at(0) == "domain.cz");

    BOOST_CHECK(notif_emails.at(0).domain_admin_list.size() == 1);
    BOOST_CHECK(notif_emails.at(0).domain_admin_list.at(0) == "domain.cz");

    BOOST_CHECK(notif_emails.at(0).nsset_tech_list.size() == 1);
    BOOST_CHECK(notif_emails.at(0).nsset_tech_list.at(0) == "NSSET_HANDLE");

    BOOST_CHECK(notif_emails.at(0).keyset_tech_list.size() == 1);
    BOOST_CHECK(notif_emails.at(0).keyset_tech_list.at(0) == "KEYSET_HANDLE");


    BOOST_CHECK( notif_emails.at(0).removed_list.size() == 4);

    BOOST_CHECK_EXCEPTION(
        Fred::MergeContactEmailNotificationData(Util::vector_of<Fred::MergeContactEmailNotificationInput>
            (Fred::MergeContactEmailNotificationInput("TEST_CONTACT4", "TEST_CONTACT4", Fred::MergeContactOutput
                (
                    Fred::MergeContactLockedContactId(
                            0 //unsigned long long _src_contact_id
                            , 0 // unsigned long long _src_contact_historyid
                            , "TEST_CONTACT4" // const std::string& _src_contact_roid
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
          .exec(ctx)
                , std::exception
                , check_std_exception);



}

BOOST_AUTO_TEST_CASE(merge_contact_notification_email_addr)
{
    std::string registrar_handle = "REG-FRED_A";
    Fred::OperationContext ctx;
    std::string xmark = RandomDataGenerator().xnumstring(6);

    std::string dst_contact_handle = std::string("TEST-MNTF-CONTACT-HANDLE")+xmark;
    Fred::CreateContact(dst_contact_handle,registrar_handle)
        .set_name(std::string("TEST-MNTF-CONTACT NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .set_notifyemail("mntf@nic.cz")
        .exec(ctx);

    std::string dst_contact_roid = ctx.get_conn().exec_params(
        "select roid from object_registry where name = $1::text"
        , Database::query_param_list(dst_contact_handle))[0][0];

    std::vector<Fred::MergeContactNotificationEmailWithAddr> notif_emails
      = Fred::MergeContactNotificationEmailAddr(
        Fred::MergeContactEmailNotificationData(Util::vector_of<Fred::MergeContactEmailNotificationInput>
        (Fred::MergeContactEmailNotificationInput("TEST_CONTACT1", dst_contact_handle, Fred::MergeContactOutput
            (
                Fred::MergeContactLockedContactId(
                        0 //unsigned long long _src_contact_id
                        , 0 // unsigned long long _src_contact_historyid
                        , "TEST_CONTACT1_ROID" // const std::string& _src_contact_roid
                        , registrar_handle
                        , 0 // unsigned long long _dst_contact_id
                        , 0 // unsigned long long _dst_contact_historyid
                        , dst_contact_roid //const std::string& _dst_contact_roid
                        , registrar_handle
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
    .exec(ctx)
    )//MergeContactNotificationEmailAddr
    .exec(ctx);

    BOOST_CHECK(notif_emails.size() == 1);
    BOOST_CHECK(notif_emails.at(0).notification_email_addr == "mntf@nic.cz");

}

struct contact_merge_duplicate_auto_fixture
    : virtual merge_contact_n_nsset_fixture
    , virtual merge_contact_n_keyset_fixture
    , virtual merge_contact_n_domain_owner_fixture
    , virtual merge_contact_n_domain_admin_fixture
    , virtual merge_contact_r_nsset_fixture
    , virtual merge_contact_r_keyset_fixture
    , virtual merge_contact_r_domain_owner_fixture
    , virtual merge_contact_r_domain_admin_fixture
{
    contact_merge_duplicate_auto_fixture(int nssets, int keysets, int domainowners, int domainadmins )
        : merge_contact_n_nsset_fixture(nssets)
        , merge_contact_n_keyset_fixture(keysets)
        , merge_contact_n_domain_owner_fixture(domainowners)
        , merge_contact_n_domain_admin_fixture(domainadmins)
        , merge_contact_r_nsset_fixture(nssets)
        , merge_contact_r_keyset_fixture(keysets)
        , merge_contact_r_domain_owner_fixture(domainowners)
        , merge_contact_r_domain_admin_fixture(domainadmins)
        {
            ctx.commit_transaction();
        }
    ~contact_merge_duplicate_auto_fixture(){}
};



BOOST_AUTO_TEST_CASE(create_merge_contact_data){(void)contact_merge_duplicate_auto_fixture(10,10,10,10);}

BOOST_AUTO_TEST_SUITE_END();//TestMergeContact

