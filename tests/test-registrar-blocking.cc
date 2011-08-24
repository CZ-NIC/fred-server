#include <boost/test/unit_test.hpp>
#include <iostream>

#include "fredlib/db_settings.h"
#include "old_utils/dbsql.h"
#include "registrar.h"

#include "cfg/config_handler_decl.h"
#include "cfg/handle_general_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"

#include "epp_corba_client_impl.h"

BOOST_AUTO_TEST_SUITE(TestRegistrarBlocking)

BOOST_AUTO_TEST_CASE( test_block_registrar )
{
     DBSharedPtr m_db = connect_DB(
             CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info()
             , std::runtime_error("NotifyClient db connection failed"));

    std::auto_ptr<Fred::Registrar::Manager> regMan(
            Fred::Registrar::Manager::create(m_db));

    //corba config
    FakedArgs orb_fa = CfgArgs::instance()->fa;

    //conf pointers
    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();

    CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
            , ns_args_ptr->nameservice_host
            , ns_args_ptr->nameservice_port
            , ns_args_ptr->nameservice_context);

    std::auto_ptr<EppCorbaClientImpl> epp_cli (new EppCorbaClientImpl());
    regMan->blockRegistrar(1, epp_cli.get());

    //std::cout << "Successfully blocked " << std::endl;

    // regMan->unblockRegistrar(1, 0);

}

BOOST_AUTO_TEST_SUITE_END();
