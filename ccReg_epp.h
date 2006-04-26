//
//  class implementing IDL interface ccReg::EPP
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
  ccReg::Response* GetTransaction(CORBA::Long clientID, const char* clTRID, CORBA::Short errCode);
  ccReg::Response* ClientLogin(const char* ClID, const char* passwd, const char* newpass, const char* clTRID, CORBA::Long& clientID);
  ccReg::Response* ClientLogout(CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactCheck(const ccReg::Check& handle, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactInfo(const char* handle, ccReg::Contact_out c, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactDelete(const char* handle, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactUpdate(const char* handle, const ccReg::Contact& c, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactCreate(const char* handle, const ccReg::Contact& c, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* HostCheck(const ccReg::Check& name, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* HostInfo(const char* name, ccReg::Host_out h, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* HostDelete(const char* name, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* HostCreate(const char* name, const ccReg::Host& h, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* HostUpdate(const char* name, const ccReg::Host& h, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* NSSetCheck(const ccReg::Check& handle, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* NSSetInfo(const char* handle, ccReg::NSSet_out n, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* NSSetDelete(const char* handle, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* NSSetCreate(const char* handle, const ccReg::NSSet& n, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* NSSetUpdate(const char* handle, const ccReg::NSSet& n, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainCheck(const ccReg::Check& fqdn, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainInfo(const char* fqdn, ccReg::Domain_out d, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainDelete(const char* fqdn, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainUpdate(const char* fqdn, const ccReg::Domain& d, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainCreate(const char* fqdn, const ccReg::Domain& d, CORBA::Long clientID, const char* clTRID);

};
