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
#include "db_settings.h"
#include "file.h"
#include "invoice_manager.h"
#include "bank_manager.h"
// #include "bank.h"
#include "mail.h"
#include "model/model_filters.h"

// #define CONNECTION_STRING       "host=localhost dbname=fred user=fred port=6655"
#define CONNECTION_STRING       "host=localhost dbname=fred user=fred"


void
bank_import_xml(const std::string &_xmlfile)
{
    using namespace Register::Banking;
    ManagerPtr bmanager(Manager::create());

    std::ifstream file(_xmlfile.c_str(), std::ios::in);
    if (file.is_open())
        bmanager->importStatementXml(file, "");
}

void
bank_pair_payment_with_statement()
{
    using namespace Register::Banking;
    ManagerPtr bmanager(Manager::create());
    bmanager->pairPaymentWithStatement(8690, 0, true);
}

void
bank_payment()
{
    using namespace Register::Banking;
    ManagerPtr bmanager(Manager::create());
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
    using namespace Register::Banking;
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

void
file_2()
{
    std::auto_ptr<Register::File::Manager> fileMan(
            Register::File::Manager::create());
    std::auto_ptr<Register::File::List> fileList(fileMan->createList());

    Database::Filters::File *filFilter;
    filFilter = new Database::Filters::FileImpl(true);

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    unionFilter->addFilter(filFilter);
    fileList->reload(*unionFilter);
    std::cout << fileList->getSize() << std::endl;
    for (int i = 0; i < (int)fileList->getSize(); i++) {
        Register::File::File *file = fileList->get(i);
        std::cout << file->getName() << ", " << file->getPath() << std::endl;
    }
}

void
invoice()
{
    std::auto_ptr<Register::Invoicing::Manager> invMan(
            Register::Invoicing::Manager::create());
    std::auto_ptr<Register::Invoicing::List> invList(invMan->createList());

    Database::Filters::Invoice *invFilter = new Database::Filters::InvoiceImpl();
    Database::Filters::Union *unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(invFilter);
    invList->reload(*unionFilter);
    std::cout << invList->getSize() << std::endl;
    for (int i = 0; i < (int)invList->getSize(); i++) {
        Register::Invoicing::Invoice *invoice = invList->get(i);
        std::cout //<< invoice->getZone()->getFqdn() << ", "
            << invoice->getCrDate() << ", " << invoice->getRegistrarId() << ": "
            //<< invoice->getRegistrar()->getHandle()
            << std::endl;
    }

}

void
deposit_invoice()
{
    std::auto_ptr<Register::Invoicing::Manager> invMan(
            Register::Invoicing::Manager::create());
    std::auto_ptr<Register::Invoicing::Invoice> invoice(
            invMan->createDepositInvoice());

    // ModelZone *zone = new ModelZone();
    // zone->setId(3);
    // zone->setFqdn("cz");
    // invoice->setZone(zone);
    // invoice->setZone(zone);
    invoice->setZoneId(3);
    invoice->setRegistrarId(1);
    invoice->setPrice(Database::Money("14000"));
    // invoice->setTaxDate(Database::Date(Database::NOW));
    invoice->save();
}

#if 0
void
pair_invoices()
{
    std::auto_ptr<Register::Invoicing::Manager> invMan(
            Register::Invoicing::Manager::create());
    invMan->pairInvoices();
}

#endif

void
factoring(unsigned long long id)
{
    std::auto_ptr<Register::Invoicing::Manager> invMan(
            Register::Invoicing::Manager::create());
    std::auto_ptr<Register::Invoicing::Invoice>
        invoice(invMan->createAccountInvoice());
    invoice->setRegistrarId(id);
    // invoice->setZoneName("cz");
    invoice->setZoneId(3);
    invoice->setToDate("2009-04-20");
    invoice->setTaxDate("2009-04-20");
    invoice->save();
}

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
    // threads.create_thread(&deposit_invoice);
    // threads.create_thread(&deposit_invoice);
    // threads.create_thread(&deposit_invoice);
    // threads.create_thread(&deposit_invoice);
    // threads.join_all();
    return 0;
}

