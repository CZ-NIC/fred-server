#ifndef WHOIS_FIXTURE_COMMON_H_
#define WHOIS_FIXTURE_COMMON_H_
#include <boost/exception/diagnostic_information.hpp> 
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include "tests/setup/fixtures.h"
#include "src/fredlib/opcontext.h"
#include "src/whois/whois.h"
#include "tests/setup/fixtures_utils.h"
#endif

struct whois_impl_instance_fixture : Test::Fixture::instantiate_db_template
{
    Registry::WhoisImpl::Server_impl impl;
};

