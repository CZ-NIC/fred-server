#include "test/backend/whois/fixture_common.hh"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_registrars)

struct get_my_registrar_list_fixture
: whois_impl_instance_fixture
{
    std::map<std::string,::LibFred::InfoRegistrarData> registrar_info;

    get_my_registrar_list_fixture()
    {
        LibFred::OperationContextCreator ctx;
        const std::vector<::LibFred::InfoRegistrarOutput> v =
                LibFred::InfoRegistrarAllExceptSystem().exec(ctx, "UTC");
        //initial registrars
        BOOST_FOREACH(const LibFred::InfoRegistrarOutput& it, v)
        {
            registrar_info[it.info_registrar_data.handle] =
                it.info_registrar_data;
        }
        //new test registrars
        for(unsigned int i=0; i < 10; ++i) //XXX
        {
            const LibFred::InfoRegistrarData& ird =
                Test::exec(
                    Test::generate_test_data(
                        Test::CreateX_factory<::LibFred::CreateRegistrar>().make())
                        .set_system(false),
                ctx);
            registrar_info[ird.handle] = ird;
        }
        //system registrars
        for(unsigned int i=0; i < 3; ++i)
        {
            Test::exec(
                Test::generate_test_data(
                    Test::CreateX_factory<::LibFred::CreateRegistrar>().make())
                    .set_system(true),
                ctx);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_nonsystem_registrars, get_my_registrar_list_fixture)
{
    std::vector<Fred::Backend::Whois::Registrar> registrar_vec = impl.get_registrars();
    BOOST_CHECK(registrar_vec.size() == registrar_info.size());
    std::map<std::string, LibFred::InfoRegistrarData>::const_iterator cit;
    BOOST_FOREACH(const Fred::Backend::Whois::Registrar& it, registrar_vec)
    {
        cit = registrar_info.find(it.handle);
        BOOST_REQUIRE(cit != registrar_info.end());
        const LibFred::InfoRegistrarData& found = cit->second;
        BOOST_CHECK(it.address.city            == found.city.get_value_or_default());
        BOOST_CHECK(it.address.country_code    == found.country.get_value_or_default());
        BOOST_CHECK(it.address.postal_code     == found.postalcode.get_value_or_default());
        BOOST_CHECK(it.address.stateorprovince == found.stateorprovince.get_value_or_default());
        BOOST_CHECK(it.address.street1         == found.street1.get_value_or_default());
        BOOST_CHECK(it.address.street2         == found.street2.get_value_or_default());
        BOOST_CHECK(it.address.street3         == found.street3.get_value_or_default());
        BOOST_CHECK(it.organization            == found.organization.get_value_or_default());
        BOOST_CHECK(it.phone                   == found.telephone.get_value_or_default());
        BOOST_CHECK(it.fax                     == found.fax.get_value_or_default());
        BOOST_CHECK(it.name                    == found.name.get_value_or_default());
        BOOST_CHECK(it.handle                  == found.handle);
        BOOST_CHECK(it.id                      == found.id);
        BOOST_CHECK(it.url                     == found.url.get_value_or_default());
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_registrars
BOOST_AUTO_TEST_SUITE_END()//TestWhois
