#ifndef WHOIS_FIXTURE_COMMON_H_
#define WHOIS_FIXTURE_COMMON_H_

#include "src/whois/whois.h"
#include "src/fredlib/opcontext.h"
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"

#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>

struct whois_impl_instance_fixture : Test::Fixture::instantiate_db_template
{
    Registry::WhoisImpl::Server_impl impl;
};

#endif //WHOIS_FIXTURE_COMMON_H_
