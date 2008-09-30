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
#include "ccReg.hh"

#ifdef RIFD
#include "epp/epp_impl.h"
#endif

#ifdef PIFD
#include "admin/admin_impl.h"
#endif

#ifdef ADIF
#include "admin/admin_impl.h"
#endif

#include <cstdlib>
#include <time.h>

#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>

#include "old_utils/log.h"
#include "old_utils/conf.h"
#include "nameservice.h"

#include "conf/manager.h"
#include "log/logger.h"

/* pointer to ORB which have to be destroyed on signal received */
static CORBA::ORB_ptr orb_to_shutdown = NULL;
static PortableServer::POAManager_ptr poa_manager = NULL; 

/* proper cleanup in case of SIGINT signal */
static void sigint_handler(int signal) {
  LOGGER(PACKAGE).info("server is closing...");
  if (orb_to_shutdown)
    orb_to_shutdown->shutdown(0);
}

/** 
 * server config reload in case of SIGHUP signal 
 * TESTING NOT USE ON PRODUCTION (check that sighup signal is not connected to this function)
 */
static void sighup_handler(int signal) {
  assert(signal == SIGHUP);
  try {
    LOGGER(PACKAGE).info("reload config request received -- setting server to HOLD state");
    poa_manager->hold_requests(true);
    Config::ConfigManager::instance_ptr()->reload();
    poa_manager->activate();
    LOGGER(PACKAGE).info("configuration reloaded -- setting server to ACTIVE state");
  }
  catch(std::exception &_ex) {
    LOGGER(PACKAGE).error(boost::format("config reload error: %1%") 
                                         % _ex.what());
  }

}

