#include "dbsql.h"
#include "nameservice.h"
#include "mailer_manager.h"
#include "auth_info.h"
#include "invoice.h"
#include "bank.h"
#include "info_buffer.h"
#include "log.h"
#include "poll.h"

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
        "/usr/bin/fred-doc2pdf"),
       "path to fred2pdf document generator")
      ("docgen_template_path",po::value<std::string>()->default_value(
        "/usr/share/fred2pdf/templates/"),
       "path to fred2pdf document generator templates")
      ("fileclient_path",po::value<std::string>()->default_value(
        "/usr/bin/filemanager_client"),
       "path to file manager corba client")
      ("log_level",po::value<unsigned>()->default_value(DEBUG_LOG),
       "minimal level of logging")
      ("log_local",po::value<unsigned>()->default_value(2),
       "syslog local facility number")
      ("restricted_handles",po::value<unsigned>()->default_value(1),
       "restricted format of handles (CID: & NSSID:)");

    po::options_description fileDesc(""); 
    fileDesc.add_options()
      ("*", po::value<std::string>(), 
       "other-options");

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

    po::options_description infoBufferDesc("Info buffer options");
    infoBufferDesc.add_options()
      ("info_buffer_make_info", po::value<unsigned>(),
       "invoke generation of list of o for given registrar")
      ("info_buffer_get_chunk", po::value<unsigned>(),
       "output chunk of buffer for given registrar")
      ("info_buffer_registrar", po::value<unsigned>(),
       "id of registrar for buffer selection")
      ("info_buffer_request", po::value<std::string>()->default_value(""),
       "handle for query");

    po::options_description pollDesc("Poll messages options");
    pollDesc.add_options()
      ("poll_type", po::value<unsigned>(),
       "set filter for poll type")
      ("poll_regid", po::value<unsigned>(),
       "set filter for registrar id")
      ("poll_nonseen",
       "set filter for non seen messages")
      ("poll_nonex",
       "set filter for non expired messages")
      ("poll_list_all",
       "list all poll messages")
      ("poll_list_next", po::value<unsigned>(),
       "list next message for given registrar")
      ("poll_set_seen", po::value<unsigned>(),
       "set given message as seen");

    po::variables_map vm;
    // parse help and config filename options
    po::store(
      po::parse_command_line(
        argc, argv, 
        desc.add(configDesc).add(invoiceDesc).add(aiDesc).add(bankDesc).
        add(infoBufferDesc).add(pollDesc)
      ), vm
    );
    // parse options from config file
    std::ifstream configFile(vm["conf"].as<std::string>().c_str());
    po::store(po::parse_config_file(configFile, fileDesc.add(configDesc)), vm);

    if (vm.count("help") || argc == 1 ) {
      stdout << desc << "\n";
      return 1;
    }
    
    Logger::get().setLevel(vm["log_level"].as<unsigned>());
    Logger::get().setFacility(vm["log_local"].as<unsigned>());    
    
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
        vm["docgen_template_path"].as<std::string>(),
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
    std::auto_ptr<Register::Zone::Manager> zoneMan(
      Register::Zone::Manager::create(&db)
    );
    std::auto_ptr<Register::Domain::Manager> domMan(
      Register::Domain::Manager::create(&db, zoneMan.get())
    );
    std::auto_ptr<Register::Contact::Manager> conMan(
      Register::Contact::Manager::create(
        &db,vm["restricted_handles"].as<unsigned>()
      )
    );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
      Register::NSSet::Manager::create(
        &db,vm["restricted_handles"].as<unsigned>()
      )
    );
    // infoBuffer
    std::auto_ptr<Register::InfoBuffer::Manager> infoBufMan(
      Register::InfoBuffer::Manager::create(
        &db, domMan.get(), nssMan.get(), conMan.get()
      )
    );
    if (vm.count("info_buffer_make_info")) {
      unsigned type = vm["info_buffer_make_info"].as<unsigned>();
      if (type < 1 && type > 7) 
        std::cerr << "info_buffer_make_info must be in 1..7.\n ";
      else {
        if (!vm.count("info_buffer_registrar"))
          std::cerr << "info_buffer_registrar must be set.\n ";
        else {
          infoBufMan->info(
            vm["info_buffer_registrar"].as<unsigned>(),
            type == 1 ? Register::InfoBuffer::T_LIST_CONTACTS :
            type == 2 ? Register::InfoBuffer::T_LIST_DOMAINS :
            type == 3 ? Register::InfoBuffer::T_LIST_NSSETS :
            type == 4 ? Register::InfoBuffer::T_DOMAINS_BY_NSSET :
            type == 5 ? Register::InfoBuffer::T_DOMAINS_BY_CONTACT :
            type == 6 ? Register::InfoBuffer::T_NSSETS_BY_CONTACT :
            Register::InfoBuffer::T_NSSETS_BY_NS,      
            vm["info_buffer_request"].as<std::string>()
          );
        }
      }
    }
    if (vm.count("info_buffer_get_chunk")) {
      unsigned chunkSize = vm["info_buffer_get_chunk"].as<unsigned>();
      if (!vm.count("info_buffer_registrar"))
        std::cerr << "info_buffer_registrar must be set.\n ";
      else {
        std::auto_ptr<Register::InfoBuffer::Chunk> chunk(
          infoBufMan->getChunk(
            vm["info_buffer_registrar"].as<unsigned>(),
            chunkSize
          )
        );
        for (unsigned long i=0; i<chunk->getCount(); i++)
          std::cout << chunk->getNext() << std::endl;
      }
    }

    // poll
    std::auto_ptr<Register::Poll::Manager> pollMan(
      Register::Poll::Manager::create(
        &db
      )
    );
    if (vm.count("poll_list_all")) {
      std::auto_ptr<Register::Poll::List> pmlist(pollMan->createList());
      if (vm.count("poll_type")) 
        pmlist->setTypeFilter(vm["poll_type"].as<unsigned>());
      if (vm.count("poll_regid")) 
        pmlist->setRegistrarFilter(vm["poll_regid"].as<unsigned>());
      if (vm.count("poll_nonseen")) 
        pmlist->setNonSeenFilter(true);
      if (vm.count("poll_nonex")) 
        pmlist->setNonExpiredFilter(true);
      pmlist->reload();
      for (unsigned i=0; i<pmlist->getCount(); i++) {
        Register::Poll::Message *m = pmlist->getMessage(i);
        if (m) {
          m->textDump(std::cout);
          std::cout << std::endl;			
        }
      }
    } else if (vm.count("poll_list_next")) {
      Register::TID reg = vm["poll_list_next"].as<unsigned>();
      unsigned count = pollMan->getMessageCount(reg);
      if (!count) { std::cout << "No message" << std::endl; }
      else {
        std::auto_ptr<Register::Poll::Message> m(pollMan->getNextMessage(reg));
        std::cout << "Messages:" << count << std::endl;
        m->textDump(std::cout);
        std::cout << std::endl;
      }
    } else if (vm.count("poll_set_seen")) {
      try {
        Register::TID reg = vm["poll_regid"].as<unsigned>();
        if (!reg) std::cout << "Muset specify owning registar" << std::endl;
        Register::TID messageId = vm["poll_set_seen"].as<unsigned>(); 
        pollMan->setMessageSeen(messageId, reg);
        std::cout << "NextId:" << pollMan->getNextMessageId(reg) << std::endl;
      }
      catch (...) { std::cout << "No message" << std::endl; }
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
