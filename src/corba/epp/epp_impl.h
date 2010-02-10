#include <memory>
#include "countrycode.h"
#include "messages.h"
#include "corba/mailer_manager.h"
#include "old_utils/dbsql.h"
#include "register/register.h"

#include "conf/manager.h"
#include <vector>

struct Session
{
  int clientID;
  int registrarID;
  int language;
  long long timestamp;
};

class Conf;
//
//  class implementing IDL interface ccReg::EPP
//
class ccReg_EPP_i : public POA_ccReg::EPP,
                    public PortableServer::RefCountServantBase
{
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  //virtual ~ccReg_EPP_i();
  std::string database; // connection string to database
  MailerManager *mm;
  Database::Manager *dbman;
  
  NameService *ns;
  Config::Conf& conf;
  DB db;
  std::auto_ptr<Register::Manager> regMan;


  void extractEnumDomainExtension(std::string&, ccReg::Disclose &publish, 
          const ccReg::ExtensionList&);

public:
  struct DB_CONNECT_FAILED
  {
  };
  // standard constructor
      ccReg_EPP_i(const std::string &db, MailerManager *_mm, NameService *_ns,
        Config::Conf& _conf) throw (DB_CONNECT_FAILED);
  virtual ~ccReg_EPP_i();

  // get zones parametrs
  int GetZoneExPeriodMin(DB *db, int id);
  int GetZoneExPeriodMax(DB *db, int id);
  int GetZoneValPeriod(DB *db, int id);
  int GetZoneDotsMax(DB *db, int id);
  bool GetZoneEnum(DB *db, int id);
  std::string GetZoneFQDN(DB *db, int id);
  std::vector<int> GetAllZonesIDs(DB *db);

  int getZone(DB *db, const char *fqdn);
  int getZoneMax(DB *db, const char *fqdn);
  int getFQDN(DB *db, char *FQDN, const char *fqdn);


  // parse extension for domain enum.exdate
  void GetValExpDateFromExtension(char *valexpDate,
    const ccReg::ExtensionList& ext);

  // session manager
  void CreateSession(int max, long wait);
  // startiong session for registrar with language
  bool LoginSession(long loginID, int registrarID, int language);
  //  logout session
  bool LogoutSession(long loginID);
  void GarbageSesion(); // clear unused sessions
  // get RegistrarID
  int GetRegistrarID(int clientID);
  // get uses language
  int GetRegistrarLang(int clientID);
  //  get number of active sessions
  int GetNumSession()
  {
    return numSession;
  }
  ;

  // send    exception ServerIntError
  void ServerInternalError(const char *fce, const char *svTRID="DUMMY-SVTRID")
      throw (ccReg::EPP::EppError);
  // EPP exception 
  void EppError(short errCode, const char *errMsg, const char *svTRID,
    ccReg::Errors_var& errors) throw (ccReg::EPP::EppError);

  void NoMessages(short errCode, const char *errMsg, const char *svTRID);

  // get version of server with timestamp
  char* version(ccReg::timestamp_out datetime);

  // default ExDate for EPP messages
  int DefaultExDateSeenMessage()
  {
    return 30;
  }

  // protected period
  int DefaultContactHandlePeriod()
  {
    return 2;
  } // protected period in days
  int DefaultDomainNSSetPeriod()
  {
    return 2;
  }
  int DefaultDomainFQDNPeriod()
  {
    return 2;
  }

  int DefaultNSSetCheckLevel()
  {
    return 0;
  } // default nsset level

  // true visible all false all hiddend for disclose flags
  bool DefaultPolicy()
  {
    return true;
  }

  int DefaultValExpInterval()
  {
    return 14;
  } //  protected period for expiration validity of enum domain 
  // for disclose flags
  bool get_DISCLOSE(bool db);
  char update_DISCLOSE(bool d, ccReg::Disclose flag);
  bool setvalue_DISCLOSE(bool d, ccReg::Disclose flag);

  // load country code
  int LoadCountryCode();
  // test country code
  bool TestCountryCode(const char *cc);

  // load and get message of lang from enum_error
  int LoadErrorMessages();
  const char * GetErrorMessage(int err, int lang);

  // load and get message of lang from enum_reason
  int LoadReasonMessages();
  const char * GetReasonMessage(int err, int lang);

  // reason handle
  short SetErrorReason(ccReg::Errors_var& errors, short errCode,
    ccReg::ParamError paramCode, short position, int reasonMsg, int lang);

  short SetReasonContactHandle(ccReg::Errors_var& err, const char *handle,
    int lang);
  short SetReasonNSSetHandle(ccReg::Errors_var& err, const char *handle,
    int lang);
  short SetReasonDomainFQDN(ccReg::Errors_var& err, const char *fqdn, int zone,
    int lang);
  short int SetReasonKeySetHandle(ccReg::Errors_var &err, const char *handle,
          int lang);

  // general list function
  ccReg::Response* FullList(short act, const char *table, const char *fname,
    ccReg::Lists_out list, CORBA::Long clientID, const char* clTRID,
    const char* XML);

  // general check function for all objects
  ccReg::Response* ObjectCheck(short act, const char * table, const char *fname,
    const ccReg::Check& chck, ccReg::CheckResp_out a, CORBA::Long clientID,
    const char* clTRID, const char* XML);

  // general send auth info for objects
  ccReg::Response
      * ObjectSendAuthInfo(short act, const char * table, const char *fname,
        const char *name, CORBA::Long clientID, const char* clTRID,
        const char* XML);

  CORBA::Boolean SaveOutXML(const char* svTRID, const char* XML);

  void sessionClosed(CORBA::Long clientID);

  // methods corresponding to defined IDL attributes and operations
  ccReg::Response* GetTransaction(CORBA::Short errCode, CORBA::Long clientID,
    const char* clTRID, const ccReg::XmlErrors& errorCodes,
    ccReg::ErrorStrings_out errStrings);

  ccReg::Response* PollAcknowledgement(const char* msgID, CORBA::Short& count,
    CORBA::String_out newmsgID, CORBA::Long clientID, const char* clTRID,
    const char* XML);
  ccReg::Response* PollRequest(CORBA::String_out msgID, CORBA::Short& count,
    ccReg::timestamp_out qDate, ccReg::PollType& type, CORBA::Any_OUT_arg msg,
    CORBA::Long clientID, const char* clTRID, const char* XML);

  ccReg::Response* ClientLogin(const char* ClID, const char* passwd,
    const char* newpass, const char* clTRID, const char* XML,
    CORBA::Long& clientID, const char* certID, ccReg::Languages lang);
  ccReg::Response* ClientLogout(CORBA::Long clientID, const char* clTRID,
    const char* XML);
  ccReg::Response* ClientCredit(ccReg::ZoneCredit_out credit,
    CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactCheck(const ccReg::Check& handle,
    ccReg::CheckResp_out a, CORBA::Long clientID, const char* clTRID,
    const char* XML);
  ccReg::Response* ContactInfo(const char* handle, ccReg::Contact_out c,
    CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactDelete(const char* handle, CORBA::Long clientID,
    const char* clTRID, const char* XML);
  ccReg::Response* ContactUpdate(const char* handle,
    const ccReg::ContactChange& c, CORBA::Long clientID, const char* clTRID,
    const char* XML);
  ccReg::Response* ContactCreate(const char* handle,
    const ccReg::ContactChange& c, ccReg::timestamp_out crDate,
    CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* ContactTransfer(const char* handle, const char* authInfo,
    CORBA::Long clientID, const char* clTRID, const char* XML);

  ccReg::Response* NSSetCheck(const ccReg::Check& handle,
    ccReg::CheckResp_out a, CORBA::Long clientID, const char* clTRID,
    const char* XML);

  ccReg::Response* NSSetInfo(const char* handle, ccReg::NSSet_out n,
    CORBA::Long clientID, const char* clTRID, const char* XML);

  ccReg::Response* NSSetDelete(const char* handle, CORBA::Long clientID,
    const char* clTRID, const char* XML);

  ccReg::Response* NSSetCreate(const char* handle, const char* authInfoPw,
    const ccReg::TechContact& tech, const ccReg::DNSHost& dns,
    CORBA::Short level, ccReg::timestamp_out crDate, CORBA::Long clientID,
    const char* clTRID, const char* XML);

  ccReg::Response* NSSetUpdate(const char* handle, const char* authInfo_chg,
    const ccReg::DNSHost& dns_add, const ccReg::DNSHost& dns_rem,
    const ccReg::TechContact& tech_add, const ccReg::TechContact& tech_rem,
    CORBA::Short level, CORBA::Long clientID, const char* clTRID,
    const char* XML);

  ccReg::Response* NSSetTransfer(const char* handle, const char* authInfo,
    CORBA::Long clientID, const char* clTRID, const char* XML);

  ccReg::Response *KeySetCheck(
          const ccReg::Check &handle,
          ccReg::CheckResp_out a,
          CORBA::Long clientID,
          const char *clTRID,
          const char *XML);

  ccReg::Response *KeySetInfo(
          const char *handle,
          ccReg::KeySet_out k,
          CORBA::Long clientID,
          const char *clTRID,
          const char *XML);

  ccReg::Response *KeySetDelete(
          const char *handle,
          CORBA::Long clientID,
          const char *clTRID,
          const char *XML);

  ccReg::Response *KeySetCreate(
          const char *handle,
          const char *authInfoPw,
          const ccReg::TechContact &tech,
          const ccReg::DSRecord &dsrec,
          const ccReg::DNSKey &dnsk,
          ccReg::timestamp_out crDate,
          CORBA::Long clientID,
          const char *clTRID,
          const char *XML);

  ccReg::Response *KeySetUpdate(
          const char *handle,
          const char *authInfo_chg,
          const ccReg::TechContact &tech_add,
          const ccReg::TechContact &tech_rem,
          const ccReg::DSRecord &dsrec_add,
          const ccReg::DSRecord &dsrec_rem,
          const ccReg::DNSKey &dnsk_add,
          const ccReg::DNSKey &dnsk_rem,
          CORBA::Long clientID,
          const char *clTRID,
          const char *XML);

  ccReg::Response *KeySetTransfer(
          const char *handle,
          const char *authInfo,
          CORBA::Long clientID,
          const char *clTRID,
          const char *XML);

  ccReg::Response* DomainCheck(const ccReg::Check& fqdn,
    ccReg::CheckResp_out a, CORBA::Long clientID, const char* clTRID,
    const char* XML);
  ccReg::Response* DomainInfo(const char* fqdn, ccReg::Domain_out d,
    CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* DomainDelete(const char* fqdn, CORBA::Long clientID,
    const char* clTRID, const char* XML);
  // TODO add keyset to domain
  ccReg::Response* DomainUpdate(const char* fqdn, const char* registrant_chg,
    const char* authInfo_chg, const char* nsset_chg, const char *keyset_chg,
    const ccReg::AdminContact& admin_add, const ccReg::AdminContact& admin_rem,
    const ccReg::AdminContact& tmpcontact_rem, CORBA::Long clientID,
    const char* clTRID, const char* XML, const ccReg::ExtensionList& ext);
  ccReg::Response* DomainCreate(const char* fqdn, const char* Registrant,
    const char* nsset, const char *keyset,
    const char* AuthInfoPw, const ccReg::Period_str& period,
    const ccReg::AdminContact& admin, ccReg::timestamp_out crDate,
    ccReg::date_out exDate, CORBA::Long clientID, const char* clTRID,
    const char* XML, const ccReg::ExtensionList& ext);
  ccReg::Response* DomainRenew(const char* fqdn, const char* curExpDate,
    const ccReg::Period_str& period, ccReg::timestamp_out exDate,
    CORBA::Long clientID, const char* clTRID, const char* XML,
    const ccReg::ExtensionList& ext);
  ccReg::Response* DomainTransfer(const char* fqdn, const char* authInfo,
    CORBA::Long clientID, const char* clTRID, const char* XML);

  // tech check nsset
  ccReg::Response* nssetTest(const char* handle, CORBA::Short level,
    const ccReg::Lists& fqdns, CORBA::Long clientID, const char* clTRID,
    const char* XML);

  //common function for transfer object 
  ccReg::Response* ObjectTransfer(short act, const char*table,
    const char *fname, const char *name, const char* authInfo,
    CORBA::Long clientID, const char* clTRID, const char* XML);

  // 
  ccReg::Response* domainSendAuthInfo(const char* fqdn, CORBA::Long clientID,
    const char* clTRID, const char* XML);
  ccReg::Response* contactSendAuthInfo(const char* handle,
    CORBA::Long clientID, const char* clTRID, const char* XML);
  ccReg::Response* nssetSendAuthInfo(const char* handle, CORBA::Long clientID,
    const char* clTRID, const char* XML);

  ccReg::Response *keysetSendAuthInfo(
          const char *handle,
          CORBA::Long clientID,
          const char *clTRID,
          const char *XML);

  // EPP print out
  ccReg::Response* ContactList(ccReg::Lists_out contacts, CORBA::Long clientID,
    const char* clTRID, const char* XML);
  ccReg::Response* NSSetList(ccReg::Lists_out nssets, CORBA::Long clientID,
    const char* clTRID, const char* XML);
  ccReg::Response* DomainList(ccReg::Lists_out domains, CORBA::Long clientID,
    const char* clTRID, const char* XML);

  ccReg::Response *KeySetList(
          ccReg::Lists_out keysets,
          CORBA::Long clientID,
          const char *clTRID,
          const char *XML);

  // Info messages
  ccReg::Response* info(ccReg::InfoType type, const char* handle,
    CORBA::Long& count, CORBA::Long clientID, const char* clTRID,
    const char* XML);
  ccReg::Response* getInfoResults(ccReg::Lists_out handles,
    CORBA::Long clientID, const char* clTRID, const char* XML);

  const std::string& getDatabaseString();

private:
  Session *session;
  int numSession; // number of active session
  int maxSession; // maximal sessions
  long long maxWaitClient; //  connection timeout 
  Mesg *ErrorMsg;
  Mesg *ReasonMsg;
  std::auto_ptr<CountryCode> CC;
  int max_zone;
  bool testInfo; // TODO: remove 
};
