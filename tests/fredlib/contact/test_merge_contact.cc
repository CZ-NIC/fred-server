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

#include <boost/test/unit_test.hpp>
#include <string>
#include <boost/asio/ip/address.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

#include "src/fredlib/opcontext.h"
#include <fredlib/registrar.h>
#include <fredlib/contact.h>
#include <fredlib/domain.h>
#include <fredlib/nsset.h>
#include <fredlib/keyset.h>
#include "src/fredlib/contact/merge_contact_selection.h"
#include "src/fredlib/contact/merge_contact_email_notification_data.h"
#include "src/fredlib/registrar/get_registrar_handles.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/contact/find_contact_duplicates.h"

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"
#include "tests/fredlib/util.h"

#include "test_merge_contact_fixture.h"

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


BOOST_FIXTURE_TEST_SUITE(TestMergeContact, Test::Fixture::instantiate_db_template)

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

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(common_contact_handle,registrar_handle)
            .set_name(common_contact_handle+" NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(src_contact_handle,registrar_handle)
            .set_name("COMMON NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        BOOST_TEST_MESSAGE(std::string("merge_contact_contacts_fixture src_contact_handle: ") + src_contact_handle);

        Fred::InfoContactOutput ic = Fred::InfoContactByHandle(src_contact_handle).exec(ctx);

        BOOST_TEST_MESSAGE(std::string("merge_contact_contacts_fixture src_contact_handle roid: ") + ic.info_contact_data.roid);

        Fred::CreateContact(dst_contact_handle,registrar_handle)
            .set_name("COMMON NAME")
            .set_disclosename(true)
            .set_place(place)
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
        namespace ip = boost::asio::ip;

        Fred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
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
        namespace ip = boost::asio::ip;
        for(int i = 0 ; i < nsset_count; ++i)
        {
            Fred::CreateNsset(get_handle(i), registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
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
        namespace ip = boost::asio::ip;

        for(int i = 0 ; i < nsset_count; ++i)
        {
            Fred::CreateNsset(get_handle(i), registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
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

        Fred::InfoContactOutput info_dst_contact_1 = Fred::InfoContactByHandle(dst_contact_handle).exec(ctx);
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
            info_nsset_with_change.info_nsset_data.tech_contacts = Util::vector_of<Fred::ObjectIdHandlePair>(Fred::ObjectIdHandlePair(
                info_dst_contact_1.info_contact_data.id, info_dst_contact_1.info_contact_data.handle));
            info_nsset_with_change.info_nsset_data.historyid = info_nsset_2.at(i).info_nsset_data.historyid;
            info_nsset_with_change.info_nsset_data.update_registrar_handle = registrar_handle;
            info_nsset_with_change.info_nsset_data.update_time = info_nsset_2.at(i).info_nsset_data.update_time;
            BOOST_CHECK(info_nsset_with_change == info_nsset_2.at(i));
        }

        for(int i = 0; i < keyset_count; ++i)
        {
            Fred::InfoKeysetOutput info_keyset_with_change = info_keyset_1.at(i);
            info_keyset_with_change.info_keyset_data.tech_contacts = Util::vector_of<Fred::ObjectIdHandlePair>(Fred::ObjectIdHandlePair(
                info_dst_contact_1.info_contact_data.id, info_dst_contact_1.info_contact_data.handle));
            info_keyset_with_change.info_keyset_data.historyid = info_keyset_2.at(i).info_keyset_data.historyid;
            info_keyset_with_change.info_keyset_data.update_registrar_handle = registrar_handle;
            info_keyset_with_change.info_keyset_data.update_time = info_keyset_2.at(i).info_keyset_data.update_time;
            BOOST_CHECK(info_keyset_with_change == info_keyset_2.at(i));
        }

        for(int i = 0; i < domain_owner_count; ++i)
        {
            Fred::InfoDomainOutput info_domain_owner_with_change = info_domain_owner_1.at(i);
            info_domain_owner_with_change.info_domain_data.registrant = Fred::ObjectIdHandlePair(
                info_dst_contact_1.info_contact_data.id,info_dst_contact_1.info_contact_data.handle);
            info_domain_owner_with_change.info_domain_data.historyid = info_domain_owner_2.at(i).info_domain_data.historyid;
            info_domain_owner_with_change.info_domain_data.update_registrar_handle = registrar_handle;
            info_domain_owner_with_change.info_domain_data.update_time = info_domain_owner_2.at(i).info_domain_data.update_time;
            BOOST_CHECK(info_domain_owner_with_change == info_domain_owner_2.at(i));
        }

        for(int i = 0; i < domain_admin_count; ++i)
        {
            Fred::InfoDomainOutput info_domain_admin_with_change = info_domain_admin_1.at(i);
            info_domain_admin_with_change.info_domain_data.admin_contacts = Util::vector_of<Fred::ObjectIdHandlePair>(
                Fred::ObjectIdHandlePair(info_dst_contact_1.info_contact_data.id,info_dst_contact_1.info_contact_data.handle));
            info_domain_admin_with_change.info_domain_data.historyid = info_domain_admin_2.at(i).info_domain_data.historyid;
            info_domain_admin_with_change.info_domain_data.update_registrar_handle = registrar_handle;
            info_domain_admin_with_change.info_domain_data.update_time = info_domain_admin_2.at(i).info_domain_data.update_time;
            BOOST_CHECK(info_domain_admin_with_change == info_domain_admin_2.at(i));
        }

        std::vector<Fred::InfoContactOutput> info_src_contact_history_2 = Fred::InfoContactHistory(
            info_src_contact_1.info_contact_data.roid).exec(ctx);
        BOOST_CHECK(!info_src_contact_history_2.at(0).info_contact_data.delete_time.isnull());//check src contact is deleted

        Fred::InfoContactOutput info_dst_contact_2 = Fred::InfoContactByHandle(dst_contact_handle).exec(ctx);
        BOOST_CHECK(info_dst_contact_1.info_contact_data.authinfopw != info_dst_contact_2.info_contact_data.authinfopw); //check dst contact has new authinfo
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


struct merge_contact_domain_plus_db : virtual Test::Fixture::instantiate_db_template, merge_contact_domain_fixture {};

/**
 * test MergeContact
 * compare state before merge with state after
 */
BOOST_FIXTURE_TEST_CASE(merge_contact, merge_contact_domain_plus_db)
{
    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    Fred::InfoContactOutput info_dst_contact_1 = Fred::InfoContactByHandle(dst_contact_handle).exec(ctx);
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
    info_domain_owner_with_change.info_domain_data.registrant = Fred::ObjectIdHandlePair(
            info_dst_contact_1.info_contact_data.id,info_dst_contact_1.info_contact_data.handle);
    info_domain_owner_with_change.info_domain_data.historyid = info_domain_owner_2.info_domain_data.historyid;
    info_domain_owner_with_change.info_domain_data.update_registrar_handle = registrar_handle;
    info_domain_owner_with_change.info_domain_data.update_time = info_domain_owner_2.info_domain_data.update_time;
    BOOST_CHECK(info_domain_owner_with_change == info_domain_owner_2);

    Fred::InfoDomainOutput info_domain_admin_with_change = info_domain_admin_1;
    info_domain_admin_with_change.info_domain_data.admin_contacts = Util::vector_of<Fred::ObjectIdHandlePair>(
        Fred::ObjectIdHandlePair(info_dst_contact_1.info_contact_data.id,info_dst_contact_1.info_contact_data.handle));
    info_domain_admin_with_change.info_domain_data.historyid = info_domain_admin_2.info_domain_data.historyid;
    info_domain_admin_with_change.info_domain_data.update_registrar_handle = registrar_handle;
    info_domain_admin_with_change.info_domain_data.update_time = info_domain_admin_2.info_domain_data.update_time;
    BOOST_CHECK(info_domain_admin_with_change == info_domain_admin_2);

    Fred::InfoKeysetOutput info_keyset_with_change = info_keyset_1;
    info_keyset_with_change.info_keyset_data.tech_contacts = Util::vector_of<Fred::ObjectIdHandlePair>(
            Fred::ObjectIdHandlePair(info_dst_contact_1.info_contact_data.id,info_dst_contact_1.info_contact_data.handle));
    info_keyset_with_change.info_keyset_data.historyid = info_keyset_2.info_keyset_data.historyid;
    info_keyset_with_change.info_keyset_data.update_registrar_handle = registrar_handle;
    info_keyset_with_change.info_keyset_data.update_time = info_keyset_2.info_keyset_data.update_time;
    BOOST_CHECK(info_keyset_with_change == info_keyset_2);

    Fred::InfoNssetOutput info_nsset_with_change = info_nsset_1;
    info_nsset_with_change.info_nsset_data.tech_contacts = Util::vector_of<Fred::ObjectIdHandlePair>(
        Fred::ObjectIdHandlePair(info_dst_contact_1.info_contact_data.id,info_dst_contact_1.info_contact_data.handle));
    info_nsset_with_change.info_nsset_data.historyid = info_nsset_2.info_nsset_data.historyid;
    info_nsset_with_change.info_nsset_data.update_registrar_handle = registrar_handle;
    info_nsset_with_change.info_nsset_data.update_time = info_nsset_2.info_nsset_data.update_time;
    BOOST_CHECK(info_nsset_with_change == info_nsset_2);

    std::vector<Fred::InfoContactOutput> info_src_contact_history_2 = Fred::InfoContactHistory(
        info_src_contact_1.info_contact_data.roid).exec(ctx);
    BOOST_CHECK(!info_src_contact_history_2.at(0).info_contact_data.delete_time.isnull());//check src contact is deleted

    Fred::InfoContactOutput info_dst_contact_2 = Fred::InfoContactByHandle(dst_contact_handle).exec(ctx);
    BOOST_CHECK(info_dst_contact_1.info_contact_data.authinfopw != info_dst_contact_2.info_contact_data.authinfopw); //check dst contact has new authinfo
}

/**
 * test MergeContact with non-existing src contact
 */
BOOST_FIXTURE_TEST_CASE(merge_contact_with_bad_src_contact, merge_contact_domain_plus_db)
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
BOOST_FIXTURE_TEST_CASE(merge_contact_with_bad_dst_contact, merge_contact_domain_plus_db)
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
BOOST_FIXTURE_TEST_CASE(merge_contact_with_different_src_contact, merge_contact_domain_plus_db)
{
    std::string different_src_contact_handle = src_contact_handle+"_different";

    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    Fred::Contact::PlaceAddress place;
    place.street1 = std::string("DIFFERENT STR1") + xmark;
    place.city = "Praha";
    place.postalcode = "11150";
    place.country = "CZ";
    Fred::CreateContact(different_src_contact_handle,registrar_handle)
                .set_name("COMMON NAME")
                .set_disclosename(true)
                .set_place(place)
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
BOOST_FIXTURE_TEST_CASE(merge_contact_with_different_dst_contact, merge_contact_domain_plus_db)
{
    std::string different_dst_contact_handle = dst_contact_handle+"_different";

    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    Fred::Contact::PlaceAddress place;
    place.street1 = std::string("DIFFERENT STR1") + xmark;
    place.city = "Praha";
    place.postalcode = "11150";
    place.country = "CZ";
    Fred::CreateContact(different_dst_contact_handle,registrar_handle)
                .set_name("COMMON NAME")
                .set_disclosename(true)
                .set_place(place)
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
BOOST_FIXTURE_TEST_CASE(merge_contact_with_same_src_and_dst_contact, merge_contact_domain_plus_db)
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

/**
 * test MergeContact with mojeid src contact
 */
BOOST_FIXTURE_TEST_CASE(merge_contact_with_mojeid_src_contact, merge_contact_domain_plus_db)
{
    unsigned long long src_contact_id = Fred::InfoContactByHandle(
            src_contact_handle).exec(ctx).info_contact_data.id;
    if(!Fred::ObjectHasState(src_contact_id,
        Fred::ObjectState::MOJEID_CONTACT).exec(ctx))
    {
        ctx.get_conn().exec_params(
            "INSERT INTO object_state_request (object_id, state_id)"
            " VALUES ($1::integer, (SELECT id FROM enum_object_states"
            " WHERE name = $2::text))",
            Database::query_param_list
                (src_contact_id)
                (Fred::ObjectState::MOJEID_CONTACT));
        Fred::PerformObjectStateRequest().set_object_id(src_contact_id).exec(ctx);
        ctx.commit_transaction();
    }

    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    try
    {
        //merge
        Fred::MergeContactOutput merge_data = Fred::MergeContact(src_contact_handle, dst_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_src_contact_invalid());
        BOOST_CHECK(ex.get_src_contact_invalid().compare(src_contact_handle) == 0);
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
 * test MergeContact with blocked src contact
 */
BOOST_FIXTURE_TEST_CASE(merge_contact_with_blocked_src_contact, merge_contact_domain_plus_db)
{
    unsigned long long src_contact_id = Fred::InfoContactByHandle(
            src_contact_handle).exec(ctx).info_contact_data.id;
    {
        Fred::CreateObjectStateRequestId(src_contact_id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_BLOCKED)).exec(ctx);
        Fred::PerformObjectStateRequest(src_contact_id).exec(ctx);
        ctx.commit_transaction();
    }

    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    try
    {
        //merge
        Fred::MergeContactOutput merge_data = Fred::MergeContact(src_contact_handle, dst_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_src_contact_invalid());
        BOOST_CHECK(ex.get_src_contact_invalid().compare(src_contact_handle) == 0);
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
 * test MergeContact with delete prohibited src contact
 */
BOOST_FIXTURE_TEST_CASE(merge_contact_with_delete_prohibited_src_contact, merge_contact_domain_plus_db)
{
    unsigned long long src_contact_id = Fred::InfoContactByHandle(
            src_contact_handle).exec(ctx).info_contact_data.id;
    {
        Fred::CreateObjectStateRequestId(src_contact_id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
        Fred::PerformObjectStateRequest(src_contact_id).exec(ctx);
        ctx.commit_transaction();
    }

    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    try
    {
        //merge
        Fred::MergeContactOutput merge_data = Fred::MergeContact(src_contact_handle, dst_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_src_contact_invalid());
        BOOST_CHECK(ex.get_src_contact_invalid().compare(src_contact_handle) == 0);
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
 * test MergeContact with blocked dst contact
 */
BOOST_FIXTURE_TEST_CASE(merge_contact_with_blocked_dst_contact, merge_contact_domain_plus_db)
{
    unsigned long long dst_contact_id = Fred::InfoContactByHandle(
            dst_contact_handle).exec(ctx).info_contact_data.id;
    {
        Fred::CreateObjectStateRequestId(dst_contact_id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_BLOCKED)).exec(ctx);
        Fred::PerformObjectStateRequest(dst_contact_id).exec(ctx);
        ctx.commit_transaction();
    }

    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    try
    {
        //merge
        Fred::MergeContactOutput merge_data = Fred::MergeContact(src_contact_handle, dst_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_dst_contact_invalid());
        BOOST_CHECK(ex.get_dst_contact_invalid().compare(dst_contact_handle) == 0);
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
 * test MergeContact with blocked domain with admin
 */
BOOST_FIXTURE_TEST_CASE(merge_contact_with_blocked_admin_domain, merge_contact_domain_plus_db)
{
    unsigned long long admin_domain_id = Fred::InfoDomainByHandle(
            test_domain_admin_handle).exec(ctx).info_domain_data.id;
    {
        Fred::CreateObjectStateRequestId(admin_domain_id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_BLOCKED)).exec(ctx);
        Fred::PerformObjectStateRequest(admin_domain_id).exec(ctx);
        ctx.commit_transaction();
    }

    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;
        //merge
        Fred::MergeContactOutput merge_data = Fred::MergeContact(src_contact_handle, dst_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_blocked());
        BOOST_CHECK(ex.get_object_blocked().compare(test_domain_admin_handle) == 0);
    }

    {
        Fred::OperationContext ctx;
        //info after merge
        Fred::InfoDomainOutput info_domain_owner_2 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
        Fred::InfoDomainOutput info_domain_admin_2 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
        Fred::InfoKeysetOutput info_keyset_2 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
        Fred::InfoNssetOutput info_nsset_2 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

        //compare state before merge with state after
        BOOST_MESSAGE(Fred::diff_domain_data(info_domain_owner_1.info_domain_data,info_domain_owner_2.info_domain_data).to_string());

        BOOST_CHECK(info_domain_owner_1 == info_domain_owner_2);
        BOOST_CHECK(info_domain_admin_1 == info_domain_admin_2);
        BOOST_CHECK(info_keyset_1 == info_keyset_2);
        BOOST_CHECK(info_nsset_1 == info_nsset_2);
    }
}

/**
 * test MergeContact with blocked domain with registrant
 */
BOOST_FIXTURE_TEST_CASE(merge_contact_with_blocked_registrant_domain, merge_contact_domain_plus_db)
{
    unsigned long long registrant_domain_id = Fred::InfoDomainByHandle(
            test_domain_owner_handle).exec(ctx).info_domain_data.id;
    {
        Fred::CreateObjectStateRequestId(registrant_domain_id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_BLOCKED)).exec(ctx);
        Fred::PerformObjectStateRequest(registrant_domain_id).exec(ctx);
        ctx.commit_transaction();
    }

    //info before merge
    Fred::InfoDomainOutput info_domain_owner_1 = Fred::InfoDomainByHandle(test_domain_owner_handle).exec(ctx);
    Fred::InfoDomainOutput info_domain_admin_1 = Fred::InfoDomainByHandle(test_domain_admin_handle).exec(ctx);
    Fred::InfoKeysetOutput info_keyset_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoNssetOutput info_nsset_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;
        //merge
        Fred::MergeContactOutput merge_data = Fred::MergeContact(src_contact_handle, dst_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_blocked());
        BOOST_CHECK(ex.get_object_blocked().compare(test_domain_owner_handle) == 0);
    }

    {
        Fred::OperationContext ctx;

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
    Fred::Contact::PlaceAddress place;
    place.street1 = std::string("STR1") + xmark;
    place.city = "Praha";
    place.postalcode = "11150";
    place.country = "CZ";
    Fred::CreateContact(dst_contact_handle,registrar_handle)
        .set_name(std::string("TEST-MNTF-CONTACT NAME")+xmark)
        .set_disclosename(true)
        .set_place(place)
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

BOOST_AUTO_TEST_SUITE(OneObject)

struct merge_admin_contacts_fixture : virtual Test::Fixture::instantiate_db_template
{
    std::string sys_registrar_handle;
    std::string registrar_handle;
    std::string xmark;
    std::string contact_handle_1;
    std::string contact_handle_2;

    merge_admin_contacts_fixture()
    : xmark(RandomDataGenerator().xnumstring(6))
    , contact_handle_1(std::string("TEST-MC-CONTACT-1")+xmark)
    , contact_handle_2(std::string("TEST-MC-CONTACT-2")+xmark)
    {
        Fred::OperationContext ctx;

        sys_registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                    "SELECT handle FROM registrar WHERE system = FALSE ORDER BY id LIMIT 1")[0][0]);

        BOOST_CHECK(!sys_registrar_handle.empty());//expecting existing system registrar
        BOOST_CHECK(!registrar_handle.empty());//expecting existing non-system registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(contact_handle_1,registrar_handle)
            .set_name("COMMON NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        BOOST_TEST_MESSAGE(std::string("test contact_handle_1: ") + contact_handle_1);

        Fred::InfoContactOutput  ic = Fred::InfoContactByHandle(contact_handle_1).exec(ctx);

        BOOST_TEST_MESSAGE(std::string("test contact_handle_1 roid: ") + ic.info_contact_data.roid);

        Fred::CreateContact(contact_handle_2,registrar_handle)
            .set_name("COMMON NAME")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        BOOST_TEST_MESSAGE(std::string("test contact_handle_2: ") + contact_handle_2);

        ctx.commit_transaction();
    }

    ~merge_admin_contacts_fixture(){}
};

struct merge_tech_contact_nsset_fixture
    : virtual merge_admin_contacts_fixture
{
    std::string test_nsset_handle;

    merge_tech_contact_nsset_fixture()
    : test_nsset_handle(std::string("TEST-MC-TECH-NSSET-HANDLE")+xmark)
    {
        namespace ip = boost::asio::ip;

        Fred::OperationContext ctx;

        Fred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                )
                .set_tech_contacts(Util::vector_of<std::string>(contact_handle_1)(contact_handle_2))
                .exec(ctx);

        BOOST_TEST_MESSAGE(std::string("test nsset: ")+ test_nsset_handle);

        ctx.commit_transaction();
    }

    ~merge_tech_contact_nsset_fixture(){}
};

struct merge_tech_contact_keyset_fixture
    : virtual merge_admin_contacts_fixture
{
    std::string test_keyset_handle;

    merge_tech_contact_keyset_fixture()
    : test_keyset_handle (std::string("TEST-MC-TECH-KEYSET-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;

        Fred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(contact_handle_1)(contact_handle_2))
                .exec(ctx);

        BOOST_TEST_MESSAGE(std::string("test keyset: ")+ test_keyset_handle);

        ctx.commit_transaction();
    }

    ~merge_tech_contact_keyset_fixture(){}
};

struct merge_admin_contact_domain_fixture
    : virtual merge_admin_contacts_fixture
{
    std::string test_nsset_handle;
    std::string test_keyset_handle;
    std::string test_domain_handle;

    merge_admin_contact_domain_fixture()
    : test_nsset_handle(std::string("TEST-MC-ADMIN-NSSET-HANDLE")+xmark)
    , test_keyset_handle (std::string("TEST-MC-ADMIN-KEYSET-HANDLE")+xmark)
    , test_domain_handle(std::string("testmerge-admins"+xmark+".cz"))
    {
        namespace ip = boost::asio::ip;

        Fred::OperationContext ctx;

        Fred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                )
                .set_tech_contacts(Util::vector_of<std::string>(contact_handle_1))
                .exec(ctx);

        BOOST_TEST_MESSAGE(std::string("test nsset: ")+ test_nsset_handle);

        Fred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(contact_handle_1))
                .exec(ctx);

        BOOST_TEST_MESSAGE(std::string("test keyset: ")+ test_keyset_handle);

        Fred::CreateDomain(
                test_domain_handle //const std::string& fqdn
                , sys_registrar_handle //const std::string& registrar
                , contact_handle_2 //registrant
                )
        .set_nsset(test_nsset_handle).set_keyset(test_keyset_handle)
        .set_admin_contacts(Util::vector_of<std::string>(contact_handle_1)(contact_handle_2))
        .exec(ctx);

        BOOST_TEST_MESSAGE(std::string("test domain: ")+ test_domain_handle);

        ctx.commit_transaction();
    }

    ~merge_admin_contact_domain_fixture(){}
};

/**
 * test merge contacts used with the one domain
 */
BOOST_FIXTURE_TEST_CASE(test_merge_domain_admin_contacts, merge_admin_contact_domain_fixture)
{
    try
    {
        Fred::OperationContext ctx;
        Fred::InfoDomainOutput domain_info_1 = Fred::InfoDomainByHandle(test_domain_handle).exec(ctx);
        Fred::InfoContactOutput contact_info_1 = Fred::InfoContactByHandle(contact_handle_1).exec(ctx);
        Fred::InfoContactOutput contact_info_2 = Fred::InfoContactByHandle(contact_handle_2).exec(ctx);
        Fred::MergeContactOutput merge_data = Fred::MergeContact(contact_handle_1, contact_handle_2, sys_registrar_handle).exec(ctx);
        Fred::InfoDomainOutput domain_info_2 = Fred::InfoDomainByHandle(test_domain_handle).exec(ctx);
        BOOST_CHECK(domain_info_1 != domain_info_2);

        //src contact is not admin
        BOOST_CHECK(std::find(domain_info_2.info_domain_data.admin_contacts.begin()
        , domain_info_2.info_domain_data.admin_contacts.end()
        , Fred::ObjectIdHandlePair(contact_info_1.info_contact_data.id, contact_info_1.info_contact_data.handle)
        ) == domain_info_2.info_domain_data.admin_contacts.end());

        //dst contact is admin
        BOOST_CHECK(std::find(domain_info_2.info_domain_data.admin_contacts.begin()
        , domain_info_2.info_domain_data.admin_contacts.end()
        , Fred::ObjectIdHandlePair(contact_info_2.info_contact_data.id, contact_info_2.info_contact_data.handle)
        ) != domain_info_2.info_domain_data.admin_contacts.end());

        //check unrelated data not changed
        domain_info_1.info_domain_data.admin_contacts = domain_info_2.info_domain_data.admin_contacts;
        domain_info_1.info_domain_data.historyid = domain_info_2.info_domain_data.historyid;
        domain_info_1.info_domain_data.update_time = domain_info_2.info_domain_data.update_time;
        domain_info_1.info_domain_data.update_registrar_handle = domain_info_2.info_domain_data.update_registrar_handle;

        BOOST_MESSAGE(Fred::diff_domain_data(domain_info_1.info_domain_data, domain_info_2.info_domain_data).to_string());

        BOOST_CHECK(Fred::diff_domain_data(domain_info_1.info_domain_data, domain_info_2.info_domain_data).is_empty());

        BOOST_MESSAGE(merge_data);
        ctx.commit_transaction();
    }
    catch(boost::exception& ex)
    {
        BOOST_FAIL(boost::diagnostic_information(ex));
    }
}

/**
 * test merge contacts used with the one nsset
 */
BOOST_FIXTURE_TEST_CASE(test_merge_nsset_tech_contacts, merge_tech_contact_nsset_fixture)
{
    try
    {
        Fred::OperationContext ctx;
        Fred::InfoNssetOutput nsset_info_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);
        Fred::InfoContactOutput contact_info_1 = Fred::InfoContactByHandle(contact_handle_1).exec(ctx);
        Fred::InfoContactOutput contact_info_2 = Fred::InfoContactByHandle(contact_handle_2).exec(ctx);
        Fred::MergeContactOutput merge_data = Fred::MergeContact(contact_handle_1, contact_handle_2, sys_registrar_handle).exec(ctx);
        Fred::InfoNssetOutput nsset_info_2 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

        BOOST_CHECK(nsset_info_1 != nsset_info_2);

        //src contact is not admin
        BOOST_CHECK(std::find(nsset_info_2.info_nsset_data.tech_contacts.begin(),
                nsset_info_2.info_nsset_data.tech_contacts.end(),
                Fred::ObjectIdHandlePair(contact_info_1.info_contact_data.id, contact_info_1.info_contact_data.handle)
            ) == nsset_info_2.info_nsset_data.tech_contacts.end());

        //dst contact is admin
        BOOST_CHECK(std::find(nsset_info_2.info_nsset_data.tech_contacts.begin(),
                nsset_info_2.info_nsset_data.tech_contacts.end(),
                Fred::ObjectIdHandlePair(contact_info_2.info_contact_data.id, contact_info_2.info_contact_data.handle)
            ) != nsset_info_2.info_nsset_data.tech_contacts.end());

        //check unrelated data not changed
        nsset_info_1.info_nsset_data.tech_contacts = nsset_info_2.info_nsset_data.tech_contacts;
        nsset_info_1.info_nsset_data.historyid = nsset_info_2.info_nsset_data.historyid;
        nsset_info_1.info_nsset_data.update_time = nsset_info_2.info_nsset_data.update_time;
        nsset_info_1.info_nsset_data.update_registrar_handle = nsset_info_2.info_nsset_data.update_registrar_handle;

        BOOST_MESSAGE(Fred::diff_nsset_data(nsset_info_1.info_nsset_data, nsset_info_2.info_nsset_data).to_string());

        BOOST_CHECK(Fred::diff_nsset_data(nsset_info_1.info_nsset_data, nsset_info_2.info_nsset_data).is_empty());

        BOOST_MESSAGE(merge_data);
        ctx.commit_transaction();
    }
    catch(boost::exception& ex)
    {
        BOOST_FAIL(boost::diagnostic_information(ex));
    }
}

/**
 * test merge contacts used with the one keyset
 */
BOOST_FIXTURE_TEST_CASE(test_merge_keyset_tech_contacts, merge_tech_contact_keyset_fixture)
{
    try
    {
        Fred::OperationContext ctx;
        Fred::InfoKeysetOutput keyset_info_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
        Fred::InfoContactOutput contact_info_1 = Fred::InfoContactByHandle(contact_handle_1).exec(ctx);
        Fred::InfoContactOutput contact_info_2 = Fred::InfoContactByHandle(contact_handle_2).exec(ctx);

        Fred::MergeContactOutput merge_data = Fred::MergeContact(contact_handle_1, contact_handle_2, sys_registrar_handle).exec(ctx);
        Fred::InfoKeysetOutput keyset_info_2 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);

        BOOST_CHECK(keyset_info_1 != keyset_info_2);

        //src contact is not admin
        BOOST_CHECK(std::find(keyset_info_2.info_keyset_data.tech_contacts.begin(),
            keyset_info_2.info_keyset_data.tech_contacts.end(),
            Fred::ObjectIdHandlePair(contact_info_1.info_contact_data.id, contact_info_1.info_contact_data.handle)
                ) == keyset_info_2.info_keyset_data.tech_contacts.end());

        //dst contact is admin
        BOOST_CHECK(std::find(keyset_info_2.info_keyset_data.tech_contacts.begin(),
            keyset_info_2.info_keyset_data.tech_contacts.end(),
            Fred::ObjectIdHandlePair(contact_info_2.info_contact_data.id, contact_info_2.info_contact_data.handle)
                ) != keyset_info_2.info_keyset_data.tech_contacts.end());

        //check unrelated data not changed
        keyset_info_1.info_keyset_data.tech_contacts = keyset_info_2.info_keyset_data.tech_contacts;
        keyset_info_1.info_keyset_data.historyid = keyset_info_2.info_keyset_data.historyid;
        keyset_info_1.info_keyset_data.update_time = keyset_info_2.info_keyset_data.update_time;
        keyset_info_1.info_keyset_data.update_registrar_handle = keyset_info_2.info_keyset_data.update_registrar_handle;

        BOOST_MESSAGE(Fred::diff_keyset_data(keyset_info_1.info_keyset_data, keyset_info_2.info_keyset_data).to_string());

        BOOST_CHECK(Fred::diff_keyset_data(keyset_info_1.info_keyset_data, keyset_info_2.info_keyset_data).is_empty());

        BOOST_MESSAGE(merge_data);

        ctx.commit_transaction();
    }
    catch(boost::exception& ex)
    {
        BOOST_FAIL(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//OneObject

BOOST_AUTO_TEST_CASE(get_registrar_handles_except_excluded)
{
    std::vector<std::string> all_registrars = Fred::Registrar::GetRegistrarHandles().exec();
    std::vector<std::string> all_excluded_registrars = Fred::Registrar::GetRegistrarHandles().set_exclude_registrars(all_registrars).exec();
    BOOST_CHECK(!all_registrars.empty());
    BOOST_CHECK(all_excluded_registrars.empty());

    std::vector<std::string> registrars = Fred::Registrar::GetRegistrarHandles().set_exclude_registrars(Util::vector_of<std::string>("REG-FRED_B")).exec();
    BOOST_CHECK(std::find(registrars.begin(),registrars.end(),std::string("REG-FRED_A")) != registrars.end());
    BOOST_CHECK(std::find(registrars.begin(),registrars.end(),std::string("REG-FRED_B")) == registrars.end());
}


struct merge_contact_contacts_plus_db_fixture : virtual Test::Fixture::instantiate_db_template, merge_contact_contacts_fixture {};

BOOST_FIXTURE_TEST_CASE(test_find_contact_duplicate, merge_contact_contacts_plus_db_fixture)
{
    std::set<std::string> contact_duplicates_1 = Fred::Contact::FindContactDuplicates().exec(ctx);
    BOOST_CHECK(!contact_duplicates_1.empty());
    BOOST_MESSAGE(Util::format_container(contact_duplicates_1,", "));

    std::set<std::string> contact_duplicates_2 = Fred::Contact::FindContactDuplicates()
    .set_registrar(merge_contact_contacts_fixture::registrar_handle).exec(ctx);
    BOOST_CHECK(!contact_duplicates_2.empty());
    BOOST_MESSAGE(Util::format_container(contact_duplicates_2,", "));

    std::set<std::string> contact_duplicates_3 = Fred::Contact::FindContactDuplicates()
    .set_registrar(merge_contact_contacts_fixture::registrar_handle)
    .set_exclude_contacts(Util::set_of<std::string>(merge_contact_contacts_fixture::dst_contact_handle)).exec(ctx);
    BOOST_CHECK(contact_duplicates_3.find(merge_contact_contacts_fixture::src_contact_handle) == contact_duplicates_3.end());
    BOOST_CHECK(contact_duplicates_3.find(merge_contact_contacts_fixture::dst_contact_handle) == contact_duplicates_3.end());
    BOOST_MESSAGE(Util::format_container(contact_duplicates_3,", "));

    std::set<std::string> contact_duplicates_4 = Fred::Contact::FindContactDuplicates()
    .set_specific_contact(merge_contact_contacts_fixture::dst_contact_handle).exec(ctx);
    BOOST_CHECK(!contact_duplicates_4.empty());
    BOOST_CHECK(contact_duplicates_4.find(merge_contact_contacts_fixture::dst_contact_handle) != contact_duplicates_4.end());
    BOOST_CHECK(contact_duplicates_4.find(merge_contact_contacts_fixture::src_contact_handle) != contact_duplicates_4.end());
    BOOST_MESSAGE(Util::format_container(contact_duplicates_4,", "));

    std::set<std::string> contact_duplicates_5 = Fred::Contact::FindContactDuplicates()
    .set_exclude_contacts(Util::set_of<std::string>(merge_contact_contacts_fixture::dst_contact_handle)).exec(ctx);
    BOOST_CHECK(contact_duplicates_5.find(merge_contact_contacts_fixture::src_contact_handle) == contact_duplicates_5.end());
    BOOST_CHECK(contact_duplicates_5.find(merge_contact_contacts_fixture::dst_contact_handle) == contact_duplicates_5.end());
    BOOST_MESSAGE(Util::format_container(contact_duplicates_5,", "));

    ctx.commit_transaction();
}



BOOST_AUTO_TEST_SUITE(Combinations)

BOOST_AUTO_TEST_SUITE(ObjectCombinations)

struct merge_fixture : MergeContactFixture::mergeable_contact_grps_with_linked_objects_and_blocking_states
{
    merge_fixture()

    : MergeContactFixture::mergeable_contact_grps_with_linked_objects_and_blocking_states(
        1u//mergeable_contact_group_count
        ,Util::set_of<unsigned>(0)(1)(2)(3)(4)(5)(6)(7)(8)(9)(10)(11)(12)(13)(14)(15)(20)//linked_object_cases
        , Util::vector_of<std::set<std::string> > (std::set<std::string>())(std::set<std::string>())//contact_state_combinations//stateless states 0, 1
        , Util::vector_of<std::set<std::string> > (std::set<std::string>())//linked_object_state_combinations
        , init_linked_object_quantities()//linked_object_quantities
        )

    {}
};

BOOST_FIXTURE_TEST_CASE(test_no_linked_objects_no_states, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    //src contact
    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    //dst contact
    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no other changes
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_linked_nsset, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 1//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.size() == 1); //updated nsset, tech contact changed from src contact to dst contact

    std::string nsset_handle= create_nsset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //tech contact
    );
    BOOST_MESSAGE(nsset_handle);

    BOOST_MESSAGE(std::string("changed nsset fields: (\"")
            + Util::format_container(map_at(changed_nssets,nsset_handle).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).changed_fields()
        == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_nssets,nsset_handle).historyid.isset());

    BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.isset());
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().first.size() == 1);
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_nssets,nsset_handle).update_time.get_value().second.isnull());

    //no other changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_linked_nsset_with_added_tech_contact, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 3//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 3//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();

    for(std::map<std::string, Fred::InfoNssetDiff>::const_iterator ci = changed_nssets.begin(); ci != changed_nssets.end(); ++ci)
    {
        BOOST_MESSAGE("changed_nsset handle: " << ci->first);
    }

    BOOST_CHECK(changed_nssets.size() == 2); //updated nsset, tech contact changed from src contact to dst contact

    std::string nsset1_handle= create_nsset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //tech contact
        , Util::vector_of<std::string>(contact_handle_dst)
    );
    BOOST_MESSAGE(nsset1_handle);

    BOOST_MESSAGE(std::string("changed nsset fields: (\"")
            + Util::format_container(map_at(changed_nssets,nsset1_handle).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).changed_fields()
        == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).historyid.isset());

    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).tech_contacts.isset());
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).tech_contacts.get_value().first.size() == 2);

    BOOST_CHECK(Util::set_of<std::string> (map_at(changed_nssets,nsset1_handle).tech_contacts.get_value().first.at(0).handle)
        (map_at(changed_nssets,nsset1_handle).tech_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string> (contact_handle_src)(contact_handle_dst));
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).tech_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_nssets,nsset1_handle).update_time.get_value().second.isnull());

    std::string nsset2_handle= create_nsset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_dst //tech contact
        , Util::vector_of<std::string>(contact_handle_src)
    );
    BOOST_MESSAGE(nsset2_handle);

    BOOST_MESSAGE(std::string("changed nsset fields: (\"")
            + Util::format_container(map_at(changed_nssets,nsset2_handle).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).changed_fields()
        == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).historyid.isset());

    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).tech_contacts.isset());
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).tech_contacts.get_value().first.size() == 2);

    BOOST_CHECK(Util::set_of<std::string> (map_at(changed_nssets,nsset2_handle).tech_contacts.get_value().first.at(0).handle)
        (map_at(changed_nssets,nsset2_handle).tech_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string> (contact_handle_src)(contact_handle_dst));
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).tech_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_nssets,nsset2_handle).update_time.get_value().second.isnull());

    //no other changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}


