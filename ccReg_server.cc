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

#include "ccReg.hh"
#ifdef RIFD
#include "ccReg_epp.h"
#endif

#ifdef PIFD
#include "admin.h"
#endif

#ifdef ADIF
#include "admin.h"
#endif

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>

#include "log.h"
#include "conf.h"
#include "nameservice.h"

#ifndef CONFIG_FILE
#define CONFIG_FILE  "/etc/fred/server.conf"
#endif

// proper cleanup in case of signal
static CORBA::ORB_ptr orbToShutdown = NULL;
static void signalHandler(int signal)
{
  LOG(ERROR_LOG, "Daemon is closing");  
  if (orbToShutdown) orbToShutdown->shutdown(0);
}

int main(int argc, char** argv) {
    pid_t pid;

    try {
	// database connection settings
	char db[256];

	Conf config; // READ CONFIG  file
	bool readConfig=false;
	int i;
	// read config file
	for( i = 1 ;   i < argc ; i ++ )
	{
	    if( strcmp( argv[i] , "-C" )  == 0  ||  strcmp( argv[i] , "--config" ) == 0    )
	    {
		if( i +1< argc  ) // test for more parametr
		{
		    std::cerr << "Read config file" << argv[i+1] << std::endl ;
			
		    if (config.ReadConfigFile(argv[i+1])) readConfig=true;
		    else
		    {
			std::cerr << "Cannot read config file: " << argv[i+1] << std::endl; 
			exit(-1);
		    }
		}        
	    }
	}
	

	if( !readConfig ) // if config is not read try read default config
	{
	    if ( config.ReadConfigFile( CONFIG_FILE ) ) readConfig=true ;
	    else  {  
		std::cerr << "Cannot read default config file:" << CONFIG_FILE <<   std::endl;   
		exit(-1);
	    }
	}


	// 	    std::cerr << "start syslog at level " 
	// 		      <<  config.GetSYSLOGlevel()   << std::endl;
	// 	    std::cerr << "start syslog facility local" 
	// 		      <<  config.GetSYSLOGlocal()   << std::endl;
	// 	    setlogmask ( LOG_UPTO(  config.GetSYSLOGlevel()  )   );
	// 	    openlog ( argv[0] , LOG_CONS | LOG_PID | LOG_NDELAY,  config.GetSYSLOGfacility() );
	Logger::get().setLevel(config.GetSYSLOGlevel());
	Logger::get().setFacility(config.GetSYSLOGlocal());

	// database string
	strcpy( db , config.GetDBconninfo() );
	LOG( INFO_LOG, "DATABASE: %s", db);

	// Initialise the ORB.
	CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
	orbToShutdown = orb;
	signal(SIGINT,signalHandler);
	// Obtain a reference to the root POA.
	CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var rootPOA = PortableServer::POA::_narrow(obj);
	
	// create POA for persistent references
	PortableServer::POAManager_var mgr = rootPOA->the_POAManager();
	CORBA::PolicyList pols;
	pols.length(2);
	pols[0] = rootPOA->create_lifespan_policy(PortableServer::PERSISTENT);
	pols[1] = rootPOA->create_id_assignment_policy(PortableServer::USER_ID);
	PortableServer::POA_var poa = 
	    rootPOA->create_POA("RegistryPOA",mgr.in(),pols);
	
	// prepare NameService object
	std::string nameService = config.GetNameServiceHost();
	std::cerr << "nameService host: " <<   nameService << std::endl;
	NameService ns(orb,nameService);

#ifdef ADIF
	PortableServer::ObjectId_var adminObjectId =
	    PortableServer::string_to_ObjectId("Admin");
	ccReg_Admin_i* myccReg_Admin_i = new ccReg_Admin_i(db,&ns,config);
	poa->activate_object_with_id(adminObjectId,myccReg_Admin_i);
	CORBA::Object_var adminObj = myccReg_Admin_i->_this();
	myccReg_Admin_i->_remove_ref();
	ns.bind("Admin",adminObj);
#endif

#ifdef PIFD
	PortableServer::ObjectId_var webWhoisObjectId =
	    PortableServer::string_to_ObjectId("WebWhois");
	ccReg_Admin_i* myccReg_Admin_i = new ccReg_Admin_i(db,&ns,config);
	poa->activate_object_with_id(webWhoisObjectId,myccReg_Admin_i);
	CORBA::Object_var webWhoisObj = myccReg_Admin_i->_this();
	myccReg_Admin_i->_remove_ref();
	ns.bind("WebWhois",webWhoisObj);
#endif

#ifdef RIFD
	MailerManager mm(&ns);
	ccReg_EPP_i* myccReg_EPP_i = new ccReg_EPP_i(db,&mm,&ns,config);


	// create session  use values from config
	std::cerr << "CreateSession: max "  << config.GetSessionMax()  << " timeout " <<  config.GetSessionWait() <<  std::endl;
	myccReg_EPP_i->CreateSession( config.GetSessionMax() , config.GetSessionWait() );


	ccReg::timestamp_var ts;
	std::cerr << "version: " << myccReg_EPP_i->version(ts) << std::endl;
	std::cerr << "timestamp: " << ts << std::endl;

	// load zone parametrs 
	if( myccReg_EPP_i->loadZones() <= 0  ){
	    LOG( ALERT_LOG ,  "Database error: load zones");
	    exit(-4);
	}
	
	if( myccReg_EPP_i->LoadCountryCode() <= 0 ){  /// load all county code from table enum_country
	    LOG( ALERT_LOG , "Database error: load country code");
	    exit(-5);
	}
       
	if( myccReg_EPP_i->LoadErrorMessages() <= 0 ){  // load error messages to memory
	    LOG( ALERT_LOG , "Database error: load  error messages");
	    exit(-6);
	}

	if( myccReg_EPP_i->LoadReasonMessages()  <= 0 ){   // loead reason messages to memory
	    LOG( ALERT_LOG , "Database error: load reason messages" );
	    exit(-7);
	}

	PortableServer::ObjectId_var myccReg_EPP_iid = 
	    PortableServer::string_to_ObjectId("Register");
	poa->activate_object_with_id(myccReg_EPP_iid,myccReg_EPP_i);
	myccReg_EPP_i->_remove_ref();
	ns.bind("EPP",myccReg_EPP_i->_this());
#endif

	// Obtain a POAManager, and tell the POA to start accepting
	// requests on its objects.
	PortableServer::POAManager_var pman = poa->the_POAManager();
	pman->activate();
	LOG(NOTICE_LOG , "Starting server %s" , argv[0] );
	// disconnect from terminal
	setsid(); 
	orb->run();
	orb->destroy();
    }
    catch (NameService::NOT_RUNNING&) {
	LOG(ERROR_LOG, "Nameservice connection error");
	exit(-8);
    }
    
    catch (MailerManager::RESOLVE_FAILED) {
	LOG(ERROR_LOG, "Cannot connect to mailer");
	exit(-9);
    }
    
    catch (CORBA::SystemException& ex) {
	LOG(ERROR_LOG, "Caught a CORBA:: %s " , ex._name() );
    }
    catch (CORBA::Exception& ex) {
	LOG(ERROR_LOG, "Caught CORBA::Exception: %s " , ex._name() );
    }
    catch (omniORB::fatalException& fe) {
	LOG(ERROR_LOG,
	    "Caught  omniORB::fatalException: %s line: %d mesg: %s" ,
	    fe.file(), fe.line(), fe.errmsg()
	    );
    }
#ifdef RIFD
    catch (ccReg_EPP_i::DB_CONNECT_FAILED&) {
	LOG(ERROR_LOG, "Cannot connect to db");
	exit(-10);
    }
#endif    
    catch (...) {
	LOG(ERROR_LOG, "Unhandled exception");
    }

    /* Do a cleanup */
    LOG(INFO_LOG, "Exiting...");
    return 0;
}
