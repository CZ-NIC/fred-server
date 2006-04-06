
//
// Example class implementing IDL interface ccReg::EPP
//

class ccReg_EPP_i: public POA_ccReg::EPP {

public:
  // standard constructor
  ccReg_EPP_i();
  virtual ~ccReg_EPP_i();


  // methods corresponding to defined IDL attributes and operations
  ccReg::Response* ClientLogin(const char* ClID, const char* clTRID, CORBA::Short& clientID);
  ccReg::Response* ClientLogout(CORBA::Short clientID , const char* clTRID);
  ccReg::Response* ContactCheck(const char* roid, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* ContactInfo(const char* roid, ccReg::Contact_out c, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* ContactDelete(const char* roid, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* ContactUpdate(const ccReg::Contact& c, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* ContactCreate(const ccReg::Contact& c, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* HostCheck(const char* name, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* HostInfo(const char* name, ccReg::Host_out h, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* HostDelete(const char* name, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* HostCreate(const ccReg::Host& h, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* HostUpdate(const ccReg::Host& h, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* DomainCheck(const char* name, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* DomainInfo(const char* name, ccReg::Domain_out d, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* DomainDelete(const char* name, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* DomainUpdate(const ccReg::Domain& d, CORBA::Short clientID, const char* clTRID);
  ccReg::Response* DomainCreate(const ccReg::Domain& d, CORBA::Short clientID, const char* clTRID);

private:

};