BOOST_FIXTURE_TEST_CASE(test_linked_keyset, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 5//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());

    //keyset changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.size() == 1); //updated keyset, tech contact changed from src contact to dst contact

    std::string keyset_handle= create_keyset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //tech contact
    );
    BOOST_MESSAGE(keyset_handle);

    BOOST_MESSAGE(std::string("changed keyset fields: (\"")
        + Util::format_container(map_at(changed_keysets,keyset_handle).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).changed_fields()
        == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_keysets,keyset_handle).historyid.isset());

    BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.isset());
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().first.size() == 1);
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_keysets,keyset_handle).update_time.get_value().second.isnull());

    //no other changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_linked_keyset_with_added_tech_contact, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 7//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 7//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());

    //keyset changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.size() == 2); //updated keyset, tech contact changed from src contact to dst contact

    for(std::map<std::string, Fred::InfoKeysetDiff>::const_iterator ci = changed_keysets.begin(); ci != changed_keysets.end(); ++ci)
    {
        BOOST_MESSAGE("changed_keyset handle: " << ci->first);
    }

    std::string keyset1_handle= create_keyset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //tech contact
        , Util::vector_of<std::string>(contact_handle_dst)
    );
    BOOST_MESSAGE(keyset1_handle);

    BOOST_MESSAGE(std::string("changed keyset fields: (\"")
        + Util::format_container(map_at(changed_keysets,keyset1_handle).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).changed_fields()
        == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).historyid.isset());

    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).tech_contacts.isset());

    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).tech_contacts.get_value().first.size() == 2);
    BOOST_CHECK(Util::set_of<std::string> (map_at(changed_keysets,keyset1_handle).tech_contacts.get_value().first.at(0).handle)
        (map_at(changed_keysets,keyset1_handle).tech_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string> (contact_handle_src)(contact_handle_dst));

    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).tech_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_keysets,keyset1_handle).update_time.get_value().second.isnull());


    std::string keyset2_handle= create_keyset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_dst //tech contact
        , Util::vector_of<std::string>(contact_handle_src)
    );
    BOOST_MESSAGE(keyset2_handle);

    BOOST_MESSAGE(std::string("changed keyset fields: (\"")
        + Util::format_container(map_at(changed_keysets,keyset2_handle).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).changed_fields()
        == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).historyid.isset());

    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).tech_contacts.isset());

    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).tech_contacts.get_value().first.size() == 2);
    BOOST_CHECK(Util::set_of<std::string> (map_at(changed_keysets,keyset2_handle).tech_contacts.get_value().first.at(0).handle)
        (map_at(changed_keysets,keyset2_handle).tech_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string> (contact_handle_src)(contact_handle_dst));
    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).tech_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_keysets,keyset2_handle).update_time.get_value().second.isnull());



    //no other changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}