int main(int argc, char** argv) {

  // orb must be initialized before options are parsed to eat all omniorb
  // program options
  CORBA::ORB_var orb;
  try {
    /* initialize ORB */
    orb = CORBA::ORB_init(argc, argv);
  } catch (...) {
    std::cerr << "ORB_init failed\n";
    exit(-1);
  }

    /* program options definition */
    po::options_description cmd_opts;
    cmd_opts.add_options()
      ("view-config", "View actual configuration");

    po::options_description database_opts("Database");
    database_opts.add_options()
      ("database.host",     po::value<std::string>()->default_value(""),     "Database hostname")
      ("database.port",     po::value<unsigned>()->default_value(5432),      "Database port")
      ("database.name",     po::value<std::string>()->default_value("fred"), "Database name")
      ("database.user",     po::value<std::string>()->default_value("fred"), "Database username")
      ("database.password", po::value<std::string>()->default_value(""),     "Database password")
      ("database.timeout",  po::value<unsigned>()->default_value(0),         "Database connection timeout");
    po::options_description nameservice_opts("CORBA Nameservice");
    nameservice_opts.add_options()
      ("nameservice.host",    po::value<std::string>()->default_value("localhost"), "CORBA nameservice hostname")
      ("nameservice.port",    po::value<unsigned>()->default_value(2809),           "CORBA nameservice port")
      ("nameservice.context", po::value<std::string>()->default_value("fred"),      "CORBA nameservice context");
    po::options_description log_opts("Logging");
    log_opts.add_options()
      ("log.type",            po::value<unsigned>()->default_value(1),             "Log type")
      ("log.level",           po::value<unsigned>()->default_value(8),             "Log level")
      ("log.file",            po::value<std::string>()->default_value("fred.log"), "Log file path (for log.type = 1)")
      ("log.syslog_facility", po::value<unsigned>()->default_value(1),             "Syslog facility (for log.type = 2)");
    po::options_description registry_opts("Registry");
    registry_opts.add_options()
      ("registry.restricted_handles",   po::value<bool>()->default_value(false), "To force using restricted handles for NSSETs, KEYSETs and CONTACTs")
      ("registry.disable_epp_notifier", po::value<bool>()->default_value(false), "Disable EPP notifier subsystem")
      ("registry.lock_epp_commands",    po::value<bool>()->default_value(true),  "Database locking of multiple update epp commands on one object")
      ("registry.nsset_level",          po::value<unsigned>()->default_value(3), "Default report level of new NSSET")
      ("registry.docgen_path",          po::value<std::string>()->default_value("/usr/local/bin/fred-doc2pdf"),       "PDF generator path")
      ("registry.docgen_template_path", po::value<std::string>()->default_value("/usr/local/share/fred-doc2pdf/"),    "PDF generator template path")
      ("registry.fileclient_path",      po::value<std::string>()->default_value("/usr/local/bin/filemanager_client"), "File manager client path");
    po::options_description rifd_opts("RIFD specific");
    rifd_opts.add_options()
      ("rifd.session_max",           po::value<unsigned>()->default_value(200), "RIFD maximum number of sessions")
      ("rifd.session_timeout",       po::value<unsigned>()->default_value(300), "RIFD session timeout")
      ("rifd.session_registrar_max", po::value<unsigned>()->default_value(5), "RIFD maximum munber active sessions per registrar");
    po::options_description adifd_opts("ADIFD specific");
    adifd_opts.add_options()
      ("adifd.session_max",     po::value<unsigned>()->default_value(0),    "ADIFD maximum number of sessions (0 mean not limited)")
      ("adifd.session_timeout", po::value<unsigned>()->default_value(3600), "ADIFD session timeout")
      ("adifd.session_garbage", po::value<unsigned>()->default_value(150),  "ADIFD session garbage interval");    

    po::options_description file_opts;
    file_opts.add(database_opts).add(nameservice_opts).add(log_opts).add(registry_opts).add(rifd_opts).add(adifd_opts);

    Config::Manager cfm = Config::ConfigManager::instance_ref();
    try {
      cfm.init(argc, argv);
      cfm.setCmdLineOptions(cmd_opts);
      cfm.setCfgFileOptions(file_opts, CONFIG_FILE, true);
      cfm.parse();
    }
    catch (Config::Manager::ConfigParseError &_err) {
      std::cerr << "config parse error: " << _err.what() << std::endl;
      exit(-20);
    }

    if (cfm.isHelp()) {
      std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
      std::cout << "Options:" << std::endl;
      cfm.printAvailableOptions(std::cout);
      return 0;
    }
   
    if (cfm.isVersion()) {
      std::cout << PACKAGE_STRING 
                << " (built " << __DATE__ << " " << __TIME__ << ")" 
                << std::endl;
      return 0;
    }

    /* get parsed configuration options */
    Config::Conf cfg = cfm.get();

    if (cfg.hasOpt("view-config")) {
      cfg.print(std::cout);
      exit(0);
    }

    /* setting up logger */
    Logging::Log::Level log_level = static_cast<Logging::Log::Level>(cfg.get<unsigned>("log.level"));
    Logging::Log::Type  log_type  = static_cast<Logging::Log::Type>(cfg.get<unsigned>("log.type"));
    boost::any param;          
    if (log_type == Logging::Log::LT_FILE) {
      param = cfg.get<std::string>("log.file");
    }
    if (log_type == Logging::Log::LT_SYSLOG) {
      param = cfg.get<unsigned>("log.syslog_facility");
    }

    Logging::Manager::instance_ref().get(PACKAGE).addHandler(log_type, param);
    Logging::Manager::instance_ref().get(PACKAGE).setLevel(log_level);

    /* print some server settings info */
    LOGGER(PACKAGE).info(boost::format("configuration succesfully read from `%1%'")
                                        % cfg.get<std::string>("config"));

    /* construct connection string */
    std::string dbhost = cfg.get<std::string>("database.host");
    dbhost = (dbhost.empty() ? "" : "host=" + dbhost + " ");
    std::string dbpass = cfg.get<std::string>("database.password");
    dbpass = (dbpass.empty() ? "" : "password=" + dbpass + " ");
    std::string dbname = cfg.get<std::string>("database.name");
    std::string dbuser = cfg.get<std::string>("database.user");
    unsigned    dbport = cfg.get<unsigned>("database.port");
    unsigned    dbtime = cfg.get<unsigned>("database.timeout");
    std::string conn_info = str(boost::format("%1%port=%2% dbname=%3% user=%4% %5%connect_timeout=%6%")
                                              % dbhost
                                              % dbport 
                                              % dbname
                                              % dbuser
                                              % dbpass
                                              % dbtime);
                              
    LOGGER(PACKAGE).info(boost::format("database connection set to: `%1%'")
                                        % conn_info);

    /* Seed random number generator (need for authinfo generation!) */
    LOGGER(PACKAGE).info("initialization of random seed generator");
    srand(time(NULL));

  try {
    orb_to_shutdown = orb;
    signal(SIGINT, sigint_handler);

    /* obtain a reference to the root POA */
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var rootPOA = PortableServer::POA::_narrow(obj);

    /* create POA for persistent references */
    PortableServer::POAManager_var mgr = rootPOA->the_POAManager();
    CORBA::PolicyList pols;
    pols.length(2);
    pols[0] = rootPOA->create_lifespan_policy(PortableServer::PERSISTENT);
    pols[1] = rootPOA->create_id_assignment_policy(PortableServer::USER_ID);
    PortableServer::POA_var poa = rootPOA->create_POA("RegistryPOA", mgr.in(), pols);

    /* configure CORBA nameservice */
    std::string nameservice = str(boost::format("%1%:%2%")
                                                % cfg.get<std::string>("nameservice.host") 
                                                % cfg.get<unsigned>("nameservice.port"));

    LOGGER(PACKAGE).info(boost::format("nameservice host set to: `%1%' context used: `%2%'") 
                                       % nameservice
                                       % cfg.get<std::string>("nameservice.context"));
    NameService ns(orb, nameservice, cfg.get<std::string>("nameservice.context"));

    /* register specific object at nameservice */
#ifdef ADIF
    PortableServer::ObjectId_var adminObjectId = PortableServer::string_to_ObjectId("Admin");
    ccReg_Admin_i* myccReg_Admin_i = new ccReg_Admin_i(conn_info, &ns, cfg);
    poa->activate_object_with_id(adminObjectId, myccReg_Admin_i);
    CORBA::Object_var adminObj = myccReg_Admin_i->_this();
    myccReg_Admin_i->_remove_ref();
    ns.bind("Admin", adminObj);
#endif

#ifdef PIFD
    PortableServer::ObjectId_var webWhoisObjectId = PortableServer::string_to_ObjectId("WebWhois");
    ccReg_Admin_i* myccReg_Admin_i = new ccReg_Admin_i(conn_info, &ns, cfg, false);
    poa->activate_object_with_id(webWhoisObjectId, myccReg_Admin_i);
    CORBA::Object_var webWhoisObj = myccReg_Admin_i->_this();
    myccReg_Admin_i->_remove_ref();
    ns.bind("WebWhois", webWhoisObj);
#endif

#ifdef RIFD
    MailerManager mm(&ns);
    ccReg_EPP_i* myccReg_EPP_i = new ccReg_EPP_i(conn_info, &mm, &ns, cfg);

    /* create session use values from config */
    unsigned rifd_session_max     = cfg.get<unsigned>("rifd.session_max");
    unsigned rifd_session_timeout = cfg.get<unsigned>("rifd.session_timeout");
    LOGGER(PACKAGE).info(boost::format("sessions max: %1%; timeout: %2%") 
                                        % rifd_session_max
                                        % rifd_session_timeout);
    myccReg_EPP_i->CreateSession(rifd_session_max, rifd_session_timeout);

    ccReg::timestamp_var ts;
    char *version;
    version = myccReg_EPP_i->version(ts);
    LOGGER(PACKAGE).info(boost::format("RIFD server version: %1% (%2%)") 
                                          % version
                                          % ts);

    free(version);

    /* load zone parametrs */
    if (myccReg_EPP_i->loadZones() <= 0) {
      LOGGER(PACKAGE).alert("database error: load zones");
      exit(-4);
    }
    
    /* load all county code from table enum_country */
    if (myccReg_EPP_i->LoadCountryCode() <= 0) {
      LOGGER(PACKAGE).alert("database error: load country code");
      exit(-5);
    }

    /* load error messages to memory */
    if (myccReg_EPP_i->LoadErrorMessages() <= 0) { 
      LOGGER(PACKAGE).alert("database error: load error messages");
      exit(-6);
    }

    /* loead reason messages to memory */
    if (myccReg_EPP_i->LoadReasonMessages() <= 0) { 
      LOGGER(PACKAGE).alert("database error: load reason messages" );
      exit(-7);
    }

    PortableServer::ObjectId_var myccReg_EPP_iid = PortableServer::string_to_ObjectId("Register");
    poa->activate_object_with_id(myccReg_EPP_iid, myccReg_EPP_i);
    myccReg_EPP_i->_remove_ref();
    ns.bind("EPP", myccReg_EPP_i->_this());
#endif

    /** 
     * obtain a POAManager, and tell the POA to start accepting
     * requests on its objects
     */
    PortableServer::POAManager_var pman = poa->the_POAManager();
    poa_manager = pman;
    // signal(SIGHUP, sighup_handler);
    pman->activate();
    LOGGER(PACKAGE).notice(boost::format("starting server %1%") % argv[0]);

    /* disconnect from terminal */
    setsid();
    orb->run();
    orb->destroy();
  }
  catch (NameService::NOT_RUNNING&) {
    LOGGER(PACKAGE).error("nameservice: connection error");
    exit(-8);
  }
  catch (MailerManager::RESOLVE_FAILED) {
    LOGGER(PACKAGE).error("mailer: connection error");
    exit(-9);
  }
  catch (CORBA::SystemException& ex) {
    LOGGER(PACKAGE).error(boost::format("CORBA system exception: %1%") % ex._name());
  }
  catch (CORBA::Exception& ex) {
    LOGGER(PACKAGE).error(boost::format("CORBA exception: %1%") % ex._name());
  }
  catch (omniORB::fatalException& fe) {
    LOGGER(PACKAGE).error(boost::format("omniORB fatal exception: %1% line: %2% mesg: %3%")
                                         % fe.file()
                                         % fe.line()
                                         % fe.errmsg());
  }
  /* catch specific exception for each server */
#ifdef RIFD
  catch (ccReg_EPP_i::DB_CONNECT_FAILED&) {
    LOGGER(PACKAGE).error("database: connection error");
    exit(-10);
  }
#endif
#ifdef ADIF
  catch (ccReg_Admin_i::DB_CONNECT_FAILED&) {
    LOGGER(PACKAGE).error("database: connection error");
    exit(-10);
  }
#endif
#ifdef PIFD 
  catch (ccReg_Admin_i::DB_CONNECT_FAILED&) {
    LOGGER(PACKAGE).error("database: connection error");
    exit(-10);
  }
#endif
  catch (Config::Conf::OptionNotFound &_err) {
    LOGGER(PACKAGE).error(boost::format("config: %1%") % _err.what());
    exit(-11);
  }
  catch (...) {
    LOGGER(PACKAGE).error("ERROR: unhandled exception");
    exit(-12);
  }

  /* do a cleanup */
  LOGGER(PACKAGE).info("exiting...");
  return 0;
}

