/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FIXTURES_HH_14EC41658E8F443DA4B9635BB3A07470
#define FIXTURES_HH_14EC41658E8F443DA4B9635BB3A07470


#include "src/libfred/opcontext.hh"
#include "src/libfred/registrable_object/domain/create_domain.hh"
#include "src/libfred/registrable_object/domain/info_domain.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_createexpireddomain_args.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/util/random_data_generator.hh"
#include "test/mockup/logger_client_dummy.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include "src/libfred/object_state/perform_object_state_request.hh"

#include <map>
#include <set>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

namespace Test {
namespace Backend {
namespace Admin {
namespace Domain {

std::unique_ptr<LibFred::Logger::LoggerClient> get_logger()
{
    FakedArgs fa = CfgArgs::instance()->fa;

    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();

    CorbaContainer::set_instance(fa.get_argc(), fa.get_argv(),
            ns_args_ptr->nameservice_host,
            ns_args_ptr->nameservice_port,
            ns_args_ptr->nameservice_context);

    std::unique_ptr<LibFred::Logger::LoggerClient> logger_client = std::make_unique<::LibFred::Logger::DummyLoggerCorbaClientImpl>();

    return logger_client;
}

struct Fqdn
{
    const std::string fqdn;

    Fqdn(const std::string& _fqdn) : fqdn(_fqdn)
    {
    }
};

struct NoExistingDomain : Fqdn
{
    NoExistingDomain(const std::string& _fqdn) : Fqdn(_fqdn)
    {
    }
};

struct ExistingDomain : Fqdn
{
    ::LibFred::InfoDomainData domain;
    ExistingDomain(const std::string& _fqdn) : Fqdn(_fqdn)
    {
        ::LibFred::OperationContextCreator ctx;
        domain = Test::exec(Test::CreateX_factory<::LibFred::CreateDomain>()
                        .make(Test::registrar::make(ctx).handle,
                              Test::contact::make(ctx).handle,
                              _fqdn),
                        ctx);
        ::LibFred::PerformObjectStateRequest(domain.id).exec(ctx);
        ctx.commit_transaction();
    }
};

struct SystemRegistrar
{
    const std::string handle;

    SystemRegistrar():
        handle(CfgArgs::instance()->get_handler_ptr_by_type<HandleCreateExpiredDomainArgs>()->registrar_handle)
    {
    }
};

struct HasNoExistingDomain {
    SystemRegistrar registrar;
    NoExistingDomain domain;
    bool delete_existing;
    const std::string registrant;
    const std::string cltrid;
    HasNoExistingDomain() :
        domain("noexistingdomain1.cz"),
        delete_existing(true),
        registrant("KONTAKT"),
        cltrid("cltrid")
    {
    }
};

struct HasNoDeleteNoExistingDomain {
    SystemRegistrar registrar;
    NoExistingDomain domain;
    bool delete_existing;
    const std::string registrant;
    const std::string cltrid;
    HasNoDeleteNoExistingDomain() :
        domain("noexistingdomain2.cz"),
        delete_existing(false),
        registrant("KONTAKT"),
        cltrid("cltrid")
    {
    }
};

struct HasExistingDomain {
    SystemRegistrar registrar;
    ExistingDomain domain;
    bool delete_existing;
    const std::string registrant;
    const std::string cltrid;
    HasExistingDomain() :
        domain("existingdomain2.cz"),
        delete_existing(true),
        registrant("KONTAKT"),
        cltrid("cltrid")
    {
    }
};

struct HasNoDeleteExistingDomain {
    SystemRegistrar registrar;
    ExistingDomain domain;
    bool delete_existing;
    const std::string registrant;
    const std::string cltrid;
    HasNoDeleteExistingDomain() :
        domain("existingdomain1.cz"),
        delete_existing(false),
        registrant("KONTAKT"),
        cltrid("cltrid")
    {
    }
};

struct HasNoExistingRegistrant {
    SystemRegistrar registrar;
    ExistingDomain domain;
    bool delete_existing;
    const std::string registrant;
    const std::string cltrid;
    HasNoExistingRegistrant() :
        domain("noexistingregistrant.cz"),
        delete_existing(false),
        registrant("no_existing_contact"),
        cltrid("cltrid")
    {
    }
};

} // namespace Test::Backend::Admin::Domain
} // namespace Test::Backend::Admin
} // namespace Test::Backend
} // namespace Test

#endif
