/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef FIXTURES_HH_14EC41658E8F443DA4B9635BB3A07470
#define FIXTURES_HH_14EC41658E8F443DA4B9635BB3A07470


#include "libfred/opcontext.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_createexpireddomain_args.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "util/random_data_generator.hh"
#include "test/mockup/logger_client_dummy.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include "libfred/object_state/perform_object_state_request.hh"

#include <map>
#include <set>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

namespace Test {
namespace Backend {
namespace Admin {
namespace Domain {

struct context_holder
    : virtual instantiate_db_template
{
    LibFred::OperationContextCreator ctx;
};

template <class T>
struct supply_fixture_ctx : context_holder, T
{
    supply_fixture_ctx()
        : context_holder(),
          T(ctx)
    {
        ctx.commit_transaction();
    }
};

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

    NonexistentDomain(LibFred::OperationContext& _ctx)
    {
        fqdn = Test::get_nonexistent_object_handle(_ctx) + ".cz";
    }
};

struct ExistingDomain
{
    std::string fqdn;

    ExistingDomain(LibFred::OperationContext& _ctx)
    {
        LibFred::InfoDomainData domain = Test::exec(Test::CreateX_factory<LibFred::CreateDomain>()
                        .make(Test::registrar::make(_ctx).handle,
                              Test::contact::make(_ctx).handle),
                        _ctx);
        fqdn = domain.fqdn;
    }
};

struct NonexistentRegistrar
{
    std::string handle;

    NonexistentRegistrar(LibFred::OperationContext& _ctx)
    {
        handle = Test::get_nonexistent_object_handle(_ctx);
    }
};

struct NonSystemRegistrar
{
    std::string handle;

    NonSystemRegistrar(LibFred::OperationContext& _ctx)
    {
        const LibFred::InfoRegistrarData registrar = Test::exec(Test::CreateX_factory<LibFred::CreateRegistrar>().make(), _ctx);
        handle = registrar.handle;
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

    ExistingContact(LibFred::OperationContext& _ctx)
    {
        const LibFred::InfoContactData contact = Test::exec(Test::CreateX_factory<LibFred::CreateContact>().make(Test::registrar::make(_ctx).handle), _ctx);
        handle = contact.handle;
    }
};

struct NonexistentContact
{
    std::string handle;

    NonexistentContact(LibFred::OperationContext& _ctx)
    {
        handle = Test::get_nonexistent_object_handle(_ctx);
    }
};

struct HasNonexistentRegistrar {
    NonexistentRegistrar registrar;
    ExistingContact registrant;
    NonexistentDomain domain;
    bool delete_existing;
    const std::string cltrid;
    HasNonexistentRegistrar(LibFred::OperationContext& _ctx):
        registrar(_ctx),
        registrant(_ctx),
        domain(_ctx),
        delete_existing(false),
        cltrid("cltrid")
    {
    }
};
struct HasNonSystemRegistrar {
    NonSystemRegistrar registrar;
    ExistingContact registrant;
    NonexistentDomain domain;
    bool delete_existing;
    const std::string cltrid;
    HasNonSystemRegistrar(LibFred::OperationContext& _ctx):
        registrar(_ctx),
        registrant(_ctx),
        domain(_ctx),
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
    HasNonexistentDomain(LibFred::OperationContext& _ctx):
        registrant(_ctx),
        domain(_ctx),
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
    HasNoDeleteNonexistentDomain(LibFred::OperationContext& _ctx):
        registrant(_ctx),
        domain(_ctx),
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
    HasExistingDomain(LibFred::OperationContext& _ctx):
        registrant(_ctx),
        domain(_ctx),
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
    HasNoDeleteExistingDomain(LibFred::OperationContext& _ctx):
        registrant(_ctx),
        domain(_ctx),
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
    HasNonexistentRegistrant(LibFred::OperationContext& _ctx):
        registrant(_ctx),
        domain(_ctx),
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
