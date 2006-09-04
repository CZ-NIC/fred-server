#ifdef HAVE_ANSI_CPLUSPLUS_HEADERS
#include <iostream>
#include <fstream>
#else
#include <iostream.h>
#include <fstream.h>
#endif

#include "ccReg.hh"
#include "ccReg_epp.h"
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

static char database[128];

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
    char db[256];

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
    PortableServer::POA_var poa = rootPOA->create_POA("ccRegPOA",mgr.in(),pols);
    mgr->activate();

    // nastaveni databaze
    Conf config; // READ CONFIG  file
    
    // read config file
    if(  config.ReadConfigFile(  CONFIG_FILE ) )
    {
      strcpy( db , config.GetDBconninfo() );
      std::cout << "DATABASE: "  << db << endl;

      // prepare NameService object
      std::string nameServiceIOR = "corbaname::";
      nameServiceIOR += config.GetNameService();
      NameService ns(orb,nameServiceIOR);

      ccReg_Admin_i* myccReg_Admin_i = new ccReg_Admin_i(db);
      CORBA::Object_var adminObj = 
        rootPOA->servant_to_reference(myccReg_Admin_i);    
      myccReg_Admin_i->_remove_ref();
      ccReg::Admin_var admin = ccReg::Admin::_narrow(adminObj);
      ns.bind("Admin",adminObj);

      ccReg_Whois_i* myccReg_Whois_i = new ccReg_Whois_i(db);
      CORBA::Object_var whoisObj = 
        rootPOA->servant_to_reference(myccReg_Whois_i);
      myccReg_Whois_i->_remove_ref();
      ccReg::Whois_var whois = ccReg::Whois::_narrow(whoisObj);
      ns.bind("Whois",whoisObj);

      ccReg_EPP_i* myccReg_EPP_i = new ccReg_EPP_i(admin, whois);
     
      // pokud projde uspesne test na pripojeni k databazi
      if( myccReg_EPP_i->TestDatabaseConnect( db) ) 
      {
        // load zones
        myccReg_EPP_i->loadZones();
        // Activate the objects.  This tells the POA that the objects are
        // ready to accept requests.
        PortableServer::ObjectId_var myccReg_EPP_iid = 
          PortableServer::string_to_ObjectId("ccReg");
        poa->activate_object_with_id(myccReg_EPP_iid,myccReg_EPP_i);
        // to delete implementaion on POA destruction
        myccReg_EPP_i->_remove_ref();
#ifdef SYSLOG
        cout << "start syslog at level " 
             <<  config.GetSYSLOGlevel()   << endl;
        cout << "start syslog facility local" 
             <<  config.GetSYSLOGlocal()   << endl;
        setlogmask ( LOG_UPTO(  config.GetSYSLOGlevel()  )   );
        openlog ( "ccReg", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
#endif
        // IDL interface: ccReg::EPP
        CORBA::Object_var ref = poa->id_to_reference(myccReg_EPP_iid);
        ns.bind("EPP",ref);
        CORBA::String_var sior(orb->object_to_string(ref));
        std::cout << "IDL object ccReg::EPP IOR = '" 
                  << (char*)sior << "'" << std::endl;

        ofstream fout ("/tmp/ccReg.ref");
        fout << (char*)sior ; // orb->object_to_string(ref)  ; //  
        fout.close ();

        // Obtain a POAManager, and tell the POA to start accepting
        // requests on its objects.
        PortableServer::POAManager_var pman = poa->the_POAManager();
        pman->activate();
        LOG(DEBUG_LOG , "Starting ccReg_server");
        // odpoji se od terminalu a zalozi vlastni skupinu
        setsid(); 
        orb->run();
        orb->destroy();
      }
    }
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