BOOST_FIXTURE_TEST_CASE(test_linked_domain_via_owner, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 13//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());

    //no keyset changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.size() == 1); //updated domain, owner contact changed from src contact to dst contact

    std::string fqdn= create_domain_with_owner_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //owner contact
    );
    BOOST_MESSAGE(fqdn);

    BOOST_MESSAGE(std::string("changed domain fields: (\"")
        + Util::format_container(map_at(changed_domains,fqdn).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_domains,fqdn).changed_fields()
        == Util::set_of<std::string>("historyid")("registrant")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_domains,fqdn).historyid.isset());

    BOOST_CHECK(map_at(changed_domains,fqdn).registrant.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).registrant.get_value().first.handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_domains,fqdn).registrant.get_value().second.handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_domains,fqdn).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_domains,fqdn).update_time.get_value().second.isnull());

    //no registrar changes
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_linked_domain_via_admin, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 9//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());

    //no keyset changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.size() == 1); //updated domain, owner contact changed from src contact to dst contact

    std::string fqdn= create_domain_with_admin_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , create_non_mergeable_contact_handle(registrar_vect.at(0)//registrar handle
            , 1u) //owner contact
        , contact_handle_src //admin contact
    );
    BOOST_MESSAGE(fqdn);

    BOOST_MESSAGE(std::string("changed domain fields: (\"")
        + Util::format_container(map_at(changed_domains,fqdn).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_domains,fqdn).changed_fields()
        == Util::set_of<std::string>("admin_contacts")("historyid")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_domains,fqdn).historyid.isset());

    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().first.at(0).handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_domains,fqdn).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_domains,fqdn).update_time.get_value().second.isnull());

    //no registrar changes
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_linked_nsset_5, merge_fixture)
{
    unsigned nsset_quantity = 5;
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 1//linked_object_case
        , 0//linked_object_state_case
        , nsset_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , nsset_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.size() == nsset_quantity); //updated nsset, tech contact changed from src contact to dst contact

    for(unsigned number = 0 ; number < nsset_quantity; ++number)
    {
        std::string nsset_handle= create_nsset_with_tech_contact_handle(
            0//linked_object_state_case
            , nsset_quantity//quantity_case
            , number//number in quantity
            , contact_handle_src //tech contact
        );
        BOOST_MESSAGE(nsset_handle);
        BOOST_MESSAGE(std::string("changed nsset fields: (\"")
                + Util::format_container(map_at(changed_nssets,nsset_handle).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).changed_fields()
            == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

        BOOST_CHECK(map_at(changed_nssets,nsset_handle).historyid.isset());

        BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.isset());
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().first.size() == 1);
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().second.size() == 1);
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

        BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.isset());
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.get_value().first.isnull());
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

        BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_nssets,nsset_handle).update_time.get_value().second.isnull());
    }

    //no other changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_linked_keyset_5, merge_fixture)
{
    unsigned keyset_quantity = 5;
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 5//linked_object_case
        , 0//linked_object_state_case
        , keyset_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , keyset_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());

    //keyset changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.size() == keyset_quantity); //updated nsset, tech contact changed from src contact to dst contact

    for(unsigned number = 0 ; number < keyset_quantity; ++number)
    {
        std::string keyset_handle= create_keyset_with_tech_contact_handle(
            0//linked_object_state_case
            , keyset_quantity//quantity_case
            , number//number in quantity
            , contact_handle_src //tech contact
        );
        BOOST_MESSAGE(keyset_handle);

        BOOST_MESSAGE(std::string("changed keyset fields: (\"")
            + Util::format_container(map_at(changed_keysets,keyset_handle).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).changed_fields()
            == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

        BOOST_CHECK(map_at(changed_keysets,keyset_handle).historyid.isset());

        BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.isset());
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().first.size() == 1);
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().second.size() == 1);
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

        BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.isset());
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.get_value().first.isnull());
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

        BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_keysets,keyset_handle).update_time.get_value().second.isnull());
    }
    //no other changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_linked_domain_via_owner_5, merge_fixture)
{
    unsigned domain_quantity = 5;
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 13//linked_object_case
        , 0//linked_object_state_case
        , domain_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , domain_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());

    //no keyset changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.size() == domain_quantity); //updated domain, owner contact changed from src contact to dst contact

    for(unsigned number = 0; number < domain_quantity; ++number)
    {
        std::string fqdn= create_domain_with_owner_contact_fqdn(
            0//linked_object_state_case
            , domain_quantity//quantity_case
            , number//number in quantity
            , contact_handle_src //owner contact
        );
        BOOST_MESSAGE(fqdn);

        BOOST_MESSAGE(std::string("changed domain fields: (\"")
            + Util::format_container(map_at(changed_domains,fqdn).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_domains,fqdn).changed_fields()
            == Util::set_of<std::string>("historyid")("registrant")("update_registrar_handle")("update_time"));

        BOOST_CHECK(map_at(changed_domains,fqdn).historyid.isset());

        BOOST_CHECK(map_at(changed_domains,fqdn).registrant.isset());
        BOOST_CHECK(map_at(changed_domains,fqdn).registrant.get_value().first.handle == contact_handle_src);
        BOOST_CHECK(map_at(changed_domains,fqdn).registrant.get_value().second.handle == contact_handle_dst);

        BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.isset());
        BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().first.isnull());
        BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

        BOOST_CHECK(map_at(changed_domains,fqdn).update_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_domains,fqdn).update_time.get_value().second.isnull());
    }
    //no registrar changes
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_linked_domain_via_admin_5, merge_fixture)
{
    unsigned domain_quantity = 5;
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 9//linked_object_case
        , 0//linked_object_state_case
        , domain_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , domain_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());

    //no keyset changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.size() == domain_quantity); //updated domain, owner contact changed from src contact to dst contact
    for(unsigned number = 0; number < domain_quantity; ++number)
    {
        std::string fqdn= create_domain_with_admin_contact_fqdn(
            0//linked_object_state_case
            , domain_quantity//quantity_case
            , number//number in quantity
            , create_non_mergeable_contact_handle(registrar_vect.at(0)//registrar handle
                , 1u) //owner contact
            , contact_handle_src //admin contact
        );
        BOOST_MESSAGE(fqdn);

        BOOST_MESSAGE(std::string("changed domain fields: (\"")
            + Util::format_container(map_at(changed_domains,fqdn).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_domains,fqdn).changed_fields()
            == Util::set_of<std::string>("admin_contacts")("historyid")("update_registrar_handle")("update_time"));

        BOOST_CHECK(map_at(changed_domains,fqdn).historyid.isset());

        BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.isset());
        BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().first.at(0).handle == contact_handle_src);
        BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().second.at(0).handle == contact_handle_dst);

        BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.isset());
        BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().first.isnull());
        BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

        BOOST_CHECK(map_at(changed_domains,fqdn).update_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_domains,fqdn).update_time.get_value().second.isnull());
    }
    //no registrar changes
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_linked_nsset_keyset_domain_via_admin_domain_via_owner, merge_fixture)
{
    unsigned accumulated_linked_object_quantity = 0;
    for(std::vector<unsigned>::const_iterator loq_ci = linked_object_quantities.begin(); loq_ci != linked_object_quantities.end(); ++loq_ci)
    {
        accumulated_linked_object_quantity += *loq_ci;

        std::string contact_handle_src = create_mergeable_contact_handle(
            registrar_vect.at(0)//registrar handle
            , 0 //grpidtag
            , 0//state_case
            , 15//linked_object_case
            , 0//linked_object_state_case
            , *loq_ci//quantity_case
        );
        BOOST_MESSAGE(contact_handle_src);
        std::string contact_handle_dst = create_mergeable_contact_handle(
            registrar_vect.at(0)//registrar handle
            , 0 //grpidtag
            , 1//state_case
            , 0//linked_object_case
            , 0//linked_object_state_case
            , *loq_ci//quantity_case
        );
        BOOST_MESSAGE(contact_handle_dst);
        try
        {
            Fred::OperationContext ctx;
            Fred::MergeContactOutput merge_data = Fred::MergeContact(
                contact_handle_src, contact_handle_dst
                , registrar_sys_handle).exec(ctx);
                ctx.commit_transaction();
                BOOST_MESSAGE(merge_data);
        }
        catch(boost::exception& ex)
        {
            BOOST_ERROR(boost::diagnostic_information(ex));
        }

        std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();

        //accumulated changed contacts 2,4,8,...
        BOOST_CHECK(changed_contacts.size() == (2u+2*(loq_ci - linked_object_quantities.begin()))); //deleted src contact, updated dst contact authinfo

        BOOST_MESSAGE(std::string("changed src contact fields: (\"")
            + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

        BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
        BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

        BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
            + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
            == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
                != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

        std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();

        BOOST_CHECK(changed_nssets.size() == accumulated_linked_object_quantity); //updated nsset, tech contact changed from src contact to dst contact

        for(unsigned number = 0; number < *loq_ci; ++number)//if src contact have linked object
        {
            std::string nsset_handle = create_nsset_with_tech_contact_handle(
                0//linked_object_state_case
                , *loq_ci//quantity_case
                , number//number in quantity
                , contact_handle_src //tech contact
            );
            BOOST_MESSAGE(nsset_handle);

            BOOST_MESSAGE(std::string("changed nsset fields: (\"")
                    + Util::format_container(map_at(changed_nssets,nsset_handle).changed_fields(), "\")(\"") + "\")");
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).changed_fields()
                == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

            BOOST_CHECK(map_at(changed_nssets,nsset_handle).historyid.isset());

            BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.isset());
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().first.size() == 1);
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().second.size() == 1);
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

            BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.isset());
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.get_value().first.isnull());
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

            BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_time.get_value().first.isnull());
            BOOST_CHECK(!map_at(changed_nssets,nsset_handle).update_time.get_value().second.isnull());
        }

        std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
        BOOST_CHECK(changed_keysets.size() == accumulated_linked_object_quantity); //updated keyset, tech contact changed from src contact to dst contact

        for(unsigned number = 0; number < *loq_ci; ++number)//if src contact have linked object
        {
            std::string keyset_handle= create_keyset_with_tech_contact_handle(
                0//linked_object_state_case
                , *loq_ci//quantity_case
                , number//number in quantity
                , contact_handle_src //tech contact
            );
            BOOST_MESSAGE(keyset_handle);

            BOOST_MESSAGE(std::string("changed keyset fields: (\"")
                + Util::format_container(map_at(changed_keysets,keyset_handle).changed_fields(), "\")(\"") + "\")");
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).changed_fields()
                == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

            BOOST_CHECK(map_at(changed_keysets,keyset_handle).historyid.isset());

            BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.isset());
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().first.size() == 1);
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().second.size() == 1);
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

            BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.isset());
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.get_value().first.isnull());
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

            BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_time.get_value().first.isnull());
            BOOST_CHECK(!map_at(changed_keysets,keyset_handle).update_time.get_value().second.isnull());
        }

        std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
        BOOST_CHECK(changed_domains.size() == (accumulated_linked_object_quantity * 2)); //updated domains, owner and admin contact changed from src contact to dst contact

        for(unsigned number = 0; number < *loq_ci; ++number)//if src contact have linked object
        {
            std::string owner_fqdn= create_domain_with_owner_contact_fqdn(
                0//linked_object_state_case
                , *loq_ci//quantity_case
                , number//number in quantity
                , contact_handle_src //owner contact
            );
            BOOST_MESSAGE(owner_fqdn);

            BOOST_MESSAGE(std::string("changed domain fields: (\"")
                + Util::format_container(map_at(changed_domains,owner_fqdn).changed_fields(), "\")(\"") + "\")");
            BOOST_CHECK(map_at(changed_domains,owner_fqdn).changed_fields()
                == Util::set_of<std::string>("historyid")("registrant")("update_registrar_handle")("update_time"));

            BOOST_CHECK(map_at(changed_domains,owner_fqdn).historyid.isset());

            BOOST_CHECK(map_at(changed_domains,owner_fqdn).registrant.isset());
            BOOST_CHECK(map_at(changed_domains,owner_fqdn).registrant.get_value().first.handle == contact_handle_src);
            BOOST_CHECK(map_at(changed_domains,owner_fqdn).registrant.get_value().second.handle == contact_handle_dst);

            BOOST_CHECK(map_at(changed_domains,owner_fqdn).update_registrar_handle.isset());
            BOOST_CHECK(map_at(changed_domains,owner_fqdn).update_registrar_handle.get_value().first.isnull());
            BOOST_CHECK(map_at(changed_domains,owner_fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

            BOOST_CHECK(map_at(changed_domains,owner_fqdn).update_time.get_value().first.isnull());
            BOOST_CHECK(!map_at(changed_domains,owner_fqdn).update_time.get_value().second.isnull());

            std::string admin_fqdn = create_domain_with_admin_contact_fqdn(
                0//linked_object_state_case
                , *loq_ci//quantity_case
                , number//number in quantity
                , create_non_mergeable_contact_handle(registrar_vect.at(0)//registrar handle
                    , 1u) //owner contact
                , contact_handle_src //admin contact
            );
            BOOST_MESSAGE(admin_fqdn);

            BOOST_MESSAGE(std::string("changed domain fields: (\"")
                + Util::format_container(map_at(changed_domains,admin_fqdn).changed_fields(), "\")(\"") + "\")");
            BOOST_CHECK(map_at(changed_domains,admin_fqdn).changed_fields()
                == Util::set_of<std::string>("admin_contacts")("historyid")("update_registrar_handle")("update_time"));

            BOOST_CHECK(map_at(changed_domains,admin_fqdn).admin_contacts.isset());
            BOOST_CHECK(map_at(changed_domains,admin_fqdn).admin_contacts.get_value().first.at(0).handle == contact_handle_src);
            BOOST_CHECK(map_at(changed_domains,admin_fqdn).admin_contacts.get_value().second.at(0).handle == contact_handle_dst);

            BOOST_CHECK(map_at(changed_domains,admin_fqdn).historyid.isset());

            BOOST_CHECK(map_at(changed_domains,admin_fqdn).update_registrar_handle.isset());
            BOOST_CHECK(map_at(changed_domains,admin_fqdn).update_registrar_handle.get_value().first.isnull());
            BOOST_CHECK(map_at(changed_domains,admin_fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

            BOOST_CHECK(map_at(changed_domains,admin_fqdn).update_time.get_value().first.isnull());
            BOOST_CHECK(!map_at(changed_domains,admin_fqdn).update_time.get_value().second.isnull());
        }

        std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
        BOOST_CHECK(changed_registrars.empty());
    }//for linked object quantities
}

BOOST_FIXTURE_TEST_CASE(test_non_existing_src_contact, merge_fixture)
{
    std::string contact_handle_src = "NONEXISTENT";
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_source_contact_handle());
        BOOST_CHECK(ex.get_unknown_source_contact_handle().compare(contact_handle_src) == 0);
    }


    //no changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.empty());
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_non_existing_dst_contact, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = "NONEXISTENT";
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_destination_contact_handle());
        BOOST_CHECK(ex.get_unknown_destination_contact_handle().compare(contact_handle_dst) == 0);
    }

    //no changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.empty());
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}


