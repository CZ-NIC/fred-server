#ifdef HAVE_ANSI_CPLUSPLUS_HEADERS
#include <iostream>
#include <fstream>
#else
#include <iostream.h>
#include <fstream.h>
#endif

#include "ccReg.hh"
#include "ccReg_epp.h"


// spusteni ccReg servru

// End of example implementational code
int main(int argc, char** argv)
{
  try {
    // Initialise the ORB.
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    // Obtain a reference to the root POA.
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);
  
    // create POA for persistent references
    PortableServer::POAManager_var mgr = poa->the_POAManager();
    CORBA::PolicyList pols;
    pols.length(2);
    pols[0] = poa->create_lifespan_policy(PortableServer::PERSISTENT);
    pols[1] = poa->create_id_assignment_policy(PortableServer::USER_ID);
    poa = poa->create_POA("ccRegPOA",mgr.in(),pols);
    mgr->activate();

    // We allocate the objects on the heap.  Since these are reference
    // counted objects, they will be deleted by the POA when they are no
    // longer needed.
    ccReg_EPP_i* myccReg_EPP_i = new ccReg_EPP_i();
//    ccReg_Whois_i* myccReg_Whois_i = new ccReg_Whois_i();
  //  ccReg_Admin_i* myccReg_Admin_i = new ccReg_Admin_i();


    // Activate the objects.  This tells the POA that the objects are
    // ready to accept requests.
    PortableServer::ObjectId_var myccReg_EPP_iid = PortableServer::string_to_ObjectId("ccReg");
    poa->activate_object_with_id(myccReg_EPP_iid,myccReg_EPP_i);
 //   PortableServer::ObjectId_var myccReg_Whois_iid = poa->activate_object(myccReg_Whois_i);
   // PortableServer::ObjectId_var myccReg_Admin_iid = poa->activate_object(myccReg_Admin_i);


    // Obtain a reference to each object and output the stringified
    // IOR to stdout
    {
      // IDL interface: ccReg::EPP
//      CORBA::Object_var ref = myccReg_EPP_i->_this();
      CORBA::Object_var ref = poa->id_to_reference(myccReg_EPP_iid);
      CORBA::String_var sior(orb->object_to_string(ref));
      std::cout << "IDL object ccReg::EPP IOR = '" << (char*)sior << "'" << std::endl;

     ofstream fout ("/tmp/ccReg.ref");
//     of "IOR='" << (char*)sior << "'"  << endl;
     fout <<  orb->object_to_string(ref)  ; //  (char*)sior ;
     fout.close ();

    }

/*
    {
      // IDL interface: ccReg::Whois
      CORBA::Object_var ref = myccReg_Whois_i->_this();
      CORBA::String_var sior(orb->object_to_string(ref));
      std::cout << "IDL object ccReg::Whois IOR = '" << (char*)sior << "'" << std::endl;
    }

    {
      // IDL interface: ccReg::Admin
      CORBA::Object_var ref = myccReg_Admin_i->_this();
      CORBA::String_var sior(orb->object_to_string(ref));
      std::cout << "IDL object ccReg::Admin IOR = '" << (char*)sior << "'" << std::endl;
    }

*/

    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    orb->run();
    orb->destroy();
  }
  catch(CORBA::TRANSIENT&) {
    cerr << "Caught system exception TRANSIENT -- unable to contact the "
         << "server." << endl;
  }
  catch(CORBA::SystemException& ex) {
    cerr << "Caught a CORBA::" << ex._name() << endl;
  }
  catch(CORBA::Exception& ex) {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
  }
  catch(omniORB::fatalException& fe) {
    cerr << "Caught omniORB::fatalException:" << endl;
    cerr << "  file: " << fe.file() << endl;
    cerr << "  line: " << fe.line() << endl;
    cerr << "  mesg: " << fe.errmsg() << endl;
  }
  return 0;
}

