#include "ccReg.hh"
#ifdef RIFD
#include "ccReg_epp.h"
#endif

#ifdef PIFD
#include "whois.h"
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

#include <libdaemon/dfork.h>
#include <libdaemon/dsignal.h>
#include <libdaemon/dlog.h>
#include <libdaemon/dpid.h>
#include <libdaemon/dexec.h>

#ifndef CONFIG_FILE
#define CONFIG_FILE  "/etc/fred/server.conf"
#endif

// proper cleanup in case of signal
static CORBA::ORB_ptr orbToShutdown = NULL;
static void signalHandler(int signal)
{
  daemon_log(LOG_ERR, "Daemon is closing");  
	if (orbToShutdown) orbToShutdown->shutdown(0);
}

int main(int argc, char** argv) {
    pid_t pid;

    /* Set identification string for the daemon for both syslog and PID file */
    daemon_pid_file_ident = daemon_log_ident = daemon_ident_from_argv0(argv[0]);

    /* Check that the daemon is not rung twice a the same time */
    if ((pid = daemon_pid_file_is_running()) >= 0) {
	daemon_log(LOG_ERR, "Daemon already running on PID file %u", pid);
	return 1;
    }

    /* Prepare for return value passing from the initialization procedure of the daemon process */
    daemon_retval_init();
    
    /* Do the fork */
    if ((pid = daemon_fork()) < 0) {

	/* Exit on error */
	daemon_retval_done();
	return 1;

    } else if (pid) { /* The parent */
	int ret;

	/* Wait for 20 seconds for the return value passed from the daemon process */
	if ((ret = daemon_retval_wait(20)) < 0) {
	    daemon_log(LOG_ERR, "Could not recieve return value from daemon process.");
	    return 255;
	}

	daemon_log(ret != 0 ? LOG_ERR : LOG_INFO, "Daemon returned %i as return value.", ret);
	return ret;
    } else {
	int fd, quit = 0;
	fd_set fds;

	/* Create the PID file */
	if (daemon_pid_file_create() < 0) {
	    daemon_log(LOG_ERR, "Could not create PID file (%s).", strerror(errno));
	    
	    /* Send the error condition to the parent process */
	    daemon_retval_send(1);
	    goto finish;
	}

	/*... do some further init work here */
	/* TODO: Move database initialization here */

	/* Send OK to parent process */
	daemon_retval_send(0);

	daemon_log(LOG_INFO, "Successfully started");
	
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

	    // database string
	    strcpy( db , config.GetDBconninfo() );
	    daemon_log(LOG_INFO, "DATABASE: %s", db);

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
	    PortableServer::ObjectId_var whoisObjectId =
		PortableServer::string_to_ObjectId("Whois");
	    ccReg_Whois_i* myccReg_Whois_i = new ccReg_Whois_i(db);
	    poa->activate_object_with_id(whoisObjectId,myccReg_Whois_i);
	    CORBA::Object_var whoisObj = myccReg_Whois_i->_this();
	    myccReg_Whois_i->_remove_ref();
	    ns.bind("Whois",whoisObj);

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
	    ccReg_EPP_i* myccReg_EPP_i = new ccReg_EPP_i(&mm,&ns,config);


	    // create session  use values from config
	    std::cerr << "CreateSession: max "  << config.GetSessionMax()  << " timeout " <<  config.GetSessionWait() <<  std::endl;
	    myccReg_EPP_i->CreateSession( config.GetSessionMax() , config.GetSessionWait() );


	    ccReg::timestamp_var ts;
	    std::cerr << "version: " << myccReg_EPP_i->version(ts) << std::endl;
	    std::cerr << "timestamp: " << ts << std::endl;

	    // must be called before loadZones
	    if (!myccReg_EPP_i->TestDatabaseConnect(db)) {
		std::cerr << "Database connection failed\n";
		LOG( ALERT_LOG , "connection  to Database [%s] failed" , db );
		exit(-2);
	    }

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
	    daemon_log(LOG_ERR, "Nameservice connection error");
	    daemon_pid_file_remove();
	    exit(-8);
	}
    
	catch (MailerManager::RESOLVE_FAILED) {
	    daemon_log(LOG_ERR, "Cannot connect to mailer");
	    daemon_pid_file_remove();
	    exit(-9);
	}
    
	catch (CORBA::SystemException& ex) {
	    daemon_log(LOG_ERR, "Caught a CORBA:: %s " , ex._name() );
	}
	catch (CORBA::Exception& ex) {
	    daemon_log(LOG_ERR, "Caught CORBA::Exception: %s " , ex._name() );
	}
	catch (omniORB::fatalException& fe) {
	    daemon_log(LOG_ERR,
		       "Caught  omniORB::fatalException: %s line: %d mesg: %s" ,
		       fe.file(), fe.line(), fe.errmsg()
		);
	}
	catch (...) {
	    daemon_log(LOG_ERR, "Unhandled exception");
	}

	/* Do a cleanup */
    finish:
	daemon_log(LOG_INFO, "Exiting...");
	daemon_pid_file_remove();
	return 0;
    }
}
