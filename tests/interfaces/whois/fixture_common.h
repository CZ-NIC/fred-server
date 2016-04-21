#ifndef WHOIS_FIXTURE_COMMON_H_
#define WHOIS_FIXTURE_COMMON_H_
#define BOOST_TEST_NO_MAIN
#include <boost/exception/diagnostic_information.hpp>
#include <boost/test/unit_test.hpp>
#include "tests/setup/fixtures.h"
#include "src/fredlib/opcontext.h"
#include "src/whois/whois.h"
#include "tests/setup/fixtures_utils.h"

struct whois_impl_instance_fixture : Test::Fixture::instantiate_db_template
{
    Registry::WhoisImpl::Server_impl impl;
};

#endif //WHOIS_FIXTURE_COMMON_H_
