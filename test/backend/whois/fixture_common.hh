#ifndef WHOIS_FIXTURE_COMMON_H_
#define WHOIS_FIXTURE_COMMON_H_

#include "src/backend/whois/whois.hh"
#include "src/libfred/opcontext.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>

struct whois_impl_instance_fixture : Test::instantiate_db_template
{
    Registry::WhoisImpl::Server_impl impl;

    whois_impl_instance_fixture()
    : impl("test-whois")
    {}
};

#endif //WHOIS_FIXTURE_COMMON_H_
