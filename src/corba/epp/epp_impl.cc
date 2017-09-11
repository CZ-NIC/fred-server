/*
 *  Copyright (C) 2016 CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fstream>
#include <iostream>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"
#include <boost/numeric/conversion/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "src/corba/EPP.hh"
#include "epp_impl.h"

#include "src/corba/connection_releaser.h"

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "src/fredlib/notifier/enqueue_notification.h"

#include "config.h"

// database functions
#include "src/old_utils/dbsql.h"

#include "action.h"    // code of the EPP operations
#include "response.h"  // errors code
#include "reason.h"    // reason messages code

#include "src/epp/epp_response_failure_localized.h"
#include "src/epp/session_data.h"
#include "src/epp/notification_data.h"

// logger
#include "src/old_utils/log.h"

// MailerManager is connected in constructor
#include "src/fredlib/common_diff.h"
#include "src/fredlib/domain.h"
#include "src/fredlib/contact.h"
#include "src/fredlib/nsset.h"
#include "src/fredlib/keyset.h"
#include "src/fredlib/info_buffer.h"
#include "src/fredlib/zone.h"
#include "src/fredlib/invoicing/invoice.h"
#include <memory>
#include "tech_check.h"

#include "src/fredlib/registrar/info_registrar.h"

#include "util/factory_check.h"
#include "src/fredlib/public_request/public_request.h"
#include "src/fredlib/public_request/public_request_authinfo_impl.h"

// logger
#include "log/logger.h"
#include "log/context.h"

#include "src/epp/contact/create_contact_localized.h"
#include "src/epp/contact/delete_contact_localized.h"
#include "src/epp/contact/create_contact_input_data.h"
#include "src/epp/contact/update_contact_post_hooks.h"
#include "src/epp/contact/info_contact_localized.h"
#include "src/epp/contact/transfer_contact_localized.h"
#include "src/epp/contact/update_contact_localized.h"

#include "src/epp/domain/check_domain_localized.h"
#include "src/epp/domain/create_domain_localized.h"
#include "src/epp/domain/delete_domain_localized.h"
#include "src/epp/domain/info_domain_localized.h"
#include "src/epp/domain/renew_domain_localized.h"
#include "src/epp/domain/transfer_domain_localized.h"
#include "src/epp/domain/update_domain_localized.h"

#include "src/epp/keyset/check_keyset_localized.h"
#include "src/epp/keyset/create_keyset_localized.h"
#include "src/epp/keyset/delete_keyset_localized.h"
#include "src/epp/keyset/info_keyset_localized.h"
#include "src/epp/keyset/transfer_keyset_localized.h"
#include "src/epp/keyset/update_keyset_localized.h"

#include "src/epp/nsset/check_nsset_localized.h"
#include "src/epp/nsset/create_nsset_localized.h"
#include "src/epp/nsset/delete_nsset_localized.h"
#include "src/epp/nsset/info_nsset_localized.h"
#include "src/epp/nsset/transfer_nsset_localized.h"
#include "src/epp/nsset/update_nsset_localized.h"

#include "src/epp/poll/poll_acknowledgement_localized.h"
#include "src/epp/poll/poll_request_localized.h"
#include "src/epp/poll/poll_request_get_update_domain_details_localized.h"
#include "src/epp/poll/poll_request_get_update_nsset_details_localized.h"
#include "src/epp/poll/poll_request_get_update_keyset_details_localized.h"

#include "src/epp/reason.h"
#include "src/epp/param.h"
#include "src/epp/session_lang.h"
#include "src/epp/get_registrar_session_data.h"
#include "src/epp/registrar_session_data.h"
#include "src/epp/request_params.h"
#include "src/epp/localization.h"
#include "src/epp/impl/disclose_policy.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/corba/epp/contact/contact_corba_conversions.h"
#include "src/corba/epp/corba_conversions.h"
#include "src/corba/epp/domain/domain_corba_conversions.h"
#include "src/corba/epp/poll/poll_corba_conversions.h"
#include "src/corba/epp/epp_legacy_compatibility.h"
#include "src/corba/epp/keyset/keyset_corba_conversions.h"
#include "src/corba/epp/nsset/nsset_corba_conversions.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/corba/util/corba_conversions_int.h"
#include "util/util.h"

#include "src/epp/contact/check_contact_config_data.h"
#include "src/epp/contact/create_contact_config_data.h"
#include "src/epp/contact/delete_contact_config_data.h"
#include "src/epp/contact/info_contact_config_data.h"
#include "src/epp/contact/transfer_contact_config_data.h"
#include "src/epp/contact/update_contact_config_data.h"

#include "src/epp/domain/check_domain_config_data.h"
#include "src/epp/domain/create_domain_config_data.h"
#include "src/epp/domain/delete_domain_config_data.h"
#include "src/epp/domain/info_domain_config_data.h"
#include "src/epp/domain/renew_domain_config_data.h"
#include "src/epp/domain/transfer_domain_config_data.h"
#include "src/epp/domain/update_domain_config_data.h"

#include "src/epp/keyset/check_keyset_config_data.h"
#include "src/epp/keyset/create_keyset_config_data.h"
#include "src/epp/keyset/delete_keyset_config_data.h"
#include "src/epp/keyset/info_keyset_config_data.h"
#include "src/epp/keyset/transfer_keyset_config_data.h"
#include "src/epp/keyset/update_keyset_config_data.h"

#include "src/epp/nsset/check_nsset_config_data.h"
#include "src/epp/nsset/create_nsset_config_data.h"
#include "src/epp/nsset/delete_nsset_config_data.h"
#include "src/epp/nsset/info_nsset_config_data.h"
#include "src/epp/nsset/transfer_nsset_config_data.h"
#include "src/epp/nsset/update_nsset_config_data.h"

#include <vector>

struct NotificationParams //for enqueue_notification call in ~EPPAction()
{
    unsigned long long id;
    Notification::notified_event event_type;
    bool disable_epp_notifier;

    NotificationParams()
    : id(), event_type(), disable_epp_notifier()
    {}

    NotificationParams(unsigned long long _id,
        Notification::notified_event _event_type,
        bool _disable_epp_notifier)
    : id(_id)
    , event_type(_event_type)
    , disable_epp_notifier(_disable_epp_notifier)
    {}
};

class EPPAction
{
  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  ccReg_EPP_i *epp;
  DBSharedPtr  db;
  int regID;
  unsigned long long clientID;
  int code; ///< needed for destructor where Response is invalidated
  Optional<NotificationParams> notification_params_ ;
  std::string cltrid;
  Database::Connection conn_;
  std::auto_ptr<Database::Transaction> tx_;

public:
  struct ACTION_START_ERROR
  {
  };
  EPPAction(
    ccReg_EPP_i *_epp, unsigned long long _clientID, int action, const char *clTRID,
    const char *xml, unsigned long long requestId
  ) :
    ret(new ccReg::Response()), errors(new ccReg::Errors()), epp(_epp),
    regID(_epp->GetRegistrarID(_clientID)), clientID(_clientID),
    notification_params_(), cltrid(clTRID), conn_(Database::Manager::acquire()), tx_()
  {
    Logging::Context::push(str(boost::format("action-%1%") % action));
    try {
        tx_.reset(new Database::Transaction(conn_));
    }
    catch (...) {
        db->EndAction(COMMAND_FAILED);
        epp->ServerInternalError("Cannot start transaction");
    }

    db.reset(new DB(conn_));
    if (!db->BeginAction(clientID, action, clTRID, xml, requestId)) {
      epp->ServerInternalError("Cannot beginAction");
    }
    if (!regID) {
      ret->code = COMMAND_MAX_SESSION_LIMIT;
      ccReg::Response_var& r(getRet());
      db->EndAction(r->code);
      epp->EppError(r->code, r->msg, r->svTRID, errors);
    }

    code = ret->code = COMMAND_OK;

    Logging::Context::push(str(boost::format("%1%") % db->GetsvTRID()));
  }

  ~EPPAction()
  {
    try
    {
        unsigned long long historyid = 0;

        if (tx_.get()) {
            /* OMG: insane macro naming condition style */
            if (CMD_FAILED(code)) {

                try
                {
                    if(notification_params_.isset())
                    {
                        historyid = conn_.exec_params("SELECT historyid FROM object_registry WHERE id = $1::bigint"
                            , Database::query_param_list(notification_params_.get_value().id))[0][0];
                    }
                }
                catch (const std::exception& ex)
                {
                    LOGGER(PACKAGE).error(boost::format(" ~EPPAction() failed to get historyid: "
                          "(svtrid %1% what: %2%)") % db->GetsvTRID() % ex.what());
                    throw;
                }
                catch (...)
                {
                    LOGGER(PACKAGE).error(boost::format(" ~EPPAction() failed to get historyid: "
                          "(svtrid %1%)") % db->GetsvTRID());
                    throw;
                }
                tx_->commit();
            }
            else {
                tx_->rollback();
            }
        }
        db->EndAction(code);

        if (notification_params_.isset() && (code == COMMAND_OK)) {
            /* disable notifier for configured cltrid prefix */
            if (boost::starts_with(cltrid, epp->get_disable_epp_notifier_cltrid_prefix())
                    && db->GetRegistrarSystem(getRegistrar()))
            {
                LOGGER(PACKAGE).debug(boost::format("disable command notification "
                      "(registrator=%1% cltrid=%2%)") % getRegistrar() % cltrid);
            }
            else {
                try
                {
                    if(!notification_params_.get_value().disable_epp_notifier)
                    {
                        Fred::OperationContextCreator ctx;
                        Notification::enqueue_notification(ctx,notification_params_.get_value().event_type,
                            getRegistrar(), historyid, db->GetsvTRID());
                        ctx.commit_transaction();
                    }
                }
                catch (const std::exception& ex)
                {
                    LOGGER(PACKAGE).error(boost::format(" ~EPPAction() enqueue_notification failed: "
                          "(svtrid %1% what: %2%)") % db->GetsvTRID() % ex.what());
                    throw;
                }
                catch (...)
                {
                    LOGGER(PACKAGE).error(boost::format(" ~EPPAction() enqueue_notification failed: "
                          "(svtrid %1%)") % db->GetsvTRID());
                    throw;
                }
            }
        }

        Logging::Context::pop();
        Logging::Context::pop();
    }
    catch(...)
    {
        try
        {
            LOGGER(PACKAGE).error(
                "~EPPAction() got exception, "
                "db transaction or anything else failed");
        }
        catch(...)
        {}
    }
  }
  DBSharedPtr getDB()
  {
    return db;
  }
  int getRegistrar()
  {
    return regID;
  }
  int getLang()
  {
    return epp->GetRegistrarLang(clientID);
  }
  ccReg::Response_var& getRet()
  {
    ret->svTRID = CORBA::string_dup(db->GetsvTRID());
    ret->msg = CORBA::string_dup(epp->GetErrorMessage(ret->code, getLang()));
    return ret;
  }
  ccReg::Errors_var& getErrors()
  {
    return errors;
  }
  void failed(
    int _code)
  {
      TRACE(">> failed");
    code = ret->code = _code;
    ccReg::Response_var& r(getRet());
    epp->EppError(r->code, r->msg, r->svTRID, errors);
  }
  // replacement of ccReg_EPP_i::SetErrorReason(...)
  short int setErrorReason(short int errCode, ccReg::ParamError paramCode,
          short position, int reasonMsg)
  {
      unsigned int seq;
      seq = errors->length();
      errors->length(seq + 1);
      setCode(errCode);

      errors[seq].code = paramCode;
      errors[seq].position = position;
      errors[seq].reason = CORBA::string_dup(
              epp->GetReasonMessage(reasonMsg, getLang()));
      return errCode;
  }
  void failedInternal(
    const char *msg)
  {
      TRACE(">> failed internal");
    code = ret->code = COMMAND_FAILED;
    epp->ServerInternalError(msg, db->GetsvTRID());
  }
  void NoMessage()
  {
    code = ret->code = COMMAND_NO_MESG;
    ccReg::Response_var& r(getRet());
    epp->NoMessages(r->code, r->msg, r->svTRID);
  }
  void setCode(
    int _code)
  {
    code = ret->code = _code;
  }

  void set_notification_params(
      unsigned long long id,
  Notification::notified_event request_type,
  bool disable_epp_notifier)
  {
      notification_params_ = NotificationParams(id,request_type, disable_epp_notifier);
  }

};