BOOST_FIXTURE_TEST_CASE(test_different_dst_contact, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_non_mergeable_contact_handle(registrar_vect.at(0)//registrar handle
        , 1u); //owner contact
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_contacts_differ());
        BOOST_CHECK(ex.get_contacts_differ().source_handle == contact_handle_src);
        BOOST_CHECK(ex.get_contacts_differ().destination_handle == contact_handle_dst);
    }

    //no changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.empty());
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_different_src_contact, merge_fixture)
{
    std::string contact_handle_src = create_non_mergeable_contact_handle(registrar_vect.at(0)//registrar handle
        , 1u); //owner contact
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_contacts_differ());
        BOOST_CHECK(ex.get_contacts_differ().source_handle == contact_handle_src);
        BOOST_CHECK(ex.get_contacts_differ().destination_handle == contact_handle_dst);
    }

    //no changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.empty());
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_identical_contact, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = contact_handle_src;
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_identical_contacts_handle());
        BOOST_CHECK(ex.get_identical_contacts_handle() == contact_handle_src);
    }

    //no changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.empty());
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_src_contact_linked_domain_via_owner_with_the_same_admin_and_merged_to_different_mergeable_admin_contact, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 20//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 20//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    //contact changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    //src contact
    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    //dst contact
    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no changes
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    for(std::map<std::string, Fred::InfoDomainDiff>::const_iterator ci = changed_domains.begin(); ci != changed_domains.end(); ++ci)
    {
        BOOST_MESSAGE("changed_domain fqdn: " << ci->first);
    }

    BOOST_CHECK(changed_domains.size() == 2); //updated domain, owner and admin contact changed from src contact to dst contact

    std::string fqdn1= create_domain_with_owner_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //owner contact
        ,  Util::vector_of<std::string>(contact_handle_src)(contact_handle_dst)
    );
    BOOST_MESSAGE(fqdn1);

    BOOST_MESSAGE(std::string("changed domain fields: (\"")
        + Util::format_container(map_at(changed_domains,fqdn1).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_domains,fqdn1).changed_fields()
        == Util::set_of<std::string>("admin_contacts")("historyid")("registrant")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_domains,fqdn1).admin_contacts.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn1).admin_contacts.get_value().first.size() == 2);

    BOOST_CHECK(Util::set_of<std::string>(map_at(changed_domains,fqdn1).admin_contacts.get_value().first.at(0).handle)
        (map_at(changed_domains,fqdn1).admin_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string>(contact_handle_src)(contact_handle_dst));

    BOOST_CHECK(map_at(changed_domains,fqdn1).admin_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_domains,fqdn1).admin_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn1).historyid.isset());

    BOOST_CHECK(map_at(changed_domains,fqdn1).registrant.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn1).registrant.get_value().first.handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_domains,fqdn1).registrant.get_value().second.handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn1).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn1).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_domains,fqdn1).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_domains,fqdn1).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_domains,fqdn1).update_time.get_value().second.isnull());

    std::string fqdn2= create_domain_with_owner_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_dst //owner contact
        ,  Util::vector_of<std::string>(contact_handle_dst)(contact_handle_src)
    );
    BOOST_MESSAGE(fqdn2);

    BOOST_MESSAGE(std::string("changed domain fields: (\"")
        + Util::format_container(map_at(changed_domains,fqdn2).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_domains,fqdn2).changed_fields()
        == Util::set_of<std::string>("admin_contacts")("historyid")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_domains,fqdn2).admin_contacts.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn2).admin_contacts.get_value().first.size() == 2);

    BOOST_CHECK(Util::set_of<std::string>(map_at(changed_domains,fqdn2).admin_contacts.get_value().first.at(0).handle)
        (map_at(changed_domains,fqdn2).admin_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string>(contact_handle_src)(contact_handle_dst));

    BOOST_CHECK(map_at(changed_domains,fqdn2).admin_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_domains,fqdn2).admin_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn2).historyid.isset());

    BOOST_CHECK(map_at(changed_domains,fqdn2).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn2).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_domains,fqdn2).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_domains,fqdn2).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_domains,fqdn2).update_time.get_value().second.isnull());

    //no registrar changes
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_AUTO_TEST_SUITE_END();//ObjectCombinations

