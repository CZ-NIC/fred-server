#include "invoice.h"
#include "dbsql.h"
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>

namespace po = boost::program_options;
using namespace boost::posix_time;
 
int main(int argc, char **argv) 
{
  try {
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help", 
       "this help")
      ("dbhost", po::value<std::string>()->default_value("curlew"),
       "database host")
      ("dbname", po::value<std::string>()->default_value("ccreg"), 
       "database name")
      ("dbuser", po::value<std::string>()->default_value("ccreg"), 
       "database user");
       
    po::options_description invoiceDesc("Invoicing options");
    invoiceDesc.add_options()
      ("invarchive", 
       "archive unarchived invoices")
      ("docgen_path",po::value<std::string>()->default_value(
        "/home/jara/enum/fred2pdf/trunk"),
       "variable symbol of payment")
      ("id", po::value<unsigned>(),
       "invoice id")
      ("registrar_id", po::value<unsigned>(),
       "registrar id")
      ("zone_id", po::value<unsigned>(),
       "zone id")
      ("type", po::value<unsigned>(),
       "type (1=advance, 2=account)")
      ("var_symbol", po::value<std::string>(),
       "variable symbol of payment")
      ("number", po::value<std::string>(),
       "invoice number")
      ("crdate_from", po::value<std::string>(),
       "created after this date")
      ("crdate_to", po::value<std::string>(),
       "created before this date")
      ("taxdate_from", po::value<std::string>(),
       "with tax date after this date")
      ("taxdate_to", po::value<std::string>(),
       "with tax date before this date")
      ("archived", po::value<unsigned>(),
       "0=not archived 1=archived other=ignore")
      ("object_id", po::value<unsigned>(),
       "object id")      
      ("adv_number", po::value<std::string>(),
       "advance invoice number");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc.add(invoiceDesc)), vm);
    po::notify(vm); 
    if (vm.count("help") || argc==1 ) {
      std::cout << desc << "\n";
      return 1;
    }
    
    DB db;
    std::stringstream connstring;
    connstring << "host=" << vm["dbhost"].as<std::string>() 
               << " dbname=" << vm["dbname"].as<std::string>() 
               << " user=" << vm["dbuser"].as<std::string>();
    if (!db.OpenDatabase(connstring.str().c_str())) {
      std::cout << "Database connection error (" << connstring.str() << ")\n";
      exit(-1);
    }
   
    // invoicing
    std::auto_ptr<Register::Document::Manager> docman(
      Register::Document::Manager::create(vm["docgen_path"].as<std::string>())
    );  
    std::auto_ptr<Register::Invoicing::Manager> im(
      Register::Invoicing::Manager::create(&db,docman.get())
    );
    if (vm.count("invarchive")) {
      im->archiveInvoices();
      db.Disconnect();
      exit(0);
    }
    std::auto_ptr<Register::Invoicing::InvoiceList> il(im->createList());
    if (vm.count("id"))
      il->setIdFilter(vm["id"].as<unsigned>());
    if (vm.count("registrar_id")) 
      il->setRegistrarFilter(vm["registrar_id"].as<unsigned>());
    if (vm.count("zone_id")) 
      il->setZoneFilter(vm["zone_id"].as<unsigned>());
    if (vm.count("type")) 
      il->setTypeFilter(vm["type"].as<unsigned>());
    if (vm.count("var_symbol")) 
      il->setVarSymbolFilter(vm["var_symbol"].as<std::string>());
    if (vm.count("number")) 
      il->setNumberFilter(vm["number"].as<std::string>());
    ptime crDateFrom(neg_infin);
    if (vm.count("crdate_from"))
      crDateFrom = time_from_string(vm["crdate_from"].as<std::string>()); 
    ptime crDateTo(pos_infin);
    if (vm.count("crdate_to"))
      crDateTo = time_from_string(vm["crdate_to"].as<std::string>());
    il->setCrDateFilter(time_period(crDateFrom,crDateTo)); 
    ptime taxDateFrom(neg_infin);
    if (vm.count("taxdate_from"))
      taxDateFrom = time_from_string(vm["taxdate_from"].as<std::string>()); 
    ptime taxDateTo(pos_infin);
    if (vm.count("taxdate_to"))
      taxDateTo = time_from_string(vm["taxdate_to"].as<std::string>());
    il->setTaxDateFilter(time_period(taxDateFrom,taxDateTo)); 
    if (vm.count("archived")) {
      Register::Invoicing::InvoiceList::ArchiveFilter af;
      switch (vm["archived"].as<unsigned>()) { 
        case 0: af = Register::Invoicing::InvoiceList::AF_UNSET; break;
        case 1: af = Register::Invoicing::InvoiceList::AF_SET; break;
        default: af = Register::Invoicing::InvoiceList::AF_IGNORE; break;
      }
      il->setArchivedFilter(af);
    }
    if (vm.count("object_id")) 
      il->setObjectFilter(vm["object_id"].as<unsigned>());
    if (vm.count("adv_number")) 
      il->setAdvanceNumberFilter(vm["adv_number"].as<std::string>());    
    il->reload();
    il->exportXML(std::cout);
    
    db.Disconnect();
  }
  catch (std::exception& e) {
    std::cout << e.what() << "\n";
  }
  catch (Register::SQL_ERROR) {
    std::cout << "SQL ERROR \n";
  }
}