/// replace GetContactID
static long int getIdOfContact(
DBSharedPtr db, const char *handle, bool restricted_handles
    , bool lock_epp_commands, bool lock = false)
{
  if (lock && !lock_epp_commands) lock = false;
  std::auto_ptr<Fred::Contact::Manager>
      cman(Fred::Contact::Manager::create(db, restricted_handles) );
  Fred::Contact::Manager::CheckAvailType caType;
  long int ret = -1;
  try {
    Fred::NameIdPair nameId;
    caType = cman->checkAvail(handle,nameId, lock);
    ret = nameId.id;
    if (caType == Fred::Contact::Manager::CA_INVALID_HANDLE)
    ret = -1;
  } catch (...) {}
  return ret;
}

/// replace GetNssetID
static long int getIdOfNsset(
DBSharedPtr db, const char *handle, bool restricted_handles
    , bool lock_epp_commands, bool lock = false)
{
  if (lock && !lock_epp_commands) lock = false;
  std::auto_ptr<Fred::Zone::Manager>
      zman(Fred::Zone::Manager::create() );
  std::auto_ptr<Fred::Nsset::Manager> man(Fred::Nsset::Manager::create(
      db, zman.get(), restricted_handles) );
  Fred::Nsset::Manager::CheckAvailType caType;
  long int ret = -1;
  try {
    Fred::NameIdPair nameId;
    caType = man->checkAvail(handle,nameId,lock);
    ret = nameId.id;
    if (caType == Fred::Nsset::Manager::CA_INVALID_HANDLE)
    ret = -1;
  } catch (...) {}
  return ret;
}

/// replace GetKeysetID
static long int
getIdOfKeyset(DBSharedPtr db, const char *handle, bool restricted_handles
    , bool lock_epp_commands, bool lock = false)
{
    if (lock && !lock_epp_commands)
        lock = false;
    std::auto_ptr<Fred::Keyset::Manager> man(
            Fred::Keyset::Manager::create(db, restricted_handles));
    Fred::Keyset::Manager::CheckAvailType caType;
    long int ret = -1;
    try {
        Fred::NameIdPair nameId;
        caType = man->checkAvail(handle, nameId, lock);
        ret = nameId.id;
        if (caType == Fred::Keyset::Manager::CA_INVALID_HANDLE)
            ret = -1;
    } catch (...) {}
    return ret;
}

//
// Example implementational code for IDL interface ccReg::EPP
//
ccReg_EPP_i::ccReg_EPP_i(
    const std::string &_db, MailerManager *_mm, NameService *_ns
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
)

    : database(_db),
    mm(_mm),
    dbman(),
    ns(_ns),

    restricted_handles_(restricted_handles)
    , disable_epp_notifier_(disable_epp_notifier)
    , lock_epp_commands_(lock_epp_commands)
    , nsset_level_(nsset_level)
    , nsset_min_hosts_(nsset_min_hosts)
    , nsset_max_hosts_(nsset_max_hosts)
    , docgen_path_(docgen_path)
    , docgen_template_path_(docgen_template_path)
    , fileclient_path_(fileclient_path)
    , disable_epp_notifier_cltrid_prefix_(disable_epp_notifier_cltrid_prefix)
    , rifd_session_max_(rifd_session_max)
    , rifd_session_timeout_(rifd_session_timeout)
    , rifd_session_registrar_max_(rifd_session_registrar_max)
    , rifd_epp_update_domain_keyset_clear_(rifd_epp_update_domain_keyset_clear)
    , rifd_epp_operations_charging_(rifd_epp_operations_charging),
    epp_update_contact_enqueue_check_(epp_update_contact_enqueue_check),
    db_disconnect_guard_(),
    regMan(),
    epp_sessions_(rifd_session_max, rifd_session_registrar_max, rifd_session_timeout),
    ErrorMsg(),
    ReasonMsg(),
    max_zone()
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");

  // factory_check - required keys are in factory
  FactoryHaveSupersetOfKeysChecker<Fred::PublicRequest::Factory>
  ::KeyVector required_keys = boost::assign::list_of
   (Fred::PublicRequest::PRT_AUTHINFO_AUTO_RIF);

  FactoryHaveSupersetOfKeysChecker<Fred::PublicRequest::Factory>
      (required_keys).check();

  // factory_check - factory keys are in database
  FactoryHaveSubsetOfKeysChecker<Fred::PublicRequest::Factory>
      (Fred::PublicRequest::get_enum_public_request_type()).check();

  // objects are shared between threads!!!
  // init at the beginning and do not change

  Database::Connection conn = Database::Manager::acquire();
  db_disconnect_guard_.reset(new DB(conn));

  LOG(NOTICE_LOG, "successfully  connect to DATABASE");
  regMan.reset(Fred::Manager::create(db_disconnect_guard_, false)); //TODO: replace 'false'
  regMan->initStates();
}
ccReg_EPP_i::~ccReg_EPP_i()
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");

  delete ReasonMsg;
  delete ErrorMsg;

  LOG( ERROR_LOG, "EPP_i destructor");
}

// HANDLE EXCEPTIONS
void ccReg_EPP_i::ServerInternalError(
  const char *fce, const char *svTRID)
{
  LOG( ERROR_LOG ,"Internal errror in fce %s svTRID[%s] " , fce , svTRID );
  throw ccReg::EPP::EppError( COMMAND_FAILED , "" , svTRID , ccReg::Errors(0) );
}

void ccReg_EPP_i::EppError(
  short errCode, const char *msg, const char *svTRID, ccReg::Errors_var& errors)
{
  LOG( WARNING_LOG ,"EppError errCode %d msg %s svTRID[%s] " , errCode , msg , svTRID );
  throw ccReg::EPP::EppError( errCode , msg , svTRID , errors );
}

void ccReg_EPP_i::NoMessages(
  short errCode, const char *msg, const char *svTRID)
{
  LOG( WARNING_LOG ,"NoMessages errCode %d msg %s svTRID[%s] " , errCode , msg , svTRID );
  throw ccReg::EPP::NoMessages ( errCode , msg , svTRID );

}
// END

void ccReg_EPP_i::destroyAllRegistrarSessions(CORBA::Long reg_id)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    ConnectionReleaser releaser;

    epp_sessions_.destroy_all_registrar_sessions(reg_id);

}

int ccReg_EPP_i::GetRegistrarID(
  unsigned long long clientID)
{
  return epp_sessions_.get_registrar_id(clientID);
}

int ccReg_EPP_i::GetRegistrarLang(
  unsigned long long clientID)
{
  return epp_sessions_.get_registrar_lang(clientID);
}

// Load table to memory for speed
int ccReg_EPP_i::LoadReasonMessages()
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");

  Database::Connection conn = Database::Manager::acquire();
  DBSharedPtr DBsql (new DB(conn));

  int i, rows;

  {
    rows=0;
    if (DBsql->ExecSelect("SELECT id , reason , reason_cs FROM enum_reason order by id;") ) {
      rows = DBsql->GetSelectRows();
      ReasonMsg = new Mesg();
      for (i = 0; i < rows; i ++)
        ReasonMsg->AddMesg(atoi(DBsql->GetFieldValue(i, 0) ),
            DBsql->GetFieldValue(i, 1) , DBsql->GetFieldValue(i, 2) );
      DBsql->FreeSelect();
    }
  }

  return rows;
}

int ccReg_EPP_i::LoadErrorMessages()
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");

  Database::Connection conn = Database::Manager::acquire();
  DBSharedPtr DBsql (new DB(conn));

  int i, rows;

  {
    rows=0;
    if (DBsql->ExecSelect("SELECT id , status , status_cs FROM enum_error order by id;") ) {
      rows = DBsql->GetSelectRows();
      ErrorMsg = new Mesg();
      for (i = 0; i < rows; i ++)
        ErrorMsg->AddMesg(atoi(DBsql->GetFieldValue(i, 0) ) ,
            DBsql->GetFieldValue(i, 1) , DBsql->GetFieldValue(i, 2));
      DBsql->FreeSelect();
    }
  }

  return rows;
}

EppString ccReg_EPP_i::GetReasonMessage(
  int err, int lang)
{
  if (lang == LANG_CS)
    return ReasonMsg->GetMesg_CS(err).c_str();
  else
    return ReasonMsg->GetMesg(err).c_str();
}

EppString ccReg_EPP_i::GetErrorMessage(
  int err, int lang)
{
  if (lang == LANG_CS)
    return ErrorMsg->GetMesg_CS(err).c_str();
  else
    return ErrorMsg->GetMesg(err).c_str();
}

short ccReg_EPP_i::SetErrorReason(
  ccReg::Errors_var& errors, short errCode, ccReg::ParamError paramCode,
  short position, int reasonMsg, int lang)
{
  unsigned int seq;

  seq = errors->length();
  errors->length(seq+1);

  errors[seq].code = paramCode;
  errors[seq].position = position;
  errors[seq].reason = CORBA::string_dup(GetReasonMessage(reasonMsg, lang) );

  LOG( WARNING_LOG, "SetErrorReason seq%d: err_code %d position [%d]  param %d msgID [%d] " , seq , errCode , position , paramCode , reasonMsg );
  return errCode;
}

short ccReg_EPP_i::SetReasonContactHandle(
  ccReg::Errors_var& err, const char *handle, int lang)
{

  LOG( WARNING_LOG, "bad format of contact [%s]" , handle );
  return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::contact_handle, 1,
  REASON_MSG_BAD_FORMAT_CONTACT_HANDLE, lang);
}

short ccReg_EPP_i::SetReasonNSSetHandle(
  ccReg::Errors_var& err, const char *handle, int lang)
{

  LOG( WARNING_LOG, "bad format of nsset  [%s]" , handle );
  return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::nsset_handle, 1,
  REASON_MSG_BAD_FORMAT_NSSET_HANDLE, lang);

}

short int
ccReg_EPP_i::SetReasonKeySetHandle(
        ccReg::Errors_var &err,
        const char *handle,
        int lang)
{
    LOG(WARNING_LOG, "bad format of keyset [%s]", handle);
    return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::keyset_handle, 1,
            REASON_MSG_BAD_FORMAT_KEYSET_HANDLE, lang);
}

short ccReg_EPP_i::SetReasonDomainFQDN(
  ccReg::Errors_var& err, const char *fqdn, int zone, int lang)
{

  LOG( WARNING_LOG, "domain in zone %s" , (const char * ) fqdn );
  if (zone == 0)
    return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::domain_fqdn, 1,
    REASON_MSG_NOT_APPLICABLE_DOMAIN, lang);
  else if (zone < 0)
    return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::domain_fqdn, 1,
    REASON_MSG_BAD_FORMAT_FQDN, lang);

  return 0;
}

// get version of the server and actual time
char* ccReg_EPP_i::version(
  ccReg::timestamp_out datetime)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  ConnectionReleaser releaser;

  time_t t;
  char dateStr[MAX_DATE+1];

  LOG( NOTICE_LOG, "get version %s BUILD %s %s", VERSION, __DATE__, __TIME__);

  // return  actual time (local time)
  t = time(NULL);
  get_rfc3339_timestamp(t, dateStr, MAX_DATE+1, false);
  datetime = CORBA::string_dup(dateStr);

  return CORBA::string_dup("DSDng");
}

// Handle disclose flags at the contact based on the DefaultPolicy of the serve
/*
 bool ccReg_EPP_i::is_null( const char *str )
 {
 // set up NULL value
 if( strcmp( str  , ccReg::SET_NULL_VALUE  ) ==  0 || str[0] == 0x8 )return true;
 else return false;

 }
 */