BOOST_AUTO_TEST_SUITE(StateCombinations)

struct merge_with_states_fixture : MergeContactFixture::mergeable_contact_grps_with_linked_objects_and_blocking_states
{
    merge_with_states_fixture()
    : MergeContactFixture::mergeable_contact_grps_with_linked_objects_and_blocking_states(
        1u//mergeable_contact_group_count
        ,Util::set_of<unsigned>(15)(18)(19)(20)//linked_object_cases
        , init_set_of_contact_state_combinations()//contact_state_combinations//stateless states 0, 1
        , init_set_of_linked_object_state_combinations()//linked_object_state_combinations
        , init_linked_object_quantities()//linked_object_quantities
        )
    {}
};

BOOST_FIXTURE_TEST_CASE(test_merge_with_states_fixture, merge_with_states_fixture)
{
    //no changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.empty());
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_invalid_src_mojeid_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 6//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_src_contact_invalid());
        BOOST_CHECK(ex.get_src_contact_invalid() == contact_handle_src);
    }

    //no changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.empty());
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_invalid_src_serverblocked_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 5//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_src_contact_invalid());
        BOOST_CHECK(ex.get_src_contact_invalid() == contact_handle_src);
    }

    //no changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.empty());
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_invalid_src_deleteprohibited_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 4//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_src_contact_invalid());
        BOOST_CHECK(ex.get_src_contact_invalid() == contact_handle_src);
    }

    //no changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.empty());
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_invalid_dst_serverblocked_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 5//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_dst_contact_invalid());
        BOOST_CHECK(ex.get_dst_contact_invalid() == contact_handle_dst);
    }

    //no changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.empty());
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}


