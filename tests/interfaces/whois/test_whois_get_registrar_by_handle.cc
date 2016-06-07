#include "tests/interfaces/whois/fixture_common.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_registrar_by_handle)

struct registrar_fixture 
: whois_impl_instance_fixture
{
    Fred::OperationContextCreator ctx;
    const Fred::InfoRegistrarData registrar;

    registrar_fixture()
    : registrar(Test::exec(
                    Test::generate_test_data(
                        Test::CreateX_factory<Fred::CreateRegistrar>().make()),
                    ctx))
    {
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_fine_registrar, registrar_fixture)
{
    Registry::WhoisImpl::Registrar reg =
        impl.get_registrar_by_handle(registrar.handle);

    BOOST_CHECK(reg.organization            == registrar.organization.get_value_or_default());
    BOOST_CHECK(reg.fax                     == registrar.fax.get_value_or_default());
    BOOST_CHECK(reg.name                    == registrar.name.get_value_or_default());
    BOOST_CHECK(reg.handle                  == registrar.handle);
    BOOST_CHECK(reg.id                      == registrar.id);
    BOOST_CHECK(reg.phone                   == registrar.telephone.get_value_or_default());
    BOOST_CHECK(reg.url                     == registrar.url.get_value_or_default());
    BOOST_CHECK(reg.address.city            == registrar.city.get_value_or_default());
    BOOST_CHECK(reg.address.country_code    == registrar.country.get_value_or_default());
    BOOST_CHECK(reg.address.postal_code     == registrar.postalcode.get_value_or_default());
    BOOST_CHECK(reg.address.stateorprovince == registrar.stateorprovince.get_value_or_default());
    BOOST_CHECK(reg.address.street1         == registrar.street1.get_value_or_default());
    BOOST_CHECK(reg.address.street2         == registrar.street2.get_value_or_default());
    BOOST_CHECK(reg.address.street3         == registrar.street3.get_value_or_default());
}

BOOST_FIXTURE_TEST_CASE(get_no_registar, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(get_registrar_by_handle("REG-ABSENT"), Registry::WhoisImpl::ObjectNotExists)
//    try
//    {
//        Registry::WhoisImpl::Registrar reg = impl.get_registrar_by_handle("REG-ABSENT");
//        BOOST_ERROR("unreported dangling registrar");
//    }
//    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
//    {
//        BOOST_CHECK(true);
//        BOOST_MESSAGE(boost::diagnostic_information(ex));
//    }
}

BOOST_FIXTURE_TEST_CASE(get_wrong_registrar, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(get_registrar_by_handle("REG@#$"), Registry::WhoisImpl::InvalidHandle)
//    try
//    {
//        Registry::WhoisImpl::Registrar reg = impl.get_registrar_by_handle("REG@#$");
//        BOOST_ERROR("registrar handle rule is wrong");
//    }
//    catch(const Registry::WhoisImpl::InvalidHandle& ex)
//    {
//        BOOST_CHECK(true);
//        BOOST_MESSAGE(boost::diagnostic_information(ex));
//    }
}

BOOST_AUTO_TEST_SUITE_END()//get_registrar_by_handle
BOOST_AUTO_TEST_SUITE_END()//TestWhois
