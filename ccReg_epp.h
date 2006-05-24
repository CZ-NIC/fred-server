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
  ccReg::Response* PollAcknowledgement(CORBA::Long msgID, CORBA::Short& count, CORBA::Long& newmsgID, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* PollRequest(CORBA::Long& msgID, CORBA::Short& count, ccReg::timestamp& qDate, CORBA::String_out mesg, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ClientLogin(const char* ClID, const char* passwd, const char* newpass, const char* clTRID, CORBA::Long& clientID);
  ccReg::Response* ClientLogout(CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactCheck(const ccReg::Check& handle, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactInfo(const char* handle, ccReg::Contact_out c, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactDelete(const char* handle, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactUpdate(const char* handle, const ccReg::ContactChange& c, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* ContactCreate(const char* handle, const ccReg::ContactChange& c, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* NSSetCheck(const ccReg::Check& handle, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* NSSetInfo(const char* handle, ccReg::NSSet_out n, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* NSSetDelete(const char* handle, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* NSSetCreate(const char* handle, const char* authInfoPw, const ccReg::TechContact& tech, const ccReg::DNSHost& dns, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* NSSetUpdate(const char* handle, const char* authInfo , const char* authInfo_chg, const ccReg::DNSHost& dns_add, const ccReg::DNSHost& dns_rem, const ccReg::TechContact& tech_add, const ccReg::TechContact& tech_rem, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* NSSetTransfer(const char* handle, const char* authInfo, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainCheck(const ccReg::Check& fqdn, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainInfo(const char* fqdn, ccReg::Domain_out d, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainDelete(const char* fqdn, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainUpdate(const char* fqdn, const char* registrant_chg,const char* authInfo,  const char* authInfo_chg, const char* nsset_chg, const ccReg::AdminContact& admin_add, const ccReg::AdminContact& admin_rem, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainCreate(const char* fqdn, const char* Registrant, const char* nsset, const char* AuthInfoPw, CORBA::Short period, const ccReg::AdminContact& admin, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainRenew(const char* fqdn, ccReg::timestamp curExpDate, CORBA::Short period, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainTransfer(const char* fqdn, const char* registrant, const char* authInfo, CORBA::Long clientID, const char* clTRID);
  ccReg::Response* DomainTrade(const char* fqdn, const char* old_registrant, const char* new_registrant, const char* authInfo, CORBA::Long clientID, const char* clTRID);
 
};
