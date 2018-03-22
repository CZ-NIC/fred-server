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

    std::unique_ptr<LibFred::Logger::LoggerClient> logger_client = std::make_unique<LibFred::Logger::DummyLoggerCorbaClientImpl>();

    return logger_client;
}

struct NonexistentDomain
{
    std::string fqdn;

    NonexistentDomain()
    {
        LibFred::OperationContextCreator ctx;
        fqdn = Test::domain::make(ctx).fqdn;
    }
};

struct ExistingDomain
{
    std::string fqdn;

    ExistingDomain()
    {
        LibFred::OperationContextCreator ctx;
        LibFred::InfoDomainData domain = Test::exec(Test::CreateX_factory<LibFred::CreateDomain>()
                        .make(Test::registrar::make(ctx).handle,
                              Test::contact::make(ctx).handle),
                        ctx);
        fqdn = domain.fqdn;
        ctx.commit_transaction();
    }
};

struct NonexistentRegistrar
{
    std::string handle;

    NonexistentRegistrar()
    {
        LibFred::OperationContextCreator ctx;
        handle = Test::registrar::make(ctx).handle;
    }
};

struct NoSystemRegistrar
{
    std::string handle;

    NoSystemRegistrar()
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::InfoRegistrarData registrar = Test::exec(Test::CreateX_factory<LibFred::CreateRegistrar>().make(), ctx);
        handle = registrar.handle;
        ctx.commit_transaction();
    }
};

struct SystemRegistrar
{
    std::string handle;

    SystemRegistrar():
        handle(CfgArgs::instance()->get_handler_ptr_by_type<HandleCreateExpiredDomainArgs>()->registrar_handle)
    {
    }
};

struct ExistingContact
{
    std::string handle;

    ExistingContact()
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::InfoContactData contact = Test::exec(Test::CreateX_factory<LibFred::CreateContact>().make(Test::registrar::make(ctx).handle), ctx);
        handle = contact.handle;
        ctx.commit_transaction();
    }
};

struct NonexistentContact
{
    std::string handle;

    NonexistentContact()
    {
        LibFred::OperationContextCreator ctx;
        handle = Test::contact::make(ctx).handle;
    }
};

struct HasNonexistentRegistrar{
    NonexistentRegistrar registrar;
    ExistingContact registrant;
    NonexistentDomain domain;
    bool delete_existing;
    const std::string cltrid;
    HasNonexistentRegistrar():
        delete_existing(false),
        cltrid("cltrid")
    {
    }
};
struct HasNoSystemRegistrar {
    NoSystemRegistrar registrar;
    ExistingContact registrant;
    NonexistentDomain domain;
    bool delete_existing;
    const std::string cltrid;
    HasNoSystemRegistrar():
        delete_existing(false),
        cltrid("cltrid")
    {
    }
};
struct HasNonexistentDomain {
    SystemRegistrar registrar;
    ExistingContact registrant;
    NonexistentDomain domain;
    bool delete_existing;
    const std::string cltrid;
    HasNonexistentDomain():
        delete_existing(true),
        cltrid("cltrid")
    {
    }
};

struct HasNoDeleteNonexistentDomain {
    SystemRegistrar registrar;
    ExistingContact registrant;
    NonexistentDomain domain;
    bool delete_existing;
    const std::string cltrid;
    HasNoDeleteNonexistentDomain():
        delete_existing(false),
        cltrid("cltrid")
    {
    }
};

struct HasExistingDomain {
    SystemRegistrar registrar;
    ExistingContact registrant;
    ExistingDomain domain;
    bool delete_existing;
    const std::string cltrid;
    HasExistingDomain():
        delete_existing(true),
        cltrid("cltrid")
    {
    }
};

struct HasNoDeleteExistingDomain {
    SystemRegistrar registrar;
    ExistingContact registrant;
    ExistingDomain domain;
    bool delete_existing;
    const std::string cltrid;
    HasNoDeleteExistingDomain():
        delete_existing(false),
        cltrid("cltrid")
    {
    }
};

struct HasNonexistentRegistrant {
    SystemRegistrar registrar;
    NonexistentContact registrant;
    NonexistentDomain domain;
    bool delete_existing;
    const std::string cltrid;
    HasNonexistentRegistrant():
        delete_existing(false),
        cltrid("cltrid")
    {
    }
};

} // namespace Test::Backend::Admin::Domain
} // namespace Test::Backend::Admin
} // namespace Test::Backend
} // namespace Test

#endif