BOOST_FIXTURE_TEST_CASE(test_src_contact_linked_domain_via_admin_serverblocked, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 18//linked_object_case
        , 2//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_blocked());
        BOOST_CHECK(ex.get_object_blocked() == create_domain_with_admin_contact_fqdn(
            2 //linked_object_state_case
            , 1 //quantity_case
            , 0 //number in quantity
            , create_non_mergeable_contact_handle(registrar_vect.at(0), 1u) //owner contact
            , contact_handle_src//admin contact
            ));
    }

    //no changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.empty());
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_FIXTURE_TEST_CASE(test_src_contact_linked_domain_via_owner_serverblocked, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 19//linked_object_case
        , 2//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_blocked());
        BOOST_CHECK(ex.get_object_blocked() == create_domain_with_owner_contact_fqdn(
            2 //linked_object_state_case
            , 1 //quantity_case
            , 0 //number in quantity
            , contact_handle_src//owner contact
            ));
    }

    //no changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.empty());
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.empty());
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}



BOOST_FIXTURE_TEST_CASE(test_src_contact_updateprohibited_linked_domain_via_owner_with_the_same_admin_and_merged_to_different_mergeable_admin_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 2//state_case
        , 20//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 20//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    //contact changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    //src contact
    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    //dst contact
    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no changes
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.empty());
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.empty());

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    for(std::map<std::string, Fred::InfoDomainDiff>::const_iterator ci = changed_domains.begin(); ci != changed_domains.end(); ++ci)
    {
        BOOST_MESSAGE("changed_domain fqdn: " << ci->first);
    }

    BOOST_CHECK(changed_domains.size() == 1); //updated domain, owner and admin contact changed from src contact to dst contact

    std::string fqdn= create_domain_with_owner_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //owner contact
        ,  Util::vector_of<std::string>(contact_handle_src)(contact_handle_dst)
    );
    BOOST_MESSAGE(fqdn);

    BOOST_MESSAGE(std::string("changed domain fields: (\"")
        + Util::format_container(map_at(changed_domains,fqdn).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_domains,fqdn).changed_fields()
        == Util::set_of<std::string>("admin_contacts")("historyid")("registrant")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().first.size() == 2);

    BOOST_CHECK(Util::set_of<std::string>(map_at(changed_domains,fqdn).admin_contacts.get_value().first.at(0).handle)
        (map_at(changed_domains,fqdn).admin_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string>(contact_handle_src)(contact_handle_dst));

    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn).historyid.isset());

    BOOST_CHECK(map_at(changed_domains,fqdn).registrant.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).registrant.get_value().first.handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_domains,fqdn).registrant.get_value().second.handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_domains,fqdn).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_domains,fqdn).update_time.get_value().second.isnull());

    //no registrar changes
    std::map<std::string, Fred::InfoRegistrarDiff> changed_registrars = diff_registrars();
    BOOST_CHECK(changed_registrars.empty());
}

BOOST_AUTO_TEST_SUITE_END();//StateCombinations
BOOST_AUTO_TEST_SUITE_END();//Combinations
BOOST_AUTO_TEST_SUITE_END();//TestMergeContact

