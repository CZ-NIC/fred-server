#include "tests/interfaces/whois/fixture_common.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_registrars)

struct get_my_registrar_list_fixture
: whois_impl_instance_fixture
{
    Fred::OperationContextCreator ctx;
    std::map<std::string,Fred::InfoRegistrarData> registrar_info;

    get_my_registrar_list_fixture()
    {
        const std::vector<Fred::InfoRegistrarOutput> v =
                Fred::InfoRegistrarAllExceptSystem().exec(ctx, "UTC");
        //initial registrars
        BOOST_FOREACH(const Fred::InfoRegistrarOutput& it, v)
        {
            registrar_info[it->info_registrar_data.handle] =
                it->info_registrar_data;
        }
        //new test registrars
        for(unsigned int i=0; i < 10; ++i) //XXX
        {
            const Fred::InfoRegistrarData& ird =
                Test::exec(
                    Test::generate_test_data(
                        Test::CreateX_factory<Fred::CreateRegistrar>().make())
                        .set_system(false),
                ctx);
            registrar_info[ird.handle] = ird;
        }
        total_registrars += v.size();
        for(unsigned int i=0; i < 3; ++i)
        {
            Test::exec(
                Test::generate_test_data(
                    Test::CreateX_factory<Fred::CreateRegistrar>().make())
                        .set_system(true),
                ctx);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_nonsystem_registrars, get_my_registrar_list_fixture)
{
    std::vector<Registry::WhoisImpl::Registrar> registrar_vec =
        impl.get_registrars();
    BOOST_CHECK(registrar_vec.size() == total_registrars);
    std::map<std::string, Fred::InfoRegistrarData>::iterator found;
    BOOST_FOREACH(const Registry::WhoisImpl::Registrar& it, registrar_vec)
    {
        found = registrar_info.find(it.handle);
        BOOST_CHECK(it.address.city ==
                found->second.city.get_value_or_default());
        BOOST_CHECK(it.address.country_code ==
                found->second.country.get_value_or_default());
        BOOST_CHECK(it.address.postal_code ==
                found->second.postalcode.get_value_or_default());
        BOOST_CHECK(it.address.stateorprovince ==
                found->second.stateorprovince.get_value_or_default());
        BOOST_CHECK(it.address.street1 ==
                found->second.street1.get_value_or_default());
        BOOST_CHECK(it.address.street2 ==
                found->second.street2.get_value_or_default());
        BOOST_CHECK(it.address.street3 ==
                found->second.street3.get_value_or_default());
        BOOST_CHECK(it.organization ==
                found->second.organization.get_value_or_default());
        BOOST_CHECK(it.phone ==
                found->second.telephone.get_value_or_default());
        BOOST_CHECK(it.fax == found->second.fax.get_value_or_default());
        BOOST_CHECK(it.name == found->second.name.get_value_or_default());
        BOOST_CHECK(it.handle == found->second.handle);
        BOOST_CHECK(it.id == found->second.id);
        BOOST_CHECK(it.url == found->second.url.get_value_or_default());
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_registrars
BOOST_AUTO_TEST_SUITE_END()//TestWhois
