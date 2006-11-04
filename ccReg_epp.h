
#include "countrycode.h"
#include "messages.h"

//
//  class implementing IDL interface ccReg::EPP
//
class ccReg_EPP_i: public POA_ccReg::EPP,
                   public PortableServer::RefCountServantBase {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  //virtual ~ccReg_EPP_i();
public:
  // standard constructor
  ccReg_EPP_i();
  virtual ~ccReg_EPP_i();



  // test spojeni na databazi
  bool TestDatabaseConnect(const char *db);

  // nacteni zone z tabulky zones
  int loadZones(); // load zones
  // parametry zone
  int GetZoneExPeriodMin(int z);
  int GetZoneExPeriodMax(int z);
  int GetZoneValPeriod(int z);
  int GetZoneDotsMax( int z);
  bool GetZoneEnum(int z);
  char * GetZoneFQDN( int z);
 


  ccReg::Zones * getZones(){ return zone; }
  int getZone( const char *fqdn );
  int getZoneMax( const char *fqdn );
  int getZZ( const char *fqdn  , bool compare );
  int getFQDN( char *FQDN , const char *fqdn );
  bool testFQDN(  const char *fqdn );

// parse extension
  void GetValExpDateFromExtension( char *valexpDate , const ccReg::ExtensionList& ext );


  
  // vraceni cisla verze
  char* version(ccReg::timestamp_out datetime);

  int DefaultContactHandlePeriod(){ return 0; } // ochrana lhuta 1 mesic na vse
  int DefaultDomainNSSetPeriod(){ return 0; }
  int DefaultDomainFQDNPeriod(){ return 0; }

  // true vse zobrazovat false vse skryt pro disclose flags
  bool DefaultPolicy(){return true;}

   int DefaultValExpInterval(){ return 14; } // ochrana lhuta 14 dni pro datum expirace validace
  // podpora disclose parametru
  bool get_DISCLOSE( bool db );
  char update_DISCLOSE( bool  d   ,  ccReg::Disclose flag );
  bool setvalue_DISCLOSE( bool  d   ,  ccReg::Disclose flag );

  //  otestovani retezce jestli neni nahodou NULL VALUE
//   bool is_null( const char *str );

  // nacita tabulku zemi enum_country z databaze 
  int LoadCountryCode();
  // testuje kod zeme 
  bool TestCountryCode( const char *cc );

  int LoadErrorMessages(); 
  char * GetErrorMessage(int err , int lang );

  int LoadReasonMessages(); 
  char * GetReasonMessage(int err , int lang);


  // obecna list funkce
  ccReg::Response* FullList(short act , const char *table , char *fname  ,  ccReg::Lists_out  list , CORBA::Long clientID, const char* clTRID, const char* XML);


  // obecna check funkce
  ccReg::Response*  ObjectCheck( short act , char * table , char *fname , const ccReg::Check& chck , ccReg::CheckResp_out   a, CORBA::Long clientID, const char* clTRID , const char* XML );

  // methods corresponding to defined IDL attributes and operations
  ccReg::Response* GetTransaction(CORBA::Long clientID, const char* clTRID, CORBA::Short errCode );
//  ccReg::Response* PollAcknowledgement(const char* msgID, CORBA::Short& count, CORBA::String_out newmsgID,  CORBA::Long clientID, const char* clTRID, const char* XML);
//  ccReg::Response* PollRequest(CORBA::String_out msgID, CORBA::Short& count, ccReg::timestamp_out qDate, CORBA::String_out mesg, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* PollAcknowledgement(CORBA::Long msgID, CORBA::Short& count, CORBA::Long& newmsgID, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* PollRequest(CORBA::Long& msgID, CORBA::Short& count, ccReg::timestamp_out qDate, CORBA::String_out mesg, CORBA::Long clientID, const char* clTRID, const char* XML);

  ccReg::Response* ClientLogin(const char* ClID, const char* passwd, const char* newpass, const char* clTRID, const char* XML ,  CORBA::Long& clientID , const char* certID , ccReg::Languages lang);
  ccReg::Response* ClientLogout(CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ClientCredit(CORBA::Long& credit, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactCheck(const ccReg::Check& handle, ccReg::CheckResp_out a, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactInfo(const char* handle, ccReg::Contact_out c, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactDelete(const char* handle, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactUpdate(const char* handle, const ccReg::ContactChange& c, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactCreate(const char* handle, const ccReg::ContactChange& c, ccReg::timestamp_out crDate, CORBA::Long clientID, const char* clTRID , const char* XML );
  ccReg::Response* ContactTransfer(const char* handle, const char* authInfo, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* NSSetCheck(const ccReg::Check& handle, ccReg::CheckResp_out  a, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* NSSetInfo(const char* handle, ccReg::NSSet_out n, CORBA::Long clientID, const char* clTRID , const char*  XML);
  ccReg::Response* NSSetDelete(const char* handle, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* NSSetCreate(const char* handle, const char* authInfoPw, const ccReg::TechContact& tech, const ccReg::DNSHost& dns, ccReg::timestamp_out crDate, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* NSSetUpdate(const char* handle, const char* authInfo_chg, const ccReg::DNSHost& dns_add, const ccReg::DNSHost& dns_rem, const ccReg::TechContact& tech_add, const ccReg::TechContact& tech_rem, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* NSSetTransfer(const char* handle, const char* authInfo, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* DomainCheck(const ccReg::Check& fqdn, ccReg::CheckResp_out  a, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* DomainInfo(const char* fqdn, ccReg::Domain_out d, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* DomainDelete(const char* fqdn, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* DomainUpdate(const char* fqdn, const char* registrant_chg,  const char* authInfo_chg, const char* nsset_chg, const ccReg::AdminContact& admin_add, const ccReg::AdminContact& admin_rem, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID , const char* XML ,const ccReg::ExtensionList& ext );
  ccReg::Response* DomainCreate(const char* fqdn, const char* Registrant, const char* nsset, const char* AuthInfoPw, CORBA::Short period, const ccReg::AdminContact& admin,  ccReg::timestamp_out crDate, ccReg::date_out exDate,  CORBA::Long clientID, const char* clTRID, const char* XML , const ccReg::ExtensionList& ext );
  ccReg::Response* DomainRenew(const char* fqdn, const char* curExpDate, CORBA::Short period, ccReg::timestamp_out exDate, CORBA::Long clientID, const char* clTRID, const char* XML, const ccReg::ExtensionList& ext);
  ccReg::Response* DomainTransfer(const char* fqdn,  const char* authInfo, CORBA::Long clientID, const char* clTRID  , const char* XML);
 

  // EPP vypis
  ccReg::Response* ContactList(ccReg::Lists_out contacts, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* NSSetList(ccReg::Lists_out nssets, CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* DomainList(ccReg::Lists_out domains, CORBA::Long clientID, const char* clTRID, const char* XML);

 
private:
Mesg *ErrorMsg;
Mesg *ReasonMsg;
CountryCode *CC;
char database[128]; // nazev spojeni na databazi
ccReg::Zones *zone;
int max_zone;
};
