/*
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *  (at your option) any later version.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <boost/thread/thread.hpp>
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include "src/libfred/db_settings.hh"
#include "src/libfred/invoicing/invoice.hh"
#include "src/libfred/banking/bank_manager.hh"
// #include "bank.h"
#include "src/libfred/mail.hh"
#include "src/deprecated/model/model_filters.hh"

// #define CONNECTION_STRING       "host=localhost dbname=fred user=fred port=6655"
#define CONNECTION_STRING       "host=localhost dbname=fred user=fred"


void
bank_import_xml(const std::string &_xmlfile)
{
    using namespace LibFred::Banking;
    ManagerPtr bmanager(Manager::create(0));

    std::ifstream file(_xmlfile.c_str(), std::ios::in);
    if (file.is_open())
        bmanager->importStatementXml(file, "");
}

void
bank_pair_payment_with_statement()
{
    using namespace LibFred::Banking;
    ManagerPtr bmanager(Manager::create(0));
    bmanager->pairPaymentWithStatement(8690, 0, true);
}

void
bank_payment()
{
    using namespace LibFred::Banking;
    ManagerPtr bmanager(Manager::create(0));
    PaymentListPtr plist(bmanager->createPaymentList());

    Database::Filters::BankPayment *pf = new Database::Filters::BankPaymentImpl();
    pf->addConstSymb().setValue("0308");

    Database::Filters::Union uf;
    uf.addFilter(pf);
    plist->reload(uf);
    std::cout << plist->getSize() << std::endl;
    if (plist->size()) {
        std::cout << plist->get(0)->toString() << std::endl;
    }
}

/*
void
bank_statement()
{
    using namespace LibFred::Banking;
    ManagerPtr bmanager(Manager::create());
    StatementListPtr slist(bmanager->createStatementList());

    Database::Filters::StatementHead *sf = new Database::Filters::StatementHeadImpl();

    Database::Filters::Union uf;
    uf.addFilter(sf);
    slist->reload(uf);
    std::cout << slist->getSize() << std::endl;
    if (slist->size()) {
        std::cout << slist->get(0)->toString() << std::endl;
    }
}
*/


int main(int argc, char **argv)
{
    boost::any param;
    param = static_cast<unsigned int>(1);

    // LT_FILE, LT_SYSLOG, LT_CONSOLE
    Logging::Manager::instance_ref().get(PACKAGE).addHandler(Logging::Log::LT_CONSOLE, param);
    Logging::Manager::instance_ref().get(PACKAGE).setLevel(Logging::Log::LL_DEBUG);

    Database::Manager::init(new Database::ConnectionFactory(CONNECTION_STRING, 1, 10));

    std::string file;
    if (argc == 2) {
        file = argv[1];
    }
    else {
        return 1;
    }
    bank_import_xml(file);
    // bank_pair_payment_with_statement();
    // bank_payment();
    // bank_statement();

    // boost::thread_group threads;
    // threads.create_thread(&);
    // threads.create_thread(&);
    // threads.create_thread(&);
    // threads.create_thread(&);
    // threads.join_all();
    return 0;
}