// DISCLOSE
/* description in english:
 info

 A)  if there is policy SHOW ALL (VSE ZOBRAZ), then flag is set up as DISCL_HIDE and items from database, which
 have value 'false', will have value 'true', rest 'false'. If there isn't once single item with 'true',
 flag DISCL_EMPTY returns (value of items aren't unsubstantial)

 B)  if there is policy HIDE ALL (VSE SKRYJ), then flag is set up as DISCL_DISPLAY and items from database, which
 have value 'true', will have value 'true', rest 'false'. If there isn't once single item with 'true',
 flag DISCL_EMPTY returns (value of items aren't unsubstantial)

 */

// db parameter true or false from DB
bool ccReg_EPP_i::get_DISCLOSE(
  bool db)
{
  if (DefaultPolicy() ) {
    if (db == false)
      return true;
    else
      return false;
  } else {
    if (db == true)
      return true;
    else
      return false;
  }

}

/*
 1)   if  there is DISCL_HIDE flag and default policy is SHOW ALL (VSE ZOBRAZ), then 'true'
 is saved into database for idl items with 'false' value and 'false' for udl items with 'true'

 2)   if there is DISCL_DISPLAY flag and default policy is SHOW ALL (VSE ZOBRAZ), then is saved into database
 to all items 'true' value

 3)   if there is DISCL_HIDE flag and default policy is HIDE ALL (VSE SKRYJ), then is saved into database
 to all items 'false' value

 4)   if there is DISCL_DISPLAY flag and default policy is HIDE ALL (VSE SKRYJ), then is saved into database 'true'
 for idl items with 'true' value and 'false' for idl items with 'false' value

 5)   if there is DISCL_EMPTY flag and default policy is SHOW ALL (VSE ZOBRAZ), then 'true' is saved into database for all

 6)   if there is DISCL_EMPTY flag and default policy is HIDE ALL (VSE SKRYJ), then 'false' is saved into database for all

 update it is same as create only if there is DISCL_EMPTY flag then database isn't updated (no matter to server policy)

 */

// for update
// set parameter disclose when update
char ccReg_EPP_i::update_DISCLOSE(
  bool d, ccReg::Disclose flag)
{

  if (flag == ccReg::DISCL_EMPTY)
    return ' '; // nothing change
  else {
    if (setvalue_DISCLOSE(d, flag) )
      return 't';
    else
      return 'f';
  }

}

// for create
bool ccReg_EPP_i::setvalue_DISCLOSE(
  bool d, ccReg::Disclose flag)
{

  switch (flag) {
    case ccReg::DISCL_DISPLAY:
      if (DefaultPolicy() )
        return true; // 2
      else // 4
      {
        if (d)
          return true;
        else
          return false;
      }

    case ccReg::DISCL_HIDE:
      if (DefaultPolicy() ) {
        // 1
        if (d)
          return false;
        else
          return true;
      } else
        return true; // 3

    case ccReg::DISCL_EMPTY:
      // use policy from server
      if (DefaultPolicy() )
        return true; // 5
      else
        return false; // 6
  }

  // default
  return false;
}

std::vector<int>
ccReg_EPP_i::GetAllZonesIDs(
    DBSharedPtr db)
{
    std::vector<int> ret;
    std::string query("SELECT id FROM zone;");
    DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
    if (!db->ExecSelect(query.c_str())) {
        LOGGER(PACKAGE).error("cannot retrieve zones ids from the database");
        return ret;
    }
    if (db->GetSelectRows() == 0) {
        LOGGER(PACKAGE).error("GetAllZonesIDs: result size is zero");
        return ret;
    }
    for (int i = 0; i < db->GetSelectRows(); i++) {
        ret.push_back(atoi(db->GetFieldValue(i, 0)));
    }

    return ret;
}

// ZONE parameters
int ccReg_EPP_i::GetZoneExPeriodMin(
    DBSharedPtr db,
  int id)
{
    std::stringstream query;
    query << "SELECT ex_period_min FROM zone WHERE id=" << id << ";";
    DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
    if (!db->ExecSelect(query.str().c_str())) {
        LOGGER(PACKAGE).error("Cannot retrieve ``ex_period_min'' from the database");
        return 0;
    }
    return atoi(db->GetFieldValue(0, 0));
}

int ccReg_EPP_i::GetZoneExPeriodMax(
  DBSharedPtr db,
  int id)
{
    std::stringstream query;
    query << "SELECT ex_period_max FROM zone WHERE id=" << id << ";";
    DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
    if (!db->ExecSelect(query.str().c_str())) {
        LOGGER(PACKAGE).error("Cannot retrieve ``ex_period_max'' from the database");
        return 0;
    }
    return atoi(db->GetFieldValue(0, 0));
}

int ccReg_EPP_i::GetZoneValPeriod(
  DBSharedPtr db,
  int id)
{
    std::stringstream query;
    query << "SELECT val_period FROM zone WHERE id=" << id << ";";
    DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
    if (!db->ExecSelect(query.str().c_str())) {
        LOGGER(PACKAGE).error("Cannot retrieve ``val_period'' from the database");
        return 0;
    }
    return atoi(db->GetFieldValue(0, 0));
}

bool ccReg_EPP_i::GetZoneEnum(
        DBSharedPtr db,
  int id)
{
    std::stringstream query;
    query << "SELECT enum_zone FROM zone WHERE id=" << id << ";";
    DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
    if (!db->ExecSelect(query.str().c_str())) {
        LOGGER(PACKAGE).error("cannot retrieve ``enum_zone'' from the database");
        return false;
    }
    if (strcmp("t", db->GetFieldValue(0, 0)) == 0) {
        return true;
    }
    return false;
}

int ccReg_EPP_i::GetZoneDotsMax(
  DBSharedPtr db,
  int id)
{
    std::stringstream query;
    query << "SELECT dots_max FROM zone WHERE id=" << id << ";";
    DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
    if (!db->ExecSelect(query.str().c_str())) {
        LOGGER(PACKAGE).error("cannot retrieve ``dots_max'' from the database");
        return 0;
    }
    return atoi(db->GetFieldValue(0, 0));
}

std::string ccReg_EPP_i::GetZoneFQDN(
        DBSharedPtr db,
  int id)
{
    std::stringstream query;
    query << "SELECT fqdn FROM zone WHERE id=" << id << ";";
    DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
    if (!db->ExecSelect(query.str().c_str())) {
        LOGGER(PACKAGE).error("cannot retrieve ``fqdn'' from the database");
        return std::string("");
    }
    return std::string(db->GetFieldValue(0, 0));
}

int ccReg_EPP_i::getZone(
        DBSharedPtr db,
  const char *fqdn)
{
    std::stringstream zoneQuery;
    std::string domain_fqdn(fqdn);
    int pos = getZoneMax(db, fqdn);
    if (pos == 0) {
        LOGGER(PACKAGE).debug("getZone: dot position is zero");
        return 0;
    }
    zoneQuery
        << "SELECT id FROM zone WHERE lower(fqdn)=lower('"
        << domain_fqdn.substr(pos + 1, std::string::npos)
        << "');";
    DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
    if (!db->ExecSelect(zoneQuery.str().c_str())) {
        LOGGER(PACKAGE).error("cannot retrieve zone id from the database");
        return 0;
    } else {
        return atoi(db->GetFieldValue(0, 0));
    }
    return 0;
}

int ccReg_EPP_i::getZoneMax(
        DBSharedPtr db,
  const char *fqdn)
{
    std::string query("SELECT fqdn FROM zone ORDER BY length(fqdn) DESC");
    DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
    if (!db->ExecSelect(query.c_str())) {
        LOGGER(PACKAGE).error("cannot retrieve list of fqdn from the database");
        return 0;
    }
    if (db->GetSelectRows() == 0) {
        LOGGER(PACKAGE).error("getZoneMax: result size is zero");
        return 0;
    }
    std::string domain(fqdn);
    boost::to_lower(domain);
    for (int i = 0; i < db->GetSelectRows(); i++) {
        std::string zone(db->GetFieldValue(i, 0));
        int from = domain.length() - zone.length();
        if (from > 1) {
            if (domain.find(zone, from) != std::string::npos) {
                return from - 1;
            }
        }
    }
    return 0;
}

int ccReg_EPP_i::getFQDN(
        DBSharedPtr db,
  char *FQDN, const char *fqdn)
{
  int i, len, max;
  int z;
  int dot=0, dot_max;
  bool en;
  z = getZone(db, fqdn);
  max = getZoneMax(db, fqdn); // return the end

  len = strlen(fqdn);

  FQDN[0] = 0;

  LOG( LOG_DEBUG , "getFQDN [%s] zone %d max %d" , fqdn , z , max );

  // maximal length of the domain
  if (len > 67) {
    LOG( LOG_DEBUG , "out ouf maximal length %d" , len );
    return -1;
  }
  if (max == 0) {
    LOG( LOG_DEBUG , "minimal length" );
    return -1;
  }

  // test double dot .. and double --
  for (i = 1; i < len; i ++) {

    if (fqdn[i] == '.' && fqdn[i-1] == '.') {
      LOG( LOG_DEBUG , "double \'.\' not allowed" );
      return -1;
    }

    if (fqdn[i] == '-' && fqdn[i-1] == '-') {
      LOG( LOG_DEBUG , "double  \'-\' not allowed" );
      return -1;
    }

  }

  if (fqdn[0] == '-') {
    LOG( LOG_DEBUG , "first \'-\' not allowed" );
    return -1;
  }

  for (i = 0; i <= max; i ++) {
    if (fqdn[i] == '.')
      dot ++;
  }

  // test on the number of maximal dots
  dot_max = GetZoneDotsMax(db, z);

  if (dot > dot_max) {
    LOG( LOG_DEBUG , "too much %d dots max %d" , dot , dot_max );
    return -1;
  }

  en = GetZoneEnum(db, z);

  for (i = 0; i < max; i ++) {

    // TEST for  eunum zone and ccTLD
    if (en) {
      if (isdigit(fqdn[i]) || fqdn[i] == '.' || fqdn[i] == '-')
        FQDN[i] = fqdn[i];
      else {
        LOG( LOG_DEBUG , "character  %c not allowed" , fqdn[i] );
        FQDN[0] = 0;
        return -1;
      }

      // test double numbers
      if (isdigit(fqdn[i]) && isdigit(fqdn[i+1]) ) {
        LOG( LOG_DEBUG , "double digit [%c%c] not allowed" , fqdn[i] , fqdn[i+1] );
        FQDN[0] = 0;
        return -1;
      }

    } else {
      // TEST allowed characters
      if (isalnum(fqdn[i]) || fqdn[i] == '-' || fqdn[i] == '.')
        FQDN[i] = tolower(fqdn[i]) ;
      else {
        LOG( LOG_DEBUG , "character  %c not allowed" , fqdn[i] );
        FQDN[0] = 0;
        return -1;
      }
    }

  }

  // character conversion to lower case
  for (i = max; i < len; i ++)
    FQDN[i] = tolower(fqdn[i]);
  FQDN[i] = 0; // on the end


  LOG( LOG_DEBUG , "zone %d -> FQDN [%s]" , z , FQDN );
  return z;

}

void ccReg_EPP_i::sessionClosed(
  CORBA::ULongLong clientID)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));
  ConnectionReleaser releaser;

  LOGGER(PACKAGE).debug( boost::format("sessionClosed called for clientID %1%") % clientID);
  epp_sessions_.logout_session(clientID);
}


