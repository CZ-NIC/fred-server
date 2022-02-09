/*
 * Copyright (C) 2006-2022  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/backend/epp/contact/config_data_filter.hh"
#include "src/backend/epp/contact/create_contact_data_filter.hh"
#include "src/backend/epp/contact/contact_data_share_policy_rules.hh"
#include "src/backend/epp/contact/update_contact_data_filter.hh"

#include "src/bin/corba/epp/messages.hh"
#include "src/bin/corba/epp/epp_session.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/libfred/registry.hh"

#include "corba/EPP.hh"

#include <vector>
#include <stdexcept>
#include <memory>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

//value class to fix return of local char*
class EppString
{
public:
    // conversion
    operator const char* () const { return string_.c_str(); }
    //ctor
    EppString(const char* str)
    : string_(str) {}
private:
    std::string string_;
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
  std::shared_ptr<Epp::Contact::CreateContactDataFilter> rifd_epp_create_contact_data_filter_;
  std::shared_ptr<Epp::Contact::ContactDataSharePolicyRules> rifd_epp_contact_data_share_policy_rules_;
  std::shared_ptr<Epp::Contact::UpdateContactDataFilter> rifd_epp_update_contact_data_filter_;

  DBSharedPtr  db_disconnect_guard_;
  std::unique_ptr<LibFred::Manager> regMan;


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
      ccReg_EPP_i(
              const std::string &db,
              MailerManager *_mm,
              NameService *_ns,
              bool restricted_handles,
              bool disable_epp_notifier,
              bool lock_epp_commands,
              unsigned int nsset_level,
              unsigned int nsset_min_hosts,
              unsigned int nsset_max_hosts,
              const std::string& docgen_path,
              const std::string& docgen_template_path,
              const std::string& fileclient_path,
              const std::string& disable_epp_notifier_cltrid_prefix,
              unsigned rifd_session_max,
              unsigned rifd_session_timeout,
              unsigned rifd_session_registrar_max,
              bool rifd_epp_update_domain_keyset_clear,
              bool rifd_epp_operations_charging,
              bool epp_update_contact_enqueue_check,
              const Epp::Contact::ConfigDataFilter& rifd_contact_data_filter,
              const Epp::Contact::ConfigDataFilter& rifd_contact_data_share_policy_rules);
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

  int getZone(DBSharedPtr db, const char *fqdn);
  int getZoneMax(DBSharedPtr db, const char *fqdn);
  int getFQDN(DBSharedPtr db, char *FQDN, const char *fqdn);

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

  void sessionClosed(CORBA::ULongLong clientID);

  // methods corresponding to defined IDL attributes and operations
 /**
  * GetTransaction returns for client from entered clTRID generated server
  *              transaction ID
  *
  * \param clTRID - client transaction number
  * \param clientID - client identification
  * \param requestId - fred-logd request ID
  * \param errCode - save error report from client into table action
  *
  * \return svTRID and errCode msg
  */
  ccReg::Response* GetTransaction(CORBA::Short errCode, CORBA::ULongLong clientID, ccReg::TID requestId, const char* clTRID, const ccReg::XmlErrors& errorCodes, ccReg::ErrorStrings_out errStrings);
 /**
  * PollAcknowledgement confirmation of message income msgID returns number of message, which are left count and next message newmsgID
  *
  * \param msgID - front message number
  * \param count -  messages numbers
  * \param newmsgID - number of new message
  * \param params - common EPP parametres
  *
  * \return svTRID and errCode
  */
  ccReg::Response* PollAcknowledgement(const char* msgID, CORBA::ULongLong& count, CORBA::String_out newmsgID, const ccReg::EppParams &params);
 /**
  * PollRequest retrieve message msgID from front return number of messages in front and message content
  *
  * \param
  * \param msgID - id of required message in front
  * \param count - number
  * \param qDate - message date and time
  * \param type - message type
  * \param msg  - message content as structure
  * \param params - common EPP parametres
  *
  * \return svTRID and errCode
  */
  ccReg::Response* PollRequest(CORBA::String_out msgID, CORBA::ULongLong& count, ccReg::timestamp_out qDate, ccReg::PollType& type, CORBA::Any_OUT_arg msg, const ccReg::EppParams &params);
  void PollRequestGetUpdateContactDetails(CORBA::ULongLong _poll_id, ccReg::Contact_out _old_data, ccReg::Contact_out _new_data, const ccReg::EppParams &params);
  void PollRequestGetUpdateDomainDetails(CORBA::ULongLong _poll_id, ccReg::Domain_out _old_data, ccReg::Domain_out _new_data, const ccReg::EppParams &params);
  void PollRequestGetUpdateNSSetDetails(CORBA::ULongLong _poll_id, ccReg::NSSet_out _old_data, ccReg::NSSet_out _new_data, const ccReg::EppParams &params);
  void PollRequestGetUpdateKeySetDetails(CORBA::ULongLong _poll_id, ccReg::KeySet_out _old_data, ccReg::KeySet_out _new_data, const ccReg::EppParams &params);

 /**
  * ClientLogin client login acquire of clientID from table login login through password registrar and its possible change
  *
  * \param ClID - registrar identifier
  * \param passwd - current password
  * \param newpasswd - new password for change
  * \param clTRID - transaction client number
  * \param XML - xml representation of the command
  * \param clientID - connected client id
  * \param requestId - fred-logd request ID associated with login
  * \param certID - certificate fingerprint
  * \param language - communication language of client en or cs empty value = en
  *
  * \return svTRID and errCode
  */
  ccReg::Response* ClientLogin(const char* ClID, const char* passwd, const char* newpass, const char *clTRID, const char* XML, CORBA::ULongLong& out_clientID, ccReg::TID requestId, const char* certID, ccReg::Languages lang);

 /**
  * ClientLogout client logout for record into table login
  *              about logout date
  * \param params - common EPP parametres
  *
  * \return svTRID and errCode
  */

  ccReg::Response* ClientLogout(const ccReg::EppParams &params);
 /**
  * ClientCredit information about credit amount of logged registrar
  *
  * \param params - common EPP parametres
  * \param credit - credit amount in haler
  *
  * \return svTRID and errCode
  */
  ccReg::Response* ClientCredit(ccReg::ZoneCredit_out credit, const ccReg::EppParams &params);


    ccReg::Response* ContactCheck(
            const ccReg::Check& _handles_to_be_checked,
            ccReg::CheckResp_out _check_results,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* ContactCreate(
            const char* _handle,
            const ccReg::ContactData& _contact_data,
            ccReg::timestamp_out _creation_time,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* ContactDelete(
            const char* _handle,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* ContactInfo(
            const char* _handle,
            const char* _authinfopw,
            const ccReg::ControlledPrivacyDataMask& _unused_disclose_flags,
            ccReg::Contact_out _contact_info,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* ContactTransfer(
            const char* _handle,
            const char* _authinfopw,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* ContactUpdate(
            const char* _handle,
            const ccReg::ContactChange& _contact_change_data,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* NSSetCheck(
            const ccReg::Check& _nsset_handles_to_be_checked,
            ccReg::CheckResp_out _check_results,
            const ccReg::EppParams& _epp_params);

     /**
      * @brief creation Nsset and subservient DNS hosts
      *
      * @param handle - nsset identifier
      * @param _authinfopw - authentication
      * @param _tech_contacts - sequence of technical contact
      * @param _dns_hosts - sequence of DNS records
      * @param _tech_check_level - tech check level
      * @param _create_time - object creation date
      * @param _epp_params - common EPP parametres
      *
      * @return svTRID and errCode
      */
    ccReg::Response* NSSetCreate(
            const char* handle,
            const char* _authinfopw,
            const ccReg::TechContact& _tech_contacts,
            const ccReg::DNSHost& _dns_hosts,
            CORBA::Short _tech_check_level,
            ccReg::timestamp_out _create_time,
            const ccReg::EppParams& _epp_params);

    /**
      * @brief deleting Nsset and saving it into history Nsset can be only deleted by registrar who created it or those who administers it nsset cannot be deleted if there is link into domain table
      *
      * @param _nsset_handle  - nsset identifier
      * @param _epp_params - common EPP parametres
      *
      * @return svTRID and errCode
      */
    ccReg::Response* NSSetDelete(
            const char* _nsset_handle,
            const ccReg::EppParams& _epp_params);

    /**
      * @brief returns detailed information about nsset and subservient DNS hosts empty value if contact doesn't exist
      *
      * @param _nsset_handle - identifier of contact
      * @param _nsset_info - structure of Nsset detailed description
      * @param _epp_params - common EPP parameters
      *
      * @return svTRID and errCode
      */
    ccReg::Response* NSSetInfo(
            const char* _nsset_handle,
            ccReg::NSSet_out _nsset_info,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* NSSetTransfer(
            const char* _nsset_handle,
            const char* _authinfopw,
            const ccReg::EppParams& _epp_params);

    /**
      * @brief change of Nsset and subservient DNS hosts and technical contacts and saving changes into history
      *
      * @param _nsset_handle - nsset identifier
      * @param _authinfopw_chg - authentication change
      * @param _dns_hosts_add - sequence of added DNS records
      * @param _dns_hosts_rem - sequence of DNS records for deleting
      * @param _tech_contacts_add - sequence of added technical contacts
      * @param _tech_contacts_rem - sequence of technical contact for deleting
      * @param _tech_check_level - tech check level
      * @param _epp_params - common EPP parametres
      *
      * @return svTRID and errCode
      */
    ccReg::Response* NSSetUpdate(
            const char* _nsset_handle,
            const char* _authinfopw_chg,
            const ccReg::DNSHost& _dns_hosts_add,
            const ccReg::DNSHost& _dns_hosts_rem,
            const ccReg::TechContact& _tech_contacts_add,
            const ccReg::TechContact& _tech_contacts_rem,
            CORBA::Short _tech_check_level,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* KeySetCheck(
            const ccReg::Check& _keyset_handles_to_be_checked,
            ccReg::CheckResp_out _check_results,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* KeySetCreate(
            const char* _keyset_handle,
            const char* _authinfopw,
            const ccReg::TechContact& _tech_contacts,
            const ccReg::DSRecord& _ds_records,
            const ccReg::DNSKey& _dns_keys,
            ccReg::timestamp_out _create_time,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* KeySetDelete(
            const char* _keyset_handle,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* KeySetInfo(
            const char* _keyset_handle,
            ccReg::KeySet_out _keyset_info,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* KeySetTransfer(
            const char* _keyset_handle,
            const char* _authinfopw,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* KeySetUpdate(
            const char* _keyset_handle,
            const char* _authinfopw_chg,
            const ccReg::TechContact& _tech_contacts_add,
            const ccReg::TechContact& _tech_contacts_rem,
            const ccReg::DSRecord& _ds_records_add,
            const ccReg::DSRecord& _ds_records_rem,
            const ccReg::DNSKey& _dns_keys_add,
            const ccReg::DNSKey& _dns_keys_rem,
            const ccReg::EppParams& _epp_params);

    /**
       * DomainCheck - retrieve states of domains identified by their FQDNs
       *
       * \param _fqdns - identifiers of domains to check
       * \param _domain_check_results - output sequence of check results
       * \param _epp_params - parameters of EPP session
       *
       * \return ccReg::Response
       *
       * \throws ccReg::EPP::EppError
       */
    ccReg::Response* DomainCheck(
            const ccReg::Check& _fqdns,
            ccReg::CheckResp_out _domain_check_results,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* DomainCreate(
            const char* _fqdn,
            const char* _registrant,
            const char* _nsset,
            const char* _keyset,
            const char* _authinfopw,
            const ccReg::Period_str& _period,
            const ccReg::AdminContact& _admin_contacts,
            ccReg::timestamp_out _create_time,
            ccReg::date_out _exdate,
            const ccReg::EppParams& _epp_params,
            const ccReg::ExtensionList& _enum_validation_extension_list);

    /**
      * DomainDelete - delete domain identified by its FQDN
      *
      * \param _fqdn - identifier of domain - fully qualified domain name
      * \param _epp_params - parameters of EPP session
      *
      * \return ccReg::Response
      *
      * \throws ccReg::EPP::EppError
      */
    ccReg::Response* DomainDelete(
            const char* _fqdn,
            const ccReg::EppParams& _epp_params);

    /**
      * DomainInfo - get information obout domain identified by its FQDN
      *
      * \param _fqdn - identifier of domain - fully qualified domain name
      * \param _domain_info - output information
      * \param _epp_params - parameters of EPP session
      *
      * \return ccReg::Response
      *
      * \throws ccReg::EPP::EppError
      */
    ccReg::Response* DomainInfo(
            const char* _fqdn,
            ccReg::Domain_out _domain_info,
            const ccReg::EppParams& _epp_params);

    ccReg::Response* DomainRenew(
            const char* _fqdn,
            const char* _current_exdate,
            const ccReg::Period_str& _period,
            ccReg::timestamp_out _exdate,
            const ccReg::EppParams& _epp_params,
            const ccReg::ExtensionList& _enum_validation_extension_list);

    /**
       * DomainTransfer - transfer domain to other registrar
       *
       * \param _fqdn - identifier of domain to transfer
       * \param _authinfopw - secret authorization information
       * \param _epp_params - parameters of EPP session
       *
       * \return ccReg::Response
       *
       * \throws ccReg::EPP::EppError
       */
    ccReg::Response* DomainTransfer(
            const char* _fqdn,
            const char* _authinfopw,
            const ccReg::EppParams& _epp_params);

    /**
      * DomainUpdate - update data of domain identified by its FQDN
      *
      * \param _fqdn - identifiers of domains to check
      * \param _registrant_chg - change of domain holder
      * \param _authinfopw_chg  - change of password
      * \param _nsset_chg - change of nsset
      * \param _keyset_chg - change of keyset
      * \param _admin_contacts_add - sequence of added administration contacts
      * \param _admin_contacts_rem - sequence of deleted administration contacts
      * \param _tmpcontacts_rem - OBSOLETE sequence of deleted temporary contacts
      * \param _epp_params - common EPP parametres
      * \param _enum_validation_extension_list - ExtensionList
      *
      * \return ccReg::Response
      *
      * \throws ccReg::EPP::EppError
      */
    ccReg::Response* DomainUpdate(
            const char* _fqdn,
            const char* _registrant_chg,
            const char* _authinfopw_chg,
            const char* _nsset_chg,
            const char* _keyset_chg,
            const ccReg::AdminContact& _admin_contacts_add,
            const ccReg::AdminContact& _admin_contacts_rem,
            const ccReg::AdminContact& _tmpcontacts_rem,
            const ccReg::EppParams& _epp_params,
            const ccReg::ExtensionList& _enum_validation_extension_list);


  // tech check nsset
  ccReg::Response* nssetTest(const char* handle, CORBA::Short _tech_check_level, const ccReg::Lists& fqdns, const ccReg::EppParams &params);

  //common function for transfer object
  ccReg::Response* ObjectTransfer(short act, const char*table, const char *fname, const char *name, const char* authInfo, const ccReg::EppParams &params);

    /**
      * domainSendAuthInfo - create Public Request to send authinfopw
      *
      * \param _fqdn - identifier of domain - fully qualified domain name
      * \param _epp_params - parameters of EPP session
      *
      * \return ccReg::Response
      *
      * \throws ccReg::EPP::EppError
      */
    ccReg::Response* domainSendAuthInfo(
            const char* _fqdn,
            const ccReg::EppParams& _epp_params);

    /**
      * contactSendAuthInfo - create Public Request to send authinfopw
      *
      * \param _handle - contact identifier
      * \param _epp_params - parameters of EPP session
      *
      * \return ccReg::Response
      *
      * \throws ccReg::EPP::EppError
      */
    ccReg::Response* contactSendAuthInfo(
            const char* _handle,
            const ccReg::EppParams& _epp_params);

    /**
      * nssetSendAuthInfo - create Public Request to send authinfopw
      *
      * \param _handle - nsset identifier
      * \param _epp_params - parameters of EPP session
      *
      * \return ccReg::Response
      *
      * \throws ccReg::EPP::EppError
      */
    ccReg::Response* nssetSendAuthInfo(
            const char* _handle,
            const ccReg::EppParams& _epp_params);

    /**
      * keysetSendAuthInfo - create Public Request to send authinfopw
      *
      * \param _handle - keyset identifier
      * \param _epp_params - parameters of EPP session
      *
      * \return ccReg::Response
      *
      * \throws ccReg::EPP::EppError
      */
    ccReg::Response* keysetSendAuthInfo(
            const char* _handle,
            const ccReg::EppParams& _epp_params);

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
  EppSessionContainer epp_sessions_;
  Mesg *ErrorMsg;
  Mesg *ReasonMsg;
  int max_zone;
};
