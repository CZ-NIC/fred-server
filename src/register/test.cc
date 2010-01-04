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

// #define CONNECTION_STRING       "host=localhost dbname=fred user=fred port=6655"
#define CONNECTION_STRING       "host=localhost dbname=onebank2 user=fred port=22345"


void
bank_item()
{
    std::auto_ptr<Register::Banking::Manager> bankMan(
            Register::Banking::Manager::create());

    std::auto_ptr<Register::Banking::ItemList> itemList(
            bankMan->createItemList());

    Database::Filters::StatementItem *itemFilter;
    itemFilter = new Database::Filters::StatementItemImpl();
    itemFilter->addConstSymb().setValue("0308");

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    unionFilter->addFilter(itemFilter);
    itemList->reload(*unionFilter);
    std::cout << itemList->getSize() << std::endl;
}

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

void
load_xml(bool create_deposit_invoice)
{
    std::auto_ptr<Register::Banking::Manager> bankMan(
            Register::Banking::Manager::create());

    std::ifstream input;
    input.open("/dev/stdin", std::ios::in);

    bankMan->importStatementXml(input, create_deposit_invoice);
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
    param = (unsigned int)1;

    // LT_FILE, LT_SYSLOG, LT_CONSOLE
    Logging::Manager::instance_ref().get("tracer").addHandler(Logging::Log::LT_CONSOLE, param);
    Logging::Manager::instance_ref().get("tracer").setLevel(Logging::Log::LL_TRACE);
    Logging::Manager::instance_ref().get("db").addHandler(Logging::Log::LT_CONSOLE, param);
    Logging::Manager::instance_ref().get("db").setLevel(Logging::Log::LL_TRACE);
    Logging::Manager::instance_ref().get("register").addHandler(Logging::Log::LT_CONSOLE, param);
    Logging::Manager::instance_ref().get("register").setLevel(Logging::Log::LL_TRACE);
    Logging::Manager::instance_ref().get("corba").addHandler(Logging::Log::LT_CONSOLE, param);
    Logging::Manager::instance_ref().get("corba").setLevel(Logging::Log::LL_TRACE);
    Logging::Manager::instance_ref().get("mailer").addHandler(Logging::Log::LT_CONSOLE, param);
    Logging::Manager::instance_ref().get("mailer").setLevel(Logging::Log::LL_TRACE);
    Logging::Manager::instance_ref().get("old_log").addHandler(Logging::Log::LT_CONSOLE, param);
    Logging::Manager::instance_ref().get("old_log").setLevel(Logging::Log::LL_TRACE);
    Logging::Manager::instance_ref().get("fred-server").addHandler(Logging::Log::LT_CONSOLE, param);
    Logging::Manager::instance_ref().get("fred-server").setLevel(Logging::Log::LL_TRACE);

    Database::Manager::init(new Database::ConnectionFactory(CONNECTION_STRING, 1, 10));
    boost::thread thdr1(boost::bind(&deposit_invoice));
    thdr1.join();

    // boost::thread_group threads;
    // threads.create_thread(&deposit_invoice);
    // threads.create_thread(&deposit_invoice);
    // threads.create_thread(&deposit_invoice);
    // threads.create_thread(&deposit_invoice);
    // threads.join_all();
    // deposit_invoice();
    // invoice();
    // file_2();
    // deposit_invoice();
    // load_xml(false);
    // bank_item();
    // pair_invoices();
    // factoring(1);
    // factoring(2);
    // factoring(3);
    // factoring(4);
    
    // insert_mail();
    // get_mail();
    // list_mail();
    return 0;
}

