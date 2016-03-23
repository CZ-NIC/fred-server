#include "tests/setup/fixtures.h"
#include "src/fredlib/opcontext.h"
#include "src/whois/whois.h"


struct whois_impl_instance_fixture : Test::Fixture::instantiate_db_template
{
    Registry::WhoisImpl::Server_impl impl;
};