ccReg::Response* ccReg_EPP_i::GetTransaction(
  CORBA::Short errCode, CORBA::ULongLong clientID, ccReg::TID requestId, const char* clTRID,
  const ccReg::XmlErrors& errorCodes, ccReg::ErrorStrings_out errStrings)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));
  ConnectionReleaser releaser;

  Database::Connection conn = Database::Manager::acquire();
  DBSharedPtr DBsql (new DB(conn));

  ccReg::Response_var ret;
  ret = new ccReg::Response;
  int i, len;

  LOG( NOTICE_LOG, "GetTransaction: clientID -> %llu clTRID [%s] ", clientID, clTRID );
  LOG( NOTICE_LOG, "GetTransaction:  errCode %d", (int ) errCode );

  len = errorCodes.length();
  LOG( NOTICE_LOG, "GetTransaction:  errorCodes length %d" , len );

  // default
  ret->code = 0;
  errStrings = new ccReg::ErrorStrings;
  errStrings->length(len);

  for (i = 0; i < len; i ++) {

    switch (errorCodes[i]) {
      case ccReg::poll_msgID_missing:
        (*errStrings)[i]
            = CORBA::string_dup(GetReasonMessage( REASON_MSG_POLL_MSGID_MISSING , GetRegistrarLang( clientID )) );
        break;
      case ccReg::contact_identtype_missing:
        (*errStrings)[i]
            = CORBA::string_dup(GetReasonMessage( REASON_MSG_CONTACT_IDENTTYPE_MISSING , GetRegistrarLang( clientID )) );
        break;
      case ccReg::transfer_op:
        (*errStrings)[i]
            = CORBA::string_dup(GetReasonMessage( REASON_MSG_TRANSFER_OP , GetRegistrarLang( clientID )) );
        break;
      case ccReg::xml_not_valid:
        (*errStrings)[i]
            = CORBA::string_dup(GetReasonMessage( REASON_MSG_XML_VALIDITY_ERROR , GetRegistrarLang( clientID )) );
        break;
      default:
        (*errStrings)[i] = CORBA::string_dup("not specified error");
    }

    LOG( NOTICE_LOG, "return reason msg: errors[%d] code %d  message %s\n" , i , errorCodes[i] , ( char * ) (*errStrings)[i] );
  }

  {
    if (errCode > 0) {
      if (DBsql->BeginAction(clientID, EPP_UnknowAction, clTRID, "", requestId)) {
          // error code
          ret->code = errCode;
          // write to the  action table
          ret->svTRID = CORBA::string_dup(DBsql->EndAction(ret->code) );
          ret->msg = CORBA::string_dup(GetErrorMessage(ret->code,
              GetRegistrarLang(clientID) ) );

          LOG( NOTICE_LOG, "GetTransaction: svTRID [%s] errCode -> %d msg [%s] ", ( char * ) ret->svTRID, ret->code, ( char * ) ret->msg );
      }
    }
  }

  if (ret->code == 0)
    ServerInternalError("GetTransaction");

  return ret._retn();

}

ccReg::Response* ccReg_EPP_i::PollAcknowledgement(
    const char* _msg_id,
    CORBA::ULongLong& _count,
    CORBA::String_out _next_msg_id,
    const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::RegistrarSessionData registrar_session_data =
            Epp::get_registrar_session_data(epp_sessions_, epp_request_params.session_id);

        const std::string message_id = Fred::Corba::unwrap_string_from_const_char_ptr(_msg_id);

        const Epp::Poll::PollAcknowledgementLocalizedResponse poll_acknowledgement_response =
            Epp::Poll::poll_acknowledgement_localized(
                message_id,
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle);

        if (poll_acknowledgement_response.data.number_of_unseen_messages == 0)
        {
            throw ccReg::EPP::NoMessages(Epp::EppResultCode::command_completed_successfully_no_messages,
                                         ccReg_EPP_i::GetErrorMessage(Epp::EppResultCode::command_completed_successfully_no_messages,
                                                                      registrar_session_data.language),
                                         server_transaction_handle.c_str());
        }

        CORBA::String_var next_msg_id = Fred::Corba::wrap_string_to_corba_string(poll_acknowledgement_response
                                                                           .data
                                                                           .oldest_unseen_message_id);

        Fred::Corba::wrap_int(poll_acknowledgement_response.data.number_of_unseen_messages, _count);

        ccReg::Response_var return_value = new ccReg::Response(
            Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                poll_acknowledgement_response.epp_response,
                server_transaction_handle));

        _next_msg_id = next_msg_id._retn();
        return return_value._retn();
    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::PollRequest(
    CORBA::String_out _msg_id,
    CORBA::ULongLong& _count,
    ccReg::timestamp_out _create_time,
    ccReg::PollType& _type,
    CORBA::Any_OUT_arg _msg,
    const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::RegistrarSessionData registrar_session_data =
            Epp::get_registrar_session_data(epp_sessions_, epp_request_params.session_id);

        const Epp::Poll::PollRequestLocalizedResponse poll_request_response =
            Epp::Poll::poll_request_localized(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle);

        if (poll_request_response.data.number_of_unseen_messages == 0)
        {
            throw ccReg::EPP::NoMessages(Epp::EppResultCode::command_completed_successfully_no_messages,
                                         ccReg_EPP_i::GetErrorMessage(Epp::EppResultCode::command_completed_successfully_no_messages,
                                                                      registrar_session_data.language),
                                         server_transaction_handle.c_str());
        }

        Fred::Corba::PollMessage message_and_type;
        message_and_type = Fred::Corba::wrap_into_poll_message(poll_request_response.data.message);
        _msg = message_and_type.content._retn();

        _type = message_and_type.type;

        ccReg::timestamp_var create_time =
            Fred::Corba::wrap_string_to_corba_string(
                Fred::Corba::convert_time_to_local_rfc3339(poll_request_response.data.creation_time));

        Fred::Corba::wrap_int(poll_request_response.data.number_of_unseen_messages, _count);

        const std::string msg_id_string = boost::lexical_cast<std::string>(poll_request_response
                                                                           .data
                                                                           .message_id);
        CORBA::String_var msg_id = Fred::Corba::wrap_string_to_corba_string(msg_id_string);

        ccReg::Response_var return_value = new ccReg::Response(
            Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                poll_request_response.epp_response,
                server_transaction_handle));

        _create_time = create_time._retn();
        _msg_id = msg_id._retn();
        return return_value._retn();
    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

/*
 * idl method for retrieving old and new data of updated domain
 *
 * \param _poll_id        database id of poll message where is stored
 *                        historyid of object we want details about
 * \param _old_data       output parameter - data of object before update
 * \param _new_data       output parameter - data of object after update
 * \param params          common epp parameters
 *
 */
void
ccReg_EPP_i::PollRequestGetUpdateDomainDetails(
    CORBA::ULongLong _message_id,
    ccReg::Domain_out _old_data,
    ccReg::Domain_out _new_data,
    const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::RegistrarSessionData registrar_session_data =
            Epp::get_registrar_session_data(epp_sessions_, epp_request_params.session_id);

        const Epp::Poll::PollRequestUpdateDomainLocalizedResponse domain_update_response =
            Epp::Poll::poll_request_get_update_domain_details_localized(
                _message_id,
                Epp::SessionData(
                    registrar_session_data.registrar_id,
                    registrar_session_data.language,
                    server_transaction_handle,
                    epp_request_params.log_request_id.get_value_or(0)));

        ccReg::Domain_var old_data = new ccReg::Domain;
        Fred::Corba::Epp::Domain::wrap_Epp_Domain_InfoDomainLocalizedOutputData(
            domain_update_response.data.old_data,
            old_data);

        ccReg::Domain_var new_data = new ccReg::Domain;
        Fred::Corba::Epp::Domain::wrap_Epp_Domain_InfoDomainLocalizedOutputData(
            domain_update_response.data.new_data,
            new_data);

        _old_data = old_data._retn();
        _new_data = new_data._retn();
    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

/*
 * idl method for retrieving old and new data of updated nsset
 *
 * \param _poll_id        database id of poll message where is stored
 *                        historyid of object we want details about
 * \param _old_data       output parameter - data of object before update
 * \param _new_data       output parameter - data of object after update
 * \param params          common epp parameters
 *
 */
void
ccReg_EPP_i::PollRequestGetUpdateNSSetDetails(
    CORBA::ULongLong _message_id,
    ccReg::NSSet_out _old_data,
    ccReg::NSSet_out _new_data,
    const ccReg::EppParams &_epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::RegistrarSessionData registrar_session_data =
            Epp::get_registrar_session_data(epp_sessions_, epp_request_params.session_id);

        const Epp::Poll::PollRequestUpdateNssetLocalizedResponse nsset_update_response =
            Epp::Poll::poll_request_get_update_nsset_details_localized(
                _message_id,
                Epp::SessionData(
                    registrar_session_data.registrar_id,
                    registrar_session_data.language,
                    server_transaction_handle,
                    epp_request_params.log_request_id.get_value_or(0)));

        ccReg::NSSet_var old_data =
            new ccReg::NSSet(Fred::Corba::wrap_localized_info_nsset(nsset_update_response.data.old_data));
        ccReg::NSSet_var new_data =
            new ccReg::NSSet(Fred::Corba::wrap_localized_info_nsset(nsset_update_response.data.new_data));

        _old_data = old_data._retn();
        _new_data = new_data._retn();
    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

/*
 * idl method for retrieving old and new data of updated keyset
 *
 * \param _poll_id        database id of poll message where is stored
 *                        historyid of object we want details about
 * \param _old_data       output parameter - data of object before update
 * \param _new_data       output parameter - data of object after update
 * \param params          common epp parameters
 *
 */
void
ccReg_EPP_i::PollRequestGetUpdateKeySetDetails(
    CORBA::ULongLong _message_id,
    ccReg::KeySet_out _old_data,
    ccReg::KeySet_out _new_data,
    const ccReg::EppParams &_epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::RegistrarSessionData registrar_session_data =
            Epp::get_registrar_session_data(epp_sessions_, epp_request_params.session_id);

        const Epp::Poll::PollRequestUpdateKeysetLocalizedResponse keyset_update_response =
            Epp::Poll::poll_request_get_update_keyset_details_localized(
                _message_id,
                Epp::SessionData(
                    registrar_session_data.registrar_id,
                    registrar_session_data.language,
                    server_transaction_handle,
                    epp_request_params.log_request_id.get_value_or(0)));

        ccReg::KeySet_var old_data = new ccReg::KeySet;
        Fred::Corba::wrap_Epp_Keyset_Localized_InfoKeysetLocalizedOutputData(
            keyset_update_response.data.old_data,
            old_data);

        ccReg::KeySet_var new_data = new ccReg::KeySet;
        Fred::Corba::wrap_Epp_Keyset_Localized_InfoKeysetLocalizedOutputData(
            keyset_update_response.data.new_data,
            new_data);

        _old_data = old_data._retn();
        _new_data = new_data._retn();
    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response *
ccReg_EPP_i::ClientCredit(ccReg::ZoneCredit_out credit, const ccReg::EppParams &params)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    unsigned int z, seq, zoneID;
    short int code = 0;

    credit = new ccReg::ZoneCredit;
    credit->length(0);
    seq=0;

    LOG( NOTICE_LOG, "ClientCredit: clientID -> %llu clTRID [%s]", params.loginID, static_cast<const char*>(params.clTRID) );

    EPPAction action(this, params.loginID, EPP_ClientCredit, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    try {
        std::vector<int> zones = GetAllZonesIDs(action.getDB());
        for (z = 0; z < zones.size(); z++) {
            zoneID = zones[z];
            // credit of the registrar
            std::string price = action.getDB()->GetRegistrarCredit(action.getRegistrar(), zoneID);

            //  return all not depend on            if( price >  0)
            {
                credit->length(seq+1);
                credit[seq].price = CORBA::string_dup(price.c_str());
                credit[seq].zone_fqdn = CORBA::string_dup(GetZoneFQDN(action.getDB(), zoneID).c_str() );
                seq++;
            }
        }
        code = COMMAND_OK;
    }
    catch (...) {
        code = COMMAND_FAILED;
    }

    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("ClientCredit");
    }

    return action.getRet()._retn();
}

ccReg::Response *
ccReg_EPP_i::ClientLogout(const ccReg::EppParams &params)
{
    Logging::Context::clear();
  Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    LOG( NOTICE_LOG, "ClientLogout: clientID -> %llu clTRID [%s]", params.loginID, static_cast<const char*>(params.clTRID) );
    EPPAction action(this, params.loginID, EPP_ClientLogout, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    epp_sessions_.logout_session(params.loginID);
    action.setCode(COMMAND_LOGOUT);

    return action.getRet()._retn();
}

ccReg::Response * ccReg_EPP_i::ClientLogin(
  const char *ClID, const char *passwd, const char *newpass,
  const char *clTRID, const char* XML,
  CORBA::ULongLong & out_clientID,
  ccReg::TID requestId,
  const char *certID, ccReg::Languages lang)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  ConnectionReleaser releaser;

  int regID=0;
  int language=0;
  ccReg::Response_var ret;
  ret = new ccReg::Response;

  // default
  ret->code = 0;
  out_clientID = 0;

  LOG( NOTICE_LOG, "ClientLogin: username-> [%s] clTRID [%s] passwd [%s]  newpass [%s] ", ClID, static_cast<const char*>(clTRID), passwd, newpass );
  LOG( NOTICE_LOG, "ClientLogin:  certID  [%s] language  [%d] ", certID, lang );

  Database::Connection conn = Database::Manager::acquire();

   {
    DBSharedPtr nodb;
    DBSharedPtr DBsql (new DB(conn));

    std::auto_ptr<Fred::Registrar::Manager> regman(
         Fred::Registrar::Manager::create(nodb));
    try {
        // get ID of registrar by handle
        if ((regID = DBsql->GetNumericFromTable("REGISTRAR", "id", "handle",
                (char *) ClID)) == 0) {
            LOG(NOTICE_LOG, "bad username [%s]", ClID);
            // bad username
            ret->code = COMMAND_AUTH_ERROR;
        } else if (regman->isRegistrarBlocked(regID)) {
            // registrar blocked
            LOGGER(PACKAGE).notice((boost::format("Registrar %1% login attempt while blocked. ") % ClID).str());
            ret->code = COMMAND_AUTOR_ERROR;
        } else if ( !DBsql->TestRegistrarACL(regID, passwd, certID) ) {
            // test password and certificate fingerprint in the table RegistrarACL
            LOG( NOTICE_LOG, "password [%s]  or certID [%s]  not accept", passwd , certID );
            ret->code = COMMAND_AUTH_ERROR;
        }
        else
        {
            //get db connection and start transaction
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);


                // change language
                if (lang == ccReg::CS) {
                    LOG( NOTICE_LOG, "SET LANG to CS" );
                    language=1;
                    }

                // change password if set new
                if (strlen(newpass) ) {
                    LOG( NOTICE_LOG, "change password  [%s]  to  newpass [%s] ", passwd, newpass );

                    DBsql->UPDATE("REGISTRARACL");
                    DBsql->SET("password", newpass);
                    DBsql->WHERE("registrarid", regID);

                    if (DBsql->EXEC() == false)
                    ret->code = COMMAND_FAILED; // if failed
                }

                if (ret->code == 0) {
                    try {
                        out_clientID = epp_sessions_.login_session(regID, language);

                        LOGGER(PACKAGE).notice(boost::format("ClientLogin: username %1%, regID %2%, clTRID %3%, lang %4% got clientID %5%")
                                                    % ClID % regID % static_cast<const char*>(clTRID) % language % out_clientID);

                        ret->code = COMMAND_OK;
                    } catch (const NumberSessionLimit &ex) {

                        LOGGER(PACKAGE).warning(boost::format("ClientLogin: username %1%, regID %2% clTRID %3%, lang %4% login FAILED, reason %5% ")
                                                    % ClID % regID % static_cast<const char*>(clTRID) % language % ex.what());
                        out_clientID=0; //  not login
                        ret->code =COMMAND_MAX_SESSION_LIMIT; // maximal limit of connection sessions
                    }
                }

            // end of transaction
            if (CMD_FAILED((ret->code))) tx.commit();
            //else rollback
        }

    } catch(std::exception &ex) {
        LOGGER(PACKAGE).error(boost::format("Exception in ccReg_EPP_i::ClientLogin: %1%") % ex.what());
        ret->code = COMMAND_FAILED;
    } catch(...) {
        LOGGER(PACKAGE).error("Unknown exception in ccReg_EPP_i::ClientLogin");
    }

    // write  to table action aand return  svTRID
    if (DBsql->BeginAction(out_clientID, EPP_ClientLogin, static_cast<const char*>(clTRID), XML, requestId) ) {
        ret->svTRID = CORBA::string_dup(DBsql->EndAction(ret->code) );

        ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
                        GetRegistrarLang(out_clientID) ) );
    } else {
        ServerInternalError("ClientLogin");
    }

  }

  if (ret->code == 0)
    ServerInternalError("ClientLogin");
  ccReg::Errors_var errors = new ccReg::Errors;
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  return ret._retn();
}

