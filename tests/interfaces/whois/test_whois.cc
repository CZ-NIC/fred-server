#include "src/whois/whois.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/registrar/info_registrar_output.h"
#include "src/fredlib/registrar/info_registrar_data.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/create_registrar.h"
#include "random_data_generator.h"

#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>

BOOST_AUTO_TEST_SUITE(TestWhois)

struct whois_impl_instance_fixture
{
    Registry::WhoisImpl::Server_impl impl;
};

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
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
        BOOST_MESSAGE(test_registrar_handle);
    }
    ~test_registrar_fixture()
    {}};

BOOST_AUTO_TEST_SUITE(get_registrar_by_handle)

struct get_registrar_fixture : whois_impl_instance_fixture, test_registrar_fixture
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

struct get_registar_no_registrar_fixuture : whois_impl_instance_fixture, test_registrar_fixture
{};

BOOST_FIXTURE_TEST_CASE(get_registar_no_registrar, get_registar_no_registrar_fixuture)
{
    try
    {
        Fred::OperationContext ctx;
        Fred::InfoRegistrarOutput ird = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
        Registry::WhoisImpl::Registrar reg = impl.get_registrar_by_handle("reg-");
        BOOST_ERROR("unreported missing registrar");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}
//invalid handle case

BOOST_AUTO_TEST_SUITE_END();//get_registrar_by_handle

BOOST_AUTO_TEST_SUITE_END();//TestWhois
