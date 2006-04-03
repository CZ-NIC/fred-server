
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
  ccReg::Response* ClientLogin(const char* ClID);
  ccReg::Response* ClientLogout();
  ccReg::Response* ContactCheck(const char* roid);
  ccReg::Response* ContactInfo(const char* roid, ccReg::Contact_out c);
  ccReg::Response* ContactDelete(const char* roid);
  ccReg::Response* ContactUpdate(const ccReg::Contact& c);
  ccReg::Response* ContactCreate(const ccReg::Contact& c);
  ccReg::Response* HostCheck(const char* name);
  ccReg::Response* HostInfo(const char* name, ccReg::Host_out h);
  ccReg::Response* HostDelete(const char* name);
  ccReg::Response* HostCreate(const ccReg::Host& h);
  ccReg::Response* HostUpdate(const ccReg::Host& h);
  ccReg::Response* DomainCheck(const char* name);
  ccReg::Response* DomainInfo(const char* name, ccReg::Domain_out d);
  ccReg::Response* DomainDelete(const char* name);
  ccReg::Response* DomainUpdate(const ccReg::Domain& d);
  ccReg::Response* DomainCreate(const ccReg::Domain& d);


private:
int clientID; // id prihlaseneho registratora
};

