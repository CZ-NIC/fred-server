/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include "old_utils/dbsql.h"
#include "old_utils/log.h"
#include "corba/nameservice.h"
#include "corba/mailer_manager.h"
#include "corba/ccReg.hh" // temporary for deleteObjects function
#include "register/auth_info.h"
#include "register/invoice.h"
#include "register/bank.h"
#include "register/info_buffer.h"
#include "register/poll.h"
#include "register/register.h"
#include "register/notify.h"

namespace po = boost::program_options;
using namespace boost::posix_time;

const char* corbaOptions[][2] = { 
  { "nativeCharCodeSet", "UTF-8" },
  { NULL, NULL },
};

class CorbaClient
{
  CORBA::ORB_var orb;
  std::auto_ptr<NameService> ns;
 public:
  CorbaClient(int argc, char **argv, const std::string& nshost) 
  {
    orb = CORBA::ORB_init(argc, argv, "", corbaOptions);
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

#define STR(x) x.str().c_str()
/// delete objects with status deleteCandidate
/** \return 0=OK -1=SQL ERROR -2=no system registrar -3=login failed */
int deleteObjects(
  CorbaClient *cc, DB *db, const std::string& typeList, 
  unsigned limit, std::ostream *debug
)
{
  // temporary done by using EPP corba interface
  // should be instead somewhere in register library (object.cc?)
  // get login information for first system registrar
  if (!db->ExecSelect(
    "SELECT r.handle,ra.cert,ra.password "
    "FROM registrar r, registraracl ra "
    "WHERE r.id=ra.registrarid AND r.system='t' LIMIT 1 "
  )) return -1;
  if (!db->GetSelectRows()) return -1;
  std::string handle = db->GetFieldValue(0,0);
  std::string cert = db->GetFieldValue(0,1);
  std::string password = db->GetFieldValue(0,2);
  db->FreeSelect();
  // before connection load all objects, zones are needed to
  // put zone id into cltrid (used in statistics - need to fix)
  std::stringstream sql;
  sql <<
    "SELECT o.name, o.type, COALESCE(d.zone,0) "
    "FROM object_state s "
    "JOIN object_registry o ON ( "
    " o.erdate ISNULL AND o.id=s.object_id "
    " AND s.state_id=17 AND s.valid_to ISNULL"
    ") "
    "LEFT JOIN domain d ON (d.id=o.id)";
  if (!typeList.empty())
    sql << "WHERE o.type IN (" << typeList << ") ";
  sql << "ORDER BY s.id ";
  if (limit)
    sql << "LIMIT " << limit;
  if (!db->ExecSelect(sql.str().c_str())) return -1;
  if (!db->GetSelectRows()) return 0;
  if (debug) {
    *debug << "<objects>\n";
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      *debug << "<object name='" << db->GetFieldValue(i,0) << "'/>\n";
    }
    *debug << "</objects>\n";
    db->FreeSelect();
    return 0;
  }
  try {
    CORBA::Object_var o = cc->getNS()->resolve("EPP");
    ccReg::EPP_var epp = ccReg::EPP::_narrow(o);
    CORBA::Long clientID = 0;
    ccReg::Response_var r = epp->ClientLogin(
      handle.c_str(),password.c_str(),"","system_delete_login",
      "<system_delete_login/>",clientID,cert.c_str(),ccReg::EN
    );
    if (r->code != 1000 || !clientID) {
      std::cerr << "Cannot connect: " << r->code << std::endl;
      throw -3;
    }
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      std::string name = db->GetFieldValue(i,0);
      std::stringstream cltrid;
      std::stringstream xml;
      xml << "<name>" << name << "</name>";
      try {
        switch (atoi(db->GetFieldValue(i,1))) {
          case 1:
            cltrid << "delete_contact";
            r = epp->ContactDelete(name.c_str(),clientID,STR(cltrid),STR(xml));
            break;
          case 2:
            cltrid << "delete_nsset";
            r = epp->NSSetDelete(name.c_str(),clientID,STR(cltrid),STR(xml));
            break;
          case 3:
            cltrid << "delete_unpaid_zone_" << db->GetFieldValue(i,2);
            r = epp->DomainDelete(name.c_str(),clientID,STR(cltrid),STR(xml));
            break;          
        }
        if (r->code != 1000)
          std::cerr << "Cannot delete: " << name << " code: " << r->code;
        else
          std::cerr << "Deleted: " << name;
        std::cerr << std::endl;
      }
      catch (...) {
        std::cerr << "Cannot delete: " << name << std::endl;
        // proceed with next domain
      }
    }
    epp->ClientLogout(
      clientID,"system_delete_logout","<system_delete_logout/>"
    );
    db->FreeSelect();
    return 0;
  }
  catch (int& i) {
    db->FreeSelect();
    return i;
  }
  catch (...) {
    db->FreeSelect();
    return -4;
  }
}

