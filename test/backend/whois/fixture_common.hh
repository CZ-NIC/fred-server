#ifndef FIXTURE_COMMON_HH_23091B98FDF94EE3B83A72ED232CDF75
#define FIXTURE_COMMON_HH_23091B98FDF94EE3B83A72ED232CDF75

#include "src/backend/whois/whois.hh"
#include "libfred/opcontext.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>

struct whois_impl_instance_fixture : Test::instantiate_db_template
{
    Fred::Backend::Whois::Server_impl impl;

    whois_impl_instance_fixture()
    : impl("test-whois")
    {}
};

#endif
