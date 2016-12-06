#include "src/corba/epp/countrycode.h"
#include "src/corba/epp/messages.h"
#include "src/corba/epp/epp_session.h"
#include "src/corba/mailer_manager.h"
#include "src/old_utils/dbsql.h"
#include "src/fredlib/registry.h"

#include "src/corba/EPP.hh"

#include <vector>
#include <stdexcept>
#include <memory>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

//value class to fix return of local char*
class EppString
{
    std::string string_;
public:
    // conversion
    operator const char * () const { return string_.c_str(); }
    //ctor
    EppString(const char * str)
    : string_(str) {}
};//class EppString

class EPPAction;

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

  //conf
  bool restricted_handles_;
  bool disable_epp_notifier_;
  bool lock_epp_commands_;
  unsigned int nsset_level_;
  unsigned int nsset_min_hosts_;
  unsigned int nsset_max_hosts_;
  std::string docgen_path_;
  std::string docgen_template_path_;
  std::string fileclient_path_;
  std::string disable_epp_notifier_cltrid_prefix_;

  unsigned rifd_session_max_;
  unsigned rifd_session_timeout_;
  unsigned rifd_session_registrar_max_;
  bool rifd_epp_update_domain_keyset_clear_;
  bool rifd_epp_operations_charging_;
  const bool epp_update_contact_enqueue_check_;

  DBSharedPtr  db_disconnect_guard_;
  std::auto_ptr<Fred::Manager> regMan;


  void extractEnumDomainExtension(std::string&, ccReg::Disclose &publish, const ccReg::ExtensionList&);
  // is IDN allowed? implements logic: system registrator has always IDN allowed
  bool idn_allowed(EPPAction& action) const;

