#ifndef WHOIS_FIXTURE_COMMON_H_
#define WHOIS_FIXTURE_COMMON_H_
#define BOOST_TEST_NO_MAIN
#include <boost/exception/diagnostic_information.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include "tests/setup/fixtures.h"
#include "src/fredlib/opcontext.h"
#include "src/whois/whois.h"
#include "src/whois/zone_list.h"
#include "tests/setup/fixtures_utils.h"
#include "util/random_data_generator.h"

struct empty_registrar_fixture
{
    Fred::OperationContext ctx;
    std::string xmark;
    const Fred::InfoRegistrarData registrar;

    empty_registrar_fixture()
    : xmark(RandomDataGenerator().xnumstring(6)),
      registrar(Test::exec(
                    Fred::CreateRegistrar(std::string("REG-FOOREG") + xmark),
                    ctx))
    {
        ctx.commit_transaction();
    }

    empty_registrar_fixture(std::string name)
    : registrar(Test::exec(
                  Fred::CreateRegistrar(name), ctx))
    {
        ctx.commit_transaction();
    }
};

struct empty_contact_fixture
{
    Fred::OperationContext ctx;
    empty_registrar_fixture erf;
    std::string xmark;
    const Fred::InfoContactData contact;

    empty_contact_fixture()
    : xmark(RandomDataGenerator().xnumstring(6)),
      contact(Test::exec(
                  Fred::CreateContact(std::string("TEST-CONTACT") + xmark,
                                      erf.registrar.handle),
                  ctx))
    {
        ctx.commit_transaction();
    }

    empty_contact_fixture(std::string name)
    : contact(Test::exec(
                  Fred::CreateContact(name, erf.registrar.handle),
                  ctx))
    {
        ctx.commit_transaction();
    }
};

struct whois_impl_instance_fixture : Test::Fixture::instantiate_db_template
{
    Registry::WhoisImpl::Server_impl impl;
};

#endif //WHOIS_FIXTURE_COMMON_H_
