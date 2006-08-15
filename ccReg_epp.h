#include "admin.h"
//
//  class implementing IDL interface ccReg::EPP
//
class ccReg_EPP_i: public POA_ccReg::EPP,
                   public PortableServer::RefCountServantBase {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  //virtual ~ccReg_EPP_i();
  ccReg::Admin_ptr admin;
public:
  // standard constructor
  ccReg_EPP_i(ccReg::Admin_ptr admin);
  virtual ~ccReg_EPP_i();

  // vrati administrativni objekt
  ccReg::Admin_ptr getAdmin();

  // test spojeni na databazi
  bool TestDatabaseConnect(char *db);

  char* version(); // vraceni cisla verze 

  // obecna check funkce
  ccReg::Response*  ObjectCheck( short act , char * table , char *fname , const ccReg::Check& chck , ccReg::CheckResp_out   a, CORBA::Long clientID, const char* clTRID , const char* XML );

  // methods corresponding to defined IDL attributes and operations
  ccReg::Response* GetTransaction(CORBA::Long clientID, const char* clTRID, CORBA::Short errCode );
  ccReg::Response* PollAcknowledgement(CORBA::Long msgID, CORBA::Short& count, CORBA::Long& newmsgID, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* PollRequest(CORBA::Long& msgID, CORBA::Short& count, ccReg::timestamp& qDate, CORBA::String_out mesg, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ClientLogin(const char* ClID, const char* passwd, const char* newpass, const char* clTRID, const char* XML ,  CORBA::Long& clientID , const char* certID , ccReg::Languages lang);
  ccReg::Response* ClientLogout(CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactCheck(const ccReg::Check& handle, ccReg::CheckResp_out a, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactInfo(const char* handle, ccReg::Contact_out c, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactDelete(const char* handle, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactUpdate(const char* handle, const ccReg::ContactChange& c, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactCreate(const char* handle, const ccReg::ContactChange& c, ccReg::timestamp& crDate, CORBA::Long clientID, const char* clTRID , const char* XML );
  ccReg::Response* ContactTransfer(const char* handle, const char* authInfo, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* NSSetCheck(const ccReg::Check& handle, ccReg::CheckResp_out  a, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* NSSetInfo(const char* handle, ccReg::NSSet_out n, CORBA::Long clientID, const char* clTRID , const char*  XML);
  ccReg::Response* NSSetDelete(const char* handle, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* NSSetCreate(const char* handle, const char* authInfoPw, const ccReg::TechContact& tech, const ccReg::DNSHost& dns, ccReg::timestamp& crDate, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* NSSetUpdate(const char* handle, const char* authInfo_chg, const ccReg::DNSHost& dns_add, const ccReg::DNSHost& dns_rem, const ccReg::TechContact& tech_add, const ccReg::TechContact& tech_rem, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* NSSetTransfer(const char* handle, const char* authInfo, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* DomainCheck(const ccReg::Check& fqdn, ccReg::CheckResp_out  a, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* DomainInfo(const char* fqdn, ccReg::Domain_out d, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* DomainDelete(const char* fqdn, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* DomainUpdate(const char* fqdn, const char* registrant_chg,  const char* authInfo_chg, const char* nsset_chg, const ccReg::AdminContact& admin_add, const ccReg::AdminContact& admin_rem, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID , const char* XML ,const ccReg::ExtensionList& ext );
  ccReg::Response* DomainCreate(const char* fqdn, const char* Registrant, const char* nsset, const char* AuthInfoPw, CORBA::Short period, const ccReg::AdminContact& admin,  ccReg::timestamp& crDate, ccReg::timestamp& exDate,  CORBA::Long clientID, const char* clTRID, const char* XML , const ccReg::ExtensionList& ext );
  ccReg::Response* DomainRenew(const char* fqdn, ccReg::timestamp curExpDate, CORBA::Short period, ccReg::timestamp& exDate, CORBA::Long clientID, const char* clTRID , const char* XML, const ccReg::ExtensionList& ext );
  ccReg::Response* DomainTransfer(const char* fqdn,  const char* authInfo, CORBA::Long clientID, const char* clTRID  , const char* XML);
 
  // pro public interface
  ccReg::RegistrarList* getRegistrars();
  ccReg::Registrar* getRegistrarByHandle(const char* handle);

  // primitivni vypis
  ccReg::Lists*  ObjectList( char* table , char *fname );
  ccReg::Lists* ListRegistrar();
  ccReg::Lists* ListDomain();
  ccReg::Lists* ListContact();
  ccReg::Lists* ListNSSet();

  // EPP vypis
  ccReg::Response* ContactList(ccReg::Lists_out contacts, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* NSSetList(ccReg::Lists_out nssets, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* DomainList(ccReg::Lists_out domains, CORBA::Long clientID, const char* clTRID, const char* XML);

  // testovaci fce na typ objektu
  ccReg::RegObjectType getRegObjectType(const char* objectName);
 
private:
char database[128]; // nazev spojeni na databazi
};
