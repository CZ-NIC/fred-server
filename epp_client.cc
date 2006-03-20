//                      This is the client.
//
// Usage: client <object reference>
//

#include <ccReg.hh>
#include <iostream.h>


//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
cout << "try" << endl ;
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);    
    ccReg::Contact *contact;
    CORBA::Short err;
    CORBA::String_var errMsg , svTR;
    int i;
//    char  *uri="file:///tmp/ccReg.ref" ;
//cout << "uri " << uri << endl ; 

      CORBA::Object_var obj = orb->string_to_object(argv[1]);

//    CORBA::Object_var obj = orb->string_to_object (uri);


  

//      EPP::EPP_var epp = ccReg::EPP::_narrow (obj);

    if (CORBA::is_nil (obj)) 
      {
        cout << "cannot bind to " << " ccReg"  << endl;
        return 1;
      }
    ccReg::EPP_var EPP = ccReg::EPP::_narrow (obj);



   EPP->ContactInfo("MAPET",  "XY-1234" , contact , errMsg , svTR );

    cout <<  contact->ROID <<   contact->Name  << " email "  << contact->Email << endl;
    cout <<  errMsg <<   svTR  << endl;


    
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
