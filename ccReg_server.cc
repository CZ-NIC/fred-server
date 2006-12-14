#include "ccReg.hh"
#include "ccReg_epp.h"

#include <signal.h>
#include <unistd.h>
#include <iostream>

#include "log.h"
#include "conf.h"
#include "nameservice.h"

#ifndef CONFIG_FILE
#define CONFIG_FILE "ccReg.conf"
#endif

// proper cleanup in case of signal
static CORBA::ORB_ptr orbToShutdown = NULL;
static void signalHandler(int signal)
{
	if (orbToShutdown) orbToShutdown->shutdown(0);
}

int main(int argc, char** argv)
{
  try {
    // database connection settings
    char db[256];

    Conf config; // READ CONFIG  file
    // read config file
    if (!config.ReadConfigFile(CONFIG_FILE)) {
      std::cerr << "Cannot read config file\n";
      exit(-1);
    }
    strcpy( db , config.GetDBconninfo() );
    std::cerr << "DATABASE: "  << db << std::endl;

#ifdef SYSLOG
    std::cerr << "start syslog at level " 
              <<  config.GetSYSLOGlevel()   << std::endl;
    std::cerr << "start syslog facility local" 
              <<  config.GetSYSLOGlocal()   << std::endl;
    setlogmask ( LOG_UPTO(  config.GetSYSLOGlevel()  )   );
    openlog ( "ccReg", LOG_CONS | LOG_PID | LOG_NDELAY,  config.GetSYSLOGfacility() );
#endif

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
    std::string nameServiceIOR = "corbaname::";
    nameServiceIOR += config.GetNameService();
    std::cerr << "nameServiceIOR: " <<   nameServiceIOR << std::endl;
    NameService ns(orb,nameServiceIOR);
    
    MailerManager mm(&ns);
    ccReg_EPP_i* myccReg_EPP_i = new ccReg_EPP_i(&mm);

    // create session TODO config
    std::cerr << "CreateSession: max "  << config.GetSessionMax()  << " timeout " <<  config.GetSessionWait() <<  std::endl;
    myccReg_EPP_i->CreateSession( config.GetSessionMax() , config.GetSessionWait() );


    ccReg::timestamp_var ts;
    std::cerr << "version: " << myccReg_EPP_i->version(ts) << std::endl;
    std::cerr << "timestamp: " << ts << std::endl;

    // must be called before loadZones
    if (!myccReg_EPP_i->TestDatabaseConnect(db)) {
      std::cerr << "Database connection failed\n";
      exit(-2);
    } 

    if( myccReg_EPP_i->loadZones() <= 0  ){
      std::cerr << "Database error: load zones\n";
      exit(-4);
    }
 
    if( myccReg_EPP_i->LoadCountryCode() <= 0 ){  /// nacti ciselnik zemi
      std::cerr << "Database error: load country code\n";
      exit(-5);
    }
       
    if( myccReg_EPP_i->LoadErrorMessages() <= 0 ){  // nacti chybove zpravy
      std::cerr << "Database error: load country error messages\n";
      exit(-6);
    }

     if( myccReg_EPP_i->LoadReasonMessages()  <= 0 ){   // nacti reason zpravy
      std::cerr << "Database error: load country reason messages\n";
      exit(-7);
    }

    PortableServer::ObjectId_var myccReg_EPP_iid = 
      PortableServer::string_to_ObjectId("Register");
    poa->activate_object_with_id(myccReg_EPP_iid,myccReg_EPP_i);
    myccReg_EPP_i->_remove_ref();
    ns.bind("EPP",myccReg_EPP_i->_this());

    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();
    LOG(DEBUG_LOG , "Starting Fred rifd");
    // disconnect from terminal
    setsid(); 
    orb->run();
    orb->destroy();
  }
  catch (NameService::NOT_RUNNING&) {
    std::cerr << "Nameservice connection error\n";
    exit(-8);
  }
  catch (MailerManager::RESOLVE_FAILED) {
    std::cerr << "Cannot connect to mailer\n";
    exit(-9);
  }
  catch (CORBA::SystemException& ex) {
    LOG(ERROR_LOG,"Caught a CORBA:: %s " , ex._name() );
  }
  catch (CORBA::Exception& ex) {
    LOG(ERROR_LOG,"Caught CORBA::Exception: %s " , ex._name() );
  }
  catch (omniORB::fatalException& fe) {
    LOG(
      ERROR_LOG,
      "Caught  omniORB::fatalException: %s line: %d mesg: %s" ,
       fe.file(),fe.line(),fe.errmsg()
    );
  }
  catch (...) {
    LOG(
      ERROR_LOG,
      "Unhandled exception"
    );        
  }

#ifdef SYSLOG
  closelog ();
#endif
  return 0;
}
