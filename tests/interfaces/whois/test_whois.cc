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
        : whois_impl_instance_fixture
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
    {}
};

BOOST_AUTO_TEST_SUITE(get_registrar_by_handle)

struct get_registrar_fixture
        : test_registrar_fixture
{
    std::string no_registrar_handle;
    std::string wrong_registrar_handle;
    get_registrar_fixture()
        : no_registrar_handle("REG-"),
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
    ~get_my_registrar_list_fixture() {}
};

BOOST_AUTO_TEST_SUITE(get_registrars)

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

        ctx.commit_transaction();//commit fixture
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
          test_fqdn(std::string("TEST-FQDN") + xmark),
          test_no_fqdn("fine-fqdn"),
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
        ctx.commit_transaction();//commit fixture
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
        Fred::OperationContext ctx;
        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_ns(test_wrong_fqdn, test_limit);
        BOOST_ERROR("domain handle rules is wrong");
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
        Fred::OperationContext ctx;
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
        Fred::OperationContext ctx;
        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_tech_c(test_wrong_handle, test_limit);
        BOOST_ERROR("domain handle rules is wrong");
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
        Fred::OperationContext ctx;
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

//    BOOST_AUTO_TEST_SUITE(get_nameserver_by_fqdn)
//
//    struct get_nameserver_by_fqdn_fixture
//            : test_registrar_fixture
//    {
//        std::string test_c_handle;
//        std::string test_no_handle;
//        std::string test_wrong_handle;
//
//        get_nameserver_by_fqdn_fixture()
//            : test_registrar_fixture(),
//              test_c_handle(std::string("TEST-NAMESERVER-FQDN") + xmark),
//              test_no_handle("fine-tech-c-handle"),
//              test_wrong_handle("")
//        {
//            Fred::OperationContext ctx;
//            Fred::Create().exec(ctx);
//                //any extra setting here?
//            }
//            ctx.commit_transaction();
//        }
//
//        ~get_nameserver_by_fqdn_fixture() {}
//    };
//
//    BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn, get_nameserver_by_fqdn_fixture)
//    {
//        Fred::OperationContext ctx;
//        std::vector<Fred::InfoNssetOutput> v_ino = Fred::InfoNssetByDNSFqdn(test_c_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
//        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_tech_c(test_c_handle, test_limit);
//
//        BOOST_CHECK(nss_s.content.at(i).changed.isnull());
//        BOOST_CHECK(nss_s.content.at(i).last_transfer.isnull());
//        BOOST_CHECK(nss_s.content.at(i).created == found.creation_time);//as that or greater than __
//        BOOST_CHECK(nss_s.content.at(i).handle == found.handle);
//        BOOST_CHECK(nss_s.content.at(i).nservers.at(0).fqdn == found.dns_hosts.at(0).get_fqdn());
//        BOOST_CHECK(nss_s.content.at(i).nservers.at(0).ip_addresses.at(0) == found.dns_hosts.at(0).get_inet_addr().at(0)); //comparing two boost::address'es
//        BOOST_CHECK(nss_s.content.at(i).registrar_handle == found.create_registrar_handle);
//        BOOST_CHECK(nss_s.content.at(i).tech_contact_handles.at(0) == found.tech_contacts.at(0).handle);
//
//    }
//
//    BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_wrong_ns, get_nameserver_by_fqdn_fixture)
//    {
//        try
//        {
//            Fred::OperationContext ctx;
//            Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_tech_c(test_wrong_handle, test_limit);
//            BOOST_ERROR("domain handle rules is wrong");
//        }
//        catch(const Registry::WhoisImpl::InvalidHandle& ex)
//        {
//            BOOST_CHECK(true);
//            BOOST_MESSAGE(boost::diagnostic_information(ex));
//        }
//    }
//
//    BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_no_ns, get_nameserver_by_fqdn_fixture)
//    {
//        try
//        {
//            Fred::OperationContext ctx;
//            Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_tech_c(test_no_handle, test_limit);
//            BOOST_ERROR("unreported dangling NSSets");
//        }
//        catch(const Registry::WhoisImpl::ObjectNotExists& ex)
//        {
//            BOOST_CHECK(true);
//            BOOST_MESSAGE(boost::diagnostic_information(ex));
//        }
//    }
//
//    BOOST_AUTO_TEST_SUITE_END()//get_nameserver_by_fqdn

BOOST_AUTO_TEST_SUITE(get_keyset_by_handle)

struct get_keyset_by_handle_fixture
        : test_registrar_fixture
{
    std::string test_keyset_handle;
    std::string no_keyset_handle;
    std::string wrong_keyset_handle;

    get_keyset_by_handle_fixture()
    : test_registrar_fixture(),
      test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark),
      no_keyset_handle("fine-keyset-handle"),
      wrong_keyset_handle("")
    {
        Fred::OperationContext ctx;

        std::vector<Fred::DnsKey> test_dns_keys;
        test_dns_keys.push_back(Fred::DnsKey(0, 0, 0, "any-key"));
        std::vector<std::string> test_tech_contacts;
        test_tech_contacts.push_back("TEST-TECH-CONTACT");
        Fred::CreateKeyset(test_keyset_handle, test_registrar_handle)
            .set_dns_keys(test_dns_keys)
            .set_tech_contacts(test_tech_contacts)
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

BOOST_AUTO_TEST_SUITE_END();//TestWhois
