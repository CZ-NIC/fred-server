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
#include <memory>
#include <iostream>
#include <string>
#include "bank.h"
#include "bank.h"
#include "invoice.h"
#include "db/database.h"
#include "file.h"
// #include "old_utils/dbsql.h"

void
create_deposit_invoice()
{
    std::auto_ptr<Database::Manager> dbMan(
            new Database::Manager(
                new Database::ConnectionFactory("host=localhost dbname=fred user=fred port=22345")));
    std::auto_ptr<Register::Invoicing::Manager>
        invMan(Register::Invoicing::Manager::create(dbMan.get()));
    Register::Invoicing::Invoice *invoice = invMan->createDepositInvoice();
    invoice->setZone(Database::ID(3));
    invoice->setZoneName("cz");
    invoice->setRegistrar(Database::ID(2));
    invoice->setPrice(Database::Money(4800));
    invoice->setTaxDate("2009-01-12");
    invoice->save();
}
void
create_account_invoice()
{
    std::auto_ptr<Database::Manager> dbMan(
            new Database::Manager(
                new Database::ConnectionFactory("host=localhost dbname=fred user=fred port=22345")));
    std::auto_ptr<Register::Invoicing::Manager>
        invMan(Register::Invoicing::Manager::create(dbMan.get()));
    Register::Invoicing::Invoice *invoice = invMan->createAccountInvoice();
    invoice->setRegistrar(Database::ID(1));
    invoice->setZone(Database::ID(3));
    invoice->setTaxDate("2009-01-12");
    invoice->setToDate("2009-01-12");
    invoice->save();
}
void
import_xml_invoice()
{
    std::auto_ptr<Database::Manager> dbMan(
            new Database::Manager(
                new Database::ConnectionFactory("host=localhost dbname=fred user=fred port=22345")));
    std::auto_ptr<Register::Banking::Manager>
        bankMan(Register::Banking::Manager::create(dbMan.get()));
    std::ifstream stdin("/dev/stdin", std::ios::in);
    bankMan->importOnlineInvoiceXml(stdin);
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
    // Database::Manager *dbMan = new Database::Manager("host=localhost dbname=fred user=fred port=22345");
    // std::auto_ptr<Database::Manager> dbMan(new Database::Manager(""));
    import_xml_invoice();

    // create_deposit_invoice();
    // create_account_invoice();
    // std::auto_ptr<Database::Manager> dbMan(
            // new Database::Manager(
                // new Database::ConnectionFactory("host=localhost dbname=fred user=fred port=22345")));
// 
    // std::auto_ptr<Register::Invoicing::Manager>
        // invMan(Register::Invoicing::Manager::create(dbMan.get()));
    // Register::Invoicing::Invoice *invoice = invMan->createInvoice(Register::Invoicing::IT_NONE);

    // invoice->setZone(Database::ID(3));
    // invoice->setRegistrar(Database::ID(1));
    // invoice->addAction(Register::Invoicing::PAT_CREATE_DOMAIN, Database::ID(123));

    // invoice->updateInvoiceCredit(Database::ID(3), Database::ID(1), Database::ID(125),
            // Database::Date("2010-01-01"), 12);

    // std::vector<Database::Money> bla;
    // bla.push_back(Database::Money(100));
    // bla.push_back(Database::Money(200));
    // bla.push_back(Database::Money(100));
// 
    // std::vector<Database::Money> sakra = invoice->countPayments(bla, Database::Money(400));
    // for (int i = 0; i < sakra.size(); i++) {
        // std::cout << sakra[i] << std::endl;
    // }

    // std::auto_ptr<Register::Invoicing::List> invList(invMan->createList());
// 
    // Database::Filters::Invoice *invFilter = new Database::Filters::InvoiceImpl();
    // Database::Filters::Union *unionFilter = new Database::Filters::Union();
    // unionFilter->addFilter(invFilter);
    // invList->setPartialLoad(true);
#if 0
#define print(bla)  std::cout << bla << std::endl

    invList->reload(*unionFilter);
    for (int i = 0; i < (int)invList->getCount(); i++) {
        Register::Invoicing::Invoice *invoice = invList->get(i);
        std::cout << "id:       " << invoice->getId() << std::endl;
        std::cout << "zoneId:   " << invoice->getZone() << std::endl;
        std::cout << "zone:     " << invoice->getZoneName() << std::endl;
        std::cout << "crtime:   " << invoice->getCrTime() << std::endl;
        std::cout << "taxdate:  " << invoice->getTaxDate() << std::endl;
        std::cout << "registrar:" << invoice->getRegistrar() << std::endl;
        std::cout << "credit:   " << invoice->getCredit() << std::endl;
        std::cout << "price:    " << invoice->getPrice() << std::endl;
        std::cout << "vatRate:  " << invoice->getVatRate() << std::endl;
        std::cout << "total:    " << invoice->getTotal() << std::endl;
        std::cout << "totalVAT: " << invoice->getTotalVAT() << std::endl;
        std::cout << "pdfId:    " << invoice->getFilePDF() << std::endl;
        std::cout << "xmlId:    " << invoice->getFileXML() << std::endl;
        std::cout << "pdfName:  " << invoice->getFileNamePDF() << std::endl;
        std::cout << "xmlName:  " << invoice->getFileNameXML() << std::endl;
        for (int j = 0; j < (int)invoice->getSourceCount(); j++) {
            Register::Invoicing::PaymentSource *source =
                const_cast<Register::Invoicing::PaymentSource *>(invoice->getSource(j));
            assert(source);
            print("<< PaymentSource >>");
            print(" id:             " << source->getId());
            print(" number:         " << source->getNumber());
            print(" credit:         " << source->getCredit());
            print(" totalPrice:     " << source->getTotalPrice());
            print(" totalVat:       " << source->getTotalVat());
            print(" totalPriceWVat: " << source->getTotalPriceWithVat());
            print(" getCrTime:      " << source->getCrTime());
            print(" price:          " << source->getPrice());
            print(" vatRate:        " << source->getVatRate());
            print(" vat:            " << source->getVat());
            print(" priceWithVat:   " << source->getPriceWithVat());
        }
        for (int j = 0; j < (int)invoice->getActionCount(); j++) {
            Register::Invoicing::PaymentAction *action =
                const_cast<Register::Invoicing::PaymentAction *>(invoice->getAction(j));
            assert(action);
            print("<< PaymentAction >>");
            print(" id:             " << action->getId());
            print(" objectName:     " << action->getObjectName());
            print(" actionTime:     " << action->getActionTime());
            print(" exDate:         " << action->getExDate());
            print(" action:         " << action->getAction());
            print(" unitsCount:     " << action->getUnitsCount());
            print(" pricePerUnit:   " << action->getPricePerUnit());
            print(" price:          " << action->getPrice());
            print(" vatRate:        " << action->getVatRate());
            print(" vat:            " << action->getVat());
            print(" priceWithVat:   " << action->getPriceWithVat());
        }
        for (int j = 0; j < (int)invoice->getPaymentCount(); j++) {
            Register::Invoicing::Payment *payment = 
                const_cast<Register::Invoicing::Payment *>(invoice->getPayment(j));
            print("<< Payment >>");
            print(" price:          " << payment->getPrice());
            print(" vatRate:        " << payment->getVatRate());
            print(" vat:            " << payment->getVat());
            print(" priceWithVat:   " << payment->getPriceWithVat());
        }
        std::cout << "------------------------------------------------" << std::endl;
    }


#endif 








#if 0

    std::auto_ptr<Register::Banking::Manager> bankMan(
            Register::Banking::Manager::create(dbMan));
    // std::auto_ptr<Register::Banking::StatementList> bankList(
            // bankMan->createStatementList());
    // std::auto_ptr<Register::Banking::List> bankList(bankMan->createList());
    // Register::Banking::Statement *stat = bankMan->createStatement();
    std::auto_ptr<Register::Banking::Statement> stat(bankMan->createStatement());
    stat->setAccountId(1);
    stat->setDate("2009-01-06");
    stat->setOldDate("2009-01-06");
    stat->setOldBalance(0);
    stat->setBalance(80000);
    stat->setCredit(80000);
    stat->setDebet(0);
    Register::Banking::StatementItem *item;
    item = stat->createStatementItem();
    item->setAccountNumber("7654321");
    item->setBankCode("0300");
    item->setCode(2);
    // item->setConstSymbol("22333");
    item->setPrice(80000);
    item->setDate("2009-01-06");
    // item = stat->createStatementItem();
    // item->setAccountNumber("987654321");
    // item->setBankCode("0100");
    // item->setCode(2);
    // item->setConstSymbol("122333");
    // item->setPrice(4000);
    // item->setDate("2008-12-02");
    stat->save();
#endif
#if 0
    std::auto_ptr<Register::Banking::List> bankList(
            bankMan->createList());
    Database::Filters::Statement *statFilter = new Database::Filters::StatementImpl();
    Database::Filters::Union *unionFilter = new Database::Filters::Union();
    unionFilter->addFilter(statFilter);
    bankList->reload(*unionFilter);
    for (int i = 0; i < bankList->getCount(); i++) {
        Register::Banking::Statement *stat = bankList->get(i);
        std::cout << stat->getId() << std::endl;
        std::cout << stat->getDate() << std::endl;
        for (int j = 0; j < stat->getStatementItemCount(); j++) {
            Register::Banking::StatementItem *item =
                (Register::Banking::StatementItem *)stat->getStatementItemByIdx(j);
            std::cout << "\t" << item->getId() << std::endl;
            std::cout << "\t" << item->getStatementId() << std::endl;
            std::cout << "\t" << item->getAccountNumber() << std::endl;
            std::cout << "\t" << item->getPrice() << std::endl;
            std::cout << "\t" << item->getPrice().to_string() << std::endl;
            std::cout << "----------------\n" << std::endl;
        }
        std::cout << "----------------" << std::endl;
    }
#endif
#if 0
    Register::Banking::Statement *stat = bankList->get(2);
    stat->setDate(Database::Date("2008-12-04"));
    stat->setOldDate("2008-12-03");
    for (int i = 0; i < stat->getStatementItemCount(); i++) {
        Register::Banking::StatementItem *item = 
            (Register::Banking::StatementItem *)stat->getStatementItemByIdx(i);
        item->setDate(Database::Date("2008-12-04"));
        item->setConstSymbol("qwerty");
    }

    stat->save();
#endif

    // delete bankMan2;
    // delete stat;
#if 0
    std::auto_ptr<Register::Banking::OnlineList> bankList(
            bankMan->createOnlineList());
    Database::Filters::OnlineStatement *statFilter = new Database::Filters::OnlineStatementImpl();
    Database::Filters::Union *unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(statFilter);
    bankList->reload(*unionFilter);

    // for (int i = 0; i < bankList->getCount(); i++) {
        // Register::Banking::OnlineStatement *stat = bankList->get(i);
        // std::cout << stat->getCrDate() << std::endl;
        // std::cout << stat->getInvoiceId() << std::endl;
        // std::cout << "---------------------" << std::endl;
    // }
    Register::Banking::OnlineStatement *stat = bankList->get(0);
    stat->setCrDate(Database::DateTime("2008-12-02 13:10:00.44423"));
    stat->save();
#endif

#if 0
    Register::Banking::OnlineStatement *stat = bankMan->createOnlineStatement();
    stat->setAccountId(Database::ID(1));
    stat->setPrice(Database::Money(910000));
    stat->setCrDate(Database::DateTime("2008-11-28 10:27:00.3"));
    // stat->setAccountNumber("81734892");
    stat->setBankCode("5500");
    // stat->setConstSymbol("");
    // stat->setVarSymbol("");
    // stat->setMemo("");
    // stat->setAccountName("");
    stat->setIdent("82129011");
    // stat->setInvoiceId(Database::ID(7));
    stat->save();
    std::cout << "s Ende" << std::endl;
#endif 
#if 0

    Database::Filters::Statement *statementFilter = new Database::Filters::StatementImpl();
    Database::Filters::Union *unionFilter = new Database::Filters::Union();

    statementFilter->addAccountId().setValue(Database::ID(1));
    unionFilter->addFilter(statementFilter);
    bankList->reload(*unionFilter);


    for (int i = 0; i < bankList->getCount(); i++) {
        Register::Bank2::Statement *stat = bankList->get(i);
        std::cout << stat->getDate() << std::endl << stat->getBalance() << std::endl;
    }
#endif
// 
    // std::auto_ptr<Register::Banking::Manager> bankMan(
            // Register::Banking::Manager::create(dbMan));
    // std::auto_ptr<Register::Banking::StatementList> bankList(
            // bankMan->createStatementList());
// 
    // Database::Filters::Statement *statementFilter = new Database::Filters::StatementImpl();
    // Database::Filters::Union *unionFilter = new Database::Filters::Union();
// 
    // statementFilter->addAccountId().setValue(Database::ID(1));

    // std::auto_ptr<Register::File::Manager> fileMan(
            // Register::File::Manager::create(dbMan));
    // std::auto_ptr<Register::File::List> fileList(
            // fileMan->createList());
// 
// 
    // std::auto_ptr<Register::File::File> file(
            // fileMan->createFile(dbMan->getConnection()));
// 
    // file->setName("pokusnej.xml");
    // file->setPath("/home/adolezal/sakrapes");
    // file->setMimeType("application/xml");
    // file->setType(1);
    // file->setSize(123445);
    // file->save(dbMan->getConnection());
#if 0
    Database::Filters::File *fileFilter = new Database::Filters::FileImpl();

    Database::Filters::Union *unionFilter = new Database::Filters::Union();

    fileFilter->addType().setValue(1);

    unionFilter->addFilter(fileFilter);

    fileList->reload(*unionFilter);

    for (unsigned int i = 0; i < fileList->getCount(); i++) {
        Register::File::File *file = fileList->get(i);
        std::cout
            << "<file>\n"
            << "\t<id>" << file->getId() << "</id>\n"
            << "\t<name>" << file->getName() << "</name>\n"
            << "\t<path>" << file->getPath() << "</path>\n"
            << "\t<mime>" << file->getMimeType() << "</mime>\n"
            << "\t<type>" << file->getType() << "</type>\n"
            << "\t<type_desc>" << file->getTypeDesc() << "</type_desc>\n"
            << "\t<crdate>" << file->getCreateTime() << "</crdate>\n"
            << "\t<size>" << file->getSize() << "</size>\n"
            << "</file>\n";
    }

    unionFilter->clear();
    // delete fileFilter;
    delete unionFilter;
#endif
    // delete dbMan;
  // DB db;
  //  db.OpenDatabase("host=localhost dbname=ccreg user=postgres");
  // db.OpenDatabase("host=curlew dbname=ccreg user=ccreg");
  // db.OpenDatabase("host=localhost dbname=fred user=fred");
  // std::auto_ptr<Register::Manager> m(Register::Manager::create(&db, true));
  // Register::Registrar::Manager *rm = m->getRegistrarManager();
/*
  Register::Registrar::RegistrarList *rl2 = rm->getList();
  //  rl2->setIdFilter(2);
  rl2->reload();
  for (unsigned i=0; i<rl2->size(); i++)
    std::cout << "R:" << rl2->get(i)->getHandle() 
              << " C:" << rl2->get(i)->getCredit()
              << std::endl;
  */
  /*
  Register::Registrar::Registrar *r = rl2->get(0);
  r->setName("Name2");
  r->setURL("URL");
  r->save();
  /// -=-=-=-=-=-
  /// DOMAINS
  /// -=-=-=-=-=-
  //Register::Domain::List *dl = dm->getList();
  ///dl->reload();
  //for (unsigned i=0; i<dl->getCount(); i++)
///    std::cout << dl->get(i)->getFQDN() << std::endl;
  /// -=-=-=-=-=-
  /// ACTIONS
  /// -=-=-=-=-=-
  Register::Registrar::EPPActionList *eal = rm->getEPPActionList();
  eal->setRegistrarHandleFilter("REG");
  eal->setTimePeriodFilter(
    time_period(
      ptime(date(2006,1,1),time_duration(0,0,0)),
      ptime(date(2006,1,1),time_duration(24,0,0))
    )
  );
  eal->setReturnCodeFilter(1000);
  eal->setHandleFilter("d");
  eal->setTextTypeFilter("c");
  eal->setClTRIDFilter("b");
  eal->setSvTRIDFilter("a");      
  eal->reload();
  for (unsigned i=0; i<eal->size(); i++) {
    const Register::Registrar::EPPAction *a = eal->get(i);
    std::cout << "id:" << a->getType() 
              << " handle: " << a->getRegistrarHandle() << std::endl;
  }
  /// -=-=-=-=-=-
  /// REGISTARS
  /// -=-=-=-=-=-
  Register::Registrar::RegistrarList *rl = rm->getList();
  std::string filtr;
//  std::cout << "Registrar filter: ";
//  std::cin >> filtr;
  rl->setNameFilter(filtr);
  rl->reload();
  for (unsigned i=0; i<rl->size(); i++) {
    const Register::Registrar::Registrar *r = rl->get(i);
    std::cout << "id:" << r->getId() 
              << " handle: " << r->getHandle() << std::endl;
    for (unsigned j=0; j<r->getACLSize(); j++)
      std::cout << "  cert:" << r->getACL(j)->getCertificateMD5()
                << " pass:" << r->getACL(j)->getPassword()
                << std::endl;
  }
  /// -=-=-=-=-=-
  /// DOMAINS
  /// -=-=-=-=-=-
  Register::Domain::List *dl = dm->getList();
  dl->setRegistrarHandleFilter("REG");
  dl->setRegistrantHandleFilter("CID");
  dl->setCrDateIntervalFilter(
    time_period(
      ptime(date(2006,1,1),time_duration(0,0,0)),
      ptime(date(2006,1,1),time_duration(24,0,0))
    )
  );
  dl->setFQDNFilter("dom");
  dl->setAdminHandleFilter("a");
  dl->setNSSetHandleFilter("n");
  dl->reload();
  */
  // im->archiveInvoices();
#if 0
  while (1) {
    std::string input;
    std::cout << "Handle: ";
    std::cin >> input;
    Register::CheckHandleList chl;
    m->checkHandle(input,chl);
    if (chl.size() < 1) return -1;
    std::map<Register::CheckHandleClass,std::string> chcMap;
    chcMap[Register::CH_UNREGISTRABLE] = "CH_UNREGISTRABLE";
    chcMap[Register::CH_UNREGISTRABLE_LONG] = "CH_UNREGISTRABLE_LONG";
    chcMap[Register::CH_REGISTRED] = "CH_REGISTRED";
    chcMap[Register::CH_REGISTRED_PARENT] = "CH_REGISTRED_PARENT";
    chcMap[Register::CH_REGISTRED_CHILD] = "CH_REGISTRED_CHILD";
    chcMap[Register::CH_PROTECTED] = "CH_PROTECTED";
    chcMap[Register::CH_FREE] = "CH_FREE";
    
    std::map<Register::HandleType,std::string> htMap;
    htMap[Register::HT_ENUM_NUMBER] = "HT_ENUM_NUMBER";
    htMap[Register::HT_ENUM_DOMAIN] = "HT_ENUM_DOMAIN";
    htMap[Register::HT_DOMAIN] = "HT_DOMAIN";
    htMap[Register::HT_CONTACT] = "HT_CONTACT";
    htMap[Register::HT_NSSET] = "HT_NSSET";
    htMap[Register::HT_REGISTRAR] = "HT_REGISTRAR";
    htMap[Register::HT_OTHER] = "HT_OTHER";
      
    std::cout << "Result of checkHandle: \n";
    for (unsigned i=0; i< chl.size(); i++) {
      std::cout << i+1 << ".\n" 
                << " Type: " << htMap[chl[i].type] << std::endl
                << " Class: " << chcMap[chl[i].handleClass] << std::endl
                << " NewHandle: " << chl[i].newHandle << std::endl
                << " Conflict: " << chl[i].conflictHandle << std::endl;
    }
  }
#endif
    // delete dbMan;
  return 0;
}
