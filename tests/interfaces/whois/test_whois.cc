#include "src/whois/whois.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/registrar/info_registrar_output.h"
#include "src/fredlib/registrar/info_registrar_data.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/create_registrar.h"
#include "random_data_generator.h"

#define BOOST_TEST_NO_MAIN

#include <sstream>
#include <map>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/foreach.hpp>


struct whois_impl_instance_fixture
{
    Registry::WhoisImpl::Server_impl impl;
};

BOOST_AUTO_TEST_SUITE(TestWhois)

struct test_registrar_fixture
        : whois_impl_instance_fixture
{
    std::string xmark;
    std::string test_registrar_handle;

    test_registrar_fixture()
    : xmark(RandomDataGenerator().xnumstring(6)),
      test_registrar_handle(std::string("TEST-REGISTRAR")+xmark)//from 3 to 16
    {
        Fred::OperationContext ctx;

        Fred::CreateRegistrar(test_registrar_handle)
            .set_name(std::string("TEST-REGISTRAR NAME")+xmark)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha")
            .set_postalcode("11150")
            .set_country("CZ")
            .exec(ctx);

        ctx.commit_transaction();
        BOOST_MESSAGE(test_registrar_handle);
    }
    ~test_registrar_fixture()
    {}
};

struct test_registrant_fixture
{
    std::string test_registrant_handle;

    test_registrant_fixture()
    : test_registrant_handle("registrant-fixture")
    {}
};

struct test_contact_fixture
{
    std::string test_admin;
    std::string test_contact;

    test_contact_fixture()
    : test_admin("TEST-ADMIN"),
      test_contact("TEST-CONTACT")
    {}
}

BOOST_AUTO_TEST_SUITE(get_registrar_by_handle)

struct get_registrar_fixture
        : test_registrar_fixture
{
    std::string no_registrar_handle;
    std::string wrong_registrar_handle;
    get_registrar_fixture()
    : no_registrar_handle("absent-registrar"),
      wrong_registrar_handle("")
    {}
};

BOOST_FIXTURE_TEST_CASE(get_fine_registrar, get_registrar_fixture)
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

