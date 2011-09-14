#include <boost/test/unit_test.hpp>
#include <iostream>
#include "time_clock.h"

#include "fredlib/db_settings.h"
#include "old_utils/dbsql.h"
#include "registrar.h"

#include "cfg/config_handler_decl.h"
#include "cfg/handle_general_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"

#include "epp_corba_client_impl.h"
#include "test-invoice-common.h"


// TODO: this should also be used in invoice test
Database::ID create_registrar(Fred::Registrar::Manager *regMan)
{
    Fred::Registrar::Registrar::AutoPtr registrar = regMan->createRegistrar();

    //current time into char string
    std::string time_string(TimeStamp::microsec());
    std::string registrar_novat_handle(std::string("REG-FRED_NOVAT_INV")+time_string);

    registrar->setHandle(registrar_novat_handle);//REGISTRAR_ADD_HANDLE_NAME
    registrar->setCountry("CZ");//REGISTRAR_COUNTRY_NAME
    registrar->setVat(true);
    registrar->save();

    return registrar->getId();
}

Database::ID block_reg_and_test(Fred::Registrar::Manager *regman)
{
    Database::ID reg_id = create_registrar(regman);

    std::auto_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());

    BOOST_REQUIRE(regman->blockRegistrar(reg_id, epp_cli.get()) == true);

    BOOST_REQUIRE(regman->isRegistrarBlocked(reg_id));
    // this is gonna be correct
    return reg_id;
}

BOOST_AUTO_TEST_SUITE(TestRegistrarBlocking)

BOOST_AUTO_TEST_CASE( test_block_registrar )
{
     DBSharedPtr m_db = connect_DB(
             CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info()
             , std::runtime_error("NotifyClient db connection failed"));

    std::auto_ptr<Fred::Registrar::Manager> regMan(
            Fred::Registrar::Manager::create(m_db));

    Database::ID registrar_id = create_registrar(regMan.get());

    init_corba_container();

    std::auto_ptr<EppCorbaClientImpl> epp_cli (new EppCorbaClientImpl());

    // TODO This test lacks cleanup and so it would prevent further manual testing
    // registrar_disconnect must be cleaned after this call or it could use own registrar
    // regMan->blockRegistrar(1, epp_cli.get());

    regMan->blockRegistrar(registrar_id, epp_cli.get() );

}

BOOST_AUTO_TEST_CASE( test_is_registrar_blocked )
{
    init_corba_container();
    std::auto_ptr<Fred::Registrar::Manager> regman(
                   Fred::Registrar::Manager::create(DBDisconnectPtr(NULL)));

    Database::ID reg_id = block_reg_and_test(regman.get());
}

BOOST_AUTO_TEST_CASE(test_unblock_registrar)
{
    init_corba_container();
    std::auto_ptr<Fred::Registrar::Manager> regman(
                   Fred::Registrar::Manager::create(DBDisconnectPtr(NULL)));

    Database::ID reg_id = block_reg_and_test(regman.get());

    regman->unblockRegistrar(reg_id, 767);

    BOOST_REQUIRE(!regman->isRegistrarBlocked(reg_id));

}

BOOST_AUTO_TEST_CASE( test_double_unlock_registrar)
{
    init_corba_container();
    std::auto_ptr<Fred::Registrar::Manager> regman(
                       Fred::Registrar::Manager::create(DBDisconnectPtr(NULL)));

    Database::ID reg_id = block_reg_and_test(regman.get());

    regman->unblockRegistrar(reg_id, 777);

    BOOST_REQUIRE_THROW(regman->unblockRegistrar(reg_id, 787),  Fred::NOT_BLOCKED);

}

BOOST_AUTO_TEST_CASE( test_block_again)
{
    init_corba_container();
    std::auto_ptr<Fred::Registrar::Manager> regman(
                       Fred::Registrar::Manager::create(DBDisconnectPtr(NULL)));
    std::auto_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());

    Database::ID reg_id = block_reg_and_test(regman.get());

    regman->unblockRegistrar(reg_id, 777);

    BOOST_REQUIRE(regman->blockRegistrar(reg_id, epp_cli.get()) == false);

}

BOOST_AUTO_TEST_CASE( test_block_again_no_special_values_used_in_interval_definition )
{
    init_corba_container();
    std::auto_ptr<Fred::Registrar::Manager> regman(
                         Fred::Registrar::Manager::create(DBDisconnectPtr(NULL)));
    std::auto_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());

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
    std::auto_ptr<Fred::Registrar::Manager> regman(
                         Fred::Registrar::Manager::create(DBDisconnectPtr(NULL)));
    std::auto_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());

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


BOOST_AUTO_TEST_SUITE_END();
