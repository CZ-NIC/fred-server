//
// Example code for implementing IDL interfaces in file ccReg.idl
//

#include <iostream>
#include <ccReg.hh>

//
// Example class implementing IDL interface ccReg::EPP
//
class ccReg_EPP_i: public POA_ccReg::EPP {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  //virtual ~ccReg_EPP_i();
public:
  // standard constructor
  ccReg_EPP_i();
  virtual ~ccReg_EPP_i();

  // methods corresponding to defined IDL attributes and operations
  ccReg::Response Login(const char* clientID, const char* pass, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID);
  ccReg::Response Logout(const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID);
  ccReg::Response ContactCheck(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID);
  ccReg::Response ContactInfo(const char* roid, const char* clTRID, ccReg::Contact_out c, CORBA::String_out errMsg, CORBA::String_out svTRID);
  ccReg::Response ContactDelete(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID);
  ccReg::Response ContactUpdate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID);
  ccReg::Response ContactCreate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID);
  char* clientID();

};

//
// Example implementational code for IDL interface ccReg::EPP
//
ccReg_EPP_i::ccReg_EPP_i(){
  // add extra constructor code here
}
ccReg_EPP_i::~ccReg_EPP_i(){
  // add extra destructor code here
}
//   Methods corresponding to IDL attributes and operations
ccReg::Response ccReg_EPP_i::Login(const char* clientID, const char* pass, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::Login(const char* clientID, const char* pass, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

ccReg::Response ccReg_EPP_i::Logout(const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::Logout(const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

ccReg::Response ccReg_EPP_i::ContactCheck(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::ContactCheck(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

ccReg::Response ccReg_EPP_i::ContactInfo(const char* roid, const char* clTRID, ccReg::Contact_out c, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::ContactInfo(const char* roid, const char* clTRID, ccReg::Contact_out c, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

ccReg::Response ccReg_EPP_i::ContactDelete(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::ContactDelete(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

ccReg::Response ccReg_EPP_i::ContactUpdate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::ContactUpdate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

ccReg::Response ccReg_EPP_i::ContactCreate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::ContactCreate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

char* ccReg_EPP_i::clientID(){
  // insert code here and remove the warning
  #warning "Code missing in function <char* ccReg_EPP_i::clientID()>"
}



// End of example implementational code

//
// Example class implementing IDL interface ccReg::Whois
//
class ccReg_Whois_i: public POA_ccReg::Whois {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  //virtual ~ccReg_Whois_i();
public:
  // standard constructor
  ccReg_Whois_i();
  virtual ~ccReg_Whois_i();

  // methods corresponding to defined IDL attributes and operations
  ccReg::DomainWhois* Domain(const char* name);

};

//
// Example implementational code for IDL interface ccReg::Whois
//
ccReg_Whois_i::ccReg_Whois_i(){
  // add extra constructor code here
}
ccReg_Whois_i::~ccReg_Whois_i(){
  // add extra destructor code here
}
//   Methods corresponding to IDL attributes and operations
ccReg::DomainWhois* ccReg_Whois_i::Domain(const char* name){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::DomainWhois* ccReg_Whois_i::Domain(const char* name)>"
}



// End of example implementational code

//
// Example class implementing IDL interface ccReg::Admin
//
class ccReg_Admin_i: public POA_ccReg::Admin {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  //virtual ~ccReg_Admin_i();
public:
  // standard constructor
  ccReg_Admin_i();
  virtual ~ccReg_Admin_i();

  // methods corresponding to defined IDL attributes and operations
  
};

//
// Example implementational code for IDL interface ccReg::Admin
//
ccReg_Admin_i::ccReg_Admin_i(){
  // add extra constructor code here
}
ccReg_Admin_i::~ccReg_Admin_i(){
  // add extra destructor code here
}
//   Methods corresponding to IDL attributes and operations


// End of example implementational code



int main(int argc, char** argv)
{
  try {
    // Initialise the ORB.
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    // Obtain a reference to the root POA.
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

    // We allocate the objects on the heap.  Since these are reference
    // counted objects, they will be deleted by the POA when they are no
    // longer needed.
    ccReg_EPP_i* myccReg_EPP_i = new ccReg_EPP_i();
    ccReg_Whois_i* myccReg_Whois_i = new ccReg_Whois_i();
    ccReg_Admin_i* myccReg_Admin_i = new ccReg_Admin_i();


    // Activate the objects.  This tells the POA that the objects are
    // ready to accept requests.
    PortableServer::ObjectId_var myccReg_EPP_iid = poa->activate_object(myccReg_EPP_i);
    PortableServer::ObjectId_var myccReg_Whois_iid = poa->activate_object(myccReg_Whois_i);
    PortableServer::ObjectId_var myccReg_Admin_iid = poa->activate_object(myccReg_Admin_i);


    // Obtain a reference to each object and output the stringified
    // IOR to stdout
    {
      // IDL interface: ccReg::EPP
      CORBA::Object_var ref = myccReg_EPP_i->_this();
      CORBA::String_var sior(orb->object_to_string(ref));
      std::cout << "IDL object ccReg::EPP IOR = '" << (char*)sior << "'" << std::endl;
    }

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