BOOST_FIXTURE_TEST_CASE(get_no_registar, get_registrar_fixture)
{
    try
    {
        Registry::WhoisImpl::Registrar reg = impl.get_registrar_by_handle(no_registrar_handle);
        BOOST_ERROR("unreported dangling registrar");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_wrong_registrar, get_registrar_fixture)
{
    try
    {
        Registry::WhoisImpl::Registrar reg = impl.get_registrar_by_handle(wrong_registrar_handle);
        BOOST_ERROR("registrar handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_registrar_by_handle


BOOST_AUTO_TEST_SUITE(get_registrars)

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
            test_handles << test_registrar_handle << i;
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
    ~get_my_registrar_list_fixture() {}
};

BOOST_FIXTURE_TEST_CASE(get_nonsystem_registrars, get_my_registrar_list_fixture)
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

struct test_contact_fixture
        : test_registrar_fixture
{
    std::string test_contact_handle;
    std::string no_contact_handle;
    std::string wrong_contact_handle;
    Fred::Contact::PlaceAddress contact_place;

    test_contact_fixture()
    : test_registrar_fixture(),
      test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark),
      no_contact_handle("fine-handle"),
      wrong_contact_handle("")
    {
        Fred::OperationContext ctx;
        contact_place.city = "Praha";
        contact_place.country = "CZ";
        contact_place.postalcode = "11150";
        contact_place.street1 = "STR1";
        Fred::CreateContact(test_contact_handle, test_registrar_handle)
            .set_place(contact_place).exec(ctx);

        ctx.commit_transaction();
        BOOST_MESSAGE(test_contact_handle);
    }
};


BOOST_FIXTURE_TEST_CASE(get_contact_by_handle, test_contact_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoContactData icd = Fred::InfoContactByHandle(test_contact_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_contact_data;
    Registry::WhoisImpl::Contact con = impl.get_contact_by_handle(test_contact_handle);
    BOOST_CHECK(con.address.city  == icd.place.get_value_or_default().city);
    BOOST_CHECK(con.address.country_code == icd.place.get_value_or_default().country);
    BOOST_CHECK(con.address.postal_code == icd.place.get_value_or_default().postalcode);
    BOOST_CHECK(con.address.street1 == icd.place.get_value_or_default().street1);
}

BOOST_FIXTURE_TEST_CASE(get_contact_by_handle_no_contact, test_contact_fixture)
{
    try
    {
        Registry::WhoisImpl::Contact con = impl.get_contact_by_handle(no_contact_handle);
        BOOST_ERROR("unreported dangling contact");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_contact_by_handle_wrong_contact, test_contact_fixture)
{
    try
    {
        Registry::WhoisImpl::Contact con = impl.get_contact_by_handle(wrong_contact_handle);//[a-zA-Z0-9_:.-]{1,63}
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
        : test_registrar_fixture
{
    std::string test_nsset_handle;
    std::string no_nsset_handle;
    std::string wrong_nsset_handle;


    get_nsset_by_handle_fixture()
    : test_registrar_fixture(),
      test_nsset_handle(std::string("TEST-NSSET-HANDLE")+xmark),
      no_nsset_handle("fine-nsset-handle"),
      wrong_nsset_handle("")
    {
        Fred::OperationContext ctx;
        std::vector<std::string> tech_contacts;
        tech_contacts.push_back("TEST-TECH-CONTACT");

        Fred::CreateNsset(test_nsset_handle, test_registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>(
                    Fred::DnsHost(std::string("TEST-FQDN")+xmark,
                    boost::asio::ip::address())))
            .set_tech_contacts(Util::vector_of<std::string>("TEST-TECH-CONTACT"))
            .exec(ctx);

        ctx.commit_transaction();
        BOOST_MESSAGE(test_nsset_handle);
    }
};

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle, get_nsset_by_handle_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoNssetData ind = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_nsset_data;
    Registry::WhoisImpl::NSSet nss = impl.get_nsset_by_handle(test_nsset_handle);
    BOOST_CHECK(nss.changed.isnull());//new nsset has to be unchanged
    BOOST_CHECK(nss.last_transfer.isnull());//new nsset has to not transferred
    BOOST_CHECK(nss.created == ind.creation_time);//as that or greater than __
    BOOST_CHECK(nss.handle == ind.handle);
    BOOST_CHECK(nss.nservers.at(0).fqdn == ind.dns_hosts.at(0).get_fqdn());
    BOOST_CHECK(nss.nservers.at(0).ip_addresses.at(0) == ind.dns_hosts.at(0).get_inet_addr().at(0)); //comparing two boost::address'es
    BOOST_CHECK(nss.registrar_handle == ind.create_registrar_handle);
    BOOST_CHECK(nss.tech_contact_handles.at(0) == ind.tech_contacts.at(0).handle);
}

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle_no_nsset, get_nsset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSet nss = impl.get_nsset_by_handle(no_nsset_handle);
        BOOST_ERROR("unreported dangling nsset");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle_wrong_nsset, get_nsset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSet nss = impl.get_nsset_by_handle(wrong_nsset_handle);
        BOOST_ERROR("nsset handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}


BOOST_AUTO_TEST_SUITE_END()//get_nsset_by_handle


BOOST_AUTO_TEST_SUITE(get_nssets_by_ns)

struct get_nssets_by_ns_fixture
        : test_registrar_fixture
{
//        std::map<std::string,Fred::InfoNssetOutput> nsset_info;
    std::string test_fqdn;
    std::string test_no_fqdn;
    std::string test_wrong_fqdn;
    unsigned long test_limit;

    get_nssets_by_ns_fixture()
    : test_registrar_fixture(),
      test_fqdn(std::string("test") + xmark + ".cz"),
      test_no_fqdn("fine-fqdn.cz"),
      test_wrong_fqdn("."),
      test_limit(10)
    {
        Fred::OperationContext ctx;
        for(int i = 0; i < test_limit; ++i)
        {
            std::ostringstream test_handles;
            test_handles << "n" << i;
            std::vector<Fred::DnsHost> v_dns;
            v_dns.push_back(Fred::DnsHost(test_fqdn,
                            boost::asio::ip::address()));
            std::vector<std::string> tech_contacts;
            tech_contacts.push_back("TEST-TECH-CONTACT" + xmark);

            Fred::CreateNsset(test_handles.str(), test_registrar_handle + i,
                              Optional<std::string>(), Optional<short>(), v_dns,
                              tech_contacts, Optional<unsigned long long>())
                .exec(ctx);
            //any extra setting here?
        }
        ctx.commit_transaction();
    }

    ~get_nssets_by_ns_fixture() {}
};

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns, get_nssets_by_ns_fixture)
{
    Fred::OperationContext ctx;
    std::vector<Fred::InfoNssetOutput> v_ino = Fred::InfoNssetByDNSFqdn(test_fqdn).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
    Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_ns(test_fqdn, test_limit);
    for(int i = 0; i < test_limit; ++i)
    {
        std::vector<Fred::InfoNssetOutput>::iterator it = v_ino.begin(), end = v_ino.end();
        while(it != end)
        {
            if(it->info_nsset_data.handle == nss_s.content.at(i).handle) break;
            ++it;
        }
        BOOST_CHECK(nss_s.content.at(i).handle == it->info_nsset_data.handle);
        Fred::InfoNssetData found = it->info_nsset_data;
        BOOST_CHECK(nss_s.content.at(i).changed.isnull());
        BOOST_CHECK(nss_s.content.at(i).last_transfer.isnull());
        BOOST_CHECK(nss_s.content.at(i).created == found.creation_time);//as that or greater than __
        BOOST_CHECK(nss_s.content.at(i).handle == found.handle);
        BOOST_CHECK(nss_s.content.at(i).nservers.at(0).fqdn == found.dns_hosts.at(0).get_fqdn());
        BOOST_CHECK(nss_s.content.at(i).nservers.at(0).ip_addresses.at(0) == found.dns_hosts.at(0).get_inet_addr().at(0)); //comparing two boost::address'es
        BOOST_CHECK(nss_s.content.at(i).registrar_handle == found.create_registrar_handle);
        BOOST_CHECK(nss_s.content.at(i).tech_contact_handles.at(0) == found.tech_contacts.at(0).handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns_wrong_ns, get_nssets_by_ns_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_ns(test_wrong_fqdn, test_limit);
        BOOST_ERROR("domain handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns_no_ns, get_nssets_by_ns_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_ns(test_no_fqdn, test_limit);
        BOOST_ERROR("unreported dangling NSSets");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_nssets_by_ns


BOOST_AUTO_TEST_SUITE(get_nssets_by_tech_c)

struct get_nssets_by_tech_c_fixture
        : test_registrar_fixture
{
    std::string test_c_handle;
    std::string test_no_handle;//better name
    std::string test_wrong_handle;
    unsigned long test_limit;

    get_nssets_by_tech_c_fixture()
    : test_registrar_fixture(),
      test_c_handle(std::string("TEST-CONTACT-HANDLE") + xmark),
      test_no_handle("fine-tech-c-handle"),
      test_wrong_handle(""),
      test_limit(10)
    {
        Fred::OperationContext ctx;
        for(int i = 0; i < test_limit; ++i)
        {
            std::ostringstream test_handles;
            test_handles << "n" << i;
            std::vector<Fred::DnsHost> v_dns;
            v_dns.push_back(Fred::DnsHost(test_c_handle,
                            boost::asio::ip::address()));
            std::vector<std::string> tech_contacts;
            tech_contacts.push_back("TEST-TECH-CONTACT" + xmark);

            Fred::CreateNsset(test_handles.str(), test_registrar_handle + i,
                              Optional<std::string>(), Optional<short>(), v_dns,
                              tech_contacts, Optional<unsigned long long>())
                .exec(ctx);
            //any extra setting here?
        }
        ctx.commit_transaction();
    }

    ~get_nssets_by_tech_c_fixture() {}
};

BOOST_FIXTURE_TEST_CASE(get_nssets_by_tech_c, get_nssets_by_tech_c_fixture)
{
    Fred::OperationContext ctx;
    std::vector<Fred::InfoNssetOutput> v_ino = Fred::InfoNssetByDNSFqdn(test_c_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
    Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_tech_c(test_c_handle, test_limit);
    for(int i = 0; i < test_limit; ++i)
    {
        std::vector<Fred::InfoNssetOutput>::iterator it = v_ino.begin(), end = v_ino.end();
        while(it != end)
        {
            if(it->info_nsset_data.handle == nss_s.content.at(i).handle) break;
            ++it;
        }
        BOOST_CHECK(nss_s.content.at(i).handle == it->info_nsset_data.handle);
        Fred::InfoNssetData found = it->info_nsset_data;
        BOOST_CHECK(nss_s.content.at(i).changed.isnull());
        BOOST_CHECK(nss_s.content.at(i).last_transfer.isnull());
        BOOST_CHECK(nss_s.content.at(i).created == found.creation_time);//as that or greater than __
        BOOST_CHECK(nss_s.content.at(i).handle == found.handle);
        BOOST_CHECK(nss_s.content.at(i).nservers.at(0).fqdn == found.dns_hosts.at(0).get_fqdn());
        BOOST_CHECK(nss_s.content.at(i).nservers.at(0).ip_addresses.at(0) == found.dns_hosts.at(0).get_inet_addr().at(0)); //comparing two boost::address'es
        BOOST_CHECK(nss_s.content.at(i).registrar_handle == found.create_registrar_handle);
        BOOST_CHECK(nss_s.content.at(i).tech_contact_handles.at(0) == found.tech_contacts.at(0).handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_tech_c_wrong_ns, get_nssets_by_tech_c_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_tech_c(test_wrong_handle, test_limit);
        BOOST_ERROR("domain handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_tech_c_no_ns, get_nssets_by_tech_c_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_tech_c(test_no_handle, test_limit);
        BOOST_ERROR("unreported dangling NSSets");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_nssets_by_tech_c


BOOST_AUTO_TEST_SUITE(get_nameserver_by_fqdn)

struct get_nameserver_by_fqdn_fixture
        : test_registrar_fixture
{
    std::string test_nameserver_fqdn;
    std::string test_no_handle;
    std::string test_wrong_handle;

    get_nameserver_by_fqdn_fixture()
    : test_registrar_fixture(),
      test_nameserver_fqdn(std::string("test-nameserver") + xmark + ".cz"),
      test_no_handle("fine-fqdn.cz"),
      test_wrong_handle("")
    {
        Fred::OperationContext ctx;
        Fred::CreateNsset("TEST-NSSET-HANDLE", test_registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>(test_nameserver_fqdn, Util::vector_of<boost::asio::ip::address>(123)))//making nameserver
            .set_tech_contacts(Util::vector_of<std::string>("TEST-TECH-CONTACT"))
            .exec(ctx);
            //any extra setting here?
        ctx.commit_transaction();
    }

    ~get_nameserver_by_fqdn_fixture() {}
};

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn, get_nameserver_by_fqdn_fixture)
{
    Fred::OperationContext ctx;
    BOOST_REQUIRE(Whois::nameserver_exists(test_nameserver_fqdn, ctx));

    Registry::WhoisImpl::NameServer ns = impl.get_nameserver_by_fqdn(test_nameserver_fqdn);
    BOOST_CHECK(ns.fqdn == test_nameserver_fqdn);
}

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_wrong_ns, get_nameserver_by_fqdn_fixture)
{
    try
    {
        Registry::WhoisImpl::NameServer ns = impl.get_nameserver_by_fqdn(test_wrong_handle);
        BOOST_ERROR("domain handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_no_ns, get_nameserver_by_fqdn_fixture)
{
    try
    {
        Registry::WhoisImpl::NameServer ns = impl.get_nameserver_by_fqdn(test_no_handle);
        BOOST_ERROR("unreported dangling nameserver");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_nameserver_by_fqdn


BOOST_AUTO_TEST_SUITE(get_keyset_by_handle)

struct get_keyset_by_handle_fixture
        : test_registrar_fixture, test_contact_fixture
{
    std::string test_keyset_handle;
    std::string no_keyset_handle;
    std::string wrong_keyset_handle;

    get_keyset_by_handle_fixture()
    : test_registrar_fixture(),
      test_contact_fixture(),
      test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark),
      no_keyset_handle("fine-keyset-handle"),
      wrong_keyset_handle("")
    {
        Fred::OperationContext ctx;
        Fred::CreateKeyset(test_keyset_handle, test_registrar_handle)
            .set_dns_keys(Util::vector_of<Fred::DnsKey>(Fred::DnsKey(42, 777, 13, "any-key")))//what key has to be here?
            .set_tech_contacts(Util::vector_of<std::string>(test_admin)(test_contact))
            .exec(ctx);
        ctx.commit_transaction();
        BOOST_MESSAGE(test_keyset_handle);
    }
};

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle, get_keyset_by_handle_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoKeysetData ikd = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
    Registry::WhoisImpl::KeySet ks = impl.get_keyset_by_handle(test_keyset_handle);
    BOOST_CHECK(ks.changed.isnull());//new has to be unchanged
    BOOST_CHECK(ks.created == ikd.creation_time);
    BOOST_CHECK(ks.dns_keys.at(0).alg == ikd.dns_keys.at(0).get_alg());
    BOOST_CHECK(ks.dns_keys.at(0).flags == ikd.dns_keys.at(0).get_flags());
    BOOST_CHECK(ks.dns_keys.at(0).protocol == ikd.dns_keys.at(0).get_protocol());
    BOOST_CHECK(ks.dns_keys.at(0).public_key == ikd.dns_keys.at(0).get_key());
    BOOST_CHECK(ks.handle == ikd.handle);
    BOOST_CHECK(ks.last_transfer.isnull());//new has to have no transfer
    BOOST_CHECK(ks.registrar_handle == ikd.create_registrar_handle);
    BOOST_CHECK(ks.tech_contact_handles.at(0) == ikd.tech_contacts.at(0));
}

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle_no_keyset, get_keyset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::KeySet ks = impl.get_keyset_by_handle(no_keyset_handle);
        BOOST_ERROR("unreported dangling keyset");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle_wrong_nsset, get_keyset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::KeySet ks = impl.get_keyset_by_handle(wrong_keyset_handle);
        BOOST_ERROR("keyset handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_keyset_by_handle


BOOST_AUTO_TEST_SUITE(get_keysets_by_tech_c)

struct get_keysets_by_tech_c_fixture
        : test_registrar_fixture, test_contact_fixture
{
    std::string test_keyset_handle;
    std::string test_no_handle;
    std::string test_wrong_handle;
    unsigned long test_limit;

    get_keysets_by_tech_c_fixture()
    : test_registrar_fixture(),
      test_contact_fixture(),
      test_keyset_handle(std::string("TEST_KEYSET_HANDLE") + xmark),
      test_no_handle("fine-tech-c-handle"),
      test_wrong_handle(""),
      test_limit(10)
    {
        Fred::OperationContext ctx;
        for(int i = 0; i < test_limit; ++i)
        {
            std::ostringstream test_handles;
            test_handles << test_keyset_handle << i;

            Fred::CreateKeyset(test_handles.str(), test_registrar_handle + i)
                .set_dns_keys(Util::vector_of<Fred::DnsKey>(42, 777, 13, "any-key"))
                .set_tech_contacts(Util::vector_of<std::string>(test_admin)(test_contact))
                .exec(ctx);
        }
        ctx.commit_transaction();
    }

    ~get_keysets_by_tech_c_fixture() {}
};
    
BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c, get_keysets_by_tech_c_fixture)
{
    Fred::OperationContext ctx;
    std::vector<Fred::InfoKeysetOutput> v_iko = Fred::InfoKeysetByTechContactHandle(test_admin).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
    Registry::WhoisImpl::KeySetSeq ks_s = impl.get_keysets_by_tech_c(test_admin, test_limit);
    for(int i = 0; i < test_limit; ++i)
    {
        std::vector<Fred::InfoKeysetOutput>::iterator it = v_iko.begin(), end = v_iko.end();
        while(it != end)
        {
            if(it->info_keyset_data.handle == ks_s.content.at(0).handle) break;
            ++it;
        }
        BOOST_CHECK(ks_s.content.at(0).handle == it->info_keyset_data.handle);
        Fred::InfoKeysetData found = it->info_keyset_data;
        BOOST_CHECK(ks_s.content.at(0).changed.isnull());
        BOOST_CHECK(ks_s.content.at(0).created == found.creation_time);
        BOOST_CHECK(ks_s.content.at(0).dns_keys.at(0) == found.dns_keys.at(0));
        BOOST_CHECK(ks_s.content.at(0).last_transfer.isnull());
        BOOST_CHECK(ks_s.content.at(0).registrar_handle == found.create_registrar_handle);
        BOOST_CHECK(ks_s.content.at(0).tech_contact_handles.at(0) == found.tech_contacts.at(0));
    }
}
    
BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_wrong_contact, get_keysets_by_tech_c_fixture)
{
    try
    {
        Registry::WhoisImpl::KeySetSeq ks_s = impl.get_nssets_by_tech_c(test_wrong_handle, test_limit);
        BOOST_ERROR("tech contact handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_no_contact, get_keysets_by_tech_c_fixture)
{
    try
    {
        Registry::WhoisImpl::KeySetSeq ks_s = impl.get_keysets_by_tech_c(test_no_handle, test_limit);
        BOOST_ERROR("unreported dangling KeySets");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_keysets_by_tech_c


BOOST_AUTO_TEST_SUITE(get_domain_by_handle)

struct test_domain_fixture
        : test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::string test_fqdn;
    std::string no_fqdn;
    std::string wrong_fqdn;
    std::string invalid_fqdn;

    test_domain_fixture()
    : test_registrar_fixture(),
      test_registrant_fixture(),
      test_contact_fixture(),
      test_fqdn(std::string("test") + xmark + ".cz"),
      no_fqdn("fine-handle.cz"),
      wrong_fqdn(""),
      invalid_fqdn("a-.cz")
    {
        Fred::OperationContext ctx;
        Fred::CreateDomain(test_fqdn, test_registrar_handle, test_registrant_handle)
            .set_admin_contacts(Util::vector_of<std::string>(test_admin))
            .exec(ctx);
        ctx.commit_transaction();
        BOOST_MESSAGE(test_fqdn);
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle, test_domain_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoDomainData idd = Fred::InfoDomainByHandle(test_fqdn).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_domain_data;
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(test_fqdn);
    BOOST_CHECK(dom.admin_contact_handles.at(0) == idd.admin_contacts.at(0));
    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.fqdn == idd.fqdn);
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.registered == idd.creation_time);
    BOOST_CHECK(dom.registrant_handle == idd.registrant);
    BOOST_CHECK(dom.registrar_handle == idd.create_registrar_handle);
    //Jiri: others?
}

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_wrong_handle, test_domain_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(wrong_fqdn);
        BOOST_ERROR("domain handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidLabel& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct wrong_zone_fixture
        : test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::string test_fqdn_bad_zone;

    wrong_zone_fixture()
    : test_registrar_fixture(),
      test_contact_fixture(),
      test_fqdn_bad_zone("a.")
    {
        std::ostringstream bad_zone;
        std::vector<std::string> list = impl.get_managed_zone_list();
        bad_zone << *list.rbegin();
        while(list.end() != std::find(list.begin(), list.end(), bad_zone.str()))
        {
            bad_zone << "a";
        }
        test_fqdn_bad_zone += bad_zone.str();
        BOOST_MESSAGE(test_fqdn_bad_zone);
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_wrong_zone, wrong_zone_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(test_fqdn_bad_zone);
        BOOST_ERROR("unreported managed zone");
    }
    catch(const Registry::WhoisImpl::UnmanagedZone& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct many_labels_fixture
        : test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::vector<std::string> domain_list;
    std::string prepare_zone(Fred::OperationContext& ctx, const std::string& zone)
    {
        Fred::Zone::Data zone_data;
        try
        {
            zone_data = Fred::Zone::find_zone_in_fqdn(ctx, zone);
        }
        catch(const Fred::Zone::Exception& ex)
        {
            BOOST_ERROR("test zone was not created properly");
        }
        std::ostringstream labeled_zone;
        for(int i=0; i < zone_data.dots_max + 1; ++i)
        {
            labeled_zone << "1.";
        }
        labeled_zone << zone_data.name;
        return labeled_zone.str();
    }
    many_labels_fixture()
    {
        Fred::OperationContext ctx;
        std::vector<std::string> zone_seq = ::Whois::get_managed_zone_list(ctx);
        for(std::vector<std::string>::iterator it = zone_seq.begin(); it != zone_seq.end(); ++it)
        {
            domain_list.push_back(prepare_zone(ctx, *it));
        }
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_many_labels, many_labels_fixture)
{
    for(std::vector<std::string>::iterator it = domain_list.begin(); it != domain_list.end(); ++it)
    {
        try
        {
            Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(*it);
            BOOST_ERROR("permitted label number is wrong");
        }
        catch(const Registry::WhoisImpl::TooManyLabels& ex)
        {
            BOOST_CHECK(true);
            BOOST_MESSAGE(boost::diagnostic_information(ex));
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_no_handle, test_domain_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(no_fqdn);
        BOOST_ERROR("unreported dangling domain");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_invalid_handle, test_domain_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(invalid_fqdn);
        BOOST_ERROR("domain checker rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidLabel& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct invalid_unmanaged_fixture
        : wrong_zone_fixture
{
    std::string invalid_unmanaged_fqdn;
    invalid_unmanaged_fixture()
    : wrong_zone_fixture()//do not create bad_zone domain for it
    {
        std::ostringstream prefix;
        for(int i=0; i < 256 - test_fqdn_bad_zone.size(); ++i)//exceed the size of valid label
        {
            prefix << "1";//invalid part
        }
        prefix << test_fqdn_bad_zone;//unmanaged part
        invalid_unmanaged_fqdn = prefix.str();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_invalid_unmanaged, invalid_unmanaged_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(invalid_unmanaged_fqdn);
        BOOST_ERROR("domain must have invalid label and unmanaged zone");
    }
    catch(const Registry::WhoisImpl::InvalidLabel& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
    catch(const Registry::WhoisImpl::UnmanagedZone& ex)
    {
        BOOST_ERROR("domain must check name validity first");
    }
}

struct unmanaged_toomany_fixture
        : wrong_zone_fixture
{
    std::vector<std::string> domain_list;

    std::string prepare_zone(Fred::OperationContext& ctx, const std::string& zone)
    {
        Fred::Zone::Data zone_data;
        try
        {
            zone_data = Fred::Zone::find_zone_in_fqdn(ctx, zone);
        }
        catch(const Fred::Zone::Exception& ex)
        {
            BOOST_ERROR("test zone was not created properly");
        }
        std::ostringstream labeled_zone;
        for(int i=0; i < zone_data.dots_max + 1; ++i)
        {
            labeled_zone << "1.";//toomany part
        }
        labeled_zone << test_fqdn_bad_zone; //unmanaged zone part
        return labeled_zone.str();
    }
    unmanaged_toomany_fixture()
    : wrong_zone_fixture()
    {
        Fred::OperationContext ctx;
        std::vector<std::string> zone_seq = ::Whois::get_managed_zone_list(ctx);
        for(std::vector<std::string>::iterator it = zone_seq.begin(); it != zone_seq.end(); ++it)
        {
            domain_list.push_back(prepare_zone(ctx, *it));
        }
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_unmanaged_toomany, unmanaged_toomany_fixture)
{
    for(std::vector<std::string>::iterator it = domain_list.begin(); it != domain_list.end(); ++it)
    {
        try
        {
            Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(*it);
            BOOST_ERROR("domain must have unmanaged zone and exceeded number of labels");
        }
        catch(const Registry::WhoisImpl::UnmanagedZone& ex)
        {
            BOOST_CHECK(true);
            BOOST_MESSAGE(boost::diagnostic_information(ex));
        }
        catch(const Registry::WhoisImpl::TooManyLabels& ex)
        {
            BOOST_ERROR("domain must check managed zone first");
        }
    }
}

struct invalid_toomany_fixture
: whois_impl_instance_fixture
{
    std::vector<std::string> domain_list;

    std::string prepare_zone(Fred::OperationContext& ctx, const std::string& zone)
    {
        Fred::Zone::Data zone_data;
        try
        {
            zone_data = Fred::Zone::find_zone_in_fqdn(ctx, zone);
        }
        catch(const Fred::Zone::Exception& ex)
        {
            BOOST_ERROR("test zone was not created properly");
        }
        std::ostringstream labeled_zone, invalid_offset;
        for(int i=0; i < zone_data.dots_max + 1; ++i)
        {
            labeled_zone << "1.";//toomany part
        }
        for(int i=0; i < 256 - labeled_zone.tellp(); ++i)
        {
            invalid_offset << '1';//invalid part
        }
        labeled_zone << zone_data.name;
        invalid_offset << labeled_zone.str() << zone_data.name;
        return invalid_offset.str();
    }
    invalid_toomany_fixture()
    : whois_impl_instance_fixture()
    {
        Fred::OperationContext ctx;
        std::vector<std::string> zone_seq = ::Whois::get_managed_zone_list(ctx);
        domain_list.reserve(zone_seq.size());
        for(std::vector<std::string>::iterator it = zone_seq.begin(); it != zone_seq.end(); ++it)
        {
            domain_list.push_back(prepare_zone(ctx, *it));
        }
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_invalid_toomany, invalid_toomany_fixture)
{
    for(std::vector<std::string>::iterator it = domain_list.begin(); it != domain_list.end(); ++it)
    {
        try
        {
            Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(*it);
            BOOST_ERROR("domain must have invalid handle and exceeded number of labels");
        }
        catch(const Registry::WhoisImpl::InvalidLabel& ex)
        {
            BOOST_CHECK(true);
            BOOST_MESSAGE(boost::diagnostic_information(ex));
        }
        catch(const Registry::WhoisImpl::TooManyLabels& ex)
        {
            BOOST_ERROR("domain must check name validity first");
        }
    }
}

struct invalid_unmanaged_toomany_fixture
: wrong_zone_fixture
{
    std::vector<std::string> domain_list;

    std::string prepare_zone(Fred::OperationContext& ctx, const std::string& zone)
    {
        Fred::Zone::Data zone_data;
        try
        {
            zone_data = Fred::Zone::find_zone_in_fqdn(ctx, zone);
        }
        catch(const Fred::Zone::Exception& ex)
        {
            BOOST_ERROR("test zone was not created properly");
        }
        std::ostringstream labeled_zone, invalid_offset;
        for(int i=0; i < zone_data.dots_max + 1; ++i)
        {
            labeled_zone << "1.";//toomany part
        }
        for(int i=0; i < 256 - labeled_zone.tellp(); ++i)
        {
            invalid_offset << '1';//invalid part
        }
        labeled_zone << test_fqdn_bad_zone; //unmanaged zone part
        invalid_offset << labeled_zone.str();
        return invalid_offset.str();
    }
    invalid_unmanaged_toomany_fixture()
    : wrong_zone_fixture()
    {
        Fred::OperationContext ctx;
        std::vector<std::string> zone_seq = ::Whois::get_managed_zone_list(ctx);
        domain_list.reserve(zone_seq.size());
        for(std::vector<std::string>::iterator it = zone_seq.begin(); it != zone_seq.end(); ++it)
        {
            domain_list.push_back(prepare_zone(ctx, *it));
        }
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_invalid_unmanaged_toomany, invalid_unmanaged_toomany_fixture)
{
    for(std::vector<std::string>::iterator it = domain_list.begin(); it != domain_list.end(); ++it)
    {
        try
        {
            Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(*it);
            BOOST_ERROR("domain must have invalid handle, unmanaged zone and exceeded number of labels");
        }
        catch(const Registry::WhoisImpl::InvalidLabel& ex)
        {
            BOOST_CHECK(true);
            BOOST_MESSAGE(boost::diagnostic_information(ex));
        }
        catch(const Registry::WhoisImpl::UnmanagedZone& ex)
        {
            BOOST_ERROR("domain must check name validity first");
        }
        catch(const Registry::WhoisImpl::TooManyLabels& ex)
        {
            BOOST_ERROR("domain must check name validity first, then managed zone");
        }
    }
}

struct delete_candidate_fixture //Jiri: check carefully
: test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::string delete_fqdn;
    std::string delete_status;

    delete_candidate_fixture()
    : delete_status("deleteCandidate")
    {
        Fred::OperationContext ctx;
        Fred::CreateDomain(delete_fqdn, test_registrar_handle, test_registrant_handle)
            .set_admin_contacts(Util::vector_of<std::string>(test_admin))
            .exec(ctx);
        ctx.commit_transaction();
        ctx.get_conn().exec_params(
            "UPDATE domain_history "
            "SET exdate = now() - "
                "(SELECT val::int * '1 day'::interval "
                    "FROM enum_parameters"
                    "WHERE name = 'expiration_registration_protection_period')"
            "WHERE id = (SELECT id FROM object_registry WHERE name = $1::text)"

            "UPDATE domain"
            "SET exdate = now() -"
                "(SELECT val::int * '1 day'::interval"
                    "FROM enum_parameters"
                    "WHERE name = 'expiration_registration_protection_period')"
            "WHERE id = (SELECT id FROM object_registry WHERE name = $1::text)",
            Database::query_param_list(delete_fqdn));
        Fred::InfoDomainOutput dom = Fred::InfoDomainByHandle(delete_fqdn).exec(ctx, impl.output_timezone);
        Fred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);
        BOOST_MESSAGE(delete_fqdn);
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_delete_candidate, delete_candidate_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoDomainData idd = Fred::InfoDomainByHandle(delete_fqdn).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_domain_data;
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(delete_fqdn);
    BOOST_CHECK(dom.fqdn == idd.fqdn);
    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.statuses.end() != std::find(dom.statuses.begin(), dom.statuses.end(), delete_status));
}

BOOST_AUTO_TEST_SUITE_END()//get_domain_by_handle


BOOST_AUTO_TEST_SUITE(get_domains_by_registrant)

struct domains_by_registrant_fixture
        : test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::string test_fqdn;
    std::string wrong_handle;
    std::string no_handle;
    int regular_domains;

    domains_by_registrant_fixture()
    : test_registrar_fixture(),
      test_registrant_fixture(),
      test_contact_fixture(),
      test_fqdn(std::string("test") + xmark),
      wrong_handle(""),
      no_handle("absent-registrant"),
      regular_domains(6)
    {
        Fred::OperationContext ctx;
        for(int i=0; i < regular_domains - 1; ++i)
        {
            Fred::CreateDomain(test_fqdn + i + ".cz", test_registrar_handle, test_registrant_handle)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        }
        for(int i=0; i < 3; ++i)//3 different domains for another registrant
        {
            Fred::CreateDomain(test_fqdn + i + ".cz", test_registrar_handle, test_registrant_handle + "another")
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);

        }
        Fred::CreateDomain(test_fqdn + ".cz", test_registrar_handle, test_registrant_handle)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        ctx.commit_transaction();
        BOOST_MESSAGE(test_fqdn);
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_registrant, domains_by_registrant_fixture)
{
    Fred::OperationContext ctx;
    const std::vector<Fred::InfoDomainOutput> domain_info =
        Fred::InfoDomainByRegistrantHandle(test_registrant_handle)
            .set_limit(regular_domains + 1)
            .exec(ctx, impl.output_timezone);
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_registrant(test_registrant_handle, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);
    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains);
    std::vector<Fred::InfoDomainOutput>::const_iterator found = domain_info.begin(), end = domain_info.end();
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        while(found != end)
        {
            if(it->fqdn == found->info_domain_data.fqdn) break;
            ++found;
        }
        BOOST_REQUIRE(it->fqdn == found->info_domain_data.fqdn);
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->info_domain_data.admin_contacts.at(0));
        BOOST_CHECK(it->changed.isnull());
        BOOST_CHECK(it->last_transfer.isnull());
        BOOST_CHECK(it->registered == found->info_domain_data.creation_time);
        BOOST_CHECK(it->registrant_handle == found->info_domain_data.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->info_domain_data.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_registrant_limit_exceeded, domains_by_registrant_fixture)
{
    Fred::OperationContext ctx;
    const std::vector<Fred::InfoDomainOutput> domain_info =
        Fred::InfoDomainByRegistrantHandle(test_registrant_handle)
            .set_limit(regular_domains + 1)
            .exec(ctx, impl.output_timezone);
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_registrant(test_registrant_handle, regular_domains - 1);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() != regular_domains);
    std::vector<Fred::InfoDomainOutput>::const_iterator found = domain_info.begin(), end = domain_info.end();
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        while(found != end)
        {
            if(it->fqdn == found->info_domain_data.fqdn) break;
            ++found;
        }
        BOOST_REQUIRE(it->fqdn == found->info_domain_data.fqdn);
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->info_domain_data.admin_contacts.at(0));
        BOOST_CHECK(it->changed.isnull());
        BOOST_CHECK(it->last_transfer.isnull());
        BOOST_CHECK(it->registered == found->info_domain_data.creation_time);
        BOOST_CHECK(it->registrant_handle == found->info_domain_data.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->info_domain_data.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_registrant_wrong_registrant, domains_by_registrant_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_registrant(wrong_handle, 0);
        BOOST_ERROR("registrant handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_registrant_no_registrant, domains_by_registrant_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_registrant(no_handle, 0);
        BOOST_ERROR("unreported dangling registrant");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_domains_by_registrant


BOOST_AUTO_TEST_SUITE(get_domains_by_admin_contact)

struct domains_by_admin_contact_fixture
: test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::string test_fqdn;
    std::string wrong_contact;
    std::string no_contact;
    int regular_domains;

    domains_by_admin_contact_fixture()
    : test_contact_fixture(),
      test_fqdn(std::string("test") + xmark),
      wrong_contact(""),
      no_contact("absent-contact"),
      regular_domains(6)
    {
        Fred::OperationContext ctx;
        for(int i=0; i < regular_domains - 1; ++i)
        {
            Fred::CreateDomain(test_fqdn + i + ".cz", test_registrar_handle, test_registrant_handle)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        }
        for(int i=0; i < 3; ++i)//3 different domains for another contact
        {
            Fred::CreateDomain(test_fqdn + i + ".cz", test_registrar_handle, test_registrant_handle)
                .set_admin_contacts(Util::vector_of<std::string>("different admin"))
                .exec(ctx);

        }
        Fred::CreateDomain(test_fqdn + ".cz", test_registrar_handle, test_registrant_handle)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        ctx.commit_transaction();
        BOOST_MESSAGE(test_fqdn);
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact, domains_by_admin_contact_fixture)
{
    Fred::OperationContext ctx;
    const std::vector<Fred::InfoDomainOutput> domain_info =
        Fred::InfoDomainByAdminContactHandle(test_admin)
            .set_limit(regular_domains + 1)
            .exec(ctx, impl.output_timezone);
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_admin_contact(test_admin, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains);
    std::vector<Fred::InfoDomainOutput>::const_iterator found = domain_info.begin(), end = domain_info.end();
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        while(found != end)
        {
            if(it->fqdn == found->info_domain_data.fqdn) break;
            ++found;
        }
        BOOST_REQUIRE(it->fqdn == found->info_domain_data.fqdn);
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->info_domain_data.admin_contacts.at(0));
        BOOST_CHECK(it->changed.isnull());
        BOOST_CHECK(it->last_transfer.isnull());
        BOOST_CHECK(it->registered == found->info_domain_data.creation_time);
        BOOST_CHECK(it->registrant_handle == found->info_domain_data.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->info_domain_data.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact_limit_exceeded, domains_by_admin_contact_fixture)
{
    Fred::OperationContext ctx;
    const std::vector<Fred::InfoDomainOutput> domain_info =
        Fred::InfoDomainByAdminContactHandle(test_admin)
            .set_limit(regular_domains + 1)
            .exec(ctx, impl.output_timezone);
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_admin_contact(test_admin, regular_domains);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() != regular_domains);
    std::vector<Fred::InfoDomainOutput>::const_iterator found = domain_info.begin(), end = domain_info.end();
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        while(found != end)
        {
            if(it->fqdn == found->info_domain_data.fqdn) break;
            ++found;
        }
        BOOST_REQUIRE(it->fqdn == found->info_domain_data.fqdn);
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->info_domain_data.admin_contacts.at(0));
        BOOST_CHECK(it->changed.isnull());
        BOOST_CHECK(it->last_transfer.isnull());
        BOOST_CHECK(it->registered == found->info_domain_data.creation_time);
        BOOST_CHECK(it->registrant_handle == found->info_domain_data.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->info_domain_data.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact_wrong_contact, domains_by_admin_contact_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_admin_contact(wrong_contact, 0);
        BOOST_ERROR("registrant handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact_no_contact, domains_by_admin_contact_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_admin_contact(no_contact, 0);
        BOOST_ERROR("unreported dangling registrant");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_domains_by_admin_contact


BOOST_AUTO_TEST_SUITE(get_domains_by_nsset)

struct domains_by_nsset_fixture
: test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::string test_fqdn;
    std::string test_nsset;
    std::string wrong_nsset;
    std::string no_nsset;
    int regular_domains;

    domains_by_nsset_fixture()
    : test_contact_fixture(),
      test_fqdn(std::string("test") + xmark),
      test_nsset("test-nsset" + xmark),
      wrong_nsset(""),
      no_nsset("absent-nsset"),
      regular_domains(6)
    {
        Fred::OperationContext ctx;
        for(int i=0; i < regular_domains - 1; ++i)
        {
            Fred::CreateDomain(test_fqdn + i + ".cz", test_registrar_handle, test_registrant_handle)
                .set_nsset(test_nsset)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        }
        for(int i=0; i < 3; ++i)//3 different domains for another nsset
        {
            Fred::CreateDomain(test_fqdn + i + ".cz", test_registrar_handle, test_registrant_handle)
                .set_nsset(std::string("different-nsset"))
                .set_admin_contacts(Util::vector_of<std::string>("different admin"))
                .exec(ctx);

        }
        //1 with no nsset
        Fred::CreateDomain(test_fqdn + ".cz", test_registrar_handle, test_registrant_handle)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        Fred::CreateDomain(test_fqdn + ".cz", test_registrar_handle, test_registrant_handle)
                .set_nsset(test_nsset)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        ctx.commit_transaction();
        BOOST_MESSAGE(test_fqdn);
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset, domains_by_nsset_fixture)
{
    Fred::OperationContext ctx;
    const std::vector<Fred::InfoDomainOutput> domain_info =
        Fred::InfoDomainByNssetHandle(test_nsset)
            .set_limit(regular_domains + 1)
            .exec(ctx, impl.output_timezone);
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_nsset(test_nsset, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains);
    std::vector<Fred::InfoDomainOutput>::const_iterator found = domain_info.begin(), end = domain_info.end();
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        while(found != end)
        {
            if(it->fqdn == found->info_domain_data.fqdn) break;
            ++found;
        }
        BOOST_REQUIRE(it->fqdn == found->info_domain_data.fqdn);
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->info_domain_data.admin_contacts.at(0));
        BOOST_CHECK(it->changed.isnull());
        BOOST_CHECK(it->last_transfer.isnull());
        BOOST_CHECK(it->registered == found->info_domain_data.creation_time);
        BOOST_CHECK(it->registrant_handle == found->info_domain_data.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->info_domain_data.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset_limit_exceeded, domains_by_nsset_fixture)
{
    Fred::OperationContext ctx;
    const std::vector<Fred::InfoDomainOutput> domain_info =
        Fred::InfoDomainByNssetHandle(test_nsset)
            .set_limit(regular_domains + 1)
            .exec(ctx, impl.output_timezone);
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_nsset(test_nsset, regular_domains);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() != regular_domains);
    std::vector<Fred::InfoDomainOutput>::const_iterator found = domain_info.begin(), end = domain_info.end();
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        while(found != end)
        {
            if(it->fqdn == found->info_domain_data.fqdn) break;
            ++found;
        }
        BOOST_REQUIRE(it->fqdn == found->info_domain_data.fqdn);
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->info_domain_data.admin_contacts.at(0));
        BOOST_CHECK(it->changed.isnull());
        BOOST_CHECK(it->last_transfer.isnull());
        BOOST_CHECK(it->registered == found->info_domain_data.creation_time);
        BOOST_CHECK(it->registrant_handle == found->info_domain_data.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->info_domain_data.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset_wrong_nsset, domains_by_nsset_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_nsset(wrong_nsset, 0);
        BOOST_ERROR("nsset handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset_no_nsset, domains_by_nsset_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_nsset(no_nsset, 0);
        BOOST_ERROR("unreported dangling nsset");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_domains_by_nsset


BOOST_AUTO_TEST_SUITE(get_domains_by_keyset)

struct domains_by_keyset_fixture
: test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::string test_fqdn;
    std::string test_keyset;
    std::string wrong_keyset;
    std::string no_keyset;
    int regular_domains;

    domains_by_keyset_fixture()
    : test_contact_fixture(),
      test_fqdn(std::string("test") + xmark),
      test_keyset("test-nsset" + xmark),
      wrong_keyset(""),
      no_keyset("absent-nsset"),
      regular_domains(6)
    {
        Fred::OperationContext ctx;
        for(int i=0; i < regular_domains - 1; ++i)
        {
            Fred::CreateDomain(test_fqdn + i + ".cz", test_registrar_handle, test_registrant_handle)
                .set_keyset(test_keyset)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        }
        for(int i=0; i < 3; ++i)//3 different domains for another keyset
        {
            Fred::CreateDomain(test_fqdn + i + ".cz", test_registrar_handle, test_registrant_handle)
                .set_keyset(std::string("different-keyset"))
                .set_admin_contacts(Util::vector_of<std::string>("different admin"))
                .exec(ctx);

        }
        //1 with no keyset
        Fred::CreateDomain(test_fqdn + ".cz", test_registrar_handle, test_registrant_handle)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        Fred::CreateDomain(test_fqdn + ".cz", test_registrar_handle, test_registrant_handle)
                .set_keyset(test_keyset)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        ctx.commit_transaction();
        BOOST_MESSAGE(test_fqdn);
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset, domains_by_keyset_fixture)
{
    Fred::OperationContext ctx;
    const std::vector<Fred::InfoDomainOutput> domain_info =
        Fred::InfoDomainByKeysetHandle(test_keyset)
            .set_limit(regular_domains + 1)
            .exec(ctx, impl.output_timezone);
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_keyset(test_keyset, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains);
    std::vector<Fred::InfoDomainOutput>::const_iterator found = domain_info.begin(), end = domain_info.end();
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        while(found != end)
        {
            if(it->fqdn == found->info_domain_data.fqdn) break;
            ++found;
        }
        BOOST_REQUIRE(it->fqdn == found->info_domain_data.fqdn);
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->info_domain_data.admin_contacts.at(0));
        BOOST_CHECK(it->changed.isnull());
        BOOST_CHECK(it->last_transfer.isnull());
        BOOST_CHECK(it->registered == found->info_domain_data.creation_time);
        BOOST_CHECK(it->registrant_handle == found->info_domain_data.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->info_domain_data.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_limit_exceeded, domains_by_keyset_fixture)
{
    Fred::OperationContext ctx;
    const std::vector<Fred::InfoDomainOutput> domain_info =
        Fred::InfoDomainByKeysetHandle(test_keyset)
            .set_limit(regular_domains + 1)
            .exec(ctx, impl.output_timezone);
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_keyset(test_keyset, regular_domains);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() != regular_domains);
    std::vector<Fred::InfoDomainOutput>::const_iterator found = domain_info.begin(), end = domain_info.end();
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        while(found != end)
        {
            if(it->fqdn == found->info_domain_data.fqdn) break;
            ++found;
        }
        BOOST_REQUIRE(it->fqdn == found->info_domain_data.fqdn);
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->info_domain_data.admin_contacts.at(0));
        BOOST_CHECK(it->changed.isnull());
        BOOST_CHECK(it->last_transfer.isnull());
        BOOST_CHECK(it->registered == found->info_domain_data.creation_time);
        BOOST_CHECK(it->registrant_handle == found->info_domain_data.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->info_domain_data.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_wrong_keyset, domains_by_keyset_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_keyset(wrong_keyset, 0);
        BOOST_ERROR("nsset handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_no_keyset, domains_by_keyset_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_keyset(no_keyset, 0);
        BOOST_ERROR("unreported dangling nsset");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_domains_by_nsset


BOOST_AUTO_TEST_SUITE(get_domain_status_descriptions)

struct domain_status_descriptions_fixture
: whois_impl_instance_fixture
{
    std::string test_lang;
    std::string no_lang;
    std::string other_lang;
    typedef std::map<std::string, std::string> map_type;
    std::map<std::string, std::string> statuses;

    domain_status_descriptions_fixture()
    : test_lang("EN"),
      no_lang(""),
      other_lang("XX")
    {
        statuses = {
                std::make_pair("expired", "description of expired"),
                std::make_pair("unguarded", "description of unguarded"),
                std::make_pair("serverTransferProhibited", "description of serverTransferProhibited")
        };
        Fred::OperationContext ctx;
        BOOST_FOREACH(map_type::value_type& p, statuses)
        {
            ctx.get_conn().exec_params(
                "INSERT INTO enum_object_states_desc "
                "VALUES ((SELECT id FROM enum_object_states WHERE name = $1::text),"
                " $2::text,"
                " $3::text",
                    Database::query_param_list(p.first)(p.second)(other_lang)
            );
        }
    }
};

template <class T>
bool private_sort(T o1, T o2)
{
    return o1.handle < o2.handle;
}

BOOST_FIXTURE_TEST_CASE(get_domain_status_descriptions, domain_status_descriptions_fixture)
{
    Fred::OperationContext ctx;
    std::vector<Fred::ObjectStateDescription> states =
                        Fred::GetObjectStateDescriptions(test_lang)
                        .set_object_type(std::string("domain"))
                        .set_external()
                        .exec(ctx);
    std::vector<Registry::WhoisImpl::ObjectStatusDesc> vec_osd = impl.get_domain_status_descriptions(test_lang);
    BOOST_CHECK(states.size() == vec_osd.size());
    std::sort(states.begin(), states.end(), private_sort<Fred::ObjectStateDescription>);
    std::sort(vec_osd.begin(), vec_osd.end(), private_sort<Registry::WhoisImpl::ObjectStatusDesc>);
    std::vector<Fred::ObjectStateDescription>::iterator it;
    std::vector<Registry::WhoisImpl::ObjectStatusDesc>::iterator it2;
    for(it = states.begin(), it2 = vec_osd.begin(); it != states.end(); ++it, ++it2)
    {
        BOOST_CHECK(it->handle == it2->handle);
        BOOST_CHECK(it->description == it2->name);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domain_status_other_lang, domain_status_descriptions_fixture)
{
    std::vector<Registry::WhoisImpl::ObjectStatusDesc> vec_osd = impl.get_domain_status_descriptions(other_lang);
    BOOST_CHECK(statuses.size() == vec_osd.size());
    for(std::vector<Registry::WhoisImpl::ObjectStatusDesc>::iterator it = vec_osd.begin(); it != vec_osd.end(); ++it)
    {
        //if not present - at() throws
        BOOST_CHECK(statuses.at(it->handle) == it->name);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domain_status_descriptions_missing, domain_status_descriptions_fixture)//not sure if done correctly
{
    try
    {
        std::vector<Registry::WhoisImpl::ObjectStatusDesc> vec_osd = impl.get_domain_status_descriptions(no_lang);
        BOOST_ERROR("this domain must not have a localization");
    }
    catch(const Registry::WhoisImpl::MissingLocalization& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_domain_status_descriptions

BOOST_AUTO_TEST_SUITE_END();//TestWhois
