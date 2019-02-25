/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#include "src/util/time_clock.hh"

#include "libfred/db_settings.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/libfred/registrar.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrar/create_registrar.hh"

#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"

#include "src/bin/corba/epp_corba_client_impl.hh"
#include "test/deprecated/test_invoice_common.hh"

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <utility>

// TODO: this should also be used in invoice test
Database::ID create_registrar(::LibFred::Registrar::Manager *regMan)
{
    ::LibFred::Registrar::Registrar::AutoPtr registrar = regMan->createRegistrar();

    //current time into char string
    std::string time_string(TimeStamp::microsec());
    std::string registrar_novat_handle(std::string("REG-FRED_NOVAT_INV")+time_string);

    registrar->setHandle(registrar_novat_handle);//REGISTRAR_ADD_HANDLE_NAME
    registrar->setCountry("CZ");//REGISTRAR_COUNTRY_NAME
    registrar->setVat(true);

    ::LibFred::OperationContextCreator ctx;
    const unsigned long long id = ::LibFred::CreateRegistrar(registrar_novat_handle)
            .set_country("CZ")
            .set_vat_payer(true)
            .exec(ctx);
    ctx.commit_transaction();
    registrar->setId(id);
    return id;
}

Database::ID block_reg_and_test(::LibFred::Registrar::Manager *regman)
{
    Database::ID reg_id = create_registrar(regman);

    std::unique_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());

    BOOST_REQUIRE(regman->blockRegistrar(reg_id, epp_cli.get()) == true);

    BOOST_REQUIRE(regman->isRegistrarBlocked(reg_id));
    // this is gonna be correct
    return reg_id;
}

BOOST_AUTO_TEST_SUITE(TestRegistrarBlocking)

BOOST_AUTO_TEST_CASE( test_block_registrar )
{
    DBSharedPtr m_db;
    Database::Connection conn = Database::Manager::acquire();
    m_db.reset(new DB(conn));

    std::unique_ptr<::LibFred::Registrar::Manager> regMan(
            ::LibFred::Registrar::Manager::create(m_db));

    Database::ID registrar_id = create_registrar(regMan.get());

    init_corba_container();

    std::unique_ptr<EppCorbaClientImpl> epp_cli (new EppCorbaClientImpl());

    // TODO This test lacks cleanup and so it would prevent further manual testing
    // registrar_disconnect must be cleaned after this call or it could use own registrar
    // regMan->blockRegistrar(1, epp_cli.get());

    regMan->blockRegistrar(registrar_id, epp_cli.get() );
}

BOOST_AUTO_TEST_CASE( test_is_registrar_blocked )
{
    init_corba_container();
    DBSharedPtr nodb;
    std::unique_ptr<::LibFred::Registrar::Manager> regman(
                   ::LibFred::Registrar::Manager::create(nodb));

    block_reg_and_test(regman.get());
}

BOOST_AUTO_TEST_CASE(test_unblock_registrar)
{
    init_corba_container();
    DBSharedPtr nodb;
    std::unique_ptr<::LibFred::Registrar::Manager> regman(
                   ::LibFred::Registrar::Manager::create(nodb));

    Database::ID reg_id = block_reg_and_test(regman.get());

    regman->unblockRegistrar(reg_id, 767);

    BOOST_REQUIRE(!regman->isRegistrarBlocked(reg_id));
}

BOOST_AUTO_TEST_CASE( test_double_unlock_registrar)
{
    init_corba_container();
    DBSharedPtr nodb;
    std::unique_ptr<::LibFred::Registrar::Manager> regman(
                       ::LibFred::Registrar::Manager::create(nodb));

    Database::ID reg_id = block_reg_and_test(regman.get());

    regman->unblockRegistrar(reg_id, 777);

    BOOST_REQUIRE_THROW(regman->unblockRegistrar(reg_id, 787),  ::LibFred::NOT_BLOCKED);
}

BOOST_AUTO_TEST_CASE( test_block_again)
{
    init_corba_container();
    DBSharedPtr nodb;
    std::unique_ptr<::LibFred::Registrar::Manager> regman(
                       ::LibFred::Registrar::Manager::create(nodb));
    std::unique_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());

    Database::ID reg_id = block_reg_and_test(regman.get());

    regman->unblockRegistrar(reg_id, 777);

    BOOST_REQUIRE(regman->blockRegistrar(reg_id, epp_cli.get()) == false);
}

BOOST_AUTO_TEST_CASE( test_block_again_no_special_values_used_in_interval_definition )
{
    init_corba_container();
    DBSharedPtr nodb;
    std::unique_ptr<::LibFred::Registrar::Manager> regman(
                         ::LibFred::Registrar::Manager::create(nodb));
    std::unique_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());

    Database::Connection conn = Database::Manager::acquire();

    Database::ID registrar_id = create_registrar(regman.get());

// pretend that registrar was blocked 'till the end of last month:
    conn.exec_params ("INSERT INTO registrar_disconnect (registrarid, blocked_from, blocked_to) VALUES "
            "($1::integer, now() - interval '1 month', date_trunc('month', now()) ) ",
            Database::query_param_list(registrar_id));

    BOOST_REQUIRE(regman->blockRegistrar(registrar_id, epp_cli.get()));
}

/// might not work shortly (couple of minutes) after begining of a month
BOOST_AUTO_TEST_CASE( test_block_last_year_interference )
{
    init_corba_container();
    DBSharedPtr nodb;
    std::unique_ptr<::LibFred::Registrar::Manager> regman(
                         ::LibFred::Registrar::Manager::create(nodb));
    std::unique_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());

    Database::Connection conn = Database::Manager::acquire();

    Database::ID registrar_id = create_registrar(regman.get());

// pretend that registrar was blocked 'till the end of last month:
    conn.exec_params ("INSERT INTO registrar_disconnect (registrarid, blocked_from, blocked_to) VALUES "
            "($1::integer, "
            " date_trunc('month', now()) - interval '1 year' + interval '1 minute', "
            " now() - interval '1 year' - interval '1 minute') ",
            Database::query_param_list(registrar_id));

    BOOST_REQUIRE(regman->blockRegistrar(registrar_id, epp_cli.get()));
}

BOOST_AUTO_TEST_SUITE_END()//TestRegistrarBlocking
