
//
// Example class implementing IDL interface ccReg::EPP
//

class ccReg_EPP_i: public POA_ccReg::EPP {

public:
  // standard constructor
  ccReg_EPP_i();
  virtual ~ccReg_EPP_i();


  // methods corresponding to defined IDL attributes and operations
  ccReg::Response* ClientLogin(const char* ClID, const char* clTRID, CORBA::Long& clientID);
  ccReg::Response* ClientLogout(CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactCheck(const char* handle, CORBA::Boolean& avial, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactInfo(const char* handle, ccReg::Contact_out c, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactDelete(const char* handle, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactUpdate(const char* handle, const ccReg::Contact& c, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactCreate(const char* handle, const ccReg::Contact& c, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* HostCheck(const char* name, CORBA::Boolean& avial, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* HostInfo(const char* name, ccReg::Host_out h, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* HostDelete(const char* name, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* HostCreate(const char* name, const ccReg::Host& h, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* HostUpdate(const char* name, const ccReg::Host& h, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainCheck(const char* fqdn, CORBA::Boolean& avial, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainInfo(const char* fqdn, ccReg::Domain_out d, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainDelete(const char* fqdn, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainUpdate(const char* fqdn, const ccReg::Domain& d, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainCreate(const char* fqdn, const ccReg::Domain& d, CORBA::Long clientID, const char* clTRID);


private:

};

