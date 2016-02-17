#include "src/whois/whois.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/registrar/info_registrar_output.h"
#include "src/fredlib/registrar/info_registrar_data.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/create_registrar.h"
#include "random_data_generator.h"

#define BOOST_TEST_NO_MAIN

#include <sstream>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>


struct whois_impl_instance_fixture
{
    Registry::WhoisImpl::Server_impl impl;
};

BOOST_AUTO_TEST_SUITE(TestWhois)

struct test_registrar_fixture
{
    std::string xmark;
    std::string test_registrar_handle;

    test_registrar_fixture()
    : xmark(RandomDataGenerator().xnumstring(6)),
      test_registrar_handle(std::string("TEST-REGISTRAR-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;

        Fred::CreateRegistrar(test_registrar_handle)
            .set_name(std::string("TEST-REGISTRAR NAME")+xmark)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha")
            .set_postalcode("11150")
            .set_country("CZ")
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
        BOOST_MESSAGE(test_registrar_handle);
    }
    ~test_registrar_fixture()
    {}};

BOOST_AUTO_TEST_SUITE(get_registrar_by_handle)

struct get_registrar_fixture
        : whois_impl_instance_fixture,
          test_registrar_fixture
{};

BOOST_FIXTURE_TEST_CASE(get_registrar_by_handle, get_registrar_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoRegistrarData ird = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_registrar_data;
    Registry::WhoisImpl::Registrar reg = impl.get_registrar_by_handle(test_registrar_handle);

    BOOST_CHECK(reg.address.city == ird.city.get_value_or_default());
    BOOST_CHECK(reg.address.country_code == ird.country.get_value_or_default());
    BOOST_CHECK(reg.address.postal_code == ird.postalcode.get_value_or_default());
    BOOST_CHECK(reg.address.stateorprovince == ird.stateorprovince.get_value_or_default());
    BOOST_CHECK(reg.address.street1 == ird.street1.get_value_or_default());
    BOOST_CHECK(reg.address.street2 == ird.street2.get_value_or_default());
    BOOST_CHECK(reg.address.street3 == ird.street3.get_value_or_default());
    BOOST_CHECK(reg.fax == ird.fax.get_value_or_default());
    BOOST_CHECK(reg.handle == ird.handle);
    BOOST_CHECK(reg.id == ird.id);
    BOOST_CHECK(reg.organization == ird.organization.get_value_or_default());
    BOOST_CHECK(reg.phone == ird.telephone.get_value_or_default());
    BOOST_CHECK(reg.url == ird.url.get_value_or_default());
}

struct get_registar_no_registrar_fixuture
        : whois_impl_instance_fixture,//need?
          test_registrar_fixture
{};

BOOST_FIXTURE_TEST_CASE(get_registar_no_registrar, get_registar_no_registrar_fixuture)
{
    try
    {
//        Fred::OperationContext ctx; //needed?
//        Fred::InfoRegistrarOutput ird = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
        Registry::WhoisImpl::Registrar reg = impl.get_registrar_by_handle("reg-");
        BOOST_ERROR("unreported dangling registrar");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

//using same fixture, as it's not needed
BOOST_FIXTURE_TEST_CASE(get_registrar_wrong_handle, get_registar_no_registrar_fixuture)
{
    try
    {
        Registry::WhoisImpl::Registrar reg = impl.get_registrar_by_handle("");
        BOOST_ERROR("registrar handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_registrar_by_handle

struct get_my_registrar_list_fixture
        : test_registrar_fixture
{
    std::map<std::string,Fred::InfoRegistrarOutput> registrar_info;
    get_my_registrar_list_fixture()
    {
        Fred::OperationContext ctx;
        for(int i=0; i<10; ++i)
        {
            std::ostringstream test_handles;
            test_handles << "n" << i << test_registrar_handle;
            Fred::CreateRegistrar& cr = Fred::CreateRegistrar(test_handles.str())
                .set_name(std::string("TEST-REGISTRAR NAME")+xmark)
                .set_street1(std::string("STR1")+xmark)
                .set_city("Praha")
                .set_postalcode("11150")
                .set_country("CZ");
            if(i>5) //4 of them to be system
            {
                cr = cr.set_system(true);
            }
            cr.exec(ctx);
            registrar_info[test_handles.str()] =
                    Fred::InfoRegistrarByHandle(test_handles.str())
                    .exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
        }

        ctx.commit_transaction();
    }
    ~get_my_registrar_list_fixture()
    {}
};

BOOST_AUTO_TEST_SUITE(get_registrars)

BOOST_FIXTURE_TEST_CASE(get_my_registrars, get_my_registrar_list_fixture)
{
    Fred::OperationContext ctx;
    std::vector<Fred::InfoRegistrarOutput> reg_list_out =
            Fred::InfoRegistrarAllExceptSystem()
            .exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
    for(int i=0; i < reg_list_out.size(); ++i)
    {
        //refactor
        BOOST_CHECK(reg_list_out.at(i).info_registrar_data.handle == map_at(registrar_info, reg_list_out.at(i).info_registrar_data.handle).info_registrar_data.handle);
        BOOST_CHECK(reg_list_out.at(i).info_registrar_data.id == map_at(registrar_info, reg_list_out.at(i).info_registrar_data.handle).info_registrar_data.id);
        BOOST_CHECK(reg_list_out.at(i).info_registrar_data.street1.get_value() == map_at(registrar_info, reg_list_out.at(i).info_registrar_data.handle).info_registrar_data.street1.get_value());
        BOOST_CHECK(reg_list_out.at(i).info_registrar_data.city.get_value() == map_at(registrar_info, reg_list_out.at(i).info_registrar_data.handle).info_registrar_data.city.get_value());
        BOOST_CHECK(reg_list_out.at(i).info_registrar_data.postalcode.get_value() == map_at(registrar_info, reg_list_out.at(i).info_registrar_data.handle).info_registrar_data.postalcode.get_value());
        BOOST_CHECK(reg_list_out.at(i).info_registrar_data.country.get_value() == map_at(registrar_info, reg_list_out.at(i).info_registrar_data.handle).info_registrar_data.country.get_value());
        BOOST_CHECK(!reg_list_out.at(i).info_registrar_data.system.get_value());//neither of them to be system
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_registrars
/*
 * SKIPPED AS NO PROPER INTERFACE PROVIDED
struct get_registrar_groups_fixture
{
    //unsigned long long
    std::map<unsigned long long, std::string> groups_info;
    //vector for groups
    get_registrar_groups_fixture()
    {
        for(int i=0; i < 10; ++i)
        {
            std::ostringstream test_groups;
            test_groups << "testgroup" << i;
//            groups_info[Fred::Registrar::ManagerImpl::createRegistrarGroup(test_groups.str())] = test_groups.str();
            groups_info[Fred::Registrar::Manager::create(0)->createRegistrarGroup(test_groups.str())] = test_groups.str();
        }
    }

};

BOOST_AUTO_TEST_SUITE(get_registrar_groups);

BOOST_AUTO_TEST_SUITE_END();//get_registrar_groups

struct get_registrar_certification_list_fixture
{

};

BOOST_AUTO_TEST_SUITE(get_registrar_certification_list);

BOOST_AUTO_TEST_SUITE_END();//get_registrar_certification_list

struct get_managed_zone_list_fixture
{

};

BOOST_AUTO_TEST_SUITE(get_managed_zone_list);

BOOST_AUTO_TEST_SUITE_END();//get_managed_zone_list
*/

BOOST_AUTO_TEST_SUITE(get_contact_by_handle);

struct get_contact_by_handle_fixture
        : whois_impl_instance_fixture,
          test_registrar_fixture
{
    std::string test_contact_handle;
    Fred::Contact::PlaceAddress contact_place;

    get_contact_by_handle_fixture()
    : test_registrar_fixture(),
      test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        contact_place.city = "Praha";
        contact_place.country = "CZ";
        contact_place.postalcode = "11150";
        contact_place.street1 = "STR1";
        Fred::CreateContact(test_contact_handle, test_registrar_handle)
            .set_place(contact_place).exec(ctx);

        ctx.commit_transaction();//commit fixture
        BOOST_MESSAGE(test_contact_handle);
    }
};


BOOST_FIXTURE_TEST_CASE(get_contact_by_handle, get_contact_by_handle_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoContactData icd = Fred::InfoContactByHandle(test_contact_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_contact_data;
    Registry::WhoisImpl::Contact con = impl.get_contact_by_handle(test_registrar_handle);
    BOOST_CHECK(con.address.city  == icd.place.get_value_or_default().city);
    BOOST_CHECK(con.address.country_code == icd.place.get_value_or_default().country);
    BOOST_CHECK(con.address.postal_code == icd.place.get_value_or_default().postalcode);
    BOOST_CHECK(con.address.street1 == icd.place.get_value_or_default().street1);
}

struct get_no_contact_by_handle_fixture
        : whois_impl_instance_fixture
{};

BOOST_FIXTURE_TEST_CASE(get_contact_by_handle_no_contact, get_no_contact_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::Contact con = impl.get_contact_by_handle("abc");
        BOOST_ERROR("unreported dangling contact");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_contact_by_handle_wrong_contact, get_no_contact_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::Contact con = impl.get_contact_by_handle("");//[a-zA-Z0-9_:.-]{1,63}
        BOOST_ERROR("contact handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_contact_by_handle

BOOST_AUTO_TEST_SUITE(get_nsset_by_handle)

struct get_nsset_by_handle_fixture
        : whois_impl_instance_fixture,
          test_registrar_fixture
{
    std::string test_nsset_handle;

    get_nsset_by_handle_fixture()
    : test_registrar_fixture(),
      test_nsset_handle(std::string("TEST-NSSET-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        std::vector<Fred::DnsHost> v_dns;
        v_dns.push_back(Fred::DnsHost(std::string("TEST-FQDN")+xmark,
                        boost::asio::ip::address()));
        std::vector<std::string> tech_contacts;
        tech_contacts.push_back("TEST-TECH-CONTACT");

        Fred::CreateNsset(test_nsset_handle, test_registrar_handle,
                          Optional<std::string>(), Optional<short>(), v_dns,
                          tech_contacts, Optional<unsigned long long>())
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
        BOOST_MESSAGE(test_nsset_handle);
    }
};

    BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle, get_nsset_by_handle_fixture)
    {
        Fred::OperationContext ctx;
        Fred::InfoNssetData ind = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_nsset_data;
        Registry::WhoisImpl::NSSet nss = impl.get_nsset_by_handle(test_nsset_handle);
        /*
handle
registrar
authinfo
tech_check_level
dns_hosts
tech_contacts
logd_request_id
         */
        //new nsset has to be unchanged and not being transferred
        BOOST_CHECK(nss.changed.isnull());
        BOOST_CHECK(nss.last_transfer.isnull());
        BOOST_CHECK(nss.created == ind.creation_time);//as that or greater than __
        BOOST_CHECK(nss.handle == ind.handle);
        BOOST_CHECK(nss.nservers.at(0).fqdn == ind.dns_hosts.at(0).get_fqdn());
        BOOST_CHECK(nss.nservers.at(0).ip_addresses.at(0) == ind.dns_hosts.at(0).get_inet_addr().at(0)); //comparing two boost::address'es
        BOOST_CHECK(nss.registrar_handle == ind.create_registrar_handle);
        BOOST_CHECK(nss.tech_contact_handles.at(0) == ind.tech_contacts.at(0).handle);
    }

struct get_no_nsset_by_handle_fixture
        : whois_impl_instance_fixture
{};

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle_no_nsset, get_no_nsset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSet nss = impl.get_nsset_by_handle("abc");
        BOOST_ERROR("unreported dangling nsset");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle_wrong_nsset, get_no_nsset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSet nss = impl.get_nsset_by_handle("");//[a-zA-Z0-9_:.-]{1,63}
        BOOST_ERROR("nsset handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}


BOOST_AUTO_TEST_SUITE_END()//get_nsset_by_handle

BOOST_AUTO_TEST_SUITE_END();//TestWhois
