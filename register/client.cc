#include "dbsql.h"
#include "nameservice.h"
#include "mailer_manager.h"
#include "auth_info.h"
#include "invoice.h"
#include "bank.h"

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>

namespace po = boost::program_options;
using namespace boost::posix_time;
 
class CorbaClient
{
  CORBA::ORB_var orb;
  std::auto_ptr<NameService> ns;
 public:
  CorbaClient(int argc, char **argv, const std::string& nshost) 
  {
    orb = CORBA::ORB_init(argc, argv);
    ns.reset(new NameService(orb,nshost));    
  }
  ~CorbaClient()
  {
    orb->destroy();
  }
  NameService *getNS()
  {
    return ns.get();
  }
};

int main(int argc, char **argv)
{
  DB db;
  bool connected = false;
  // to avoid valgrind showing memory leak because std::cout don't delete
  // imbued facets properly, using own created output stream
  std::ofstream stdout("/dev/stdout",std::ios::out);   
  try {
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help", 
       "this help")
      ("conf", po::value<std::string>()->default_value("/etc/fred/server.conf"),
       "configuration file")
      ("lang", po::value<std::string>()->default_value("cs"), 
       "language of communication");

    po::options_description configDesc("Config options");
    configDesc.add_options()
      ("dbname", po::value<std::string>()->default_value("fred"), 
       "database name")
      ("user", po::value<std::string>()->default_value("fred"), 
       "database user")
      ("password", po::value<std::string>(), 
       "database password")
      ("host", po::value<std::string>(),
       "database host")
      ("port", po::value<unsigned>(), 
       "database port")
      ("nameservice", po::value<std::string>()->default_value("localhost"), 
       "corba name service host")
      ("docgen_path",po::value<std::string>()->default_value(
        ""),
       "path to fred2pdf document generator")
      ("fileclient_path",po::value<std::string>()->default_value(
        ""),
       "path to file manager corba client")
      ("connect_timeout", po::value<unsigned>(),
       "timeout for trying to connect to database")
      ("log_mask", po::value<unsigned>(),
       "obsolete config option")
      ("log_level", po::value<std::string>(),
       "syslog level")
      ("log_local", po::value<unsigned>(),
       "syslog facilty is local#")
      ("session_max", po::value<unsigned>(),
       "maximal number of concurrent EPP sessions")
      ("session_wait", po::value<unsigned>(),
       "maximal length of EPP session")
      ("ebanka_url", po::value<std::string>(),
       "web url for download of incoming payments");

    po::options_description invoiceDesc("Invoicing options");
    invoiceDesc.add_options()
      ("invoice_archive", 
       "archive unarchived invoices")
      ("invoice_list", 
       "list xml of invoices according to specified filter")
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
      ("object_name", po::value<std::string>(),
       "object name")      
      ("adv_number", po::value<std::string>(),
       "advance invoice number");

    po::options_description aiDesc("AuthInfo options");
    aiDesc.add_options()
      ("authinfopdf", po::value<unsigned>(),
       "generate pdf of auth into request");

    po::options_description bankDesc("Banking options");
    bankDesc.add_options()
      ("bank_online_list", 
       "list of online payments")
      ("bank_statement_list", 
       "list of bank statements");

    po::variables_map vm;
    // parse help and config filename options
    po::store(
      po::parse_command_line(
        argc, argv, 
        desc.add(configDesc).add(invoiceDesc).add(aiDesc).add(bankDesc)
      ), vm
    );
    // parse options from config file
    std::ifstream configFile(vm["conf"].as<std::string>().c_str());
    po::store(po::parse_config_file(configFile, configDesc), vm);

    if (vm.count("help") || argc == 1 ) {
      stdout << desc << "\n";
      return 1;
    }
    
    std::stringstream connstring;
    connstring << "dbname=" << vm["dbname"].as<std::string>() 
               << " user=" << vm["user"].as<std::string>();
    if (vm.count("host"))
      connstring << " host=" << vm["host"].as<std::string>();
    if (vm.count("port"))
      connstring << " port=" << vm["port"].as<unsigned>();
    if (vm.count("password"))
      connstring << " password=" << vm["password"].as<std::string>();
                
    if (!db.OpenDatabase(connstring.str().c_str())) {
      stdout << "Database connection error (" << connstring.str() << ")\n";
      return -1;
    }
    connected = true;
    
    // invoicing
    std::auto_ptr<Register::Document::Manager> docman(
      Register::Document::Manager::create(
        vm["docgen_path"].as<std::string>(),
        vm["fileclient_path"].as<std::string>(),
        vm["nameservice"].as<std::string>()
      )
    );
    CorbaClient cc(argc,argv,vm["nameservice"].as<std::string>());
    MailerManager mm(cc.getNS());    
    std::auto_ptr<Register::Invoicing::Manager> im(
      Register::Invoicing::Manager::create(&db,docman.get(),&mm)
    );
    if (vm.count("invoice_archive")) {
      im->archiveInvoices();
    }
    if (vm.count("invoice_list")) {
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
        il->setObjectIdFilter(vm["object_id"].as<unsigned>());
      if (vm.count("object_name")) 
        il->setObjectNameFilter(vm["object_name"].as<std::string>());
      if (vm.count("adv_number")) 
        il->setAdvanceNumberFilter(vm["adv_number"].as<std::string>());    
      il->reload();
      
      il->exportXML(stdout);
    }
    
    // authinfo
    if (vm.count("authinfopdf")) {
      std::auto_ptr<Register::AuthInfoRequest::Manager> authman(
        Register::AuthInfoRequest::Manager::create(&db,&mm,docman.get())
      );
      authman->getRequestPDF(
        vm["authinfopdf"].as<unsigned>(),
        vm["lang"].as<std::string>(),
        stdout
      );
    }
    
    // banking
    std::auto_ptr<Register::Banking::Manager> bankMan(
      Register::Banking::Manager::create(&db)
    );
    if (vm.count("bank_online_list")) {
      std::auto_ptr<Register::Banking::OnlinePaymentList> onlineList(
        bankMan->createOnlinePaymentList()
      );
      onlineList->reload();
      onlineList->exportXML(stdout);
    }
    if (vm.count("bank_statement_list")) {
      std::auto_ptr<Register::Banking::StatementList> stList(
        bankMan->createStatementList()
      );
      stList->reload();
      stList->exportXML(stdout);
    }
    if (connected) db.Disconnect();
  }
  catch (std::exception& e) {
    stdout << e.what() << "\n";
    if (connected) db.Disconnect();
  }
  catch (Register::SQL_ERROR) {
    stdout << "SQL ERROR \n";
    if (connected) db.Disconnect();
  }
}
