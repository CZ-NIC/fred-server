#ifdef HAVE_ANSI_CPLUSPLUS_HEADERS
#include <iostream>
#include <fstream>
#else
#include <iostream.h>
#include <fstream.h>
#endif

#include "ccReg.hh"
#include "ccReg_epp.h"
#include "whois.h"

#include <signal.h>
#include <unistd.h>

// pouzivani LOGu 
#include "log.h"

// nacitani config souboru
#include "conf.h"

#include "nameservice.h"

#ifndef CONFIG_FILE
#define CONFIG_FILE "ccReg.conf"
#endif


static CORBA::ORB_ptr orbToShutdown = NULL;
static void signalHandler(int signal)
{
	if (orbToShutdown) orbToShutdown->shutdown(0);
}
// spusteni ccReg servru

// End of example implementational code
int main(int argc, char** argv)
{
  try {
    // nastaveni databaze
    char db[256];
    Conf config; // READ CONFIG  file
    // read config file
    if (!config.ReadConfigFile(CONFIG_FILE)) {
      std::cout << "Cannot read config file\n";
      exit(-1);
    }
    strcpy( db , config.GetDBconninfo() );
    std::cout << "DATABASE: "  << db << endl;

#ifdef SYSLOG
    cout << "start syslog at level " 
         <<  config.GetSYSLOGlevel()   << endl;
    cout << "start syslog facility local" 
         <<  config.GetSYSLOGlocal()   << endl;
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
      rootPOA->create_POA("ccRegPOA",mgr.in(),pols);

    // prepare NameService object
    std::string nameServiceIOR = "corbaname::";
    nameServiceIOR += config.GetNameService();
    cout << "nameServiceIOR: " <<   nameServiceIOR << endl;
    NameService ns(orb,nameServiceIOR);

    PortableServer::ObjectId_var whoisObjectId = 
      PortableServer::string_to_ObjectId("Whois");
    ccReg_Whois_i* myccReg_Whois_i = new ccReg_Whois_i(db);
    poa->activate_object_with_id(whoisObjectId,myccReg_Whois_i);
    CORBA::Object_var whoisObj = myccReg_Whois_i->_this();
    myccReg_Whois_i->_remove_ref();
    ns.bind("Whois",whoisObj);

    // pristup na admin a whois je mozna z EPP vyhodit
    ccReg::Whois_var whois = ccReg::Whois::_narrow(whoisObj);
 
    ccReg_EPP_i* myccReg_EPP_i = new ccReg_EPP_i();

    ccReg::timestamp_var ts;
    cout << "version: " << myccReg_EPP_i->version(ts) << endl;
    cout << "timestamp: " << ts << endl;

    // musi byt zavolano pred loadZones
    if (!myccReg_EPP_i->TestDatabaseConnect(db)) {
      std::cout << "Database connection failed\n";
      exit(-2);
    } 

    if( myccReg_EPP_i->loadZones() <= 0  ){
      std::cout << "Database error: load zones\n";
      exit(-4);
    }
 
    if( myccReg_EPP_i->LoadCountryCode() <= 0 ){  /// nacti ciselnik zemi
      std::cout << "Database error: load country code\n";
      exit(-5);
    }
       
    if( myccReg_EPP_i->LoadErrorMessages() <= 0 ){  // nacti chybove zpravy
      std::cout << "Database error: load country error messages\n";
      exit(-6);
    }

     if( myccReg_EPP_i->LoadReasonMessages()  <= 0 ){   // nacti reason zpravy
      std::cout << "Database error: load country reason messages\n";
      exit(-7);
    }


    PortableServer::ObjectId_var myccReg_EPP_iid = 
      PortableServer::string_to_ObjectId("ccReg");
    poa->activate_object_with_id(myccReg_EPP_iid,myccReg_EPP_i);
    
    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();
    LOG(DEBUG_LOG , "Starting pif_server");
    // odpoji se od terminalu a zalozi vlastni skupinu
    setsid(); 
    orb->run();
    orb->destroy();
  }
  catch (CORBA::TRANSIENT&) {
    LOG(
      ERROR_LOG,
      "Caught system exception TRANSIENT -- unable to contact the server."
    );
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
  catch (NameService::NOT_RUNNING) {
    LOG(ERROR_LOG,"NameService not running or cannot be found");
  }

#ifdef SYSLOG
  closelog ();
#endif
  return 0;
}