public:
  struct DB_CONNECT_FAILED : public std::runtime_error
  {
      DB_CONNECT_FAILED()
              : std::runtime_error("Database connection failed")
      {}
  };
  // standard constructor
      ccReg_EPP_i(const std::string &db, MailerManager *_mm, NameService *_ns
          , bool restricted_handles
          , bool disable_epp_notifier
          , bool lock_epp_commands
          , unsigned int nsset_level
          , unsigned int nsset_min_hosts
          , unsigned int nsset_max_hosts
          , const std::string& docgen_path
          , const std::string& docgen_template_path
          , const std::string& fileclient_path
          , const std::string& disable_epp_notifier_cltrid_prefix
          , unsigned rifd_session_max
          , unsigned rifd_session_timeout
          , unsigned rifd_session_registrar_max
          , bool rifd_epp_update_domain_keyset_clear
          , bool rifd_epp_operations_charging
          , bool epp_update_contact_enqueue_check
          );
  virtual ~ccReg_EPP_i();


  const std::string& get_disable_epp_notifier_cltrid_prefix() const
  {
      return disable_epp_notifier_cltrid_prefix_;
  }

  // get zones parametrs
  int GetZoneExPeriodMin(DBSharedPtr db, int id);
  int GetZoneExPeriodMax(DBSharedPtr db, int id);
  int GetZoneValPeriod(DBSharedPtr db, int id);
  int GetZoneDotsMax(DBSharedPtr db, int id);
  bool GetZoneEnum(DBSharedPtr db, int id);
  std::string GetZoneFQDN(DBSharedPtr db, int id);
  std::vector<int> GetAllZonesIDs(DBSharedPtr db);

  int getZone(DBSharedPtr db, const char *fqdn);
  int getZoneMax(DBSharedPtr db, const char *fqdn);
  int getFQDN(DBSharedPtr db, char *FQDN, const char *fqdn);


  // parse extension for domain enum.exdate
  void GetValExpDateFromExtension(char *valexpDate, const ccReg::ExtensionList& ext);

  // get RegistrarID
  int GetRegistrarID(unsigned long long clientID);
  // get uses language
  int GetRegistrarLang(unsigned long long clientID);

  // send    exception ServerIntError
  void ServerInternalError(const char *fce, const char *svTRID="DUMMY-SVTRID");
  // EPP exception
  void EppError(short errCode, const char *errMsg, const char *svTRID,
    ccReg::Errors_var& errors);

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
  EppString GetErrorMessage(int err, int lang);

  // load and get message of lang from enum_reason
  int LoadReasonMessages();
  EppString GetReasonMessage(int err, int lang);

  // reason handle
  short SetErrorReason(ccReg::Errors_var& errors, short errCode, ccReg::ParamError paramCode, short position, int reasonMsg, int lang);

  short SetReasonContactHandle(ccReg::Errors_var& err, const char *handle, int lang);
  short SetReasonNSSetHandle(ccReg::Errors_var& err, const char *handle, int lang);
  short SetReasonDomainFQDN(ccReg::Errors_var& err, const char *fqdn, int zone, int lang);
  short int SetReasonKeySetHandle(ccReg::Errors_var &err, const char *handle, int lang);

  // general list function
  ccReg::Response* FullList(short act, const char *table, const char *fname, ccReg::Lists_out list, const ccReg::EppParams &params);

  // general check function for all objects
  ccReg::Response* ObjectCheck(short act, const char * table, const char *fname, const ccReg::Check& chck, ccReg::CheckResp_out a, const ccReg::EppParams &params);

  // general send auth info for objects
  ccReg::Response * ObjectSendAuthInfo(short act, const char * table, const char *fname, const char *name, const ccReg::EppParams &params);

  void sessionClosed(CORBA::ULongLong clientID);

  // methods corresponding to defined IDL attributes and operations
  ccReg::Response* GetTransaction(CORBA::Short errCode, CORBA::ULongLong clientID, ccReg::TID requestId, const char* clTRID, const ccReg::XmlErrors& errorCodes, ccReg::ErrorStrings_out errStrings);
  ccReg::Response* PollAcknowledgement(const char* msgID, CORBA::ULongLong& count, CORBA::String_out newmsgID, const ccReg::EppParams &params);
  ccReg::Response* PollRequest(CORBA::String_out msgID, CORBA::ULongLong& count, ccReg::timestamp_out qDate, ccReg::PollType& type, CORBA::Any_OUT_arg msg, const ccReg::EppParams &params);
  void PollRequestGetUpdateDomainDetails(CORBA::ULongLong _poll_id, ccReg::Domain_out _old_data, ccReg::Domain_out _new_data, const ccReg::EppParams &params);
  void PollRequestGetUpdateNSSetDetails(CORBA::ULongLong _poll_id, ccReg::NSSet_out _old_data, ccReg::NSSet_out _new_data, const ccReg::EppParams &params);
  void PollRequestGetUpdateKeySetDetails(CORBA::ULongLong _poll_id, ccReg::KeySet_out _old_data, ccReg::KeySet_out _new_data, const ccReg::EppParams &params);

  ccReg::Response* ClientLogin(const char* ClID, const char* passwd, const char* newpass, const char *clTRID, const char* XML, CORBA::ULongLong& out_clientID, ccReg::TID requestId, const char* certID, ccReg::Languages lang);
  ccReg::Response* ClientLogout(const ccReg::EppParams &params);
  ccReg::Response* ClientCredit(ccReg::ZoneCredit_out credit, const ccReg::EppParams &params);
  ccReg::Response* ContactCheck(const ccReg::Check& handle, ccReg::CheckResp_out a, const ccReg::EppParams &params);
  ccReg::Response* ContactInfo(const char* handle, ccReg::Contact_out c, const ccReg::EppParams &params);
  ccReg::Response* ContactDelete(const char* handle, const ccReg::EppParams &params);
  ccReg::Response* ContactUpdate(const char* handle, const ccReg::ContactChange& c, const ccReg::EppParams &params);
  ccReg::Response* ContactCreate(const char* handle, const ccReg::ContactChange& c, ccReg::timestamp_out crDate, const ccReg::EppParams &params);
  ccReg::Response* ContactTransfer(const char* handle, const char* authInfo, const ccReg::EppParams &params);

  ccReg::Response* NSSetCheck(const ccReg::Check& _handles_to_be_checked, ccReg::CheckResp_out _check_results, const ccReg::EppParams &_epp_params);

  ccReg::Response* NSSetInfo(const char* handle, ccReg::NSSet_out n, const ccReg::EppParams &params);

  ccReg::Response* NSSetDelete(const char* handle, const ccReg::EppParams &params);

  ccReg::Response* NSSetCreate(const char* handle, const char* authInfoPw, const ccReg::TechContact& tech, const ccReg::DNSHost& dns, CORBA::Short level, ccReg::timestamp_out crDate, const ccReg::EppParams &params);

  ccReg::Response* NSSetUpdate(const char* handle, const char* authInfo_chg, const ccReg::DNSHost& dns_add, const ccReg::DNSHost& dns_rem, const ccReg::TechContact& tech_add, const ccReg::TechContact& tech_rem, CORBA::Short level, const ccReg::EppParams &params);

  ccReg::Response* NSSetTransfer(const char* handle, const char* authInfo, const ccReg::EppParams &params);

  ccReg::Response *KeySetCheck( const ccReg::Check &handle, ccReg::CheckResp_out a, const ccReg::EppParams &params);

  ccReg::Response *KeySetInfo( const char *handle, ccReg::KeySet_out k, const ccReg::EppParams &params);

  ccReg::Response *KeySetDelete( const char *handle, const ccReg::EppParams &params);

  ccReg::Response *KeySetCreate( const char *handle, const char *authInfoPw, const ccReg::TechContact &tech, const ccReg::DSRecord &dsrec, const ccReg::DNSKey &dnsk, ccReg::timestamp_out crDate, const ccReg::EppParams &params);

  ccReg::Response *KeySetUpdate( const char *handle, const char *authInfo_chg, const ccReg::TechContact &tech_add, const ccReg::TechContact &tech_rem, const ccReg::DSRecord &dsrec_add, const ccReg::DSRecord &dsrec_rem, const ccReg::DNSKey &dnsk_add, const ccReg::DNSKey &dnsk_rem, const ccReg::EppParams &params);

  ccReg::Response *KeySetTransfer( const char *handle, const char *authInfo, const ccReg::EppParams &params);

    /**
     * DomainCheck - retrieve states of domains identified by their FQDNs
     *
     * \param _domain_fqdns - identifiers of domains to check
     * \param _domain_check_results - output sequence of check results
     * \param _epp_params - parameters of EPP session
     *
     * \return ccReg::Response
     *
     * \throws ccReg::EPP::EppError
     */
    ccReg::Response* DomainCheck(
        const ccReg::Check& _domain_fqdns,
        ccReg::CheckResp_out _domain_check_results,
        const ccReg::EppParams& _epp_params
    );

    /**
     * DomainInfo - get information obout domain identified by its FQDN
     *
     * \param _domain_fqdn - identifier of domain - fully qualified domain name
     * \param _domain_info - output information
     * \param _epp_params - parameters of EPP session
     *
     * \return ccReg::Response
     *
     * \throws ccReg::EPP::EppError
     */
    ccReg::Response* DomainInfo(
        const char* _domain_fqdn,
        ccReg::Domain_out _domain_info,
        const ccReg::EppParams& _epp_params
    );

    /**
     * DomainDelete - delete domain identified by its FQDN
     *
     * \param _domain_fqdn - identifier of domain - fully qualified domain name
     * \param _epp_params - parameters of EPP session
     *
     * \return ccReg::Response
     *
     * \throws ccReg::EPP::EppError
     */
    ccReg::Response* DomainDelete(
        const char* _domain_fqdn,
        const ccReg::EppParams& _epp_params
    );

    /**
     * DomainUpdate - update data of domain identified by its FQDN
     *
     * \param _domain_fqdn - identifiers of domains to check
     * \param _registrant_chg - change of domain holder
     * \param _auth_info_pw_chg  - change of password
     * \param _nsset_chg - change of nsset
     * \param _keyset_chg - change of keyset
     * \param _admin_contacts_add - sequence of added administration contacts
     * \param _admin_contacts_rem - sequence of deleted administration contacts
     * \param _tmpcontacts_rem - OBSOLETE sequence of deleted temporary contacts
     * \param _epp_params - common EPP parametres
     * \param _ext - ExtensionList
     *
     * \return ccReg::Response
     *
     * \throws ccReg::EPP::EppError
     */
    ccReg::Response* DomainUpdate(
        const char* _domain_fqdn,
        const char* _registrant_chg,
        const char* _auth_info_pw_chg,
        const char* _nsset_chg,
        const char* _keyset_chg,
        const ccReg::AdminContact& _admin_contacts_add,
        const ccReg::AdminContact& _admin_contacts_rem,
        const ccReg::AdminContact& _tmpcontacts_rem,
        const ccReg::EppParams& _epp_params,
        const ccReg::ExtensionList& _ext
    );

  ccReg::Response* DomainCreate(const char* fqdn, const char* Registrant, const char* nsset, const char *keyset, const char* AuthInfoPw, const ccReg::Period_str& period, const ccReg::AdminContact& admin, ccReg::timestamp_out crDate, ccReg::date_out exDate, const ccReg::EppParams &params, const ccReg::ExtensionList& ext);

  ccReg::Response* DomainRenew(const char* fqdn, const char* curExpDate, const ccReg::Period_str& period, ccReg::timestamp_out exDate, const ccReg::EppParams &params, const ccReg::ExtensionList& ext);

    /**
     * DomainTransfer - transfer domain to other registrar
     *
     * \param _domain_fqdn - identifier of domain to transfer
     * \param _auth_info_pw - secret authorization information
     * \param _epp_params - parameters of EPP session
     *
     * \return ccReg::Response
     *
     * \throws ccReg::EPP::EppError
     */
    ccReg::Response* DomainTransfer(
        const char* _domain_fqdn,
        const char* _auth_info_pw,
        const ccReg::EppParams& _epp_params
    );

  // tech check nsset
  ccReg::Response* nssetTest(const char* handle, CORBA::Short level, const ccReg::Lists& fqdns, const ccReg::EppParams &params);

  //common function for transfer object
  ccReg::Response* ObjectTransfer(short act, const char*table, const char *fname, const char *name, const char* authInfo, const ccReg::EppParams &params);

  //
  ccReg::Response* domainSendAuthInfo(const char* fqdn, const ccReg::EppParams &params);

  ccReg::Response* contactSendAuthInfo(const char* handle, const ccReg::EppParams &params);

  ccReg::Response* nssetSendAuthInfo(const char* handle, const ccReg::EppParams &params);

  ccReg::Response *keysetSendAuthInfo( const char *handle, const ccReg::EppParams &params);

  // EPP print out
  ccReg::Response* ContactList(ccReg::Lists_out contacts, const ccReg::EppParams &params);

  ccReg::Response* NSSetList(ccReg::Lists_out nssets, const ccReg::EppParams &params);

  ccReg::Response* DomainList(ccReg::Lists_out domains, const ccReg::EppParams &params);

  ccReg::Response *KeySetList( ccReg::Lists_out keysets, const ccReg::EppParams &params);

  // Info messages
  ccReg::Response* info(ccReg::InfoType type, const char* handle, CORBA::Long& count, const ccReg::EppParams &params);
  ccReg::Response* getInfoResults(ccReg::Lists_out handles, const ccReg::EppParams &params);

  const std::string& getDatabaseString();


  // block registrar - this typically isn't called by apache EPP
  void destroyAllRegistrarSessions(CORBA::Long reg_id);


private:
  EppSessionContainer epp_sessions;
  Mesg *ErrorMsg;
  Mesg *ReasonMsg;
  std::auto_ptr<CountryCode> CC;
  int max_zone;
};