ccReg::Response* ccReg_EPP_i::ContactCheck(
        const ccReg::Check& _contact_handles_to_be_checked,
        ccReg::CheckResp_out _check_results,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        // output data must be ordered exactly the same
        const std::vector<std::string> handles_to_be_checked = Fred::Corba::unwrap_handle_sequence_to_string_vector(_contact_handles_to_be_checked);

        // underscore-prefixed (not unwrapped) input arguments shall not be used from here onwards

        const Epp::Contact::CheckContactConfigData check_contact_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::Contact::CheckContactLocalizedResponse check_contact_localized_response =
                Epp::Contact::check_contact_localized(
                        std::set<std::string>(handles_to_be_checked.begin(), handles_to_be_checked.end()),
                        check_contact_config_data,
                        session_data);

        ccReg::CheckResp_var contact_check_results = new ccReg::CheckResp(
                Fred::Corba::wrap_localized_check_info(
                        handles_to_be_checked,
                        check_contact_localized_response.contact_statuses));

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                check_contact_localized_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _check_results = contact_check_results._retn();
        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::NSSetCheck(
        const ccReg::Check& _nsset_handles_to_be_checked,
        ccReg::CheckResp_out _check_results,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        // output data must be ordered exactly the same
        const std::vector<std::string> handles_to_be_checked = Fred::Corba::unwrap_handle_sequence_to_string_vector(_nsset_handles_to_be_checked);

        // underscore-prefixed (not unwrapped) input arguments shall not be used from here onwards

        const Epp::Nsset::CheckNssetConfigData check_nsset_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::Nsset::CheckNssetLocalizedResponse check_nsset_localized_response =
                Epp::Nsset::check_nsset_localized(
                        std::set<std::string>(handles_to_be_checked.begin(), handles_to_be_checked.end()),
                        check_nsset_config_data,
                        session_data);

        ccReg::CheckResp_var check_results = new ccReg::CheckResp(
            Fred::Corba::wrap_localized_check_info(
                handles_to_be_checked,
                check_nsset_localized_response.nsset_statuses
            )
        );

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                check_nsset_localized_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _check_results = check_results._retn();
        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::DomainCheck(
        const ccReg::Check& _fqdns,
        ccReg::CheckResp_out _domain_check_results,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        // output data must be ordered exactly the same
        const std::vector<std::string> fqdns = Fred::Corba::unwrap_handle_sequence_to_string_vector(_fqdns);

        // underscore-prefixed (not unwrapped) input arguments shall not be used from here onwards

        const Epp::Domain::CheckDomainConfigData check_domain_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::Domain::CheckDomainLocalizedResponse check_domain_localized_response =
                Epp::Domain::check_domain_localized(
                        std::set<std::string>(fqdns.begin(), fqdns.end()),
                        check_domain_config_data,
                        session_data);

        ccReg::CheckResp_var domain_check_results = new ccReg::CheckResp(
                Fred::Corba::Epp::Domain::wrap_Epp_Domain_CheckDomainLocalizedResponse(
                        fqdns,
                        check_domain_localized_response.fqdn_to_domain_localized_registration_obstruction));

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                check_domain_localized_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _domain_check_results = domain_check_results._retn();
        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::KeySetCheck(
    const ccReg::Check& _keyset_handles_to_be_checked,
    ccReg::CheckResp_out _check_results,
    const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const std::vector<std::string> handles_to_be_checked =
            Fred::Corba::unwrap_handle_sequence_to_string_vector(_keyset_handles_to_be_checked);

        // underscore-prefixed (not unwrapped) input arguments shall not be used from here onwards

        const Epp::Keyset::CheckKeysetConfigData check_keyset_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::Keyset::CheckKeysetLocalizedResponse localized_response =
                Epp::Keyset::check_keyset_localized(
                        std::set<std::string>(handles_to_be_checked.begin(), handles_to_be_checked.end()),
                        check_keyset_config_data,
                        session_data);

        ccReg::CheckResp_var check_results = new ccReg::CheckResp;
        Fred::Corba::wrap_Epp_Keyset_Localized_CheckKeysetLocalizedResponse_Results(
                handles_to_be_checked,
                localized_response.results,
                check_results);

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                localized_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _check_results = check_results._retn();
        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::ContactInfo(
        const char* const _contact_handle,
        ccReg::Contact_out _contact_info,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Contact::InfoContactConfigData info_contact_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::Contact::InfoContactLocalizedResponse info_contact_localized_response =
                Epp::Contact::info_contact_localized(
                        Fred::Corba::unwrap_string(_contact_handle),
                        info_contact_config_data,
                        session_data);

        ccReg::Contact_var contact_info = new ccReg::Contact;
        Fred::Corba::wrap_InfoContactLocalizedOutputData(info_contact_localized_response.data, contact_info.inout());

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                info_contact_localized_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _contact_info = contact_info._retn();
        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::ContactDelete(
        const char* const _contact_handle,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Contact::DeleteContactConfigData delete_contact_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::EppResponseSuccessLocalized epp_response_success_localized =
                Epp::Contact::delete_contact_localized(
                        Fred::Corba::unwrap_string_from_const_char_ptr(_contact_handle),
                        delete_contact_config_data,
                        session_data,
                        notification_data);

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::ContactUpdate(
        const char* const _contact_handle,
        const ccReg::ContactChange& _contact_change_data,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        Epp::Contact::ContactChange contact_update_data;
        Fred::Corba::unwrap_ContactChange(_contact_change_data, contact_update_data);

        const Epp::Contact::UpdateContactConfigData update_contact_config_data(
                rifd_epp_operations_charging_,
                epp_update_contact_enqueue_check_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::EppResponseSuccessLocalized epp_response_success_localized =
                Epp::Contact::update_contact_localized(
                        Fred::Corba::unwrap_string(_contact_handle),
                        contact_update_data,
                        update_contact_config_data,
                        session_data,
                        notification_data);

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::ContactCreate(
        const char* const _contact_handle,
        const ccReg::ContactData& _contact_data,
        ccReg::timestamp_out _create_time,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try
    {
        Epp::Contact::ContactData contact_create_data;
        Fred::Corba::unwrap_ContactData(_contact_data, contact_create_data);

        const Epp::Contact::CreateContactInputData create_contact_input_data(contact_create_data);

        const Epp::Contact::CreateContactConfigData create_contact_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::Contact::CreateContactLocalizedResponse create_contact_localized_response =
                Epp::Contact::create_contact_localized(
                        Fred::Corba::unwrap_string(_contact_handle),
                        create_contact_input_data,
                        create_contact_config_data,
                        session_data,
                        notification_data);

        ccReg::timestamp_var create_time =
                Fred::Corba::wrap_string_to_corba_string(
                        Fred::Corba::convert_time_to_local_rfc3339(create_contact_localized_response.crdate));

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                create_contact_localized_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _create_time = create_time._retn();
        return return_value._retn();
    }
    catch (const Epp::EppResponseFailureLocalized& e)
    {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::ContactTransfer(
        const char* const _contact_handle,
        const char* const _authinfopw,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Contact::TransferContactConfigData transfer_contact_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::EppResponseSuccessLocalized epp_response_success_localized =
                Epp::Contact::transfer_contact_localized(
                        Fred::Corba::unwrap_string_from_const_char_ptr(_contact_handle),
                        Fred::Corba::unwrap_string_from_const_char_ptr(_authinfopw),
                        transfer_contact_config_data,
                        session_data,
                        notification_data);

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::NSSetTransfer(
        const char* _nsset_handle,
        const char* _authinfopw,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Nsset::TransferNssetConfigData transfer_nsset_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::EppResponseSuccessLocalized epp_response_success_localized =
                Epp::Nsset::transfer_nsset_localized(
                        Fred::Corba::unwrap_string(_nsset_handle),
                        Fred::Corba::unwrap_string(_authinfopw),
                        transfer_nsset_config_data,
                        session_data,
                        notification_data);

        ccReg::Response_var return_value = new ccReg::Response(
                Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                        epp_response_success_localized,
                        server_transaction_handle));

        // no exception shall be thrown from here onwards

        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::DomainTransfer(
        const char* _fqdn,
        const char* _authinfopw,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Domain::TransferDomainConfigData transfer_domain_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::EppResponseSuccessLocalized epp_response_success_localized =
                Epp::Domain::transfer_domain_localized(
                        Fred::Corba::unwrap_string_from_const_char_ptr(_fqdn),
                        Fred::Corba::unwrap_string_from_const_char_ptr(_authinfopw),
                        transfer_domain_config_data,
                        session_data,
                        notification_data);

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::KeySetTransfer(
        const char* _keyset_handle,
        const char* _authinfopw,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Keyset::TransferKeysetConfigData transfer_keyset_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::EppResponseSuccessLocalized epp_response_success_localized =
                Epp::Keyset::transfer_keyset_localized(
                        Fred::Corba::unwrap_string_from_const_char_ptr(_keyset_handle),
                        Fred::Corba::unwrap_string_from_const_char_ptr(_authinfopw),
                        transfer_keyset_config_data,
                        session_data,
                        notification_data);

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::NSSetInfo(
        const char* _nsset_handle,
        ccReg::NSSet_out _nsset_info,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Nsset::InfoNssetConfigData info_nsset_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::Nsset::InfoNssetLocalizedResponse info_nsset_localized_response =
                Epp::Nsset::info_nsset_localized(
                        Fred::Corba::unwrap_string(_nsset_handle),
                        info_nsset_config_data,
                        session_data);

        ccReg::NSSet_var nsset_info = new ccReg::NSSet(Fred::Corba::wrap_localized_info_nsset(info_nsset_localized_response.data));

        ccReg::Response_var return_value =
                new ccReg::Response(
                    Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                info_nsset_localized_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _nsset_info = nsset_info._retn();
        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::NSSetDelete(
        const char* _nsset_handle,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Nsset::DeleteNssetConfigData delete_nsset_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::EppResponseSuccessLocalized epp_response_success_localized =
                Epp::Nsset::delete_nsset_localized(
                        Fred::Corba::unwrap_string(_nsset_handle),
                        delete_nsset_config_data,
                        session_data,
                        notification_data);

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::NSSetCreate(
        const char* _nsset_handle,
        const char* _authinfopw,
        const ccReg::TechContact& _tech_contacts,
        const ccReg::DNSHost& _dns_hosts,
        CORBA::Short _tech_check_level,
        ccReg::timestamp_out _create_time,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Nsset::CreateNssetInputData create_nsset_input_data(
                Fred::Corba::unwrap_string_from_const_char_ptr(_nsset_handle),
                Fred::Corba::unwrap_string_from_const_char_ptr(_authinfopw),
                Fred::Corba::unwrap_ccreg_techcontacts_to_vector_string(_tech_contacts),
                Fred::Corba::unwrap_ccreg_dnshosts_to_vector_dnshosts(_dns_hosts),
                Fred::Corba::unwrap_tech_check_level(_tech_check_level));

        // underscore-prefixed (not unwrapped) input arguments shall not be used from here onwards

        const Epp::Nsset::CreateNssetConfigData create_nsset_config_data(
                rifd_epp_operations_charging_,
                nsset_level_,
                nsset_min_hosts_,
                nsset_max_hosts_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::Nsset::CreateNssetLocalizedResponse create_nsset_localized_response =
                Epp::Nsset::create_nsset_localized(
                        create_nsset_input_data,
                        create_nsset_config_data,
                        session_data,
                        notification_data);

        ccReg::timestamp_var create_time =
                Fred::Corba::wrap_string_to_corba_string(
                        Fred::Corba::convert_time_to_local_rfc3339(create_nsset_localized_response.crdate));

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                create_nsset_localized_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _create_time = create_time._retn();
        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::NSSetUpdate(
        const char* _nsset_handle,
        const char* _authinfopw_chg,
        const ccReg::DNSHost& _dns_hosts_add,
        const ccReg::DNSHost& _dns_hosts_rem,
        const ccReg::TechContact& _tech_contacts_add,
        const ccReg::TechContact& _tech_contacts_rem,
        CORBA::Short _tech_check_level,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::Nsset::UpdateNssetInputData update_nsset_input_data(
                Fred::Corba::unwrap_string_from_const_char_ptr(_nsset_handle),
                Fred::Corba::unwrap_string_for_change_or_remove_to_Optional_string(_authinfopw_chg),
                Fred::Corba::unwrap_ccreg_dnshosts_to_vector_dnshosts(_dns_hosts_add),
                Fred::Corba::unwrap_ccreg_dnshosts_to_vector_dnshosts(_dns_hosts_rem),
                Fred::Corba::unwrap_ccreg_techcontacts_to_vector_string(_tech_contacts_add),
                Fred::Corba::unwrap_ccreg_techcontacts_to_vector_string(_tech_contacts_rem),
                Fred::Corba::unwrap_tech_check_level(_tech_check_level));

        const Epp::Nsset::UpdateNssetConfigData update_nsset_config_data(
                rifd_epp_operations_charging_,
                nsset_min_hosts_,
                nsset_max_hosts_);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::EppResponseSuccessLocalized epp_response_success_localized =
                Epp::Nsset::update_nsset_localized(
                        update_nsset_input_data,
                        update_nsset_config_data,
                        session_data,
                        notification_data);

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::DomainInfo(
        const char* _fqdn,
        ccReg::Domain_out _domain_info,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::Domain::InfoDomainConfigData info_domain_config_data(
                rifd_epp_operations_charging_);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::Domain::InfoDomainLocalizedResponse info_domain_response =
                Epp::Domain::info_domain_localized(
                        Fred::Corba::unwrap_string_from_const_char_ptr(_fqdn),
                        info_domain_config_data,
                        session_data);

        ccReg::Domain_var domain_info = new ccReg::Domain;
        Fred::Corba::Epp::Domain::wrap_Epp_Domain_InfoDomainLocalizedOutputData(
                info_domain_response.info_domain_localized_output_data,
                domain_info.inout());

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                info_domain_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _domain_info = domain_info._retn();
        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::DomainDelete(
        const char* _fqdn,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::Domain::DeleteDomainConfigData delete_domain_config_data(
                rifd_epp_operations_charging_);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::EppResponseSuccessLocalized epp_response_success_localized =
                Epp::Domain::delete_domain_localized(
                        Fred::Corba::unwrap_string_from_const_char_ptr(_fqdn),
                        delete_domain_config_data,
                        session_data,
                        notification_data);

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::DomainUpdate(
        const char* _fqdn,
        const char* _registrant_chg,
        const char* _authinfopw_chg,
        const char* _nsset_chg,
        const char* _keyset_chg,
        const ccReg::AdminContact& _admin_contacts_add,
        const ccReg::AdminContact& _admin_contacts_rem,
        const ccReg::AdminContact& _tmpcontacts_rem,
        const ccReg::EppParams& _epp_params,
        const ccReg::ExtensionList& _enum_validation_extension_list)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Domain::UpdateDomainInputData update_domain_input_data(
                Fred::Corba::unwrap_string_from_const_char_ptr(_fqdn),
                Fred::Corba::unwrap_string_for_change_to_Optional_string(_registrant_chg),
                Fred::Corba::unwrap_string_for_change_or_remove_to_Optional_string(_authinfopw_chg),
                Fred::Corba::Epp::Domain::unwrap_string_for_change_or_remove_to_Optional_Nullable_string(_nsset_chg),
                Fred::Corba::Epp::Domain::unwrap_string_for_change_or_remove_to_Optional_Nullable_string(_keyset_chg),
                Fred::Corba::Epp::Domain::unwrap_ccreg_admincontacts_to_vector_string(_admin_contacts_add),
                Fred::Corba::Epp::Domain::unwrap_ccreg_admincontacts_to_vector_string(_admin_contacts_rem),
                Fred::Corba::Epp::Domain::unwrap_ccreg_admincontacts_to_vector_string(_tmpcontacts_rem),
                Fred::Corba::Epp::Domain::unwrap_enum_validation_extension_list(_enum_validation_extension_list));

        // underscore-prefixed (not unwrapped) input arguments shall not be used from here onwards

        const Epp::Domain::UpdateDomainConfigData update_domain_config_data(
                rifd_epp_operations_charging_,
                rifd_epp_update_domain_keyset_clear_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::EppResponseSuccessLocalized epp_response_success_localized =
                Epp::Domain::update_domain_localized(
                        update_domain_input_data,
                        update_domain_config_data,
                        session_data,
                        notification_data);

        ccReg::Response_var update_domain_response =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        return update_domain_response._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::DomainCreate(
        const char* _fqdn,
        const char* _registrant,
        const char* _nsset,
        const char* _keyset,
        const char* _authinfopw,
        const ccReg::Period_str& _period,
        const ccReg::AdminContact& _admin_contact,
        ccReg::timestamp_out _create_time,
        ccReg::timestamp_out _exdate,
        const ccReg::EppParams& _epp_params,
        const ccReg::ExtensionList& _enum_validation_extension_list)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const std::string authinfopw_value = Fred::Corba::unwrap_string_from_const_char_ptr(_authinfopw);

        const Epp::Domain::CreateDomainInputData create_domain_input_data(
                Fred::Corba::unwrap_string_from_const_char_ptr(_fqdn),
                Fred::Corba::unwrap_string_from_const_char_ptr(_registrant),
                Fred::Corba::unwrap_string_from_const_char_ptr(_nsset),
                Fred::Corba::unwrap_string_from_const_char_ptr(_keyset),
                authinfopw_value.empty()
                        ? boost::optional<std::string>()
                        : boost::optional<std::string>(authinfopw_value),
                Fred::Corba::Epp::Domain::unwrap_domain_registration_period(_period),
                Fred::Corba::Epp::Domain::unwrap_ccreg_admincontacts_to_vector_string(_admin_contact),
                Fred::Corba::Epp::Domain::unwrap_enum_validation_extension_list(_enum_validation_extension_list));

        // underscore-prefixed (not unwrapped) input arguments shall not be used from here onwards

        const Epp::Domain::CreateDomainConfigData create_domain_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::Domain::CreateDomainLocalizedResponse create_domain_localized_response =
                Epp::Domain::create_domain_localized(
                        create_domain_input_data,
                        create_domain_config_data,
                        session_data,
                        notification_data);

        ccReg::timestamp_var create_time =
                Fred::Corba::wrap_string_to_corba_string(
                        Fred::Corba::convert_time_to_local_rfc3339(create_domain_localized_response.crtime));

        ccReg::timestamp_var exdate =
                Fred::Corba::wrap_string_to_corba_string(
                        boost::gregorian::to_iso_extended_string(create_domain_localized_response.expiration_date));

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                create_domain_localized_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _create_time = create_time._retn();
        _exdate = exdate._retn();
        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::DomainRenew(
        const char* _fqdn,
        const char* _current_exdate,
        const ccReg::Period_str& _period,
        ccReg::timestamp_out _exdate,
        const ccReg::EppParams& _epp_params,
        const ccReg::ExtensionList& _enum_validation_extension_list)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Domain::RenewDomainInputData renew_domain_input_data(
                Fred::Corba::unwrap_string_from_const_char_ptr(_fqdn),
                Fred::Corba::unwrap_string_from_const_char_ptr(_current_exdate),
                Fred::Corba::Epp::Domain::unwrap_domain_registration_period(_period),
                Fred::Corba::Epp::Domain::unwrap_enum_validation_extension_list(_enum_validation_extension_list));

        // underscore-prefixed (not unwrapped) input arguments shall not be used from here onwards

        const Epp::Domain::RenewDomainConfigData renew_domain_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::Domain::RenewDomainLocalizedResponse renew_domain_localized_response =
                renew_domain_localized(
                        renew_domain_input_data,
                        renew_domain_config_data,
                        session_data,
                        notification_data);

        ccReg::timestamp_var exdate =
                Fred::Corba::wrap_string_to_corba_string(
                        boost::gregorian::to_iso_extended_string(renew_domain_localized_response.expiration_date));

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                renew_domain_localized_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _exdate = exdate._retn();
        return return_value._retn();
    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::KeySetInfo(
        const char* const _keyset_handle,
        ccReg::KeySet_out _keyset_info,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const std::string keyset_handle = Fred::Corba::unwrap_string_from_const_char_ptr(_keyset_handle);

        // underscore-prefixed (not unwrapped) input arguments shall not be used from here onwards

        const Epp::Keyset::InfoKeysetConfigData info_keyset_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::Keyset::InfoKeysetLocalizedResponse info_keyset_localized_response =
                Epp::Keyset::info_keyset_localized(
                        keyset_handle,
                        info_keyset_config_data,
                        session_data);

        ccReg::KeySet_var keyset_info = new ccReg::KeySet;
        Fred::Corba::wrap_Epp_Keyset_Localized_InfoKeysetLocalizedOutputData(
                info_keyset_localized_response.data,
                keyset_info.inout());

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                info_keyset_localized_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _keyset_info = keyset_info._retn();
        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::KeySetDelete(
        const char* _keyset_handle,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Keyset::DeleteKeysetConfigData delete_keyset_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::EppResponseSuccessLocalized epp_response_success_localized =
                Epp::Keyset::delete_keyset_localized(
                        Fred::Corba::unwrap_string_from_const_char_ptr(_keyset_handle),
                        delete_keyset_config_data,
                        session_data,
                        notification_data);

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

/*
 * testDSRecordDuplicity
 *
 * returns true if ``first'' and ``second'' dsrecords are same (e.g. has same
 * values)
 */
bool
testDSRecordDuplicity(ccReg::DSRecord_str first, ccReg::DSRecord_str second)
{
    if (first.keyTag == second.keyTag &&
            first.alg == second.alg &&
            first.digestType == second.digestType &&
            first.maxSigLife == second.maxSigLife &&
            std::strcmp(first.digest, second.digest) == 0) {
        return true;
    }
    return false;
}

/*
 * testDNSKeyDuplicity
 *
 * returns true if ``first'' and ``second'' dnskey records are same (e.g. has same
 * values)
 */
bool
testDNSKeyDuplicity(ccReg::DNSKey_str first, ccReg::DNSKey_str second)
{
    if (first.flags == second.flags &&
            first.protocol == second.protocol &&
            first.alg == second.alg &&
            std::strcmp(first.key, second.key) == 0) {
        return true;
    }
    return false;
}

ccReg::Response* ccReg_EPP_i::KeySetCreate(
        const char* _keyset_handle,
        const char* _authinfopw,
        const ccReg::TechContact& _tech_contacts,
        const ccReg::DSRecord& _ds_records,
        const ccReg::DNSKey& _dns_keys,
        ccReg::timestamp_out _create_time,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const std::string authinfopw_value = Fred::Corba::unwrap_string_from_const_char_ptr(_authinfopw);
        const Epp::Keyset::CreateKeysetInputData create_keyset_input_data(
                Fred::Corba::unwrap_string_from_const_char_ptr(_keyset_handle),
                authinfopw_value.empty() ? Optional<std::string>() : Optional<std::string>(authinfopw_value),
                Fred::Corba::unwrap_TechContact_to_vector_string(_tech_contacts),
                Fred::Corba::unwrap_ccReg_DSRecord_to_vector_Epp_Keyset_DsRecord(_ds_records),
                Fred::Corba::unwrap_ccReg_DNSKey_to_vector_Epp_Keyset_DnsKey(_dns_keys));

        // underscore-prefixed (not unwrapped) input arguments shall not be used from here onwards

        const Epp::Keyset::CreateKeysetConfigData create_nsset_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::Keyset::CreateKeysetLocalizedResponse create_keyset_localized_response =
                Epp::Keyset::create_keyset_localized(
                        create_keyset_input_data,
                        create_nsset_config_data,
                        session_data,
                        notification_data);

        ccReg::timestamp_var create_time =
                Fred::Corba::wrap_string_to_corba_string(
                        Fred::Corba::convert_time_to_local_rfc3339(create_keyset_localized_response.crdate));

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                create_keyset_localized_response.epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        _create_time = create_time._retn();
        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::KeySetUpdate(
        const char* _keyset_handle,
        const char* _authinfopw,
        const ccReg::TechContact& _tech_contacts_add,
        const ccReg::TechContact& _tech_contacts_rem,
        const ccReg::DSRecord& _ds_records_add,
        const ccReg::DSRecord& _ds_records_rem,
        const ccReg::DNSKey& _dns_keys_add,
        const ccReg::DNSKey& _dns_keys_rem,
        const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Fred::Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();

    try {
        const Epp::Keyset::UpdateKeysetInputData update_keyset_input_data(
                Fred::Corba::unwrap_string_from_const_char_ptr(_keyset_handle),
                Fred::Corba::unwrap_string_for_change_or_remove_to_Optional_string(_authinfopw),
                Fred::Corba::unwrap_TechContact_to_vector_string(_tech_contacts_add),
                Fred::Corba::unwrap_TechContact_to_vector_string(_tech_contacts_rem),
                Fred::Corba::unwrap_ccReg_DSRecord_to_vector_Epp_Keyset_DsRecord(_ds_records_add),
                Fred::Corba::unwrap_ccReg_DSRecord_to_vector_Epp_Keyset_DsRecord(_ds_records_rem),
                Fred::Corba::unwrap_ccReg_DNSKey_to_vector_Epp_Keyset_DnsKey(_dns_keys_add),
                Fred::Corba::unwrap_ccReg_DNSKey_to_vector_Epp_Keyset_DnsKey(_dns_keys_rem));

        // underscore-prefixed (not unwrapped) input arguments shall not be used from here onwards

        const Epp::Keyset::UpdateKeysetConfigData update_keyset_config_data(
                rifd_epp_operations_charging_);

        const Epp::RegistrarSessionData registrar_session_data =
                Epp::get_registrar_session_data(
                        epp_sessions_,
                        epp_request_params.session_id);

        const Epp::SessionData session_data(
                registrar_session_data.registrar_id,
                registrar_session_data.language,
                server_transaction_handle,
                epp_request_params.log_request_id.get_value_or(0));

        const Epp::NotificationData notification_data(
                epp_request_params.client_transaction_id,
                disable_epp_notifier_,
                disable_epp_notifier_cltrid_prefix_);

        const Epp::EppResponseSuccessLocalized epp_response_success_localized =
                Epp::Keyset::update_keyset_localized(
                        update_keyset_input_data,
                        update_keyset_config_data,
                        session_data,
                        notification_data);

        ccReg::Response_var return_value =
                new ccReg::Response(
                        Fred::Corba::wrap_Epp_EppResponseSuccessLocalized(
                                epp_response_success_localized,
                                server_transaction_handle));

        // no exception shall be thrown from here onwards

        return return_value._retn();

    }
    catch (const Epp::EppResponseFailureLocalized& e) {
        throw Fred::Corba::wrap_Epp_EppResponseFailureLocalized(e, server_transaction_handle);
    }
}

// primitive list of objects
ccReg::Response *
ccReg_EPP_i::FullList(short act, const char *table, const char *fname,
        ccReg::Lists_out list, const ccReg::EppParams &params)
{
    int rows =0, i;
    int type;
    char sqlString[128];
    short int code = 0;

    list = new ccReg::Lists;

    LOG( NOTICE_LOG , "LIST %d  clientID -> %llu clTRID [%s] " , act , params.loginID, static_cast<const char*>(params.clTRID) );

    EPPAction action(this, params.loginID, act, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    // by the object
    switch (act) {
        case EPP_ListContact:
            type=1;
            break;
        case EPP_ListNsset:
            type=2;
            break;
        case EPP_ListDomain:
            type=3;
            break;
        case EPP_ListKeyset:
            type=4;
            break;
        default:
            type=0;
    }

    // list all objects of registrar
    snprintf(sqlString, sizeof(sqlString),
            "SELECT obr.name FROM  object_registry obr, object o "
            "WHERE obr.id=o.id AND o.clid=%d AND obr.type=%d", action.getRegistrar(), type);

    if (action.getDB()->ExecSelect(sqlString) ) {
        rows = action.getDB()->GetSelectRows();

        LOG( NOTICE_LOG, "Full List: %s  num -> %d ClID %d", table , rows , action.getRegistrar() );
        list->length(rows);

        for (i = 0; i < rows; i ++) {
            (*list)[i]=CORBA::string_dup(action.getDB()->GetFieldValue(i, 0) );
        }

        action.getDB()->FreeSelect();
        code = COMMAND_OK;
    }

    // command OK
    if (code == 0) {
        action.failedInternal("FullList");
    }

    return action.getRet()._retn();
}

ccReg::Response* ccReg_EPP_i::ContactList(
  ccReg::Lists_out contacts, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  return FullList( EPP_ListContact , "CONTACT" , "HANDLE" , contacts , params);
}

ccReg::Response* ccReg_EPP_i::NSSetList(
  ccReg::Lists_out nssets, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  return FullList( EPP_ListNsset , "NSSET" , "HANDLE" , nssets , params);
}

ccReg::Response* ccReg_EPP_i::DomainList(
  ccReg::Lists_out domains, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  return FullList( EPP_ListDomain , "DOMAIN" , "fqdn" , domains , params);
}

ccReg::Response *
ccReg_EPP_i::KeySetList(
        ccReg::Lists_out keysets, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

    return FullList(
            EPP_ListKeyset, "KEYSET", "HANDLE", keysets, params);
}

// function for run nsset tests
ccReg::Response* ccReg_EPP_i::nssetTest(
  const char* handle, CORBA::Short level, const ccReg::Lists& fqdns,
  const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;


  ccReg::Response_var ret = new ccReg::Response;
  int regID;
  int nssetid;
  bool internalError = false;

  LOG( NOTICE_LOG , "nssetTest nsset %s  clientID -> %llu clTRID [%s] \n" , handle, params.loginID, static_cast<const char*>(params.clTRID) );

  Database::Connection conn = Database::Manager::acquire();
  DBSharedPtr DBsql (new DB(conn));

  if ( (regID = GetRegistrarID(params.loginID) ))
    {

      if ( (DBsql->BeginAction(params.loginID, EPP_NssetTest, static_cast<const char*>(params.clTRID), params.XML, params.requestID) )) {

        if ( (nssetid = getIdOfNsset(DBsql, handle
                , restricted_handles_, lock_epp_commands_) > 0 ))// TODO   ret->code =  SetReasonNSSetHandle( errors  , nssetid , GetRegistrarLang( clientID ) );
        {
          std::stringstream strid;
          strid << regID;
          std::string regHandle = DBsql->GetValueFromTable("registrar",
              "handle", "id", strid.str().c_str() );
          ret->code=COMMAND_OK;
          TechCheckManager tc(ns);
          TechCheckManager::FQDNList ifqdns;
          for (unsigned i=0; i<fqdns.length(); i++)
            ifqdns.push_back((const char *)fqdns[i]);
          try {
            tc.checkFromRegistrar(regHandle,handle,level,ifqdns,params.clTRID);
          }
          catch (TechCheckManager::INTERNAL_ERROR) {
            LOG(ERROR_LOG,"Tech check internal error nsset [%s] clientID -> %llu clTRID [%s] " , handle , params.loginID , static_cast<const char*>(params.clTRID) );
            internalError = true;
          }
          catch (TechCheckManager::REGISTRAR_NOT_FOUND) {
            LOG(ERROR_LOG,"Tech check reg not found nsset [%s] clientID -> %llu clTRID [%s] " , handle , params.loginID , static_cast<const char*>(params.clTRID) );
            internalError = true;
          }
          catch (TechCheckManager::NSSET_NOT_FOUND) {
            LOG(ERROR_LOG,"Tech check nsset not found nset [%s] clientID -> %llu clTRID [%s] " , handle , params.loginID , static_cast<const char*>(params.clTRID) );
            internalError = true;
          }
        } else {
          LOG( WARNING_LOG, "nsset handle [%s] NOT_EXIST", handle );
          ret->code = COMMAND_OBJECT_NOT_EXIST;
        }

        ret->svTRID = CORBA::string_dup(DBsql->EndAction(ret->code) ) ;
      }

      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(params.loginID) ) );

      if (internalError)
        ServerInternalError("NssetTest");
    }

  return ret._retn();
}

// function for send authinfo
ccReg::Response *
ccReg_EPP_i::ObjectSendAuthInfo(
        short act, const char * table, const char *fname, const char *name,
        const ccReg::EppParams &params)
{
    int id = 0;
    std::string FQDN(name);
    boost::to_lower(FQDN);
    short int code = 0;

    EPPAction action(this, params.loginID, act, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    // Database::Manager db(new Database::ConnectionFactory(database));
    // std::auto_ptr<Database::Connection> conn;
    // try { conn.reset(db.getConnection()); } catch (...) {}

    LOG( NOTICE_LOG , "ObjectSendAuthInfo type %d  object [%s]  clientID -> %llu clTRID [%s] " , act , name , params.loginID , static_cast<const char*>(params.clTRID) );

    std::auto_ptr<Fred::Zone::Manager> zm( Fred::Zone::Manager::create() );
    std::vector<std::string> dev_null;

    switch (act) {
        case EPP_ContactSendAuthInfo:
            if ( (id = getIdOfContact(action.getDB(), name
                    , restricted_handles_, lock_epp_commands_) ) < 0) {
                LOG(WARNING_LOG, "bad format of contact [%s]", name);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::contact_handle, 1,
                        REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
            } else if (id == 0)
                code=COMMAND_OBJECT_NOT_EXIST;
            break;
        case EPP_NssetSendAuthInfo:
            if ( (id = getIdOfNsset(action.getDB(), name, restricted_handles_
                    , lock_epp_commands_) ) < 0) {
                LOG(WARNING_LOG, "bad format of nsset [%s]", name);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::nsset_handle, 1,
                        REASON_MSG_BAD_FORMAT_NSSET_HANDLE);
            } else if (id == 0)
                code=COMMAND_OBJECT_NOT_EXIST;
            break;
        case EPP_DomainSendAuthInfo:
            {
                const Database::Result db_result = Database::Manager::acquire().exec_params(
                        "SELECT id FROM object_registry ore "
                        "WHERE ore.type = get_object_type_id('domain'::text) "
                          "AND ore.name = $1::text "
                          "AND ore.erdate IS NULL",
                        Database::query_param_list(FQDN));

                if (db_result.size() == 1) {
                    try {
                        id = boost::numeric_cast<int>(static_cast<unsigned long long>(db_result[0][0]));
                    }
                    catch (...) {
                        LOG(WARNING_LOG, "domain [%s] id to int cast failed", name);
                        ServerInternalError("ObjectSendAuthInfo");
                    }
                }
                else if (db_result.size() < 1) {
                    LOG(WARNING_LOG, "domain [%s] NOT_EXIST", name);
                    code = COMMAND_OBJECT_NOT_EXIST;
                }
                else { // db_result.size() > 1
                    ServerInternalError("ObjectSendAuthInfo");
                }
            }

            break;
        case EPP_KeysetSendAuthInfo:
            if ((id = getIdOfKeyset(action.getDB(), name, restricted_handles_
                        , lock_epp_commands_)) < 0) {
                LOG(WARNING_LOG, "bad format of keyset [%s]", name);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_handle, 1,
                        REASON_MSG_BAD_FORMAT_KEYSET_HANDLE);
            } else if (id == 0)
                code = COMMAND_OBJECT_NOT_EXIST;
            break;
    }
    if (code == 0) {
        std::auto_ptr<Fred::Document::Manager> doc_manager(
                Fred::Document::Manager::create(
                    docgen_path_,
                    docgen_template_path_,
                    fileclient_path_,
                    ns->getHostName()
                    )
                );
        std::auto_ptr<Fred::PublicRequest::Manager> request_manager(
                Fred::PublicRequest::Manager::create(
                    regMan->getDomainManager(),
                    regMan->getContactManager(),
                    regMan->getNssetManager(),
                    regMan->getKeysetManager(),
                    mm,
                    doc_manager.get(),
                    regMan->getMessageManager()
                    )
                );
        try {
            LOG(
                    NOTICE_LOG , "createRequest objectID %d" ,
                    id
               );
            std::auto_ptr<Fred::PublicRequest::PublicRequest> new_request(request_manager->createRequest(
                        Fred::PublicRequest::PRT_AUTHINFO_AUTO_RIF));

            new_request->setRequestId(params.requestID);
            new_request->setRegistrarId(GetRegistrarID(params.loginID));
            new_request->addObject(Fred::PublicRequest::OID(id));
            if (!new_request->check()) {
                LOG(WARNING_LOG, "authinfo request for %s is prohibited",name);
                code = COMMAND_STATUS_PROHIBITS_OPERATION;
            } else {
                code=COMMAND_OK;
                new_request->save();
            }
        } catch (...) {
            LOG( WARNING_LOG, "cannot create and process request");
            code=COMMAND_FAILED;
        }
    }

    // EPP exception
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("ObjectSendAuthInfo");
    }

    return action.getRet()._retn();
}

ccReg::Response* ccReg_EPP_i::domainSendAuthInfo(
  const char* fqdn, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  return ObjectSendAuthInfo( EPP_DomainSendAuthInfo , "DOMAIN" , "fqdn" , fqdn , params);
}
ccReg::Response* ccReg_EPP_i::contactSendAuthInfo(
  const char* handle, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  return ObjectSendAuthInfo( EPP_ContactSendAuthInfo , "CONTACT" , "handle" , handle , params);
}
ccReg::Response* ccReg_EPP_i::nssetSendAuthInfo(
  const char* handle, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  return ObjectSendAuthInfo( EPP_NssetSendAuthInfo , "NSSET" , "handle" , handle , params);
}

ccReg::Response *
ccReg_EPP_i::keysetSendAuthInfo(
        const char *handle,
        const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

    return ObjectSendAuthInfo(
            EPP_KeysetSendAuthInfo, "KEYSET",
            "handle", handle, params);
}

ccReg::Response* ccReg_EPP_i::info(
  ccReg::InfoType type, const char* handle, CORBA::Long& count,
  const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  LOG(
      NOTICE_LOG,
      "Info: clientID -> %llu clTRID [%s]", params.loginID, static_cast<const char*>(params.clTRID)
  );
  // start EPP action - this will handle all init stuff
  EPPAction a(this, params.loginID, EPP_Info, static_cast<const char*>(params.clTRID), params.XML, params.requestID);
  try {
    std::auto_ptr<Fred::Zone::Manager> zoneMan(
        Fred::Zone::Manager::create()
    );
    std::auto_ptr<Fred::Domain::Manager> domMan(
        Fred::Domain::Manager::create(a.getDB(), zoneMan.get())
    );
    std::auto_ptr<Fred::Contact::Manager> conMan(
        Fred::Contact::Manager::create(a.getDB(),restricted_handles_)
    );
    std::auto_ptr<Fred::Nsset::Manager> nssMan(
        Fred::Nsset::Manager::create(
            a.getDB(),zoneMan.get(),restricted_handles_
        )
    );
    std::auto_ptr<Fred::Keyset::Manager> keyMan(
            Fred::Keyset::Manager::create(
                a.getDB(), restricted_handles_
                )
            );
    std::auto_ptr<Fred::InfoBuffer::Manager> infoBufMan(
        Fred::InfoBuffer::Manager::create(
            a.getDB(),
            domMan.get(),
            nssMan.get(),
            conMan.get(),
            keyMan.get()
        )
    );
    count = infoBufMan->info(
        a.getRegistrar(),
        type == ccReg::IT_LIST_CONTACTS ?
        Fred::InfoBuffer::T_LIST_CONTACTS :
        type == ccReg::IT_LIST_DOMAINS ?
        Fred::InfoBuffer::T_LIST_DOMAINS :
        type == ccReg::IT_LIST_NSSETS ?
        Fred::InfoBuffer::T_LIST_NSSETS :
        type == ccReg::IT_LIST_KEYSETS ?
        Fred::InfoBuffer::T_LIST_KEYSETS :
        type == ccReg::IT_DOMAINS_BY_NSSET ?
        Fred::InfoBuffer::T_DOMAINS_BY_NSSET :
        type == ccReg::IT_DOMAINS_BY_CONTACT ?
        Fred::InfoBuffer::T_DOMAINS_BY_CONTACT :
        type == ccReg::IT_NSSETS_BY_CONTACT ?
        Fred::InfoBuffer::T_NSSETS_BY_CONTACT :
        type == ccReg::IT_NSSETS_BY_NS ?
        Fred::InfoBuffer::T_NSSETS_BY_NS :
        type == ccReg::IT_DOMAINS_BY_KEYSET ?
        Fred::InfoBuffer::T_DOMAINS_BY_KEYSET :
        Fred::InfoBuffer::T_KEYSETS_BY_CONTACT,
        handle
    );
  }
  catch (...) {a.failedInternal("Connection problems");}
  return a.getRet()._retn();
}

ccReg::Response* ccReg_EPP_i::getInfoResults(
  ccReg::Lists_out handles, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  LOG(
      NOTICE_LOG,
      "getResults: clientID -> %llu clTRID [%s]", params.loginID, static_cast<const char*>(params.clTRID)
  );
  // start EPP action - this will handle all init stuff
  EPPAction a(this, params.loginID, EPP_GetInfoResults, static_cast<const char*>(params.clTRID), params.XML, params.requestID);
  try {
    std::auto_ptr<Fred::Zone::Manager> zoneMan(
        Fred::Zone::Manager::create()
    );
    std::auto_ptr<Fred::Domain::Manager> domMan(
        Fred::Domain::Manager::create(a.getDB(), zoneMan.get())
    );
    std::auto_ptr<Fred::Contact::Manager> conMan(
        Fred::Contact::Manager::create(a.getDB(),restricted_handles_)
    );
    std::auto_ptr<Fred::Nsset::Manager> nssMan(
        Fred::Nsset::Manager::create(
            a.getDB(),zoneMan.get(),restricted_handles_
        )
    );
    std::auto_ptr<Fred::Keyset::Manager> keyMan(
            Fred::Keyset::Manager::create(
                a.getDB(), restricted_handles_
                )
            );
    std::auto_ptr<Fred::InfoBuffer::Manager> infoBufMan(
        Fred::InfoBuffer::Manager::create(
            a.getDB(),
            domMan.get(),
            nssMan.get(),
            conMan.get(),
            keyMan.get()
        )
    );
    std::auto_ptr<Fred::InfoBuffer::Chunk> chunk(
        infoBufMan->getChunk(a.getRegistrar(),1000)
    );
    handles = new ccReg::Lists();
    handles->length(chunk->getCount());
    for (unsigned long i=0; i<chunk->getCount(); i++)
    (*handles)[i] = CORBA::string_dup(chunk->getNext().c_str());
  }
  catch (...) {a.failedInternal("Connection problems");}
  return a.getRet()._retn();
}

const std::string& ccReg_EPP_i::getDatabaseString()
{
  return database;
}
/* vi:set ts=4 sw=4: */
