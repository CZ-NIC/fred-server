#include <boost/exception/diagnostic_information.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#define BOOST_TEST_NO_MAIN

BOOST_AUTO_TEST_SUITE(TestWhois)

// registrar!
BOOST_AUTO_TEST_SUITE(get_registrar_by_handle)

struct get_registrar_fixture
//!! : test_registrar_fixture
{
    std::string no_registrar_handle;
    std::string wrong_registrar_handle;

    get_registrar_fixture()
    : no_registrar_handle("absent-registrar"),
      wrong_registrar_handle("")
    {
        Fred::OperationContext ctx;
        Fred::InfoRegistrarData ird = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_registrar_data;
    }
};

BOOST_FIXTURE_TEST_CASE(get_fine_registrar, get_registrar_fixture)
{
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

BOOST_AUTO_TEST_SUITE_END();//TestWhois
