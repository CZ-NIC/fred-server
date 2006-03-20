
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
  ccReg::Response Login(const char* clientID, const char* pass, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out
svTRID);
  ccReg::Response Logout(const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID);
  ccReg::Response ContactCheck(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID);
  ccReg::Response ContactInfo(const char* roid, const char* clTRID, ccReg::Contact_out c, CORBA::String_out errMsg, CORBA::String_out svTRID);
  ccReg::Response ContactDelete(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID);
  ccReg::Response ContactUpdate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID);
  ccReg::Response ContactCreate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID);
  char* clientID();

};

