
#include <boost/assign/list_of.hpp>

#include "config.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"


#define BOOST_TEST_MODULE TestInvoicing

// #include <boost/test/included/unit_test.hpp>


HandlerPtrVector global_hpv =
boost::assign::list_of
(HandleArgsPtr(new HandleGeneralArgs))
(HandleArgsPtr(new HandleServerArgs))
(HandleArgsPtr(new HandleLoggingArgs))
(HandleArgsPtr(new HandleDatabaseArgs))

;

// (HandleArgsPtr(new HandleInvoiceTestDatabaseArgs))

// TODO use this custom main
#include "cfg/test_custom_main.h"
// #include "tests-common.h"


#include "invoicing/invoice.h"

using namespace Database;

// TODO
//CfgArgs::instance<HandleParseDatabaseArgs>(global_hpv)

/**
 * check newly created deposit invoice:
 *  * VAT must correspond to taxdate
 *  * prices must match VAT & inserted price
 */
bool checkDepositInvoice(Connection &conn, Database::ID id, const Database::Date &taxdate, int price)
{
    Result res = conn.exec_params("SELECT * from invoice where id = $1::bigint",
            Database::query_param_list( id ) );

    if(res.size() != 1) BOOST_FAIL("Did not find the invoice");
   
    return true;
}

/**
 * get actual id sequence value using nextval
 */
Database::ID get_actual_id(Connection &conn, const std::string &table_name)
{
    Result res_id =
            conn.exec( boost::format("SELECT nextval('%1%_id_seq'::regclass) ") % table_name  );

    return res_id[0][0];
}


const std::string LOG_FILE_NAME("log_test_invoicing.txt");

struct InvFixture {

    InvFixture() {
        Logging::Manager::instance_ref().get(PACKAGE).addHandler(Logging::Log::LT_FILE, std::string(LOG_FILE_NAME));
        Logging::Manager::instance_ref().get(PACKAGE).setLevel(Logging::Log::LL_TRACE);
        LOGGER(PACKAGE).info("Logging initialized");
    }

    ~InvFixture() { }
};


BOOST_GLOBAL_FIXTURE( InvFixture );

BOOST_AUTO_TEST_SUITE(TestInvoicing)


// these 'constants' should be initialized from database ....
int ZONE_CC=1;
int ZONE_ENUM=2;

int REG_STD = 1; // REG_FRED_A
int REG_SYS = 3; // REG_FRED_B
// int REG_NOVAT =      // not present yet

BOOST_AUTO_TEST_CASE(Test)
{
    std::auto_ptr<Fred::Invoicing::Manager> man (Fred::Invoicing::Manager::create());

    LOGGER(PACKAGE).debug ( "Test case for createDepositInvoice");

    Database::Date taxdate(2011,1,31);  // some other date - maybe close to VAT change
    int zone = ZONE_CC;  // | ZONE_ENUM
    int registrar = REG_STD;   //  | REG_SYSTEM

    Connection conn = Manager::acquire();

    Database::ID act_id = get_actual_id(conn, "invoice");

    man->createDepositInvoice(taxdate, zone, registrar, 10000);

    checkDepositInvoice(conn, act_id+1, taxdate, 10000);

}


BOOST_AUTO_TEST_SUITE_END(); // TestInvoicing

