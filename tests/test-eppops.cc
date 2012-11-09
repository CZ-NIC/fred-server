/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include <boost/function.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "setup_server_decl.h"
#include "time_clock.h"
#include "fredlib/registrar.h"
#include "fredlib/domain/update_domain.h"
#include "fredlib/nsset/update_nsset.h"
#include "fredlib/keyset/update_keyset.h"
#include "fredlib/contact/delete_contact.h"
#include "util/util.h"

#include "fredlib/db_settings.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestEPPops)

const std::string server_name = "test-eppops";

BOOST_AUTO_TEST_CASE( test_eppops )
{
    //db
    Database::Connection conn = Database::Manager::acquire();
    boost::shared_ptr<Database::Transaction> txp ( new Database::Transaction(conn));
    //Database::Transaction tx(conn);
    Database::Result res = conn.exec("select 1");
    txp->commit();
    BOOST_CHECK_EQUAL(std::string("1").compare(std::string(res[0][0])),0);

    //opctx
    Fred::OperationContext opctx;
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_authinfo("testauth").exec(opctx);
    Fred::UpdateNsset("fred.cz", "REG-FRED_A").set_authinfo("testauth").add_dns(Fred::DnsHost("host", Util::vector_of<std::string>("127.0.0.1")("127.1.1.1"))).exec(opctx);
    Fred::UpdateKeyset("fred.cz", "REG-FRED_A").set_authinfo("testauth").add_dns_key(Fred::DnsKey(0,1,2,"key")).exec(opctx);
    Fred::DeleteContact("KONTAKT").exec(opctx);
    opctx.commit_transaction();

}

BOOST_AUTO_TEST_SUITE_END();//TestEPPops
