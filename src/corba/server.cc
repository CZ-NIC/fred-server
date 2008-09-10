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

/* proper cleanup in case of SIGINT signal */
static void sigint_handler(int signal) {
  LOGGER("server").info("server is closing...");
  if (orb_to_shutdown)
    orb_to_shutdown->shutdown(0);
}


int main(int argc, char** argv) {
    /* program options definition */
    po::options_description cmd_opts;
    cmd_opts.add_options()
      ("config-test", "Run configuration check and view actual configuration");

    po::options_description database_opts("Database");
    database_opts.add_options()
      ("database.host",     po::value<std::string>(), "Database hostname")
      ("database.port",     po::value<unsigned>(),    "Database port")
      ("database.name",     po::value<std::string>(), "Database name")
      ("database.user",     po::value<std::string>(), "Database username")
      ("database.password", po::value<std::string>(), "Database password")
      ("database.timeout",  po::value<unsigned>(),    "Database connection timeout");
    po::options_description nameservice_opts("CORBA Nameservice");
    nameservice_opts.add_options()
      ("nameservice.host",  po::value<std::string>(), "CORBA nameservice hostname")
      ("nameservice.port",  po::value<unsigned>(),    "CORBA nameservice port");
    po::options_description log_opts("Logging");
    log_opts.add_options()
      ("log.type",          po::value<unsigned>(),    "Log type")
      ("log.level",         po::value<unsigned>(),    "Log level");
    po::options_description registry_opts("Registry");
    registry_opts.add_options()
      ("registry.restricted_handles",   po::value<bool>(),        "To force using restricted handles for NSSETs and CONTACTs")
      ("registry.disable_epp_notifier", po::value<bool>(),        "Disable EPP notifier subsystem")
      ("registry.lock_epp_commands",    po::value<bool>(),        "Database locking of multiple update epp commands on one object")
      ("registry.nsset_level",          po::value<unsigned>(),    "Default report level of new NSSET")
      ("registry.docgen_path",          po::value<std::string>(), "PDF generator path")
      ("registry.docgen_template_path", po::value<std::string>(), "PDF generator template path")
      ("registry.fileclient_path",      po::value<std::string>(), "File manager client path");
    po::options_description rifd_opts("RIFD specific");
    rifd_opts.add_options()
      ("rifd.session_max",           po::value<unsigned>(), "RIFD maximum number of sessions")
      ("rifd.session_timeout",       po::value<unsigned>(), "RIFD session timeout")
      ("rifd.session_registrar_max", po::value<unsigned>(), "RIFD maximum munber active sessions per registrar");
    po::options_description adifd_opts("ADIFD specific");
    adifd_opts.add_options()
      ("adifd.session_max",     po::value<unsigned>(), "ADIFD maximum number of sessions")
      ("adifd.session_timeout", po::value<unsigned>(), "ADIFD session timeout")
      ("adifd.session_garbage", po::value<unsigned>(), "ADIFD session garbage interval");    

    po::options_description file_opts;
    file_opts.add(database_opts).add(nameservice_opts).add(log_opts).add(registry_opts).add(rifd_opts).add(adifd_opts);

    Config::Manager cfm = Config::ConfigManager::instance_ref();
    try {
      cfm.init(argc, argv);
      cfm.setCmdLineOptions(cmd_opts);
      cfm.setCfgFileOptions(file_opts, CONFIG_FILE);
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

#ifdef ADIF
    Logging::Manager::instance_ref().prefix("adifd");
#endif
#ifdef PIFD
    Logging::Manager::instance_ref().prefix("pifd");
#endif
#ifdef RIFD
    Logging::Manager::instance_ref().prefix("rifd");
#endif

    /* get parsed configuration options */
    Config::Conf cfg = cfm.get();

    /* setting up logger */
    Logging::Log::Level log_level = static_cast<Logging::Log::Level>(cfg.get<unsigned>("log.level"));
    Logging::Log::Type  log_type  = static_cast<Logging::Log::Type>(cfg.get<unsigned>("log.type"));

    Logging::Manager::instance_ref().get("tracer").addHandler(log_type);
    Logging::Manager::instance_ref().get("tracer").setLevel(log_level);
    Logging::Manager::instance_ref().get("db").addHandler(log_type);
    Logging::Manager::instance_ref().get("db").setLevel(log_level);
    Logging::Manager::instance_ref().get("register").addHandler(log_type);
    Logging::Manager::instance_ref().get("register").setLevel(log_level);
    Logging::Manager::instance_ref().get("corba").addHandler(log_type);
    Logging::Manager::instance_ref().get("corba").setLevel(log_level);
    Logging::Manager::instance_ref().get("mailer").addHandler(log_type);
    Logging::Manager::instance_ref().get("mailer").setLevel(log_level);
    Logging::Manager::instance_ref().get("old_log").addHandler(log_type);
    Logging::Manager::instance_ref().get("old_log").setLevel(log_level);
    
    Logging::Manager::instance_ref().get("server").addHandler(log_type);
    Logging::Manager::instance_ref().get("server").setLevel(Logging::Log::LL_DEBUG);
    if (log_type != Logging::Log::LT_CONSOLE) {
      Logging::Manager::instance_ref().get("server").addHandler(Logging::Log::LT_CONSOLE);
    }

    /* print some server settings info */
    LOGGER("server").info(boost::format("configuration succesfully read from `%1%'")
                                        % cfg.get<std::string>("conf"));

    std::string conn_info = str(boost::format("host=%1% port=%2% dbname=%3% user=%4% password=%5% connect_timeout=%6%")
                                          % cfg.get<std::string>("database.host")
                                          % cfg.get<unsigned>("database.port")
                                          % cfg.get<std::string>("database.name")
                                          % cfg.get<std::string>("database.user")
                                          % cfg.get<std::string>("database.password")
                                          % cfg.get<unsigned>("database.timeout"));
                              
    LOGGER("server").info(boost::format("database connection set to: `%1%'")
                                        % conn_info);

    /* Seed random number generator (need for authinfo generation!) */
    LOGGER("server").info("initialization of random seed generator");
    srand(time(NULL));

  try {
    /* initialize ORB */
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
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

    LOGGER("server").info(boost::format("nameservice host set to: `%1%'") % nameservice);
    NameService ns(orb, nameservice);

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
    LOGGER("server").info(boost::format("sessions max: %1%; timeout: %2%") 
                                        % rifd_session_max
                                        % rifd_session_timeout);
    myccReg_EPP_i->CreateSession(rifd_session_max, rifd_session_timeout);

    ccReg::timestamp_var ts;
    LOGGER("server").info(boost::format("RIFD server version: %1% (%2%)") 
                                          % myccReg_EPP_i->version(ts)
                                          % ts);

    /* load zone parametrs */
    if (myccReg_EPP_i->loadZones() <= 0) {
      LOGGER("server").alert("database error: load zones");
      exit(-4);
    }
    
    /* load all county code from table enum_country */
    if (myccReg_EPP_i->LoadCountryCode() <= 0) {
      LOGGER("server").alert("database error: load country code");
      exit(-5);
    }

    /* load error messages to memory */
    if (myccReg_EPP_i->LoadErrorMessages() <= 0) { 
      LOGGER("server").alert("database error: load error messages");
      exit(-6);
    }

    /* loead reason messages to memory */
    if (myccReg_EPP_i->LoadReasonMessages() <= 0) { 
      LOGGER("server").alert("database error: load reason messages" );
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
    pman->activate();
    LOGGER("server").notice(boost::format("starting server %1%") % argv[0]);

    /* disconnect from terminal */
    setsid();
    orb->run();
    orb->destroy();
  }
  catch (NameService::NOT_RUNNING&) {
    LOGGER("server").error("nameservice: connection error");
    exit(-8);
  }
  catch (MailerManager::RESOLVE_FAILED) {
    LOGGER("server").error("mailer: connection error");
    exit(-9);
  }
  catch (CORBA::SystemException& ex) {
    LOGGER("server").error(boost::format("CORBA system exception: %1%") % ex._name());
  }
  catch (CORBA::Exception& ex) {
    LOGGER("server").error(boost::format("CORBA exception: %1%") % ex._name());
  }
  catch (omniORB::fatalException& fe) {
    LOGGER("server").error(boost::format("omniORB fatal exception: %1% line: %2% mesg: %3%")
                                         % fe.file()
                                         % fe.line()
                                         % fe.errmsg());
  }
  /* catch specific exception for each server */
#ifdef RIFD
  catch (ccReg_EPP_i::DB_CONNECT_FAILED&) {
    LOGGER("server").error("database: connection error");
    exit(-10);
  }
#endif
#ifdef ADIF
  catch (ccReg_Admin_i::DB_CONNECT_FAILED&) {
    LOGGER("server").error("database: connection error");
    exit(-10);
  }
#endif
#ifdef PIFD 
  catch (ccReg_Admin_i::DB_CONNECT_FAILED&) {
    LOGGER("server").error("database: connection error");
    exit(-10);
  }
#endif
  catch (Config::Conf::OptionNotFound &_err) {
    LOGGER("server").error(boost::format("config: %1%") % _err.what());
    exit(-11);
  }
  catch (...) {
    LOGGER("server").error("ERROR: unhandled exception");
    exit(-12);
  }

  /* do a cleanup */
  LOGGER("server").info("exiting...");
  return 0;
}