int createObjectStateRequest(DB *db, Register::TID object, unsigned state)
{
  std::stringstream sql;
  sql << "SELECT COUNT(*) FROM object_state_request "
      << "WHERE object_id=" << object << " AND state_id=" << state 
      << " AND canceled ISNULL "
      << " AND (valid_to ISNULL OR valid_to>CURRENT_TIMESTAMP) ";
  if (!db->ExecSelect(sql.str().c_str())) return -1;
  if (atoi(db->GetFieldValue(0,0))) return -2;
  db->FreeSelect();
  sql.str("");
  sql << "INSERT INTO object_state_request "
      << "(object_id,state_id,crdate, valid_from,valid_to) VALUES "
      << "(" << object << "," << state 
      << ",CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, "
      << "CURRENT_TIMESTAMP + INTERVAL '7 days');";
  if (!db->ExecSQL(sql.str().c_str())) return -1;
  return 0;
}

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
      ("conf", po::value<std::string>()->default_value(CONFIG_FILE),
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
    fileDesc.add(configDesc);
    fileDesc.add_options()
      ("*", po::value<std::string>(), 
       "other-options");

    po::options_description invoiceDesc("Invoicing options");
    invoiceDesc.add_options()
      ("invoice_archive", 
       "archive unarchived invoices")
      ("invoice_dont_send", 
       "dont send mails with invoices during archivation")
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
      ("object_id", po::value<Register::TID>(),
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
       "set given message as seen")
      ("poll_create_statechanges",
       "create messages for state changes")
      ("poll_limit", po::value<unsigned>()->default_value(0),
       "limit for number of messages generated in one pass (0=no limit)")
      ("poll_debug",
       "don't do anything, just list xml with values")
      ("poll_except_types", po::value<std::string>()->default_value("6,7"),
       "list of poll message types ignored in creation (only states now)")
      ("poll_create_lowcredit",
        "create messages for lowcredit");

    po::options_description objDesc("Objects options");
    objDesc.add_options()
      ("object_list",
       "list xml of objects according to filter")
      ("object_list_limit",po::value<unsigned>()->default_value(50),
       "limit for object list")
      ("object_id",po::value<Register::TID>(),
       "filter for id")
      ("object_name",po::value<std::string>(),
       "filter for name of object")
      ("object_new_state_request",po::value<unsigned>(),
       "set request for object state with specified state id")
      ("object_update_states",
       "globaly update all states of all objects")
      ("object_delete_candidates",
       "delete all objects marked with deleteCandidate state")
      ("object_delete_debug",
       "don't delete anything, just list objects")
      ("object_delete_limit",po::value<unsigned>()->default_value(0),
       "limit for deletion od object list")
      ("object_delete_types",po::value<std::string>()->default_value("3"),
       "only this types of object will be deleted during mass delete")
      ("object_regular_procedure",
       "shortcut for 2x update_object_states, notify_state_changes, "
       "poll_create_statechanges, object_delete_candidates, "
       "poll_create_lowcredit, "
       );

    po::options_description notDesc("Notification options");
    notDesc.add_options()
      ("notify_state_changes",
       "send emails to contacts about object state changes")
      ("notify_use_history_tables",
       "slower queries into history tables, but can handle deleted objects")
       // default behavior ignore contact and nsset delete notification
       // must be explicitly overwritten by setting thist parameter to "" 
      ("notify_except_types", po::value<std::string>()->default_value("4,5"),
       "list of notification types ignored in notification")
      ("notify_limit", po::value<unsigned>()->default_value(0),
       "limit for number of emails generated in one pass (0=no limit)")
      ("notify_debug",
       "don't do anything, just list xml with values")
      ("notify_letters_create",
       "generate pdf with domain registration warning");

    po::options_description regDesc("Registrar options");
    regDesc.add_options()
      ("zone_add",
       "add new zone")
      ("registrar_add",
       "add new registrar (make a copy of REG-FRED_A)")
      ("registrar_add_zone",
       "add access right for registrar to zone")
      ("zone_fqdn",po::value<std::string>(),
       "add new zone")
      ("registrar_handle",po::value<std::string>(),
       "add registrar handle");
   
    po::variables_map vm;
    // parse help and config filename options
    po::store(
      po::parse_command_line(
        argc, argv, 
        desc.add(configDesc).add(invoiceDesc).add(aiDesc).add(bankDesc).
        add(infoBufferDesc).add(pollDesc).add(objDesc).add(notDesc).
        add(regDesc)
      ), vm
    );
    // parse options from config file
    std::ifstream configFile(vm["conf"].as<std::string>().c_str());
    po::store(po::parse_config_file(configFile, fileDesc), vm);

    if (vm.count("help") || argc == 1 ) {
      stdout << desc << "\n";
      return 1;
    }
    
    SysLogger::get().setLevel(vm["log_level"].as<unsigned>());
    SysLogger::get().setFacility(vm["log_local"].as<unsigned>());    
    
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
      im->archiveInvoices(!vm.count("invoice_dont_send"));
    }
    if (vm.count("invoice_list")) {
      std::auto_ptr<Register::Invoicing::List> il(im->createList());
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
        Register::Invoicing::List::ArchiveFilter af;
        switch (vm["archived"].as<unsigned>()) { 
          case 0: af = Register::Invoicing::List::AF_UNSET; break;
          case 1: af = Register::Invoicing::List::AF_SET; break;
          default: af = Register::Invoicing::List::AF_IGNORE; break;
        }
        il->setArchivedFilter(af);
      }
      if (vm.count("object_id")) 
        il->setObjectIdFilter(vm["object_id"].as<Register::TID>());
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
        &db,zoneMan.get(),vm["restricted_handles"].as<unsigned>()
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
    } else if (vm.count("poll_create_statechanges"))
      pollMan->createStateMessages(
        vm["poll_except_types"].as<std::string>(),
        vm["poll_limit"].as<unsigned>(),
        vm.count("poll_debug") ? &std::cout : NULL
      );
    else if (vm.count("poll_create_lowcredit"))
      pollMan->createLowCreditMessages();
    
    std::auto_ptr<Register::Manager> regMan(
      Register::Manager::create(&db,vm["restricted_handles"].as<unsigned>())
    );
    if (vm.count("object_update_states")) {
      regMan->updateObjectStates();
    }
    if (vm.count("object_delete_candidates")) {
      return deleteObjects(
        &cc,&db,vm["object_delete_types"].as<std::string>(),
        vm["object_delete_limit"].as<unsigned>(),
        vm.count("object_delete_debug") ? &std::cout : NULL        
      );
    }

    std::auto_ptr<Register::Registrar::Manager> rMan(
      Register::Registrar::Manager::create(&db)
    );
    std::auto_ptr<Register::Notify::Manager> notifyMan(
      Register::Notify::Manager::create(
        &db, &mm, conMan.get(), nssMan.get(), domMan.get(), docman.get(),
        rMan.get()
      )
    );
    if (vm.count("notify_state_changes")) {
      notifyMan->notifyStateChanges(
        vm["notify_except_types"].as<std::string>(),
        vm["notify_limit"].as<unsigned>(),
        vm.count("notify_debug") ? &std::cout : NULL,
        vm.count("notify_use_history_tables")
      );
    }
    if (vm.count("notify_letters_create")) {
      notifyMan->generateLetters();
    }
    if (vm.count("object_regular_procedure")) {
      regMan->updateObjectStates();
      regMan->updateObjectStates();
      notifyMan->notifyStateChanges(
        vm["notify_except_types"].as<std::string>(),0,NULL,false
      );
      pollMan->createStateMessages(
        vm["poll_except_types"].as<std::string>(),0,NULL
      );
      // unless notification is done by queries into non-history tables
      // notification must be called before delete, because after
      // delete objects are removed from non-history tables
      deleteObjects(&cc,&db,vm["object_delete_types"].as<std::string>(),0,NULL);
      pollMan->createLowCreditMessages();
    }
    if (vm.count("object_list")) {
      std::auto_ptr<Register::Domain::List> dl(domMan->createList());
      if (vm.count("object_id"))
        dl->setIdFilter(vm["object_id"].as<Register::TID>());
      if (vm.count("object_name"))
        dl->setFQDNFilter(vm["object_name"].as<std::string>());
      dl->setLimit(vm["object_list_limit"].as<unsigned>());
      dl->reload();
      std::cout << "<objects>" << std::endl;
      for (unsigned i=0; i<dl->getCount(); i++) {
        std::cout << "<object>" 
                  << "<id>" << dl->getDomain(i)->getId() << "</id>"
                  << "<name>" << dl->getDomain(i)->getFQDN() << "</name>"
                  << "</object>";
      }
      std::cout << "</objects>" << std::endl;
    }
    if (vm.count("object_new_state_request")) {
      if (!vm.count("object_id"))
        std::cerr << "object_id parameter must be specified" << std::endl;
      else {
        int res = createObjectStateRequest(
          &db, 
          vm["object_id"].as<Register::TID>(),
          vm["object_new_state_request"].as<unsigned>()
        );
        switch (res) {
          case -1: std::cerr << "SQL_ERROR" << std::endl; break;
          case -2: std::cerr << "Already exists" << std::endl; break;
          case  0: break;
          default: std::cerr << "Unknown result" << std::endl; break;
        }
      }
    }
    if (vm.count("zone_add")) {
      if (!vm.count("zone_fqdn"))
        std::cerr << "zone_fqdn parameter must be specified" << std::endl;
      else {
        try {
          zoneMan->addZone(vm["zone_fqdn"].as<std::string>());
        } catch (Register::ALREADY_EXISTS) {
          std::cerr << "zone '" << vm["zone_fqdn"].as<std::string>()
                    << "'" << " already exists in configuration " << std::endl;
        }
      }
    }
    if (vm.count("registrar_add")) {
      if (!vm.count("registrar_handle"))
        std::cerr << "registrar_handle parameter must be specified" 
                  << std::endl;
      else {
        rMan->addRegistrar(vm["registrar_handle"].as<std::string>());
      }
    }
    if (vm.count("registrar_add_zone")) {
      if (!vm.count("registrar_handle"))
        std::cerr << "registrar_handle parameter must be specified" 
                  << std::endl;
      else {
        if (!vm.count("zone_fqdn"))
          std::cerr << "zone_fqdn parameter must be specified" << std::endl;
        else
          rMan->addRegistrarZone(
            vm["registrar_handle"].as<std::string>(),
            vm["zone_fqdn"].as<std::string>()
          );
      }
    }
    if (connected) db.Disconnect();
  }
  catch (std::exception& e) {
    std::cerr << e.what() << "\n";
    if (connected) db.Disconnect();
    return 2;
  }
  catch (Register::SQL_ERROR) {
    std::cerr << "SQL ERROR \n";
    if (connected) db.Disconnect();
    return 1;
  }
  return 0;
}
