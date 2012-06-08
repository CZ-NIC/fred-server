/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
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

//  implementing IDL interfaces for file ccReg.idl
// author: Petr Blaha petr.blaha@nic.cz

#include <fstream>
#include <iostream>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"
#include <boost/algorithm/string.hpp>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <corba/EPP.hh>
#include "epp_impl.h"

#include "corba/connection_releaser.h"

#include "config.h"

// database functions
#include "old_utils/dbsql.h"

// support function
#include "old_utils/util.h"

#include "action.h"    // code of the EPP operations
#include "response.h"  // errors code
#include "reason.h"    // reason messages code

// logger
#include "old_utils/log.h"

// MailerManager is connected in constructor
#include "fredlib/domain.h"
#include "fredlib/contact.h"
#include "fredlib/nsset.h"
#include "fredlib/keyset.h"
#include "fredlib/info_buffer.h"
#include "fredlib/poll.h"
#include "fredlib/invoicing/invoice.h"
#include <memory>
#include "tech_check.h"

// Notifier
#include "notifier.h"

// logger
#include "log/logger.h"
#include "log/context.h"


#define FLAG_serverDeleteProhibited 1
#define FLAG_serverRenewProhibited 2
#define FLAG_serverTransferProhibited 3
#define FLAG_serverUpdateProhibited 4
#define FLAG_serverRegistrantChangeProhibited 18
#define FLAG_deleteCandidate 17

// return values from ``isValidBase64`` function
#define BASE64_OK               0
#define BASE64_BAD_LENGTH       1
#define BASE64_BAD_CHAR         2
#define BASE64_UNKNOWN          3
/*
 * isValidBase64 - returns 0 if some string is valid base64 encoded.
 * -if return value is BASE64_OK then ret is -1
 * -if return value is BASE64_BAD_LENGTH then ret is length of string
 * -if return value is BASE64_BAD_CHAR then ret is zero based position
 *  of first invalid character
 */
int isValidBase64(const char *str, int *ret);
char *removeWhitespaces(const char *encoded);

static bool testObjectHasState(
  DBSharedPtr db, Fred::TID object_id, unsigned state_id)
{
  bool returnState;
  std::stringstream sql;
  sql << "SELECT COUNT(*) FROM object_state " << "WHERE object_id="
      << object_id << " AND state_id=" << state_id << " AND valid_to ISNULL";
  DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
  if (!db->ExecSelect(sql.str().c_str()))
    throw Fred::SQL_ERROR();
  returnState = atoi(db->GetFieldValue(0, 0));
  return returnState;
}


Database::Connection wrapped_acquire(ccReg_EPP_i *epp)
{

    try {
        return Database::Manager::acquire();
    } catch(...) {
        epp->ServerInternalError("Cannot connect to DB");
    }

    /* unreachable code - only for suppress gcc warning */
    return Database::Manager::acquire();
}


class EPPAction
{
  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  ccReg_EPP_i *epp;
  DBSharedPtr  db;
  int regID;
  unsigned long long clientID;
  int code; ///< needed for destructor where Response is invalidated
  EPPNotifier *notifier;
  std::string cltrid;
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
    notifier(0), cltrid(clTRID)
  {
    Logging::Context::push(str(boost::format("action-%1%") % action));

    DBAutoPtr _db( new DB);
    if (!_db->OpenDatabase(epp->getDatabaseString())) {
        epp->ServerInternalError("Cannot connect to DB");
    }
    db = DBDisconnectPtr(_db.release());

    if (!db->BeginAction(clientID, action, clTRID, xml, requestId)) {
      epp->ServerInternalError("Cannot beginAction");
    }
    if (!regID) {
      ret->code = COMMAND_MAX_SESSION_LIMIT;
      ccReg::Response_var& r(getRet());
      db->EndAction(r->code);
      epp->EppError(r->code, r->msg, r->svTRID, errors);
    }
    if (!db->BeginTransaction()) {
      db->EndAction(COMMAND_FAILED);
      epp->ServerInternalError("Cannot start transaction",
          CORBA::string_dup(db->GetsvTRID()) );
    }
    code = ret->code = COMMAND_OK;

    Logging::Context::push(str(boost::format("%1%") % db->GetsvTRID()));
  }

  ///// TODO hack for new invoicing
  EPPAction(
      ccReg_EPP_i *_epp, unsigned long long _clientID, int action, const char *clTRID,
      const char *xml, Database::Connection conn, unsigned long long requestId
    ) :
      ret(new ccReg::Response()), errors(new ccReg::Errors()), epp(_epp),
      regID(_epp->GetRegistrarID(_clientID)), clientID(_clientID),
      notifier(0), cltrid(clTRID)
    {
      Logging::Context::push(str(boost::format("action-inv-%1%") % action));

      /*
      DBAutoPtr _db( new DB(conn));
      db = DBDisconnectPtr(_db.release());
      */
      // ConnectionReleaser will take care of disconnect
      db.reset(new DB(conn));

      if (!db->BeginAction(clientID, action, clTRID, xml, requestId)) {
        epp->ServerInternalError("Cannot beginAction");
      }
      if (!regID) {
        ret->code = COMMAND_MAX_SESSION_LIMIT;
        ccReg::Response_var& r(getRet());
        db->EndAction(r->code);
        epp->EppError(r->code, r->msg, r->svTRID, errors);
      }
      if (!db->BeginTransaction()) {
        db->EndAction(COMMAND_FAILED);
        epp->ServerInternalError("Cannot start transaction",
            CORBA::string_dup(db->GetsvTRID()) );
      }
      code = ret->code = COMMAND_OK;

      Logging::Context::push(str(boost::format("%1%") % db->GetsvTRID()));
    }

  ~EPPAction()
  {
    try
    {
        db->QuitTransaction(code);
        db->EndAction(code);

        if (notifier && (code == COMMAND_OK)) {
            /* disable notifier for configured cltrid prefix */
            if (boost::starts_with(cltrid, epp->get_disable_epp_notifier_cltrid_prefix())
                    && db->GetRegistrarSystem(getRegistrar()))
            {
                LOGGER(PACKAGE).debug(boost::format("disable command notification "
                      "(registrator=%1% cltrid=%2%)") % getRegistrar() % cltrid);
            }
            else {
                notifier->Send();
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
  void setNotifier(EPPNotifier *_notifier)
  {
      notifier = _notifier;
  }
};

/* Ticket #3197 - wrap/overload for function
 * testObjectHasState(DB *db, Fred::TID object_id, unsigned state_id)
 * to test system registrar (should have no restriction)
 */
static bool testObjectHasState(EPPAction &action, Fred::TID object_id,
        unsigned state_id)
{
    DBSharedPtr db = action.getDB();
    if (!db.get()) {
        throw Fred::SQL_ERROR();
    }
    if (db->GetRegistrarSystem(action.getRegistrar())) {
        return 0;
    }
    else {
        return testObjectHasState(db, object_id, state_id);
    }
}

/// timestamp formatting function
static std::string formatTime(
  boost::posix_time::ptime tm)
{
  char buffer[100];
  convert_rfc3339_timestamp(buffer, sizeof(buffer), to_iso_extended_string(tm).c_str());
  return buffer;
}

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

/// replace GetNSSetID
static long int getIdOfNSSet(
DBSharedPtr db, const char *handle, bool restricted_handles
    , bool lock_epp_commands, bool lock = false)
{
  if (lock && !lock_epp_commands) lock = false;
  std::auto_ptr<Fred::Zone::Manager>
      zman(Fred::Zone::Manager::create() );
  std::auto_ptr<Fred::NSSet::Manager> man(Fred::NSSet::Manager::create(
      db, zman.get(), restricted_handles) );
  Fred::NSSet::Manager::CheckAvailType caType;
  long int ret = -1;
  try {
    Fred::NameIdPair nameId;
    caType = man->checkAvail(handle,nameId,lock);
    ret = nameId.id;
    if (caType == Fred::NSSet::Manager::CA_INVALID_HANDLE)
    ret = -1;
  } catch (...) {}
  return ret;
}

/// replace GetKeySetID
static long int
getIdOfKeySet(DBSharedPtr db, const char *handle, bool restricted_handles
    , bool lock_epp_commands, bool lock = false)
{
    if (lock && !lock_epp_commands)
        lock = false;
    std::auto_ptr<Fred::KeySet::Manager> man(
            Fred::KeySet::Manager::create(db, restricted_handles));
    Fred::KeySet::Manager::CheckAvailType caType;
    long int ret = -1;
    try {
        Fred::NameIdPair nameId;
        caType = man->checkAvail(handle, nameId, lock);
        ret = nameId.id;
        if (caType == Fred::KeySet::Manager::CA_INVALID_HANDLE)
            ret = -1;
    } catch (...) {}
    return ret;
}

/// replace GetDomainID
static long int getIdOfDomain(
DBSharedPtr db, const char *handle, bool lock_epp_commands
    , bool lock = false, int* zone = NULL)
{
  if (lock && !lock_epp_commands) lock = false;
  std::auto_ptr<Fred::Zone::Manager> zm(
    Fred::Zone::Manager::create()
  );
  std::auto_ptr<Fred::Domain::Manager> dman(
    Fred::Domain::Manager::create(db,zm.get())
  );
  Fred::NameIdPair nameId;
  long int ret = -1;
  try {
    switch (dman->checkAvail(handle, nameId, lock)) {
      case Fred::Domain::CA_REGISTRED :
        ret = nameId.id;
        break;
      case Fred::Domain::CA_INVALID_HANDLE :
      case Fred::Domain::CA_BAD_LENGHT :
      case Fred::Domain::CA_BLACKLIST :
        ret = -2;
        break;
      case Fred::Domain::CA_BAD_ZONE :
        ret = -1;
        break;
      case Fred::Domain::CA_PARENT_REGISTRED :
      case Fred::Domain::CA_CHILD_REGISTRED :
      case Fred::Domain::CA_AVAILABLE :
        ret = 0;
        break;
    }
    const Fred::Zone::Zone *z = zm->findApplicableZone(handle);
    if (zone && z) *zone = z->getId();
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
    , const std::string& docgen_path
    , const std::string& docgen_template_path
    , const std::string& fileclient_path
    , const std::string& disable_epp_notifier_cltrid_prefix
    , unsigned rifd_session_max
    , unsigned rifd_session_timeout
    , unsigned rifd_session_registrar_max
    , bool rifd_epp_update_domain_keyset_clear
)

    : database(_db),
    mm(_mm),
    dbman(),
    ns(_ns),

    restricted_handles_(restricted_handles)
    , disable_epp_notifier_(disable_epp_notifier)
    , lock_epp_commands_(lock_epp_commands)
    , nsset_level_(nsset_level)
    , docgen_path_(docgen_path)
    , docgen_template_path_(docgen_template_path)
    , fileclient_path_(fileclient_path)
    , disable_epp_notifier_cltrid_prefix_(disable_epp_notifier_cltrid_prefix)
    , rifd_session_max_(rifd_session_max)
    , rifd_session_timeout_(rifd_session_timeout)
    , rifd_session_registrar_max_(rifd_session_registrar_max)
    , rifd_epp_update_domain_keyset_clear_(rifd_epp_update_domain_keyset_clear) ,

    db_disconnect_guard_(DBDisconnectPtr(0)),
    regMan(),
    epp_sessions(rifd_session_max, rifd_session_registrar_max, rifd_session_timeout),
    ErrorMsg(),
    ReasonMsg(),
    CC(),
    max_zone()
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");

  // objects are shared between threads!!!
  // init at the beginning and do not change

  db_disconnect_guard_ = connect_DB(database , DB_CONNECT_FAILED());

  LOG(NOTICE_LOG, "successfully  connect to DATABASE %s", database.c_str());
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

    epp_sessions.destroy_all_registrar_sessions(reg_id);

}

int ccReg_EPP_i::GetRegistrarID(
  unsigned long long clientID)
{
  return epp_sessions.get_registrar_id(clientID);
}

int ccReg_EPP_i::GetRegistrarLang(
  unsigned long long clientID)
{
  return epp_sessions.get_registrar_lang(clientID);
}

// Load table to memory for speed

int ccReg_EPP_i::LoadReasonMessages()
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");

  DB DBsql;
  int i, rows;

  if (DBsql.OpenDatabase(database) ) {
    rows=0;
    if (DBsql.ExecSelect("SELECT id , reason , reason_cs FROM enum_reason order by id;") ) {
      rows = DBsql.GetSelectRows();
      ReasonMsg = new Mesg();
      for (i = 0; i < rows; i ++)
        ReasonMsg->AddMesg(atoi(DBsql.GetFieldValue(i, 0) ),
            DBsql.GetFieldValue(i, 1) , DBsql.GetFieldValue(i, 2) );
      DBsql.FreeSelect();
    }

    DBsql.Disconnect();
  } else
    return -1;

  return rows;
}

int ccReg_EPP_i::LoadErrorMessages()
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");

  DB DBsql;
  int i, rows;

  if (DBsql.OpenDatabase(database) ) {
    rows=0;
    if (DBsql.ExecSelect("SELECT id , status , status_cs FROM enum_error order by id;") ) {
      rows = DBsql.GetSelectRows();
      ErrorMsg = new Mesg();
      for (i = 0; i < rows; i ++)
        ErrorMsg->AddMesg(atoi(DBsql.GetFieldValue(i, 0) ) ,
            DBsql.GetFieldValue(i, 1) , DBsql.GetFieldValue(i, 2));
      DBsql.FreeSelect();
    }

    DBsql.Disconnect();
  } else
    return -1;

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

// load country code table  enum_country from database
int ccReg_EPP_i::LoadCountryCode()
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  ConnectionReleaser releaser;

  try
  {
      CC.reset(new CountryCode);
      CC->load();
      return CC->GetNum();
  }
  catch(...)
  {
      return -1;
  }
}

bool ccReg_EPP_i::TestCountryCode(
  const char *cc)
{
    LOG( NOTICE_LOG , "CCREG:: TestCountryCode  [%s]" , cc );
    return CC->TestCountryCode(cc);
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

void ccReg_EPP_i::extractEnumDomainExtension(std::string &valexdate, ccReg::Disclose &publish,
        const ccReg::ExtensionList &ext)
{
    const ccReg::ENUMValidationExtension *enum_ext;
    unsigned int len = ext.length();

    for (unsigned int i = 0; i < len; ++i) {
        if (ext[i] >>= enum_ext) {
            /* extract validation exdate */
            valexdate = enum_ext->valExDate;
            /* extract enum publish flag */
            publish = enum_ext->publish;
            LOGGER(PACKAGE).debug(boost::format("valexdate=%1% publish=%2%")
                    % valexdate % publish);
        }
        else {
            LOGGER(PACKAGE).debug(boost::format("unknown extension found when"
                    " extracting domain enum extension (list idx=%1%)") % i);
            break;
        }
    }
}

// parse extension fom domain.ValExDate
void ccReg_EPP_i::GetValExpDateFromExtension(
  char *valexpDate, const ccReg::ExtensionList& ext)
{
  int len, i;
  const ccReg::ENUMValidationExtension * enumVal;

  valexpDate[0] = 0;

  len = ext.length();
  if (len > 0) {
    LOG( DEBUG_LOG, "extension length %d", (int ) ext.length() );
    for (i = 0; i < len; i++) {
      if (ext[i] >>= enumVal) {
        strncpy(valexpDate, enumVal->valExDate, MAX_DATE);
        LOG( DEBUG_LOG, "enumVal %s ", valexpDate );
      } else {
        LOG( ERROR_LOG, "Unknown value extension[%d]", i );
        break;
      }

    }
  }

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
  epp_sessions.logout_session(clientID);
}


/***********************************************************************
 *
 * FUNCTION:	GetTransaction
 *
 * DESCRIPTION: returns for client from entered clTRID generated server
 *              transaction ID
 *
 * PARAMETERS:  clTRID - client transaction number
 *              clientID - client identification
 *              requestId - fred-logd request ID
 *              errCode - save error report from client into table action
 *
 * RETURNED:    svTRID and errCode msg
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::GetTransaction(
  CORBA::Short errCode, CORBA::ULongLong clientID, ccReg::TID requestId, const char* clTRID,
  const ccReg::XmlErrors& errorCodes, ccReg::ErrorStrings_out errStrings)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));
  ConnectionReleaser releaser;

  DB DBsql;
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

  if (DBsql.OpenDatabase(database) ) {
    if (errCode > 0) {
      if (DBsql.BeginAction(clientID, EPP_UnknowAction, clTRID, "", requestId)) {
          // error code
          ret->code = errCode;
          // write to the  action table
          ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
          ret->msg = CORBA::string_dup(GetErrorMessage(ret->code,
              GetRegistrarLang(clientID) ) );

          LOG( NOTICE_LOG, "GetTransaction: svTRID [%s] errCode -> %d msg [%s] ", ( char * ) ret->svTRID, ret->code, ( char * ) ret->msg );
      }
    }

    DBsql.Disconnect();
  }

  if (ret->code == 0)
    ServerInternalError("GetTransaction");

  return ret._retn();

}

/***********************************************************************
 * FUNCTION:    PollAcknowledgement
 *
 * DESCRIPTION: confirmation of message income msgID
 *		returns number of message, which are left count
 *		and next message newmsgID
 *
 * PARAMETERS:  msgID - front message number
 *        OUT:  count -  messages numbers
 *        OUT:  newmsgID - number of new message
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::PollAcknowledgement(
  const char* msgID, CORBA::Short& count, CORBA::String_out newmsgID,
  const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  LOG(
      NOTICE_LOG,
      "PollAcknowledgement: clientID -> %llu clTRID [%s] msgID -> %s",
       params.loginID, static_cast<const char*>(params.clTRID), msgID
  );
  // start EPP action - this will handle all init stuff
  EPPAction a(this, params.loginID, EPP_PollAcknowledgement,static_cast<const char*>(params.clTRID), params.XML, params.requestID);
  try {
    std::auto_ptr<Fred::Poll::Manager> pollMan(
        Fred::Poll::Manager::create(a.getDB())
    );
    pollMan->setMessageSeen(STR_TO_ID(msgID), a.getRegistrar());
    /// convert count of messages and next message id to string
    count = pollMan->getMessageCount(a.getRegistrar());
    if (!count) a.NoMessage();
    std::stringstream buffer;
    buffer << pollMan->getNextMessageId(a.getRegistrar());
    newmsgID = CORBA::string_dup(buffer.str().c_str());
    return a.getRet()._retn();
  }
  catch (Fred::NOT_FOUND) {
    // message id not found
    a.failed(SetErrorReason(
            a.getErrors(),COMMAND_PARAMETR_ERROR,
            ccReg::poll_msgID,1,REASON_MSG_MSGID_NOTEXIST,a.getLang()
        ));
  }
  catch (ccReg::EPP::NoMessages) {throw;}
  catch (...) {a.failedInternal("Connection problems");}
  // previous commands throw exception so this code
  // will never be called
  return NULL;
}

/***********************************************************************
 *
 * FUNCTION:    PollRequest
 *
 * DESCRIPTION: retrieve message msgID from front
 *              return number of messages in front and message content
 *
 * PARAMETERS:
 *        OUT:  msgID - id of required message in front
 *        OUT:  count - number
 *        OUT:  qDate - message date and time
 *        OUT:  type - message type
 *        OUT:  msg  - message content as structure
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::PollRequest(
  CORBA::String_out msgID, CORBA::Short& count, ccReg::timestamp_out qDate,
  ccReg::PollType& type, CORBA::Any_OUT_arg msg, const ccReg::EppParams &params
  )
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  LOG(
      NOTICE_LOG,
      "PollRequest: clientID -> %llu clTRID [%s]", params.loginID,static_cast<const char*>(params.clTRID)
  );
  // start EPP action - this will handle all init stuff
  EPPAction a(this, params.loginID, EPP_PollResponse, static_cast<const char*>(params.clTRID), params.XML, params.requestID);
  std::auto_ptr<Fred::Poll::Message> m;
  try {
    std::auto_ptr<Fred::Poll::Manager> pollMan(
        Fred::Poll::Manager::create(a.getDB())
    );
    // fill count
    count = pollMan->getMessageCount(a.getRegistrar());
    if (!count) a.NoMessage(); // throw exception NoMessage
    m.reset(pollMan->getNextMessage(a.getRegistrar()));
  }
  catch (ccReg::EPP::NoMessages) {throw;}
  catch (...) {a.failedInternal("Connection problems");}
  if (!m.get())
    a.failedInternal("Cannot get message"); // throw internal exception
  a.setCode(COMMAND_ACK_MESG);
  msg = new CORBA::Any;
  // first fill common fields
  // transform numeric id to string and fill msgID
  std::stringstream buffer;
  buffer << m->getId();
  msgID = CORBA::string_dup(buffer.str().c_str());
  // fill qdate
  qDate = CORBA::string_dup(formatTime(m->getCrTime()).c_str());
  // Test type of Message object wheter it's one of
  // MessageEvent, MessageEventReg, MessageTechCheck or MessageLowCredit
  // check MessageEventReg before MessageEvent because
  // MessageEvent is ancestor of MessageEventReg
  Fred::Poll::MessageEventReg *mer =
      dynamic_cast<Fred::Poll::MessageEventReg *>(m.get());
  if (mer) {
    switch (m->getType()) {
      case Fred::Poll::MT_TRANSFER_CONTACT:
        type = ccReg::polltype_transfer_contact;
        break;
      case Fred::Poll::MT_TRANSFER_NSSET:
        type = ccReg::polltype_transfer_nsset;
        break;
      case Fred::Poll::MT_TRANSFER_DOMAIN:
        type = ccReg::polltype_transfer_domain;
        break;
      case Fred::Poll::MT_TRANSFER_KEYSET:
        type = ccReg::polltype_transfer_keyset;
        break;
      default:
        a.failedInternal("Invalid message type"); // thow exception
    }
    ccReg::PollMsg_HandleDateReg *hdm = new ccReg::PollMsg_HandleDateReg;
    hdm->handle = CORBA::string_dup(mer->getObjectHandle().c_str());
    hdm->date = CORBA::string_dup(to_iso_extended_string(mer->getEventDate()).c_str() );
    hdm->clID = CORBA::string_dup(mer->getRegistrarHandle().c_str());
    *msg <<= hdm;
    return a.getRet()._retn();
  }
  Fred::Poll::MessageEvent *me =
      dynamic_cast<Fred::Poll::MessageEvent *>(m.get());
  if (me) {
    switch (m->getType()) {
      case Fred::Poll::MT_DELETE_CONTACT:
        type = ccReg::polltype_delete_contact;
        break;
      case Fred::Poll::MT_DELETE_NSSET:
        type = ccReg::polltype_delete_nsset;
        break;
      case Fred::Poll::MT_DELETE_DOMAIN:
        type = ccReg::polltype_delete_domain;
        break;
      case Fred::Poll::MT_DELETE_KEYSET:
        type = ccReg::polltype_delete_keyset;
        break;
      case Fred::Poll::MT_IMP_EXPIRATION:
        type = ccReg::polltype_impexpiration;
        break;
      case Fred::Poll::MT_EXPIRATION:
        type = ccReg::polltype_expiration;
        break;
      case Fred::Poll::MT_IMP_VALIDATION:
        type = ccReg::polltype_impvalidation;
        break;
      case Fred::Poll::MT_VALIDATION:
        type = ccReg::polltype_validation;
        break;
      case Fred::Poll::MT_OUTZONE:
        type = ccReg::polltype_outzone;
        break;
      default:
        a.failedInternal("Invalid message type"); // thow exception
    }
    ccReg::PollMsg_HandleDate *hdm = new ccReg::PollMsg_HandleDate;
    hdm->handle = CORBA::string_dup(me->getObjectHandle().c_str());
    hdm->date = CORBA::string_dup(to_iso_extended_string(me->getEventDate()).c_str() );
    *msg <<= hdm;
    return a.getRet()._retn();
  }
  Fred::Poll::MessageLowCredit *mlc =
      dynamic_cast<Fred::Poll::MessageLowCredit *>(m.get());
  if (mlc) {
    type = ccReg::polltype_lowcredit;
    ccReg::PollMsg_LowCredit *hdm = new ccReg::PollMsg_LowCredit;
    hdm->zone = CORBA::string_dup(mlc->getZone().c_str());
    hdm->limit = mlc->getLimit();
    hdm->credit = mlc->getCredit();
    *msg <<= hdm;
    return a.getRet()._retn();
  }
  Fred::Poll::MessageTechCheck *mtc =
      dynamic_cast<Fred::Poll::MessageTechCheck *>(m.get());
  if (mtc) {
    type = ccReg::polltype_techcheck;
    ccReg::PollMsg_Techcheck *hdm = new ccReg::PollMsg_Techcheck;
    hdm->handle = CORBA::string_dup(mtc->getHandle().c_str());
    hdm->fqdns.length(mtc->getFQDNS().size());
    for (unsigned i=0; i<mtc->getFQDNS().size(); i++)
      hdm->fqdns[i] = mtc->getFQDNS()[i].c_str();
    hdm->tests.length(mtc->getTests().size());
    for (unsigned i=0; i<mtc->getTests().size(); i++) {
      Fred::Poll::MessageTechCheckItem *test = mtc->getTests()[i];
      hdm->tests[i].testname = CORBA::string_dup(test->getTestname().c_str());
      hdm->tests[i].status = test->getStatus();
      hdm->tests[i].note = CORBA::string_dup(test->getNote().c_str());
    }
    *msg <<= hdm;
    return a.getRet()._retn();
  }
  Fred::Poll::MessageRequestFeeInfo *mrf =
      dynamic_cast<Fred::Poll::MessageRequestFeeInfo*>(m.get());
  if (mrf)
  {
      type = ccReg::polltype_request_fee_info;
      ccReg::PollMsg_RequestFeeInfo *hdm = new ccReg::PollMsg_RequestFeeInfo;
      hdm->periodFrom = CORBA::string_dup(formatTime(mrf->getPeriodFrom()).c_str());
      hdm->periodTo = CORBA::string_dup(formatTime(mrf->getPeriodTo() - boost::posix_time::seconds(1)).c_str());
      hdm->totalFreeCount = mrf->getTotalFreeCount();
      hdm->usedCount = mrf->getUsedCount();
      hdm->price = CORBA::string_dup(mrf->getPrice().c_str());
      *msg <<= hdm;
      LOGGER(PACKAGE).debug("poll message request_fee_info packed");
      return a.getRet()._retn();
  }
  a.failedInternal("Invalid message structure");
  // previous command throw exception in any case so this code
  // will never be called
  return NULL;
}

/***********************************************************************
 *
 * FUNCTION:    ClientCredit
 *
 * DESCRIPTION: information about credit amount of logged registrar
 * PARAMETERS:  params - common EPP parametres
 *        OUT:  credit - credit amount in haler
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

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

/***********************************************************************
 *
 * FUNCTION:    ClientLogout
 *
 * DESCRIPTION: client logout for record into table login
 *              about logout date
 * PARAMETERS:  params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response *
ccReg_EPP_i::ClientLogout(const ccReg::EppParams &params)
{
    Logging::Context::clear();
  Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    LOG( NOTICE_LOG, "ClientLogout: clientID -> %llu clTRID [%s]", params.loginID, static_cast<const char*>(params.clTRID) );
    EPPAction action(this, params.loginID, EPP_ClientLogout, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    epp_sessions.logout_session(params.loginID);
    action.setCode(COMMAND_LOGOUT);

    return action.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    ClientLogin
 *
 * DESCRIPTION: client login acquire of clientID from table login
 *              login through password registrar and its possible change
 *
 * PARAMETERS:  ClID - registrar identifier
 *              passwd - current password
 *              newpasswd - new password for change
 *              clTRID - transaction client number
 *              XML - xml representation of the command
 *        OUT:  clientID - connected client id
 *              requestId - fred-logd request ID associated with login
 *              certID - certificate fingerprint
 *              language - communication language of client en or cs empty value = en
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

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

  DBAutoPtr db_connect(new DB());
  int regID=0;
  int language=0;
  ccReg::Response_var ret;
  ret = new ccReg::Response;

  // default
  ret->code = 0;
  out_clientID = 0;

  LOG( NOTICE_LOG, "ClientLogin: username-> [%s] clTRID [%s] passwd [%s]  newpass [%s] ", ClID, static_cast<const char*>(clTRID), passwd, newpass );
  LOG( NOTICE_LOG, "ClientLogin:  certID  [%s] language  [%d] ", certID, lang );

  if (db_connect->OpenDatabase(database)) {
    DBSharedPtr DBsql = DBDisconnectPtr(db_connect.release());

    std::auto_ptr<Fred::Registrar::Manager> regman(
         Fred::Registrar::Manager::create(DBDisconnectPtr(NULL)));
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
        } else if (DBsql->BeginTransaction() ) {

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
                        out_clientID = epp_sessions.login_session(regID, language);

                        LOGGER(PACKAGE).notice(boost::format("ClientLogin: username %1%, regID %2%, clTRID %3%, lang %4% got clientID %5%")
                                                    % ClID % regID % static_cast<const char*>(clTRID) % language % out_clientID);

                        ret->code = COMMAND_OK;
                    } catch (const NumberSessionLimit &ex) {

                        LOGGER(PACKAGE).error(boost::format("ClientLogin: username %1%, regID %2% clTRID %3%, lang %4% login FAILED, reason %5% ")
                                                    % ClID % regID % static_cast<const char*>(clTRID) % language % ex.what());
                        out_clientID=0; //  not login
                        ret->code =COMMAND_MAX_SESSION_LIMIT; // maximal limit of connection sessions
                    }
                }

            // end of transaction
            DBsql->QuitTransaction(ret->code);
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

/***********************************************************************
 *
 * FUNCTION:    ObjectCheck
 *
 * DESCRIPTION: general control of nsset, domain, object, contact and keyset
 *
 * PARAMETERS:
 *              act - check action type
 *              table - name of table CONTACT NSSET DOMAIN or KEYSET
 *              fname - name of array in database HANDLE or FQDN
 *              chck - string sequence of object type Check
 *              param - common EPP parametres
 *        OUT:  a - (1) object doesn't exist and it is free
 *                  (0) object is already established
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response *
ccReg_EPP_i::ObjectCheck(short act, const char * table, const char *fname,
        const ccReg::Check& chck, ccReg::CheckResp_out a, const ccReg::EppParams &params)
{
    unsigned long i, len;

    Fred::NameIdPair caConflict;
    Fred::Domain::CheckAvailType caType;
    Fred::Contact::Manager::CheckAvailType cType;
    Fred::NSSet::Manager::CheckAvailType nType;
    Fred::KeySet::Manager::CheckAvailType kType;
    short int code = 0;

    a = new ccReg::CheckResp;

    len = chck.length();
    a->length(len);

    EPPAction action(this, params.loginID, act, static_cast<const char*>(params.clTRID), params.XML, params.requestID);



    LOG( NOTICE_LOG , "OBJECT %d  Check: clientID -> %llu clTRID [%s] " , act , params.loginID , static_cast<const char*>(params.clTRID) );

    for (i = 0; i < len; i ++) {
        switch (act) {
            case EPP_ContactCheck:
                try {
                    std::auto_ptr<Fred::Contact::Manager> cman( Fred::Contact::Manager::create(action.getDB(),restricted_handles_) );

                    LOG( NOTICE_LOG , "contact checkAvail handle [%s]" , (const char * ) chck[i] );

                    cType = cman->checkAvail( ( const char * ) chck[i] , caConflict );
                    LOG( NOTICE_LOG , "contact type %d" , cType );
                    switch (cType) {
                        case Fred::Contact::Manager::CA_INVALID_HANDLE:
                            a[i].avail = ccReg::BadFormat;
                            a[i].reason
                                = CORBA::string_dup(GetReasonMessage( REASON_MSG_INVALID_FORMAT , action.getLang()));
                            LOG( NOTICE_LOG , "bad format %s of contact handle" , (const char * ) chck[i] );
                            break;
                        case Fred::Contact::Manager::CA_REGISTRED:
                            a[i].avail = ccReg::Exist;
                            a[i].reason
                                = CORBA::string_dup(GetReasonMessage( REASON_MSG_REGISTRED , action.getLang()) );
                            LOG( NOTICE_LOG , "contact %s exist not Avail" , (const char * ) chck[i] );
                            break;

                        case Fred::Contact::Manager::CA_PROTECTED:
                            a[i].avail = ccReg::DelPeriod;
                            a[i].reason
                                = CORBA::string_dup(GetReasonMessage( REASON_MSG_PROTECTED_PERIOD , action.getLang()) ); // v ochrane lhute
                            LOG( NOTICE_LOG , "contact %s in delete period" ,(const char * ) chck[i] );
                            break;
                        case Fred::Contact::Manager::CA_FREE:
                            a[i].avail = ccReg::NotExist;
                            a[i].reason = CORBA::string_dup(""); // free
                            LOG( NOTICE_LOG , "contact %s not exist  Avail" ,(const char * ) chck[i] );
                            break;
                    }
                }
                catch (...) {
                    LOG( WARNING_LOG, "cannot run Fred::Contact::checkAvail");
                    code=COMMAND_FAILED;
                }
                break;

            case EPP_KeySetCheck:
                try {
                    std::auto_ptr<Fred::KeySet::Manager> kman(
                            Fred::KeySet::Manager::create(
                                action.getDB(), restricted_handles_));
                    LOG(NOTICE_LOG, "keyset checkAvail handle [%s]",
                            (const char *)chck[i]);

                    kType = kman->checkAvail((const char *)chck[i], caConflict);
                    LOG(NOTICE_LOG, "keyset check type %d", kType);
                    switch (kType) {
                        case Fred::KeySet::Manager::CA_INVALID_HANDLE:
                            a[i].avail = ccReg::BadFormat;
                            a[i].reason = CORBA::string_dup(
                                    GetReasonMessage(
                                        REASON_MSG_INVALID_FORMAT,
                                        action.getLang())
                                    );
                            LOG(NOTICE_LOG, "bad format %s of keyset handle",
                                    (const char *)chck[i]);
                            break;
                        case Fred::KeySet::Manager::CA_REGISTRED:
                            a[i].avail = ccReg::Exist;
                            a[i].reason = CORBA::string_dup(
                                    GetReasonMessage(
                                        REASON_MSG_REGISTRED,
                                        action.getLang())
                                    );
                            LOG(NOTICE_LOG, "keyset %s exist not avail",
                                    (const char *)chck[i]);
                            break;
                        case Fred::KeySet::Manager::CA_PROTECTED:
                            a[i].avail = ccReg::DelPeriod;
                            a[i].reason = CORBA::string_dup(
                                    GetReasonMessage(
                                        REASON_MSG_PROTECTED_PERIOD,
                                        action.getLang())
                                    );
                            LOG(NOTICE_LOG, "keyset %s in delete period",
                                    (const char *)chck[i]);
                            break;
                        case Fred::KeySet::Manager::CA_FREE:
                            a[i].avail = ccReg::NotExist;
                            a[i].reason = CORBA::string_dup(""); //free
                            LOG(NOTICE_LOG, "keyset %s not exist Available",
                                    (const char *)chck[i]);
                            break;
                    }
                }
                catch (...) {
                    LOG(WARNING_LOG, "cannot run Fred::Contact::checkAvail");
                    code = COMMAND_FAILED;
                }
                break;

            case EPP_NSsetCheck:

                try {
                    std::auto_ptr<Fred::Zone::Manager> zman( Fred::Zone::Manager::create() );
                    std::auto_ptr<Fred::NSSet::Manager> nman( Fred::NSSet::Manager::create(action.getDB(),zman.get(),restricted_handles_) );

                    LOG( NOTICE_LOG , "nsset checkAvail handle [%s]" , (const char * ) chck[i] );

                    nType = nman->checkAvail( ( const char * ) chck[i] , caConflict );
                    LOG( NOTICE_LOG , "nsset check type %d" , nType );
                    switch (nType) {
                        case Fred::NSSet::Manager::CA_INVALID_HANDLE:
                            a[i].avail = ccReg::BadFormat;
                            a[i].reason
                                = CORBA::string_dup(GetReasonMessage( REASON_MSG_INVALID_FORMAT , action.getLang()));
                            LOG( NOTICE_LOG , "bad format %s of nsset handle" , (const char * ) chck[i] );
                            break;
                        case Fred::NSSet::Manager::CA_REGISTRED:
                            a[i].avail = ccReg::Exist;
                            a[i].reason
                                = CORBA::string_dup(GetReasonMessage( REASON_MSG_REGISTRED , action.getLang()) );
                            LOG( NOTICE_LOG , "nsset %s exist not Avail" , (const char * ) chck[i] );
                            break;

                        case Fred::NSSet::Manager::CA_PROTECTED:
                            a[i].avail = ccReg::DelPeriod;
                            a[i].reason
                                = CORBA::string_dup(GetReasonMessage( REASON_MSG_PROTECTED_PERIOD , action.getLang()) ); // v ochrane lhute
                            LOG( NOTICE_LOG , "nsset %s in delete period" ,(const char * ) chck[i] );
                            break;
                        case Fred::NSSet::Manager::CA_FREE:
                            a[i].avail = ccReg::NotExist;
                            a[i].reason = CORBA::string_dup("");
                            LOG( NOTICE_LOG , "nsset %s not exist  Avail" ,(const char * ) chck[i] );
                            break;
                    }
                }
                catch (...) {
                    LOG( WARNING_LOG, "cannot run Fred::NSSet::checkAvail");
                    code=COMMAND_FAILED;
                }


                break;

            case EPP_DomainCheck:

                try {
                    std::auto_ptr<Fred::Zone::Manager> zm( Fred::Zone::Manager::create() );
                    std::auto_ptr<Fred::Domain::Manager> dman( Fred::Domain::Manager::create(action.getDB(),zm.get()) );

                    LOG( NOTICE_LOG , "domain checkAvail fqdn [%s]" , (const char * ) chck[i] );

                    caType = dman->checkAvail( ( const char * ) chck[i] , caConflict);
                    LOG( NOTICE_LOG , "domain type %d" , caType );
                    switch (caType) {
                        case Fred::Domain::CA_INVALID_HANDLE:
                        case Fred::Domain::CA_BAD_LENGHT:
                            a[i].avail = ccReg::BadFormat;
                            a[i].reason
                                = CORBA::string_dup(GetReasonMessage(REASON_MSG_INVALID_FORMAT , action.getLang()) );
                            LOG( NOTICE_LOG , "bad format %s of fqdn" , (const char * ) chck[i] );
                            break;
                        case Fred::Domain::CA_REGISTRED:
                        case Fred::Domain::CA_CHILD_REGISTRED:
                        case Fred::Domain::CA_PARENT_REGISTRED:
                            a[i].avail = ccReg::Exist;
                            a[i].reason
                                = CORBA::string_dup(GetReasonMessage( REASON_MSG_REGISTRED , action.getLang()) );
                            LOG( NOTICE_LOG , "domain %s exist not Avail" , (const char * ) chck[i] );
                            break;
                        case Fred::Domain::CA_BLACKLIST:
                            a[i].avail = ccReg::BlackList;
                            a[i].reason
                                = CORBA::string_dup(GetReasonMessage( REASON_MSG_BLACKLISTED_DOMAIN , action.getLang()) );
                            LOG( NOTICE_LOG , "blacklisted  %s" , (const char * ) chck[i] );
                            break;
                        case Fred::Domain::CA_AVAILABLE:
                            a[i].avail = ccReg::NotExist;
                            a[i].reason = CORBA::string_dup(""); // free
                            LOG( NOTICE_LOG , "domain %s not exist  Avail" ,(const char * ) chck[i] );
                            break;
                        case Fred::Domain::CA_BAD_ZONE:
                            a[i].avail = ccReg::NotApplicable; // unusable domain isn't in zone
                            a[i].reason
                                = CORBA::string_dup(GetReasonMessage(REASON_MSG_NOT_APPLICABLE_DOMAIN , action.getLang()) );
                            LOG( NOTICE_LOG , "not applicable %s" , (const char * ) chck[i] );
                            break;
                    }

                    /*
#      CA_INVALID_HANDLE, ///< bad formed handle
#      CA_BAD_ZONE, ///< domain outside of registry
#      CA_BAD_LENGHT, ///< domain longer then acceptable
#      CA_PROTECTED, ///< domain temporary protected for registration
#      CA_BLACKLIST, ///< registration blocked in blacklist
#      CA_REGISTRED, ///< domain registred
#      CA_PARENT_REGISTRED, ///< parent already registred
#      CA_CHILD_REGISTRED, ///< child already registred
#      CA_AVAILABLE ///< domain is available
*/
                }
                catch (...) {
                    LOG( WARNING_LOG, "cannot run Fred::Domain::checkAvail");
                    code=COMMAND_FAILED;
                }
                break;

        }
    }

    // command OK
    if (code == 0) {
        code=COMMAND_OK; // OK not errors
    }


    if (code == 0) {
        action.failedInternal("ObjectCheck");
    }

    return action.getRet()._retn();
}

ccReg::Response* ccReg_EPP_i::ContactCheck(
  const ccReg::Check& handle, ccReg::CheckResp_out a, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  return ObjectCheck( EPP_ContactCheck , "CONTACT" , "handle" , handle , a , params);
}

ccReg::Response* ccReg_EPP_i::NSSetCheck(
  const ccReg::Check& handle, ccReg::CheckResp_out a, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  return ObjectCheck( EPP_NSsetCheck , "NSSET" , "handle" , handle , a , params);
}

ccReg::Response* ccReg_EPP_i::DomainCheck(
  const ccReg::Check& fqdn, ccReg::CheckResp_out a, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  return ObjectCheck( EPP_DomainCheck , "DOMAIN" , "fqdn" , fqdn , a , params);
}

ccReg::Response *
ccReg_EPP_i::KeySetCheck(
        const ccReg::Check &handle,
        ccReg::CheckResp_out a,
        const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

    return ObjectCheck(
            EPP_KeySetCheck, "KEYSET", "handle", handle,
            a, params);
}

/***********************************************************************
 *
 * FUNCTION:    ContactInfo
 *
 * DESCRIPTION: returns detailed information about contact
 *              empty value if contact doesn't exist
 * PARAMETERS:  handle - contact identifier
 *        OUT:  c - contact structure detailed description
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::ContactInfo(
  const char* handle, ccReg::Contact_out c, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  LOG(
      NOTICE_LOG ,
      "ContactInfo: clientID -> %llu clTRID [%s] handle [%s] ",
       params.loginID, static_cast<const char*>(params.clTRID), handle
  );
  // start EPP action - this will handle all init stuff
  EPPAction a(this, params.loginID, EPP_ContactInfo, static_cast<const char*>(params.clTRID), params.XML, params.requestID);
  // initialize managers for contact manipulation
  std::auto_ptr<Fred::Contact::Manager>
      cman(Fred::Contact::Manager::create(a.getDB(),
          restricted_handles_) );
  // first check handle for proper format
  if (!cman->checkHandleFormat(handle))
    // failure in handle check, throw exception
    a.failed(SetReasonContactHandle(a.getErrors(), handle, a.getLang()));
  // now load contact by handle
  std::auto_ptr<Fred::Contact::List> clist(cman->createList());
  clist->setHandleFilter(handle);
  try {clist->reload();}
  catch (...) {a.failedInternal("Cannot load contacts");}
  if (clist->getCount() != 1)
    // failer because non existance, throw exception
    a.failed(COMMAND_OBJECT_NOT_EXIST);
  // start filling output contact structure
  Fred::Contact::Contact *con = clist->getContact(0);
  c = new ccReg::Contact;
  // fill common object data
  c->ROID = CORBA::string_dup(con->getROID().c_str());
  c->CrDate = CORBA::string_dup(formatTime(con->getCreateDate()).c_str());
  c->UpDate = CORBA::string_dup(formatTime(con->getUpdateDate()).c_str());
  c->TrDate = CORBA::string_dup(formatTime(con->getTransferDate()).c_str());
  c->ClID = CORBA::string_dup(con->getRegistrarHandle().c_str());
  c->CrID = CORBA::string_dup(con->getCreateRegistrarHandle().c_str());
  c->UpID = CORBA::string_dup(con->getUpdateRegistrarHandle().c_str());
  // authinfo is filled only if session registar is ownering registrar
  c->AuthInfoPw = CORBA::string_dup(
   a.getRegistrar() == (int)con->getRegistrarId()?con->getAuthPw().c_str():""
  );
  // states
  for (unsigned i=0; i<con->getStatusCount(); i++) {
    Fred::TID stateId = con->getStatusByIdx(i)->getStatusId();
    const Fred::StatusDesc* sd = regMan->getStatusDesc(stateId);
    if (!sd || !sd->getExternal())
      continue;
    c->stat.length(c->stat.length()+1);
    c->stat[c->stat.length()-1].value = CORBA::string_dup(sd->getName().c_str() );
    c->stat[c->stat.length()-1].text = CORBA::string_dup(sd->getDesc(
        a.getLang() == LANG_CS ? "CS" : "EN"
    ).c_str());
  }
  if (!c->stat.length()) {
    const Fred::StatusDesc* sd = regMan->getStatusDesc(0);
    if (sd) {
      c->stat.length(1);
      c->stat[0].value = CORBA::string_dup(sd->getName().c_str());
      c->stat[0].text = CORBA::string_dup(sd->getDesc(
          a.getLang() == LANG_CS ? "CS" : "EN"
      ).c_str());
    }
  }
  // fill contact specific data
  c->handle = CORBA::string_dup(con->getHandle().c_str());
  c->Name = CORBA::string_dup(con->getName().c_str());
  c->Organization = CORBA::string_dup(con->getOrganization().c_str());
  unsigned num = !con->getStreet3().empty() ? 3 : !con->getStreet2().empty() ? 2 : !con->getStreet1().empty() ? 1 : 0;
  c->Streets.length(num);
  if (num > 0)
    c->Streets[0] = CORBA::string_dup(con->getStreet1().c_str());
  if (num > 1)
    c->Streets[1] = CORBA::string_dup(con->getStreet2().c_str());
  if (num > 2)
    c->Streets[2] = CORBA::string_dup(con->getStreet3().c_str());
  c->City = CORBA::string_dup(con->getCity().c_str());
  c->StateOrProvince = CORBA::string_dup(con->getProvince().c_str());
  c->PostalCode = CORBA::string_dup(con->getPostalCode().c_str());
  c->Telephone = CORBA::string_dup(con->getTelephone().c_str());
  c->Fax = CORBA::string_dup(con->getFax().c_str());
  c->Email = CORBA::string_dup(con->getEmail().c_str());
  c->NotifyEmail = CORBA::string_dup(con->getNotifyEmail().c_str());
  c->CountryCode = CORBA::string_dup(con->getCountry().c_str());
  c->VAT = CORBA::string_dup(con->getVAT().c_str());
  c->ident = CORBA::string_dup(con->getSSN().c_str());
  switch (con->getSSNTypeId()) {
    case 1:
      c->identtype = ccReg::EMPTY;
      break;
    case 2:
      c->identtype = ccReg::OP;
      break;
    case 3:
      c->identtype = ccReg::PASS;
      break;
    case 4:
      c->identtype = ccReg::ICO;
      break;
    case 5:
      c->identtype = ccReg::MPSV;
      break;
    case 6:
      c->identtype = ccReg::BIRTHDAY;
      break;
    default:
      c->identtype = ccReg::EMPTY;
      break;
  }
  // DiscloseFlag by the default policy of the server
  if (DefaultPolicy())
    c->DiscloseFlag = ccReg::DISCL_HIDE;
  else
    c->DiscloseFlag = ccReg::DISCL_DISPLAY;
  // set disclose flags according to default policy
  c->DiscloseName = get_DISCLOSE(con->getDiscloseName());
  c->DiscloseOrganization = get_DISCLOSE(con->getDiscloseOrganization());
  c->DiscloseAddress = get_DISCLOSE(con->getDiscloseAddr());
  c->DiscloseTelephone = get_DISCLOSE(con->getDiscloseTelephone());
  c->DiscloseFax = get_DISCLOSE(con->getDiscloseFax());
  c->DiscloseEmail = get_DISCLOSE(con->getDiscloseEmail());
  c->DiscloseVAT = get_DISCLOSE(con->getDiscloseVat());
  c->DiscloseIdent = get_DISCLOSE(con->getDiscloseIdent());
  c->DiscloseNotifyEmail = get_DISCLOSE(con->getDiscloseNotifyEmail());
  // if not set return flag empty
  if (!c->DiscloseName && !c->DiscloseOrganization && !c->DiscloseAddress
      && !c->DiscloseTelephone && !c->DiscloseFax && !c->DiscloseEmail
      && !c->DiscloseVAT && !c->DiscloseIdent && !c->DiscloseNotifyEmail)
    c->DiscloseFlag = ccReg::DISCL_EMPTY;
  return a.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    ContactDelete
 *
 * DESCRIPTION: delete contact from tabel Contact and save them into history
 *              returns contact wasn't find or contact has yet links
 *              to other tables and cannot be deleted
 *              contact can be DELETED only registrar, who created contact
 * PARAMETERS:  handle - contact identifier
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::ContactDelete(
  const char* handle, const ccReg::EppParams &params)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    std::auto_ptr<EPPNotifier> ntf;
    int id;
    short int code = 0;

    EPPAction action(this, params.loginID, EPP_ContactDelete, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    LOGGER(PACKAGE).notice(boost::format("ContactDelete: clientID -> %1% clTRID [%2%] handle [%3%] ") % (int ) params.loginID % (const char*)params.clTRID % handle );

    id = getIdOfContact(action.getDB(), handle, restricted_handles_
            , lock_epp_commands_, true);

    if (id < 0) {
        LOG(WARNING_LOG, "bad format of contact [%s]", handle);
        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                ccReg::contact_handle, 1,
                REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
    } else if (id ==0) {
        LOG( WARNING_LOG, "contact handle [%s] NOT_EXIST", handle );
        code= COMMAND_OBJECT_NOT_EXIST;
    }
    if (!code && !action.getDB()->TestObjectClientID(id, action.getRegistrar()) ) //  if registrar is not client of the object
    {
        LOG( WARNING_LOG, "bad autorization not  creator of handle [%s]", handle );
        code = action.setErrorReason(COMMAND_AUTOR_ERROR,
                ccReg::registrar_autor, 0,
                REASON_MSG_REGISTRAR_AUTOR);
    }
    try {
        if (!code && (
                    testObjectHasState(action,id,FLAG_serverDeleteProhibited) ||
                    testObjectHasState(action,id,FLAG_serverUpdateProhibited) ||
                    testObjectHasState(action,id,FLAG_deleteCandidate)
        ))
        {
            LOG( WARNING_LOG, "delete of object %s is prohibited" , handle );
            code = COMMAND_STATUS_PROHIBITS_OPERATION;
        }
    } catch (...) {
        code = COMMAND_FAILED;
    }
    if (!code) {
        ntf.reset(new EPPNotifier(disable_epp_notifier_,mm , action.getDB(), action.getRegistrar() , id )); // notifier maneger before delete
        ntf->constructMessages(); // need to run all sql queries before delete take place (Ticket #1622)

        // test to  table  domain domain_contact_map and nsset_contact_map for relations
        if (action.getDB()->TestContactRelations(id) ) // can not be deleted
        {
            LOG( WARNING_LOG, "test contact handle [%s] relations: PROHIBITS_OPERATION", handle );
            code = COMMAND_PROHIBITS_OPERATION;
        } else {
            if (action.getDB()->SaveObjectDelete(id) ) // save to delete object object_registry.ErDate
            {
                if (action.getDB()->DeleteContactObject(id) )
                    code = COMMAND_OK; // if deleted successfully
            }

        }

        if (code == COMMAND_OK)
            action.setNotifier(ntf.get());

    }

    // EPP exception
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("ContactDelete");
    }

    return action.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    ContactUpdate
 *
 * DESCRIPTION: change of contact information and save into history
 *		contact can be CHANGED only by registrar
 *		who created contact or those, who has by contact some domain
 * PARAMETERS:  handle - contact identifier
 *              c      - ContactChange  changed information about contact
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::ContactUpdate(
  const char *handle, const ccReg::ContactChange & c, 
  const ccReg::EppParams &params)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    std::auto_ptr<EPPNotifier> ntf;
    int id;
    int s, snum;
    char streetStr[10];
    short int code = 0;

    EPPAction action(this, params.loginID, EPP_ContactUpdate, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    LOGGER(PACKAGE).notice(boost::format("ContactUpdate: clientID -> %1% clTRID [%2%] handle [%3%] ") % (int ) params.loginID % (const char*)params.clTRID % handle );
    LOGGER(PACKAGE).notice(boost::format("Discloseflag %1%: Disclose Name %2% Org %3% Add %4% Tel %5% Fax %6% Email %7% VAT %8% Ident %9% NotifyEmail %10%") % c.DiscloseFlag % c.DiscloseName % c.DiscloseOrganization % c.DiscloseAddress % c.DiscloseTelephone % c.DiscloseFax % c.DiscloseEmail % c.DiscloseVAT % c.DiscloseIdent % c.DiscloseNotifyEmail );
            

    id = getIdOfContact(action.getDB(), handle, restricted_handles_
            , lock_epp_commands_, true);
    // for notification to old notify address, this address must be
    // discovered before change happen
    std::string oldNotifyEmail;
    if (strlen(c.NotifyEmail) && !disable_epp_notifier_)
        oldNotifyEmail = action.getDB()->GetValueFromTable(
                "contact", "notifyemail", "id", id
                );
    if (id < 0) {
        LOG(WARNING_LOG, "bad format of contact [%s]", handle);
        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                ccReg::contact_handle, 1,
                REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
    } else if (id ==0) {
        LOG( WARNING_LOG, "contact handle [%s] NOT_EXIST", handle );
        code= COMMAND_OBJECT_NOT_EXIST;
    }
    if (!code && !action.getDB()->TestObjectClientID(id, action.getRegistrar()) ) {
        LOG( WARNING_LOG, "bad autorization not  client of contact [%s]", handle );
        code = action.setErrorReason(COMMAND_AUTOR_ERROR,
                ccReg::registrar_autor, 0, REASON_MSG_REGISTRAR_AUTOR);
    }
    try {
        if (!code && (testObjectHasState(action,id,FLAG_serverUpdateProhibited) ||
            testObjectHasState(action,id,FLAG_deleteCandidate)))
        {
            LOG( WARNING_LOG, "update of object %s is prohibited" , handle );
            code = COMMAND_STATUS_PROHIBITS_OPERATION;
        }
    } catch (...) {
        code = COMMAND_FAILED;
    }
    if (!code) {
        if ( !TestCountryCode(c.CC) ) {
            LOG(WARNING_LOG, "Reason: unknown country code: %s", (const char *)c.CC);
            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                    ccReg::contact_cc, 1,
                    REASON_MSG_COUNTRY_NOTEXIST);
        } else if (action.getDB()->ObjectUpdate(id, action.getRegistrar(), c.AuthInfoPw) ) // update OBJECT table
        {

            // begin update
            action.getDB()->UPDATE("Contact");

            action.getDB()->SET("Name", c.Name);
            action.getDB()->SET("Organization", c.Organization);
            // whole adrress must be updated at once
            // it's not allowed to update only part of it
            if (strlen(c.City) || strlen(c.CC)) {
                snum = c.Streets.length();

                for (s = 0; s < 3; s ++) {
                    snprintf(streetStr, sizeof(streetStr), "Street%d", s +1);
                    if (s < snum)
                        action.getDB()->SET(streetStr, c.Streets[s]);
                    else
                        action.getDB()->SETNULL(streetStr);
                }

                action.getDB()->SET("City", c.City);
                if (strlen(c.StateOrProvince))
                    action.getDB()->SET("StateOrProvince", c.StateOrProvince);
                else
                    action.getDB()->SETNULL("StateOrProvince");
                if (strlen(c.PostalCode))
                    action.getDB()->SET("PostalCode", c.PostalCode);
                else
                    action.getDB()->SETNULL("PostalCode");
                action.getDB()->SET("Country", c.CC);
            }
            action.getDB()->SET("Telephone", c.Telephone);
            action.getDB()->SET("Fax", c.Fax);
            action.getDB()->SET("Email", c.Email);
            action.getDB()->SET("NotifyEmail", c.NotifyEmail);
            action.getDB()->SET("VAT", c.VAT);
            action.getDB()->SET("SSN", c.ident);
            if (c.identtype != ccReg::EMPTY) {
                int identtype = 0;
                switch (c.identtype) {
                    case ccReg::OP:
                        identtype = 2;
                        break;
                    case ccReg::PASS:
                        identtype = 3;
                        break;
                    case ccReg::ICO:
                        identtype = 4;
                        break;
                    case ccReg::MPSV:
                        identtype = 5;
                        break;
                    case ccReg::BIRTHDAY:
                        identtype = 6;
                        break;
                    case ccReg::EMPTY:
                        // just to keep compiler satisfied :)
                        break;
                }
                action.getDB()->SET("SSNtype", identtype); // type ssn
            }

            //  Disclose parameters flags translate
            action.getDB()->SETBOOL("DiscloseName", update_DISCLOSE(c.DiscloseName,
                        c.DiscloseFlag) );
            action.getDB()->SETBOOL("DiscloseOrganization", update_DISCLOSE(
                        c.DiscloseOrganization, c.DiscloseFlag) );
            action.getDB()->SETBOOL("DiscloseAddress", update_DISCLOSE(
                        c.DiscloseAddress, c.DiscloseFlag) );
            action.getDB()->SETBOOL("DiscloseTelephone", update_DISCLOSE(
                        c.DiscloseTelephone, c.DiscloseFlag) );
            action.getDB()->SETBOOL("DiscloseFax", update_DISCLOSE(c.DiscloseFax,
                        c.DiscloseFlag) );
            action.getDB()->SETBOOL("DiscloseEmail", update_DISCLOSE(c.DiscloseEmail,
                        c.DiscloseFlag) );
            action.getDB()->SETBOOL("DiscloseVAT", update_DISCLOSE(c.DiscloseVAT,
                        c.DiscloseFlag) );
            action.getDB()->SETBOOL("DiscloseIdent", update_DISCLOSE(c.DiscloseIdent,
                        c.DiscloseFlag) );
            action.getDB()->SETBOOL("DiscloseNotifyEmail", update_DISCLOSE(
                        c.DiscloseNotifyEmail, c.DiscloseFlag) );

            // the end of UPDATE SQL
            action.getDB()->WHEREID(id);

            // make update and save to history
            if (action.getDB()->EXEC() )
                if (action.getDB()->SaveContactHistory(id, params.requestID) )
                    code = COMMAND_OK;

        }
    }
    if (code == COMMAND_OK) // run notifier
    {
        ntf.reset(new EPPNotifier(
                      disable_epp_notifier_,
                      mm, action.getDB(), action.getRegistrar(), id, regMan.get()));
        ntf->addExtraEmails(oldNotifyEmail);
        action.setNotifier(ntf.get()); // schedule message send
    }

    // EPP exception
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("ContactUpdate");
    }

    return action.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    ContactCreate
 *
 * DESCRIPTION: creation of contact
 *
 * PARAMETERS:  handle - identifier of contact
 *              c      - ContactChange information about contact
 *        OUT:  crDate - object creation date
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::ContactCreate(
  const char *handle, const ccReg::ContactChange & c,
  ccReg::timestamp_out crDate, const ccReg::EppParams & params)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    std::auto_ptr<EPPNotifier> ntf;

    int id;
    int s, snum;
    char streetStr[10];
    short int code = 0;

    crDate = CORBA::string_dup("");


    EPPAction action(this, params.loginID, EPP_ContactCreate, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    LOGGER(PACKAGE).notice(boost::format("ContactCreate: clientID -> %1% clTRID [%2%] handle [%3%]") % (int ) params.loginID % (const char*)params.clTRID % handle );

    LOGGER(PACKAGE).notice(boost::format("Discloseflag %1%: Disclose Name %2% Org %3% Add %4% Tel %5% Fax %6% Email %7% VAT %8% Ident %9% NotifyEmail %10%") % c.DiscloseFlag %
            c.DiscloseName % c.DiscloseOrganization % c.DiscloseAddress % c.DiscloseTelephone % c.DiscloseFax % c.DiscloseEmail % c.DiscloseVAT % c.DiscloseIdent % c.DiscloseNotifyEmail);

    Fred::Contact::Manager::CheckAvailType caType;
    try {
        std::auto_ptr<Fred::Contact::Manager> cman(
                Fred::Contact::Manager::create(action.getDB(),restricted_handles_)
                );
        Fred::NameIdPair nameId;
        caType = cman->checkAvail(handle,nameId);
        id = nameId.id;
    }
    catch (...) {
        id = -1;
        caType = Fred::Contact::Manager::CA_INVALID_HANDLE;
    }
    if (id<0 || caType == Fred::Contact::Manager::CA_INVALID_HANDLE) {
        LOG(WARNING_LOG, "bad format of contact [%s]", handle);
        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                ccReg::contact_handle, 1, REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
    } else if (caType == Fred::Contact::Manager::CA_REGISTRED) {
        LOG( WARNING_LOG, "contact handle [%s] EXIST", handle );
        code= COMMAND_OBJECT_EXIST;
    } else if (caType == Fred::Contact::Manager::CA_PROTECTED) {
        LOG(WARNING_LOG, "object [%s] in history period", handle);
        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                ccReg::contact_handle, 1, REASON_MSG_PROTECTED_PERIOD);
    }
    // test  if country code  is valid
    if ( !TestCountryCode(c.CC) ) {
        LOG(WARNING_LOG, "Reason: unknown country code: %s", (const char *)c.CC);
        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                ccReg::contact_cc, 1, REASON_MSG_COUNTRY_NOTEXIST);
    } else if (code == 0) {
        // create object generate ROID
        id= action.getDB()->CreateObject("C", action.getRegistrar(), handle, c.AuthInfoPw);
        if (id<=0) {
            if (id == 0) {
                LOG( WARNING_LOG, "contact handle [%s] EXIST", handle );
                code= COMMAND_OBJECT_EXIST;
            } else {
                LOG( WARNING_LOG, "Cannot insert [%s] into object_registry", handle );
                code= COMMAND_FAILED;
            }

        } else {

            action.getDB()->INSERT("CONTACT");
            action.getDB()->INTO("id");

            action.getDB()->INTOVAL("Name", c.Name);
            action.getDB()->INTOVAL("Organization", c.Organization);

            // insert streets
            snum = c.Streets.length();
            for (s = 0; s < snum; s ++) {
                snprintf(streetStr, sizeof(streetStr), "Street%d", s +1);
                action.getDB()->INTOVAL(streetStr, c.Streets[s]);
            }

            action.getDB()->INTOVAL("City", c.City);
            action.getDB()->INTOVAL("StateOrProvince", c.StateOrProvince);
            action.getDB()->INTOVAL("PostalCode", c.PostalCode);
            action.getDB()->INTOVAL("Country", c.CC);
            action.getDB()->INTOVAL("Telephone", c.Telephone);
            action.getDB()->INTOVAL("Fax", c.Fax);
            action.getDB()->INTOVAL("Email", c.Email);
            action.getDB()->INTOVAL("NotifyEmail", c.NotifyEmail);
            action.getDB()->INTOVAL("VAT", c.VAT);
            action.getDB()->INTOVAL("SSN", c.ident);
            if (c.identtype != ccReg::EMPTY)
                action.getDB()->INTO("SSNtype");

            // disclose are write true or false
            action.getDB()->INTO("DiscloseName");
            action.getDB()->INTO("DiscloseOrganization");
            action.getDB()->INTO("DiscloseAddress");
            action.getDB()->INTO("DiscloseTelephone");
            action.getDB()->INTO("DiscloseFax");
            action.getDB()->INTO("DiscloseEmail");
            action.getDB()->INTO("DiscloseVAT");
            action.getDB()->INTO("DiscloseIdent");
            action.getDB()->INTO("DiscloseNotifyEmail");

            action.getDB()->VALUE(id);

            action.getDB()->VAL(c.Name);
            action.getDB()->VAL(c.Organization);
            snum = c.Streets.length();
            for (s = 0; s < snum; s ++) {
                snprintf(streetStr, sizeof(streetStr), "Street%d", s +1);
                action.getDB()->VAL(c.Streets[s]);
            }

            action.getDB()->VAL(c.City);
            action.getDB()->VAL(c.StateOrProvince);
            action.getDB()->VAL(c.PostalCode);
            action.getDB()->VAL(c.CC);
            action.getDB()->VAL(c.Telephone);
            action.getDB()->VAL(c.Fax);
            action.getDB()->VAL(c.Email);
            action.getDB()->VAL(c.NotifyEmail);
            action.getDB()->VAL(c.VAT);
            action.getDB()->VAL(c.ident);
            if (c.identtype != ccReg::EMPTY) {
                int identtype = 0;
                switch (c.identtype) {
                    case ccReg::OP:
                        identtype = 2;
                        break;
                    case ccReg::PASS:
                        identtype = 3;
                        break;
                    case ccReg::ICO:
                        identtype = 4;
                        break;
                    case ccReg::MPSV:
                        identtype = 5;
                        break;
                    case ccReg::BIRTHDAY:
                        identtype = 6;
                        break;
                    case ccReg::EMPTY:
                        // just to keep compiler satisfied
                        break;
                }
                action.getDB()->VALUE(identtype);
            }

            // insert DiscloseFlag by a  DefaultPolicy of server
            action.getDB()->VALUE(setvalue_DISCLOSE(c.DiscloseName, c.DiscloseFlag) );
            action.getDB()->VALUE(setvalue_DISCLOSE(c.DiscloseOrganization,
                        c.DiscloseFlag) );
            action.getDB()->VALUE(setvalue_DISCLOSE(c.DiscloseAddress, c.DiscloseFlag) );
            action.getDB()->VALUE(setvalue_DISCLOSE(c.DiscloseTelephone, c.DiscloseFlag) );
            action.getDB()->VALUE(setvalue_DISCLOSE(c.DiscloseFax, c.DiscloseFlag) );
            action.getDB()->VALUE(setvalue_DISCLOSE(c.DiscloseEmail, c.DiscloseFlag) );
            action.getDB()->VALUE(setvalue_DISCLOSE(c.DiscloseVAT, c.DiscloseFlag) );
            action.getDB()->VALUE(setvalue_DISCLOSE(c.DiscloseIdent, c.DiscloseFlag) );
            action.getDB()->VALUE(setvalue_DISCLOSE(c.DiscloseNotifyEmail,
                        c.DiscloseFlag) );

            // if is inserted
            if (action.getDB()->EXEC() ) {
                // get local timestamp of created  object
                CORBA::string_free(crDate);
                crDate= CORBA::string_dup(action.getDB()->GetObjectCrDateTime(id) );
                if (action.getDB()->SaveContactHistory(id, params.requestID)) // save history
                    if (action.getDB()->SaveObjectCreate(id) )
                        code = COMMAND_OK; // if saved
            }
        }
    }

    if (code == COMMAND_OK) // run notifier
    {
        ntf.reset(new EPPNotifier(
                    disable_epp_notifier_, mm ,
                    action.getDB(), action.getRegistrar() , id ));
        action.setNotifier(ntf.get());
    }


    // EPP exception
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("ContactCreate");
    }

    return action.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    ObjectTransfer
 *
 * DESCRIPTION: contact transfer from original into new registrar
 *              and saving change into history
 * PARAMETERS:  handle - contact identifier
 *              authInfo - password  authentication
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::ObjectTransfer(
  short act, const char*table, const char*fname, const char* name,
  const char* authInfo, 
  const ccReg::EppParams &params)
{
    std::auto_ptr<EPPNotifier> ntf;
    char pass[PASS_LEN+1];
    int oldregID;
    int type = 0;
    int id = 0;
    short int code = 0;

    EPPAction action(this, params.loginID, (int)act, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    LOGGER(PACKAGE).notice(boost::format("ObjectContact: act %1%  clientID -> %2% clTRID [%3%] object [%4%] authInfo [%5%] ") % act % (int ) params.loginID % (const char*)params.clTRID % name % authInfo );

    int zone = 0; // for domain zone check
    switch (act) {
        case EPP_ContactTransfer:
            if ( (id = getIdOfContact(action.getDB(), name, restricted_handles_
                    , lock_epp_commands_, true)) < 0) {
                LOG(WARNING_LOG, "bad format of contact [%s]", name);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::contact_handle, 1, REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
            } else if (id == 0) {
                code=COMMAND_OBJECT_NOT_EXIST;
            }
            break;

        case EPP_NSsetTransfer:
            if ( (id = getIdOfNSSet(action.getDB(), name, restricted_handles_
                    , lock_epp_commands_, true) ) < 0) {
                LOG(WARNING_LOG, "bad format of nsset [%s]", name);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::nsset_handle, 1, REASON_MSG_BAD_FORMAT_NSSET_HANDLE);
            } else if (id == 0) {
                code=COMMAND_OBJECT_NOT_EXIST;
            }
            break;

        case EPP_KeySetTransfer:
            if ((id = getIdOfKeySet(action.getDB(), name, restricted_handles_
                    , lock_epp_commands_, true)) < 0) {
                LOG(WARNING_LOG, "bad format of keyset [%s]", name);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_handle, 1, REASON_MSG_BAD_FORMAT_KEYSET_HANDLE);
            } else if (id == 0) {
                code = COMMAND_OBJECT_NOT_EXIST;
            }
            break;

        case EPP_DomainTransfer:
            if ( (id = getIdOfDomain(action.getDB(), name, lock_epp_commands_
                    , true, &zone) ) <= 0) {
                code=COMMAND_OBJECT_NOT_EXIST;
            }
            if (action.getDB()->TestRegistrarZone(action.getRegistrar(), zone) == false) {
                LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
                code = COMMAND_AUTHENTICATION_ERROR;
            }
            break;
        default:
            code = COMMAND_PARAMETR_ERROR;
            break;
    }

    if (!code) {
        // transfer can not be run by existing client
        if (action.getDB()->TestObjectClientID(id, action.getRegistrar())
                && !action.getDB()->GetRegistrarSystem(action.getRegistrar()) ) {
            LOG( WARNING_LOG, "client can not transfer  object %s" , name
               );
            code = COMMAND_NOT_ELIGIBLE_FOR_TRANSFER;
        }

        try {
            if (!code && (testObjectHasState(action,id,FLAG_serverTransferProhibited) ||
                    testObjectHasState(action,id,FLAG_deleteCandidate))) {
                LOG( WARNING_LOG, "transfer of object %s is prohibited" , name );
                code = COMMAND_STATUS_PROHIBITS_OPERATION;
            }
        } catch (...) {
            code = COMMAND_FAILED;
        }
        if (!code) {

            // if  authInfo is ok
            std::stringstream sql;
            switch (act) {
                case EPP_ContactTransfer:
                    sql << "SELECT authinfopw FROM object c " << "WHERE c.id="
                        << id;
                    break;
                case EPP_NSsetTransfer:
                    sql << "SELECT n.authinfopw FROM object n " << "WHERE n.id="
                        << id << " UNION "
                        << "SELECT c.authinfopw FROM nsset_contact_map ncm, object c "
                        << "WHERE ncm.nssetid=" << id
                        << " AND ncm.contactid=c.id ";
                    break;
                case EPP_DomainTransfer:
                    sql << "SELECT d.authinfopw FROM object d " << "WHERE d.id="
                        << id << " UNION "
                        << "SELECT r.authinfopw FROM domain d, object r "
                        << "WHERE d.registrant=r.id AND d.id=" << id << " UNION "
                        << "SELECT c.authinfopw FROM domain_contact_map dcm, object c "
                        << "WHERE dcm.domainid=" << id
                        << " AND dcm.contactid=c.id " << "AND dcm.role=1";
                    break;
                case EPP_KeySetTransfer:
                    sql << "SELECT k.authinfopw FROM object k WHERE k.id="
                        << id
                        << " UNION SELECT c.authinfopw FROM keyset_contact_map kcm, object c"
                        << " WHERE kcm.keysetid="
                        << id
                        << " AND kcm.contactid=c.id ";
                    break;
            }
            code = COMMAND_AUTOR_ERROR;
            if (!action.getDB()->ExecSelect(sql.str().c_str())) {
                LOG( WARNING_LOG , "autorization failed - sql error");
            } else {
                for (unsigned i=0; i < (unsigned)action.getDB()->GetSelectRows(); i++) {
                    if (!strcmp(action.getDB()->GetFieldValue(i, 0), (char *)authInfo)) {
                        code = 0;
                        break;
                    }
                }
                action.getDB()->FreeSelect();
            }
            if (code) {
                LOG( WARNING_LOG , "autorization failed");
            } else {

                //  get ID of old registrant
                // oldaction.getRegistrar()
                oldregID
                    = action.getDB()->GetNumericFromTable("object", "clid", "id", id);

                // after transfer generete new  authinfo
                random_pass(pass);

                // change registrant
                action.getDB()->UPDATE("OBJECT");
                action.getDB()->SSET("TrDate", "now"); // tr timestamp now
                action.getDB()->SSET("AuthInfoPw", pass);
                action.getDB()->SET("ClID", action.getRegistrar());
                action.getDB()->WHEREID(id);

                // IF ok save to history
                if (action.getDB()->EXEC() ) {
                    switch (act) {
                        case EPP_ContactTransfer:
                            type=1;
                            if (action.getDB()->SaveContactHistory(id, params.requestID))
                                code = COMMAND_OK;
                            break;
                        case EPP_NSsetTransfer:
                            type=2;
                            if (action.getDB()->SaveNSSetHistory(id, params.requestID))
                                code = COMMAND_OK;
                            break;
                        case EPP_DomainTransfer:
                            type =3;
                            if (action.getDB()->SaveDomainHistory(id, params.requestID))
                                code = COMMAND_OK;
                            break;
                        case EPP_KeySetTransfer:
                            type = 4;
                            if (action.getDB()->SaveKeySetHistory(id, params.requestID))
                                code = COMMAND_OK;
                            break;
                    }

                    if (code == COMMAND_OK) {
                        try {
                            std::auto_ptr<Fred::Poll::Manager> pollMan(
                                    Fred::Poll::Manager::create(action.getDB())
                                    );
                            pollMan->createActionMessage(
                                    // oldaction.getRegistrar(),
                                    oldregID,
                                    type == 1 ? Fred::Poll::MT_TRANSFER_CONTACT :
                                    type == 2 ? Fred::Poll::MT_TRANSFER_NSSET :
                                    type == 3 ? Fred::Poll::MT_TRANSFER_DOMAIN :
                                    Fred::Poll::MT_TRANSFER_KEYSET,
                                    id
                                    );
                        } catch (...) {code = COMMAND_FAILED;}
                    }
                }
            }
        }

        if (code == COMMAND_OK) // run notifier
        {
            ntf.reset(new EPPNotifier(disable_epp_notifier_,
                        mm , action.getDB(), action.getRegistrar() , id ));
            action.setNotifier(ntf.get());
        }

    }
    // EPP exception
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("ObjectTransfer");
    }

    return action.getRet()._retn();
}

ccReg::Response* ccReg_EPP_i::ContactTransfer(
  const char* handle, const char* authInfo, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  return ObjectTransfer( EPP_ContactTransfer , "CONTACT" , "handle" , handle, authInfo, params);
}

ccReg::Response* ccReg_EPP_i::NSSetTransfer(
  const char* handle, const char* authInfo, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  return ObjectTransfer( EPP_NSsetTransfer , "NSSET" , "handle" , handle, authInfo, params);
}

ccReg::Response* ccReg_EPP_i::DomainTransfer(
  const char* fqdn, const char* authInfo, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  return ObjectTransfer( EPP_DomainTransfer , "DOMAIN" , "fqdn" , fqdn, authInfo, params);
}

ccReg::Response *
ccReg_EPP_i::KeySetTransfer(
        const char *handle,
        const char *authInfo,
        const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

    return ObjectTransfer(EPP_KeySetTransfer, "KEYSET", "handle", handle,
            authInfo, params);
}

/***********************************************************************
 *
 * FUNCTION:    NSSetInfo
 *
 * DESCRIPTION: returns detailed information about nsset and
 *              subservient DNS hosts
 *              empty value if contact doesn't exist
 *
 * PARAMETERS:  handle - identifier of contact
 *        OUT:  n - structure of NSSet detailed description
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::NSSetInfo(
  const char* handle, ccReg::NSSet_out n,
  const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  LOG(
      NOTICE_LOG,
      "NSSetInfo: clientID -> %llu clTRID [%s] handle [%s] ",
       params.loginID, static_cast<const char*>(params.clTRID), handle
  );
  // start EPP action - this will handle all init stuff
  EPPAction a(this, params.loginID, EPP_NSsetInfo, static_cast<const char*>(params.clTRID), params.XML, params.requestID);
  // initialize managers for nsset manipulation
  std::auto_ptr<Fred::Zone::Manager>
      zman(Fred::Zone::Manager::create() );
  std::auto_ptr<Fred::NSSet::Manager>
      nman(Fred::NSSet::Manager::create(a.getDB(), zman.get(),
          restricted_handles_ ) );
  // first check handle for proper format
  if (!nman->checkHandleFormat(handle))
    // failure in handle check, throw exception
    a.failed(SetReasonNSSetHandle(a.getErrors(), handle, a.getLang()));
  // now load nsset by handle
  std::auto_ptr<Fred::NSSet::List> nlist(nman->createList());
  nlist->setHandleFilter(handle);
  try {nlist->reload();}
  catch (...) {a.failedInternal("Cannot load nsset");}
  if (nlist->getCount() != 1)
    // failer because non existance, throw exception
    a.failed(COMMAND_OBJECT_NOT_EXIST);
  // start filling output nsset structure
  Fred::NSSet::NSSet *nss = nlist->getNSSet(0);
  n = new ccReg::NSSet;
  // fill common object data
  n->ROID = CORBA::string_dup(nss->getROID().c_str());
  n->CrDate = CORBA::string_dup(formatTime(nss->getCreateDate()).c_str());
  n->UpDate = CORBA::string_dup(formatTime(nss->getUpdateDate()).c_str());
  n->TrDate = CORBA::string_dup(formatTime(nss->getTransferDate()).c_str());
  n->ClID = CORBA::string_dup(nss->getRegistrarHandle().c_str());
  n->CrID = CORBA::string_dup(nss->getCreateRegistrarHandle().c_str());
  n->UpID = CORBA::string_dup(nss->getUpdateRegistrarHandle().c_str());
  // authinfo is filled only if session registar is ownering registrar
  n->AuthInfoPw = CORBA::string_dup(
    a.getRegistrar() == (int)nss->getRegistrarId() ? nss->getAuthPw().c_str()
          : ""
  );
  // states
  for (unsigned i=0; i<nss->getStatusCount(); i++) {
    Fred::TID stateId = nss->getStatusByIdx(i)->getStatusId();
    const Fred::StatusDesc* sd = regMan->getStatusDesc(stateId);
    if (!sd || !sd->getExternal())
      continue;
    n->stat.length(n->stat.length()+1);
    n->stat[n->stat.length()-1].value = CORBA::string_dup(sd->getName().c_str() );
    n->stat[n->stat.length()-1].text = CORBA::string_dup(sd->getDesc(
        a.getLang() == LANG_CS ? "CS" : "EN"
    ).c_str());
  }
  if (!n->stat.length()) {
    const Fred::StatusDesc* sd = regMan->getStatusDesc(0);
    if (sd) {
      n->stat.length(1);
      n->stat[0].value = CORBA::string_dup(sd->getName().c_str());
      n->stat[0].text = CORBA::string_dup(sd->getDesc(
          a.getLang() == LANG_CS ? "CS" : "EN"
      ).c_str());
    }
  }
  // nsset specific data
  n->handle = CORBA::string_dup(nss->getHandle().c_str());
  n->level = nss->getCheckLevel();
  n->tech.length(nss->getAdminCount());
  for (unsigned i=0; i<nss->getAdminCount(); i++)
    n->tech[i] = CORBA::string_dup(nss->getAdminByIdx(i).c_str());
  n->dns.length(nss->getHostCount());
  for (unsigned i=0; i<nss->getHostCount(); i++) {
    const Fred::NSSet::Host *h = nss->getHostByIdx(i);
    n->dns[i].fqdn = CORBA::string_dup(h->getName().c_str());
    n->dns[i].inet.length(h->getAddrCount());
    for (unsigned j=0; j<h->getAddrCount(); j++)
      n->dns[i].inet[j] = CORBA::string_dup(h->getAddrByIdx(j).c_str());
  }
  return a.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    NSSetDelete
 *
 * DESCRIPTION: deleting NSSet and saving it into history
 *              NSSet can be only deleted by registrar who created it
 *              or those who administers it
 *              nsset cannot be deleted if there is link into domain table
 * PARAMETERS:  handle - nsset identifier
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::NSSetDelete(
  const char* handle, const ccReg::EppParams &params)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    std::auto_ptr<EPPNotifier> ntf;
    int id;
    short int code = 0;

    EPPAction action(this, params.loginID, EPP_NSsetDelete, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    LOGGER(PACKAGE).notice(boost::format("NSSetDelete: clientID -> %1% clTRID [%2%] handle [%3%] ") % (int ) params.loginID % (const char*)params.clTRID % handle );

    // lock row
    id = getIdOfNSSet(action.getDB(), handle, restricted_handles_
            , lock_epp_commands_, true);
    if (id < 0) {
        LOG(WARNING_LOG, "bad format of nsset [%s]", handle);
        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                ccReg::nsset_handle, 1,
                REASON_MSG_BAD_FORMAT_NSSET_HANDLE);
    } else if (id == 0) {
        LOG( WARNING_LOG, "nsset handle [%s] NOT_EXIST", handle );
        code = COMMAND_OBJECT_NOT_EXIST;
    }
    if (!code &&  !action.getDB()->TestObjectClientID(id, action.getRegistrar()) ) // if not client od the object
    {
        LOG( WARNING_LOG, "bad autorization not client of nsset [%s]", handle );
        code = action.setErrorReason(COMMAND_AUTOR_ERROR,
                ccReg::registrar_autor, 0, REASON_MSG_REGISTRAR_AUTOR);
    }
    try {
        if (!code && (
                    testObjectHasState(action,id,FLAG_serverDeleteProhibited) ||
                    testObjectHasState(action,id,FLAG_serverUpdateProhibited) ||
                    testObjectHasState(action,id,FLAG_deleteCandidate)
                    ))
        {
            LOG( WARNING_LOG, "delete of object %s is prohibited" , handle );
            code = COMMAND_STATUS_PROHIBITS_OPERATION;
        }
    } catch (...) {
        code = COMMAND_FAILED;
    }
    if (!code) {
        ntf.reset(new EPPNotifier(disable_epp_notifier_,mm , action.getDB(), action.getRegistrar() , id ));

        // test to  table domain if relations to nsset
        if (action.getDB()->TestNSSetRelations(id) ) //  can not be delete
        {
            LOG( WARNING_LOG, "database relations" );
            code = COMMAND_PROHIBITS_OPERATION;
        } else {
            if (action.getDB()->SaveObjectDelete(id) ) // save to delete object
            {
                if (action.getDB()->DeleteNSSetObject(id) )
                    code = COMMAND_OK; // if is OK
            }
        }

        if (code == COMMAND_OK)
            action.setNotifier(ntf.get());
    }

    // EPP exception
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("NSSetDelete");
    }

    return action.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    NSSetCreate
 *
 * DESCRIPTION: creation NSSet and subservient DNS hosts
 *
 * PARAMETERS:  handle - nsset identifier
 *              authInfoPw - authentication
 *              tech - sequence of technical contact
 *              dns - sequence of DNS records
 *              level - tech check  level
 *        OUT:  crDate - object creation date
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::NSSetCreate(
  const char *handle, const char *authInfoPw, const ccReg::TechContact & tech,
  const ccReg::DNSHost & dns, CORBA::Short level, ccReg::timestamp_out crDate,
  const ccReg::EppParams &params)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    std::auto_ptr<EPPNotifier> ntf;
    char NAME[256]; // to upper case of name of DNS hosts
    int id, techid, hostID;
    unsigned int i, j, l;
    short inetNum;
    int *tch= NULL;
    short int code = 0;

    LOGGER(PACKAGE).notice(boost::format("NSSetCreate: clientID -> %1% clTRID [%2%] handle [%3%]  authInfoPw [%4%]") % (int ) params.loginID % (const char*)params.clTRID % handle % authInfoPw );
    LOGGER(PACKAGE).notice(boost::format("NSSetCreate: tech check level %1% tech num %2%") % (int) level % (int) tech.length() );

    crDate = CORBA::string_dup("");
    EPPAction action(this, params.loginID, EPP_NSsetCreate, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    std::auto_ptr<Fred::Zone::Manager> zman(
            Fred::Zone::Manager::create());
    std::auto_ptr<Fred::NSSet::Manager> nman(
            Fred::NSSet::Manager::create(action.getDB(),zman.get(),restricted_handles_));

    if (tech.length() < 1) {
        LOG( WARNING_LOG, "NSSetCreate: not any tech Contact " );
        code = action.setErrorReason(COMMAND_PARAMETR_MISSING,
                ccReg::nsset_tech, 0, REASON_MSG_TECH_NOTEXIST);
    } else if (tech.length() > 9) {
        LOG(WARNING_LOG, "NSSetCreate: too many tech contacts (max is 9)");
        code = action.setErrorReason(COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                ccReg::nsset_tech, 0, REASON_MSG_TECHADMIN_LIMIT);
    } else if (dns.length() < 2) {
        //  minimal two dns hosts
        if (dns.length() == 1) {
            LOG( WARNING_LOG, "NSSetCreate: minimal two dns host create one %s" , (const char *) dns[0].fqdn );
            code = action.setErrorReason(COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                    ccReg::nsset_dns_name, 1, REASON_MSG_MIN_TWO_DNS_SERVER);
        } else {
            LOG( WARNING_LOG, "NSSetCreate: minimal two dns DNS hosts" );
            code = action.setErrorReason(COMMAND_PARAMETR_MISSING,
                    ccReg::nsset_dns_name, 0, REASON_MSG_MIN_TWO_DNS_SERVER);
        }

    } else if (dns.length() > 9) {
        LOG(WARNING_LOG, "NSSetCreate: too many dns hosts (maximum is 9)");
        code = action.setErrorReason(COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                ccReg::nsset_dns_name, 0, REASON_MSG_NSSET_LIMIT);
    }
    if (code == 0) {
        Fred::NSSet::Manager::CheckAvailType caType;

        tch = new int[tech.length()];

        try {
            Fred::NameIdPair nameId;
            caType = nman->checkAvail(handle, nameId);
            id = nameId.id;
        }
        catch (...) {
            caType = Fred::NSSet::Manager::CA_INVALID_HANDLE;
            id = -1;
        }

        if (id<0 || caType == Fred::NSSet::Manager::CA_INVALID_HANDLE) {
            LOG(WARNING_LOG, "bad format of nssset [%s]", handle);
            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                    ccReg::nsset_handle, 1, REASON_MSG_BAD_FORMAT_NSSET_HANDLE);
        } else if (caType == Fred::NSSet::Manager::CA_REGISTRED) {
            LOG( WARNING_LOG, "nsset handle [%s] EXIST", handle );
            code = COMMAND_OBJECT_EXIST;
        } else if (caType == Fred::NSSet::Manager::CA_PROTECTED) {
            LOG(WARNING_LOG, "object [%s] in history period", handle);
            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                    ccReg::nsset_handle, 1, REASON_MSG_PROTECTED_PERIOD);
        }
    }
    if (code == 0) {
        // test tech-c
        std::auto_ptr<Fred::Contact::Manager>
            cman(Fred::Contact::Manager::create(action.getDB(),
                        restricted_handles_) );
        for (i = 0; i < tech.length() ; i++) {
            Fred::Contact::Manager::CheckAvailType caType;
            try {
                Fred::NameIdPair nameId;
                caType = cman->checkAvail((const char *)tech[i],nameId);
                techid = nameId.id;
            } catch (...) {
                caType = Fred::Contact::Manager::CA_INVALID_HANDLE;
                techid = 0;
            }

            if (caType != Fred::Contact::Manager::CA_REGISTRED) {
                if (techid < 0) {
                    LOG(WARNING_LOG, "bad format of Contact %s", (const char *)tech[i]);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::nsset_tech, i + 1, REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
                } else if (techid == 0) {
                    LOG(WARNING_LOG, "Contact %s not exist", (const char *)tech[i]);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::nsset_tech, i, REASON_MSG_TECH_NOTEXIST);
                }
            } else {
                tch[i] = techid;
                for (j = 0; j < i; j ++) // test duplicity
                {
                    LOG( DEBUG_LOG , "tech compare j %d techid %d ad %d" , j , techid , tch[j] );
                    if (tch[j] == techid && tch[j] > 0) {
                        tch[j]= 0;
                        LOG(WARNING_LOG, "Contact [%s] duplicity", (const char *)tech[i]);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::nsset_tech, i, REASON_MSG_DUPLICITY_CONTACT);
                    }
                }
            }
        }
    }

    if (code == 0) {

        LOG( DEBUG_LOG , "NSSetCreate:  dns.length %d" , (int ) dns.length() );
        // test DNS host

        // test IP address of  DNS host

        for (i = 0, inetNum=0; i < dns.length() ; i++) {

            LOG( DEBUG_LOG , "NSSetCreate: test host %s" , (const char *) dns[i].fqdn );

            //  list sequence
            for (j = 0; j < dns[i].inet.length(); j++) {
                LOG( DEBUG_LOG , "NSSetCreate: test inet[%d] = %s " , j , (const char *) dns[i].inet[j] );
                if (TestInetAddress(dns[i].inet[j]) ) {
                    for (l = 0; l < j; l ++) // test to duplicity
                    {
                        if (strcmp(dns[i].inet[l], dns[i].inet[j]) == 0) {
                            LOG( WARNING_LOG, "NSSetCreate: duplicity host address %s " , (const char *) dns[i].inet[j] );
                            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                    ccReg::nsset_dns_addr, inetNum + j + 1,
                                    REASON_MSG_DUPLICITY_DNS_ADDRESS);
                        }
                    }

                } else {
                    LOG( WARNING_LOG, "NSSetCreate: bad host address %s " , (const char *) dns[i].inet[j] );
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::nsset_dns_addr, inetNum + j + 1,
                            REASON_MSG_BAD_IP_ADDRESS);
                }

            }

            // test DNS hosts
            unsigned hostnameTest = nman->checkHostname(
                    (const char *)dns[i].fqdn, dns[i].inet.length() > 0);
            if (hostnameTest == 1) {

                LOG( WARNING_LOG, "NSSetCreate: bad host name %s " , (const char *) dns[i].fqdn );
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::nsset_dns_name, i + 1, REASON_MSG_BAD_DNS_NAME);
            } else {
                LOG( NOTICE_LOG , "NSSetCreate: test DNS Host %s", (const char *) dns[i].fqdn );
                convert_hostname(NAME, dns[i].fqdn);

                // not in defined zones and exist record of ip address
                if (hostnameTest == 2) {

                    for (j = 0; j < dns[i].inet.length() ; j ++) {

                        LOG( WARNING_LOG, "NSSetCreate:  ipaddr  glue not allowed %s " , (const char *) dns[i].inet[j] );
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::nsset_dns_addr, inetNum + j + 1,
                                REASON_MSG_IP_GLUE_NOT_ALLOWED);
                    }

                }

                // test to duplicity of nameservers
                for (l = 0; l < i; l ++) {
                    char PREV_NAME[256]; // to upper case of name of DNS hosts
                    convert_hostname(PREV_NAME, dns[l].fqdn);
                    if (strcmp(NAME, PREV_NAME) == 0) {
                        LOG( WARNING_LOG, "NSSetCreate: duplicity host name %s " , (const char *) NAME );
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::nsset_dns_name, i + 1,
                                REASON_MSG_DNS_NAME_EXIST);
                    }
                }

            }

            inetNum+= dns[i].inet.length(); //  InetNum counter  for return errors
        } // end of cycle

        LOG( DEBUG_LOG , "NSSetCreate: code %d" , code );

    }

    if (code == 0) {

        id = action.getDB()->CreateObject("N", action.getRegistrar(), handle, authInfoPw);
        if (id<=0) {
            if (id==0) {
                LOG( WARNING_LOG, "nsset handle [%s] EXIST", handle );
                code= COMMAND_OBJECT_EXIST;
            } else {
                LOG( WARNING_LOG, "Cannot insert [%s] into object_registry", handle );
                code= COMMAND_FAILED;
            }
        } else {

            if (level<0)
                level = nsset_level_;
            // write to nsset table
            action.getDB()->INSERT("NSSET");
            action.getDB()->INTO("id");
            if (level >= 0)
                action.getDB()->INTO("checklevel");
            action.getDB()->VALUE(id);
            if (level >= 0)
                action.getDB()->VALUE(level);

            // nsset first
            if ( !action.getDB()->EXEC() )
                code = COMMAND_FAILED;
            else {

                // get local timestamp with timezone of created object
                CORBA::string_free(crDate);
                crDate= CORBA::string_dup(action.getDB()->GetObjectCrDateTime(id) );

                // insert all tech-c
                for (i = 0; i < tech.length() ; i++) {
                    LOG( DEBUG_LOG, "NSSetCreate: add tech Contact %s id %d " , (const char *) tech[i] , tch[i]);
                    if ( !action.getDB()->AddContactMap("nsset", id, tch[i]) ) {
                        code = COMMAND_FAILED;
                        break;
                    }
                }

                // insert all DNS hosts


                for (i = 0; i < dns.length() ; i++) {

                    // convert host name to lower case
                    LOG( NOTICE_LOG , "NSSetCreate: DNS Host %s ", (const char *) dns[i].fqdn );
                    convert_hostname(NAME, dns[i].fqdn);

                    // ID  sequence
                    hostID = action.getDB()->GetSequenceID("host");

                    // HOST  informations
                    action.getDB()->INSERT("HOST");
                    action.getDB()->INTO("ID");
                    action.getDB()->INTO("NSSETID");
                    action.getDB()->INTO("fqdn");
                    action.getDB()->VALUE(hostID);
                    action.getDB()->VALUE(id);
                    action.getDB()->VVALUE(NAME);
                    if (action.getDB()->EXEC() ) {

                        // save ip address of host
                        for (j = 0; j < dns[i].inet.length(); j++) {
                            LOG( NOTICE_LOG , "NSSetCreate: IP address hostID  %d [%s] ", hostID , (const char *) dns[i].inet[j] );

                            // HOST_IPADDR insert IP address of DNS host
                            action.getDB()->INSERT("HOST_IPADDR_map");
                            action.getDB()->INTO("HOSTID");
                            action.getDB()->INTO("NSSETID");
                            action.getDB()->INTO("ipaddr");
                            action.getDB()->VALUE(hostID);
                            action.getDB()->VALUE(id); // write nssetID
                            action.getDB()->VVALUE(dns[i].inet[j]);

                            if (action.getDB()->EXEC() == false) {
                                code = COMMAND_FAILED;
                                break;
                            }

                        }

                    } else {
                        code = COMMAND_FAILED;
                        break;
                    }

                } // end of host cycle


                //  save to  historie if is OK
                if (code != COMMAND_FAILED)
                    if (action.getDB()->SaveNSSetHistory(id, params.requestID))
                        if (action.getDB()->SaveObjectCreate(id) )
                            code = COMMAND_OK;
            } //


            if (code == COMMAND_OK) // run notifier
            {
                ntf.reset(new EPPNotifier(disable_epp_notifier_,mm , action.getDB(), action.getRegistrar() , id ));
                action.setNotifier(ntf.get());
            }

        }
    }

    delete[] tch;


    // EPP exception
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("NSSetCreate");
    }

    return action.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    NSSetUpdate
 *
 * DESCRIPTION: change of NSSet and subservient DNS hosts and technical contacts
 *              and saving changes into history
 * PARAMETERS:  handle - nsset identifier
 *              authInfo_chg - authentication change
 *              dns_add - sequence of added DNS records
 *              dns_rem - sequence of DNS records for deleting
 *              tech_add - sequence of added technical contacts
 *              tech_rem - sequence of technical contact for deleting
 *              level - tech check level
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response *
ccReg_EPP_i::NSSetUpdate(const char* handle, const char* authInfo_chg,
        const ccReg::DNSHost& dns_add, const ccReg::DNSHost& dns_rem,
        const ccReg::TechContact& tech_add, const ccReg::TechContact& tech_rem,
        CORBA::Short level, const ccReg::EppParams &params)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    std::auto_ptr<EPPNotifier> ntf;
    char NAME[256], REM_NAME[256];
    int nssetID, techid, hostID;
    unsigned int i, j, k, l;
    short inetNum;
    int hostNum, techNum;
    bool findRem; // test for change DNS hosts
    short int code = 0;

    int *tch_add = new int[ tech_add.length() ];
    if (tech_add.length() > 0) {
        for (unsigned int i = 0; i < tech_add.length(); ++i)
            tch_add[i] = 0;
    }

    int *tch_rem = new int[ tech_rem.length() ];
    if (tech_rem.length() > 0) {
        for (unsigned int i = 0; i < tech_rem.length(); ++i)
            tch_rem[i] = 0;
    }

    EPPAction action(this, params.loginID, EPP_NSsetUpdate, static_cast<const char*>(params.clTRID), params.XML, params.requestID);


    LOGGER(PACKAGE).notice( boost::format("NSSetUpdate: clientID -> %1% clTRID [%2%] handle [%3%] authInfo_chg  [%4%] ") % (int ) params.loginID % (const char*)params.clTRID % handle % authInfo_chg);
    LOGGER(PACKAGE).notice( boost::format("NSSetUpdate: tech check level %1%") % (int) level );

    std::auto_ptr<Fred::Zone::Manager> zman(
            Fred::Zone::Manager::create());
    std::auto_ptr<Fred::NSSet::Manager> nman(
            Fred::NSSet::Manager::create(action.getDB(),zman.get(),restricted_handles_));

    if ( (nssetID = getIdOfNSSet(action.getDB(), handle, restricted_handles_
            , lock_epp_commands_, true) ) < 0) {
        LOG(WARNING_LOG, "bad format of nsset [%s]", handle);
    } else if (nssetID == 0) {
        LOG( WARNING_LOG, "nsset handle [%s] NOT_EXIST", handle );
        code = COMMAND_OBJECT_NOT_EXIST;
    }
    // registrar of the object
    if (!code && !action.getDB()->TestObjectClientID(nssetID, action.getRegistrar()) ) {
        LOG( WARNING_LOG, "bad autorization not  client of nsset [%s]", handle );
        code = action.setErrorReason(COMMAND_AUTOR_ERROR,
                ccReg::registrar_autor, 0,
                REASON_MSG_REGISTRAR_AUTOR);
    }
    try {
        if (!code && (testObjectHasState(action,nssetID,FLAG_serverUpdateProhibited) ||
                testObjectHasState(action,nssetID,FLAG_deleteCandidate)))
        {
            LOG( WARNING_LOG, "update of object %s is prohibited" , handle );
            code = COMMAND_STATUS_PROHIBITS_OPERATION;
        }
    } catch (...) {
        code = COMMAND_FAILED;
    }

    if (!code) {

        // test  ADD tech-c
        for (i = 0; i < tech_add.length(); i++) {
            if ( (techid = getIdOfContact(action.getDB(), tech_add[i], restricted_handles_
                    , lock_epp_commands_) ) <= 0) {
                if (techid < 0) {
                    LOG(WARNING_LOG, "bad format of contact %s", (const char *)tech_add[i]);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::nsset_tech_add, i + 1,
                            REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
                } else if (techid == 0) {
                    LOG(WARNING_LOG, "Contact %s not exist", (const char *)tech_add[i]);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::nsset_tech_add, i + 1,
                            REASON_MSG_TECH_NOTEXIST);
                }
            } else if (action.getDB()->CheckContactMap("nsset", nssetID, techid, 0) ) {
                LOG(WARNING_LOG, "Tech Contact [%s] exist in contact map table",
                        (const char *)tech_add[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::nsset_tech_add, i + 1,
                        REASON_MSG_TECH_EXIST);
            } else {
                tch_add[i] = techid;
                for (j = 0; j < i; j ++)
                    // duplicity test
                    if (tch_add[j] == techid && tch_add[j] > 0) {
                        tch_add[j] = 0;
                        LOG(WARNING_LOG, "Contact [%s] duplicity", (const char *)tech_add[i]);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::nsset_tech_add, i,
                                REASON_MSG_DUPLICITY_CONTACT);
                    }
            }

            LOG( NOTICE_LOG , "ADD  tech  techid ->%d [%s]" , techid , (const char *) tech_add[i] );
        }

        // test REM tech-c
        for (i = 0; i < tech_rem.length(); i++) {

            if ( (techid = getIdOfContact(action.getDB(), tech_rem[i], restricted_handles_
                    , lock_epp_commands_) ) <= 0) {
                LOG(WARNING_LOG, "bad format of contact %s", (const char *)tech_rem[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::nsset_tech_rem, i + 1,
                        REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
            } else if ( !action.getDB()->CheckContactMap("nsset", nssetID, techid, 0) ) {
                LOG(WARNING_LOG, "Contact %s not exist", (const char *)tech_rem[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::nsset_tech_rem, i + 1,
                        REASON_MSG_TECH_NOTEXIST);
            } else {
                tch_rem[i] = techid;
                for (j = 0; j < i; j ++)
                    // test  duplicity
                    if (tch_rem[j] == techid && tch_rem[j] > 0) {
                        tch_rem[j] = 0;
                        LOG(WARNING_LOG, "Contact [%s] duplicity", (const char *)tech_rem[i]);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::nsset_tech_rem, i,
                                REASON_MSG_DUPLICITY_CONTACT);
                    }
            }
            LOG( NOTICE_LOG , "REM  tech  techid ->%d [%s]" , techid , (const char *) tech_rem[i] );

        }

        // ADD DNS HOSTS  and TEST IP address and name of the  DNS HOST
        for (i = 0, inetNum =0; i < dns_add.length(); i++) {

            /// test DNS host
            if (nman->checkHostname((const char *)dns_add[i].fqdn, false)) {
                LOG( WARNING_LOG, "NSSetUpdate: bad add host name %s " , (const char *) dns_add[i].fqdn );
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::nsset_dns_name_add, i + 1,
                        REASON_MSG_BAD_DNS_NAME);
            } else {
                LOG( NOTICE_LOG , "NSSetUpdate: add dns [%s]" , (const char * ) dns_add[i].fqdn );

                convert_hostname(NAME, dns_add[i].fqdn); // convert to lower case
                // HOST is not in defined zone and contain ip address
                if (getZone(action.getDB(), dns_add[i].fqdn) == 0
                        && (int ) dns_add[i].inet.length() > 0) {
                    for (j = 0; j < dns_add[i].inet.length() ; j ++) {
                        LOG( WARNING_LOG, "NSSetUpdate:  ipaddr  glue not allowed %s " , (const char *) dns_add[i].inet[j] );
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::nsset_dns_addr, inetNum + j + 1,
                                REASON_MSG_IP_GLUE_NOT_ALLOWED);
                    }
                } else {

                    if (action.getDB()->GetHostID(NAME, nssetID) ) // already exist can not add
                    {
                        // TEST if the add DNS host is in REM   dns_rem[i].fqdn
                        findRem=false;
                        for (k = 0; k < dns_rem.length(); k++) {
                            convert_hostname(REM_NAME, dns_rem[k].fqdn);
                            if (strcmp(NAME, REM_NAME) == 0) {
                                LOG( NOTICE_LOG ,"NSSetUpdate: add HOST %s find remove host %s" , NAME , REM_NAME );
                                findRem=true;
                                break;
                            }
                        }

                        if ( !findRem) // if is not in the REM DNS hosts
                        {
                            LOG( WARNING_LOG, "NSSetUpdate:  host name %s exist" , (const char *) dns_add[i].fqdn );
                            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                    ccReg::nsset_dns_name_add, i + 1,
                                    REASON_MSG_DNS_NAME_EXIST);
                        }

                    }

                }

            }

            // TEST IP addresses
            for (j = 0; j < dns_add[i].inet.length(); j++) {

                if (TestInetAddress(dns_add[i].inet[j]) ) {
                    for (l = 0; l < j; l ++) // duplicity test
                    {
                        if (strcmp(dns_add[i].inet[l], dns_add[i].inet[j]) == 0) {
                            LOG( WARNING_LOG, "NSSetUpdate: duplicity host address %s " , (const char *) dns_add[i].inet[j] );
                            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                    ccReg::nsset_dns_addr, inetNum + j + 1,
                                    REASON_MSG_DUPLICITY_DNS_ADDRESS);
                        }
                    }

                } else // not valid IP address
                {
                    LOG( WARNING_LOG, "NSSetUpdate: bad add host address %s " , (const char *) dns_add[i].inet[j] );
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::nsset_dns_addr, inetNum + j + 1,
                            REASON_MSG_BAD_IP_ADDRESS);
                }
            }

            inetNum+= dns_add[i].inet.length(); //  count  InetNum for errors

            // test to duplicity of added nameservers
            for (l = 0; l < i; l ++) {
                char PREV_NAME[256]; // to upper case of name of DNS hosts
                convert_hostname(PREV_NAME, dns_add[l].fqdn);
                if (strcmp(NAME, PREV_NAME) == 0) {
                    LOG( WARNING_LOG, "NSSetUpdate:  host name %s duplicate" , (const char *) dns_add[i].fqdn );
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::nsset_dns_name_add, i + 1,
                            REASON_MSG_DNS_NAME_EXIST);
                }
            }

        } // end of cycle


        // test for DNS HOSTS to REMOVE if is valid format and if is exist in the table
        for (i = 0; i < dns_rem.length(); i++) {
            LOG( NOTICE_LOG , "NSSetUpdate:  delete  host  [%s] " , (const char *) dns_rem[i].fqdn );

            if (nman->checkHostname((const char *)dns_rem[i].fqdn, false)) {
                LOG( WARNING_LOG, "NSSetUpdate: bad rem host name %s " , (const char *) dns_rem[i].fqdn );
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::nsset_dns_name_rem, i + 1,
                        REASON_MSG_BAD_DNS_NAME);
            } else {
                convert_hostname(NAME, dns_rem[i].fqdn);
                if ( (hostID = action.getDB()->GetHostID(NAME, nssetID) ) == 0) {
                    LOG( WARNING_LOG, "NSSetUpdate:  host  [%s] not in table" , (const char *) dns_rem[i].fqdn );
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::nsset_dns_name_rem, i + 1,
                            REASON_MSG_DNS_NAME_NOTEXIST);
                }
            }

            // test to duplicity of removing nameservers
            for (l = 0; l < i; l ++) {
                char PREV_NAME[256]; // to upper case of name of DNS hosts
                convert_hostname(PREV_NAME, dns_rem[l].fqdn);
                if (strcmp(NAME, PREV_NAME) == 0) {
                    LOG( WARNING_LOG, "NSSetUpdate:  host name %s duplicate" , (const char *) dns_rem[i].fqdn );
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::nsset_dns_name_rem, i + 1,
                            REASON_MSG_DNS_NAME_NOTEXIST);
                }
            }

        }

        // if not any errors in the parametrs run update
        if (code == 0)
            if (action.getDB()->ObjectUpdate(nssetID, action.getRegistrar(), authInfo_chg) ) {

                // notifier
                ntf.reset(new EPPNotifier(
                              disable_epp_notifier_,
                              mm, action.getDB(), action.getRegistrar(), nssetID, regMan.get()));

                //  add to current tech-c added tech-c
                for (i = 0; i < tech_add.length(); i++)
                    ntf->AddTechNew(tch_add[i]);

                // update tech level
                if (level >= 0) {
                    LOG( NOTICE_LOG, "update nsset check level %d ", (int ) level );
                    action.getDB()->UPDATE("nsset");
                    action.getDB()->SET("checklevel", level);
                    action.getDB()->WHERE("id", nssetID);
                    if (action.getDB()->EXEC() == false)
                        code = COMMAND_FAILED;
                }

                //-------- TECH contacts

                // add tech contacts
                for (i = 0; i < tech_add.length(); i++) {

                    LOG( NOTICE_LOG , "INSERT add techid ->%d [%s]" , tch_add[i] , (const char *) tech_add[i] );
                    if ( !action.getDB()->AddContactMap("nsset", nssetID, tch_add[i]) ) {
                        code = COMMAND_FAILED;
                        break;
                    }

                }

                // delete  tech contacts
                for (i = 0; i < tech_rem.length(); i++) {

                    LOG( NOTICE_LOG , "DELETE rem techid ->%d [%s]" , tch_rem[i] , (const char *) tech_rem[i] );
                    if ( !action.getDB()->DeleteFromTableMap("nsset", nssetID, tch_rem[i]) ) {
                        code = COMMAND_FAILED;
                        break;
                    }

                }

                //--------- TEST for numer of tech-c after  ADD & REM
                // only if the tech-c remove
                if (tech_rem.length() > 0) {
                    techNum = action.getDB()->GetNSSetContacts(nssetID);
                    LOG(NOTICE_LOG, "NSSetUpdate: tech Contact  %d" , techNum );

                    if (techNum == 0) // can not be nsset without tech-c
                    {

                        // marked all  REM tech-c as param  errors
                        for (i = 0; i < tech_rem.length(); i++) {
                            code = action.setErrorReason(COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                                    ccReg::nsset_tech_rem, i + 1,
                                    REASON_MSG_CAN_NOT_REMOVE_TECH);
                        }
                    }
                }

                // delete DNS HOSTY  first step
                for (i = 0; i < dns_rem.length(); i++) {
                    LOG( NOTICE_LOG , "NSSetUpdate:  delete  host  [%s] " , (const char *) dns_rem[i].fqdn );

                    convert_hostname(NAME, dns_rem[i].fqdn);
                    hostID = action.getDB()->GetHostID(NAME, nssetID);
                    LOG( NOTICE_LOG , "DELETE  hostID %d" , hostID );
                    if ( !action.getDB()->DeleteFromTable("HOST", "id", hostID) )
                        code = COMMAND_FAILED;
                    else if ( !action.getDB()->DeleteFromTable("HOST_IPADDR_map", "hostID",
                                hostID) )
                        code = COMMAND_FAILED;
                }

                //-------- add  DNS HOSTs second step

                for (i = 0; i < dns_add.length(); i++) {
                    // to lowe case
                    convert_hostname(NAME, dns_add[i].fqdn);

                    // hostID from sequence
                    hostID = action.getDB()->GetSequenceID("host");

                    // HOST information
                    action.getDB()->INSERT("HOST");
                    action.getDB()->INTO("ID");
                    action.getDB()->INTO("nssetid");
                    action.getDB()->INTO("fqdn");
                    action.getDB()->VALUE(hostID);
                    action.getDB()->VALUE(nssetID); // add nssetID
                    action.getDB()->VALUE(NAME);
                    if (action.getDB()->EXEC()) // add all IP address
                    {

                        for (j = 0; j < dns_add[i].inet.length(); j++) {
                            LOG( NOTICE_LOG , "insert  IP address hostID  %d [%s] ", hostID , (const char *) dns_add[i].inet[j] );

                            // insert ipaddr with hostID and nssetID
                            action.getDB()->INSERT("HOST_IPADDR_map");
                            action.getDB()->INTO("HOSTID");
                            action.getDB()->INTO("NSSETID");
                            action.getDB()->INTO("ipaddr");
                            action.getDB()->VALUE(hostID);
                            action.getDB()->VALUE(nssetID);
                            action.getDB()->VVALUE(dns_add[i].inet[j]);

                            // if failed
                            if (action.getDB()->EXEC() == false) {
                                code = COMMAND_FAILED;
                                break;
                            }

                        }

                    } else {
                        code = COMMAND_FAILED;
                        break;
                    } // if add host failed


                }

                //------- TEST number DNS host after REM & ADD
                //  only if the add or rem
                if (dns_rem.length() > 0 || dns_add.length() > 0) {
                    hostNum = action.getDB()->GetNSSetHosts(nssetID);
                    LOG(NOTICE_LOG, "NSSetUpdate:  hostNum %d" , hostNum );

                    if (hostNum < 2) //  minimal two DNS
                    {
                        for (i = 0; i < dns_rem.length(); i++) {
                            // marked all  REM DNS hots as param error
                            code = action.setErrorReason(COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                                    ccReg::nsset_dns_name_rem, i + 1,
                                    REASON_MSG_CAN_NOT_REM_DNS);
                        }
                    }

                    if (hostNum > 9) // maximal number
                    {
                        for (i = 0; i < dns_add.length(); i++) {
                            // marked all ADD dns host  as param error
                            code = action.setErrorReason(COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                                    ccReg::nsset_dns_name_add, i + 1,
                                    REASON_MSG_CAN_NOT_ADD_DNS);
                        }
                    }

                }

                // save to history if not errors
                if (code == 0)
                    if (action.getDB()->SaveNSSetHistory(nssetID, params.requestID) )
                        code = COMMAND_OK; // set up successfully as default


                if (code == COMMAND_OK)
                    action.setNotifier(ntf.get()); // schedule message send


            }

    }
    // free mem
    delete[] tch_add;
    delete[] tch_rem;

    // EPP exception
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("NSSetUpdate");
    }

    return action.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    DomainInfo
 *
 * DESCRIPTION: return detailed information about domain
 *              empty value if domain doesn't exists
 * PARAMETERS:  fqdn - domain identifier its name
 *        OUT:  d - domain structure detailed description
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::DomainInfo(
  const char* fqdn, ccReg::Domain_out d, const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

  LOG(
      NOTICE_LOG, "DomainInfo: clientID -> %llu clTRID [%s] fqdn  [%s] ",
       params.loginID, static_cast<const char*>(params.clTRID), fqdn
  );
  // start EPP action - this will handle all init stuff
  EPPAction a(this, params.loginID, EPP_DomainInfo, static_cast<const char*>(params.clTRID), params.XML, params.requestID);
  // initialize managers for domain manipulation
  std::auto_ptr<Fred::Zone::Manager>
      zman(Fred::Zone::Manager::create() );
  std::auto_ptr<Fred::Domain::Manager>
      dman(Fred::Domain::Manager::create(a.getDB(), zman.get()) );
  // first check handle for proper format
  Fred::Domain::CheckAvailType caType = dman->checkHandle(fqdn);
  if (caType != Fred::Domain::CA_AVAILABLE) {
    // failure in FQDN check, throw exception
    a.failed(SetReasonDomainFQDN(a.getErrors(), fqdn, caType
        != Fred::Domain::CA_BAD_ZONE ? -1 : 0, a.getLang() ));
  }
  // now load domain by fqdn
  std::auto_ptr<Fred::Domain::List> dlist(dman->createList());
  dlist->setFQDNFilter(fqdn);
  try {dlist->reload();}
  catch (...) {a.failedInternal("Cannot load domains");}
  if (dlist->getCount() != 1)
    // failer because non existance, throw exception
    a.failed(COMMAND_OBJECT_NOT_EXIST);
  // start filling output domain structure
  Fred::Domain::Domain *dom = dlist->getDomain(0);
  d = new ccReg::Domain;
  // fill common object data
  d->ROID = CORBA::string_dup(dom->getROID().c_str());
  d->name = CORBA::string_dup(dom->getFQDN().c_str());
  d->CrDate = CORBA::string_dup(formatTime(dom->getCreateDate()).c_str());
  d->UpDate = CORBA::string_dup(formatTime(dom->getUpdateDate()).c_str());
  d->TrDate = CORBA::string_dup(formatTime(dom->getTransferDate()).c_str());
  d->ClID = CORBA::string_dup(dom->getRegistrarHandle().c_str());
  d->CrID = CORBA::string_dup(dom->getCreateRegistrarHandle().c_str());
  d->UpID = CORBA::string_dup(dom->getUpdateRegistrarHandle().c_str());
  // authinfo is filled only if session registar is ownering registrar
  d->AuthInfoPw = CORBA::string_dup(
    a.getRegistrar() == (int)dom->getRegistrarId() ? dom->getAuthPw().c_str()
          : ""
  );
  // states
  for (unsigned i=0; i<dom->getStatusCount(); i++) {
    Fred::TID stateId = dom->getStatusByIdx(i)->getStatusId();
    const Fred::StatusDesc* sd = regMan->getStatusDesc(stateId);
    if (!sd || !sd->getExternal())
      continue;
    d->stat.length(d->stat.length()+1);
    d->stat[d->stat.length()-1].value = CORBA::string_dup(sd->getName().c_str() );
    d->stat[d->stat.length()-1].text = CORBA::string_dup(sd->getDesc(
        a.getLang() == LANG_CS ? "CS" : "EN"
    ).c_str());
  }
  if (!d->stat.length()) {
    const Fred::StatusDesc* sd = regMan->getStatusDesc(0);
    if (sd) {
      d->stat.length(1);
      d->stat[0].value = CORBA::string_dup(sd->getName().c_str());
      d->stat[0].text = CORBA::string_dup(sd->getDesc(
          a.getLang() == LANG_CS ? "CS" : "EN"
      ).c_str());
    }
  }
  // fill domain specific data
  d->nsset = CORBA::string_dup(dom->getNSSetHandle().c_str());
  d->keyset = CORBA::string_dup(dom->getKeySetHandle().c_str());
  d->ExDate = CORBA::string_dup(to_iso_extended_string(dom->getExpirationDate()).c_str() );
  // registrant and contacts are disabled for other registrars
  // in case of enum domain
  bool disabled = a.getRegistrar() != (int)dom->getRegistrarId()
      && zman->findApplicableZone(fqdn)->isEnumZone();
  // registrant
  d->Registrant = CORBA::string_dup(disabled ? "" : dom->getRegistrantHandle().c_str() );
  // admin
  unsigned adminCount = disabled ? 0 : dom->getAdminCount(1);
  d->admin.length(adminCount);
  for (unsigned i=0; i<adminCount; i++)
    d->admin[i] = CORBA::string_dup(dom->getAdminHandleByIdx(i,1).c_str());
  // temps
  unsigned tempCount = disabled ? 0 : dom->getAdminCount(2);
  d->tmpcontact.length(tempCount);
  for (unsigned i=0; i<tempCount; i++)
    d->tmpcontact[i] = CORBA::string_dup(dom->getAdminHandleByIdx(i,2).c_str());
  // validation
  if (!dom->getValExDate().is_special()) {
    ccReg::ENUMValidationExtension *enumVal =
        new ccReg::ENUMValidationExtension();
    enumVal->valExDate = CORBA::string_dup(to_iso_extended_string(dom->getValExDate()).c_str() );
    enumVal->publish = dom->getPublish() ? ccReg::DISCL_DISPLAY : ccReg::DISCL_HIDE;
    d->ext.length(1);
    d->ext[0] <<= enumVal;
  }
  return a.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    DomainDelete
 *
 * DESCRIPTION: domain delete and save into history
 *
 * PARAMETERS:  fqdn - domain identifier its name
 *              params - common EPP parametres
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::DomainDelete(
  const char* fqdn, const ccReg::EppParams &params)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    std::auto_ptr<EPPNotifier> ntf;
    int id, zone;
    short int code = 0;

    EPPAction action(this, params.loginID, EPP_DomainDelete, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    LOGGER(PACKAGE).notice(boost::format("DomainDelete: clientID -> %1% clTRID [%2%] fqdn  [%3%] ") % (int ) params.loginID % static_cast<const char*>(params.clTRID) % fqdn );

    if ( (id = getIdOfDomain(action.getDB(), fqdn, lock_epp_commands_
            , true, &zone) ) <= 0) {
        LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
        code=COMMAND_OBJECT_NOT_EXIST;
    }
    else if (action.getDB()->TestRegistrarZone(action.getRegistrar(), zone) == false) {
        LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
        code = COMMAND_AUTHENTICATION_ERROR;
    }
    else if ( !action.getDB()->TestObjectClientID(id, action.getRegistrar()) ) {
        LOG( WARNING_LOG, "bad autorization not client of fqdn [%s]", fqdn );
        code = action.setErrorReason(COMMAND_AUTOR_ERROR,
                ccReg::registrar_autor, 0, REASON_MSG_REGISTRAR_AUTOR);
    }
    try {
        if (!code && (
                    testObjectHasState(action,id,FLAG_serverDeleteProhibited) ||
                    testObjectHasState(action,id,FLAG_serverUpdateProhibited) ||
                    testObjectHasState(action,id,FLAG_deleteCandidate)
                    ))
        {
            LOG( WARNING_LOG, "delete of object %s is prohibited" , fqdn );
            code = COMMAND_STATUS_PROHIBITS_OPERATION;
        }
    } catch (...) {
        code = COMMAND_FAILED;
    }
    if (!code) {
        ntf.reset(new EPPNotifier(disable_epp_notifier_,mm , action.getDB(), action.getRegistrar() , id ));

        if (action.getDB()->SaveObjectDelete(id) ) //save object as delete
        {
            if (action.getDB()->DeleteDomainObject(id) )
                code = COMMAND_OK; // if succesfully deleted
        }
        if (code == COMMAND_OK)
            action.setNotifier(ntf.get());
    }

    // EPP exception
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("DomainDelete");
    }

    return action.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    DomainUpdate
 *
 * DESCRIPTION: change of information about domain and save into history
 *
 * PARAMETERS:  fqdn - domain identifier its name
 *              registrant_chg - change of domain holder
 *              authInfo_chg  - change of password
 *              nsset_chg - change of nsset
 *              keyset_chg - change of keyset
 *              admin_add - sequence of added administration contacts
 *              admin_rem - sequence of deleted administration contacts
 *              tmpcontact_rem - sequence of deleted temporary contacts
 *              params - common EPP parametres
 *              ext - ExtensionList
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/
ccReg::Response * ccReg_EPP_i::DomainUpdate(
  const char *fqdn, const char *registrant_chg, const char *authInfo_chg,
  const char *nsset_chg, const char *keyset_chg,
  const ccReg::AdminContact & admin_add, const ccReg::AdminContact & admin_rem,
  const ccReg::AdminContact& tmpcontact_rem, const ccReg::EppParams &params,
  const ccReg::ExtensionList & ext)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    std::auto_ptr<EPPNotifier> ntf;
    std::string valexdate;
    ccReg::Disclose publish;
    int id, nssetid, contactid, adminid, keysetid;
    int seq, zone;
    std::vector<int> ac_add, ac_rem, tc_rem;
    unsigned int i, j;
    short int code = 0;

    seq=0;

    EPPAction action(this, params.loginID, EPP_DomainUpdate, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    LOGGER(PACKAGE).notice(boost::format ("DomainUpdate: clientID -> %1% clTRID [%2%] fqdn  [%3%] registrant_chg  [%4%] authInfo_chg [%5%]  nsset_chg [%6%] keyset_chg[%7%] ext.length %8%") %
            (int ) params.loginID % (const char*)params.clTRID % fqdn % registrant_chg % authInfo_chg % nsset_chg % keyset_chg % (long)ext.length() );

    ac_add.resize(admin_add.length());
    ac_rem.resize(admin_rem.length());
    tc_rem.resize(tmpcontact_rem.length());

    // parse enum.Exdate extension
    extractEnumDomainExtension(valexdate, publish, ext);

    if ( (id = getIdOfDomain(action.getDB(), fqdn, lock_epp_commands_
            , true, &zone) ) <= 0) {
        LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
        code=COMMAND_OBJECT_NOT_EXIST;
    }
    else if (action.getDB()->TestRegistrarZone(action.getRegistrar(), zone) == false) // test registrar autority to the zone
    {
        LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
        code = COMMAND_AUTHENTICATION_ERROR;
    }
    // if not client of the domain
    else if ( !action.getDB()->TestObjectClientID(id, action.getRegistrar()) ) {
        LOG( WARNING_LOG, "bad autorization not client of domain [%s]", fqdn );
        code = action.setErrorReason(COMMAND_AUTOR_ERROR,
                ccReg::registrar_autor, 0,
                REASON_MSG_REGISTRAR_AUTOR);
    }

    if (!code) {
        try {
            if (!code && (testObjectHasState(action,id,FLAG_serverUpdateProhibited) ||
                    testObjectHasState(action,id,FLAG_deleteCandidate))) {
                LOG( WARNING_LOG, "update of object %s is prohibited" , fqdn );
                code = COMMAND_STATUS_PROHIBITS_OPERATION;
            }
        } catch (...) {
            code = COMMAND_FAILED;
        }
    }

    if ( !code) {

        // test  ADD admin-c
        for (i = 0; i < admin_add.length(); i++) {
            LOG( NOTICE_LOG , "admin ADD contact %s" , (const char *) admin_add[i] );
            adminid = getIdOfContact(action.getDB(), admin_add[i], restricted_handles_
                    , lock_epp_commands_);
            if (adminid < 0) {
                LOG(WARNING_LOG, "bad format of contact %s", (const char *)admin_add[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_admin_add, i + 1,
                        REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
            } else if (adminid == 0) {
                LOG(WARNING_LOG, "Contact %s not exist", (const char *)admin_add[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_admin_add, i + 1,
                        REASON_MSG_ADMIN_NOTEXIST);
            } else {
                if (action.getDB()->CheckContactMap("domain", id, adminid, 1) ) {
                    LOG(WARNING_LOG, "Admin Contact [%s] exist in contact map table",
                            (const char *)admin_add[i]);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::domain_admin_add, i + 1,
                            REASON_MSG_ADMIN_EXIST);
                } else {
                    ac_add[i] = adminid;
                    for (j = 0; j < i; j ++)
                        // test  duplicity
                        if (ac_add[j] == adminid && ac_add[j] > 0) {
                            ac_add[j] = 0;
                            LOG( WARNING_LOG, "Contact [%s] duplicity " , (const char *) admin_add[i] );
                            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                    ccReg::domain_admin_add, i + 1,
                                    REASON_MSG_DUPLICITY_CONTACT);
                        }
                }
                // admin cannot be added if there is equivalent id in temp-c
                if (action.getDB()->CheckContactMap("domain", id, adminid, 2)) {
                    // exception is when in this command there is schedulet remove of thet temp-c
                    std::string adminHandle = (const char *)admin_add[i];
                    bool tmpcFound = false;
                    for (unsigned ti=0; ti<tmpcontact_rem.length(); ti++) {
                        if (adminHandle == (const char *)tmpcontact_rem[ti]) {
                            tmpcFound = true;
                            break;
                        }
                    }
                    if (!tmpcFound) {
                        LOG(WARNING_LOG, "Admin Contact [%s] exist in contact map table",
                                (const char *)admin_add[i]);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::domain_admin_add, i + 1,
                                REASON_MSG_ADMIN_EXIST);
                    }
                }
            }
        }

        // test REM admin-c
        for (i = 0; i < admin_rem.length(); i++) {
            LOG( NOTICE_LOG , "admin REM contact %s" , (const char *) admin_rem[i] );
            adminid = getIdOfContact(action.getDB(), admin_rem[i], restricted_handles_
                    , lock_epp_commands_);
            if (adminid < 0) {
                LOG(WARNING_LOG, "bad format of contact %s", (const char *)admin_rem[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_admin_rem, i + 1,
                        REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
            } else if (adminid == 0) {
                LOG(WARNING_LOG, "Contact %s not exist", (const char *)admin_rem[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_admin_rem, i + 1,
                        REASON_MSG_ADMIN_NOTEXIST);
            } else {
                if ( !action.getDB()->CheckContactMap("domain", id, adminid, 1) ) {
                    LOG(WARNING_LOG, "Admin Contact [%s] not exist in contact map table",
                            (const char *)admin_rem[i]);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::domain_admin_rem, i + 1,
                            REASON_MSG_ADMIN_NOTEXIST);
                } else {
                    ac_rem[i] = adminid;
                    for (j = 0; j < i; j ++) {
                        // test  duplicity
                        if (ac_rem[j] == adminid && ac_rem[j] > 0) {
                            ac_rem[j] = 0;
                            LOG(WARNING_LOG, "Contact [%s] duplicity", (const char *)admin_rem[i]);
                            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                    ccReg::domain_admin_rem, i,
                                    REASON_MSG_DUPLICITY_CONTACT);
                        }
                    }
                }
            }
        }

        // test REM temp-c
        for (i = 0; i < tmpcontact_rem.length(); i++) {
            LOG( NOTICE_LOG , "temp REM contact %s" , (const char *) tmpcontact_rem[i] );
            adminid = getIdOfContact(action.getDB(), tmpcontact_rem[i], restricted_handles_
                    , lock_epp_commands_);
            if (adminid < 0) {
                LOG(WARNING_LOG, "bad format of contact %s", (const char *)tmpcontact_rem[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_tmpcontact, i + 1,
                        REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
            } else if (adminid == 0) {
                LOG(WARNING_LOG, "Contact %s not exist", (const char *)tmpcontact_rem[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_tmpcontact, i + 1,
                        REASON_MSG_ADMIN_NOTEXIST);
            } else {
                if ( !action.getDB()->CheckContactMap("domain", id, adminid, 2) ) {
                    LOG(WARNING_LOG, "Temp Contact [%s] notexist in contact map table",
                            (const char *)tmpcontact_rem[i]);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::domain_tmpcontact, i + 1,
                            REASON_MSG_ADMIN_NOTEXIST);
                } else {

                    tc_rem[i] = adminid;
                    for (j = 0; j < i; j ++)
                        // test  duplicity
                        if (tc_rem[j] == adminid && ac_rem[j] > 0) {
                            tc_rem[j] = 0;
                            LOG(WARNING_LOG, "Contact [%s] duplicity", (const char *)tmpcontact_rem[i]);
                            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                    ccReg::domain_admin_rem, i,
                                    REASON_MSG_DUPLICITY_CONTACT);
                        }

                }
            }
        }

        if (strlen(nsset_chg) == 0)
            nssetid = 0; // not change nsset;
        else {
            if (nsset_chg[0] == 0x8)
                nssetid = -1; // backslash escape to  NULL value
            else {
                nssetid = getIdOfNSSet(action.getDB(), nsset_chg
                        , restricted_handles_, lock_epp_commands_);
                if (nssetid < 0) {
                    LOG(WARNING_LOG, "bad format of domain nsset [%s]", nsset_chg);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::domain_nsset, 1, REASON_MSG_BAD_FORMAT_NSSET_HANDLE);
                } else if (nssetid == 0) {
                    LOG(WARNING_LOG, "domain nsset not exist [%s]", nsset_chg);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::domain_nsset, 1, REASON_MSG_NSSET_NOTEXIST);
                }
            }
        }

        if (strlen(keyset_chg) == 0)
            keysetid = 0; // not change keyset
        else {
            // XXX ungly hack because i dont know how to write 0x8 on console :(
            // should be removed before going to standard usage
            if (keyset_chg[0] == 0x8 || keyset_chg[0] == '-')
                keysetid = -1;
            else {
                keysetid = getIdOfKeySet(action.getDB(), keyset_chg
                            , restricted_handles_, lock_epp_commands_);
                if (keysetid < 0) {
                    LOG(WARNING_LOG, "bad format of domain keyset [%s]", keyset_chg);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::domain_keyset, 1, REASON_MSG_KEYSET_NOTEXIST);
                } else if (keysetid == 0) {
                    LOG(WARNING_LOG, "domain keyset not exist [%s]", keyset_chg);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::domain_keyset, 1, REASON_MSG_KEYSET_NOTEXIST);
                }
            }
        }


        if (rifd_epp_update_domain_keyset_clear_ == true) {
            // if request contains change of nsset and no change to keyset
            // remove keyset from domain
            if (nssetid != 0 && keysetid == 0) {
                keysetid = -1;
            }
        }


        //  owner of domain
        if (strlen(registrant_chg) == 0) {
            contactid = 0; // not change owner
        } else {
            contactid = getIdOfContact(action.getDB(), registrant_chg, restricted_handles_
                    , lock_epp_commands_);
            if (contactid < 0) {
                LOG(WARNING_LOG, "bad format of registrar [%s]", registrant_chg);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_registrant, 1, REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
            } else if (contactid == 0) {
                LOG(WARNING_LOG, "domain registrar not exist [%s]", registrant_chg);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_registrant, 1, REASON_MSG_REGISTRANT_NOTEXIST);
            }

        }
        if (valexdate.length() > 0) {
            // Test for  enum domain
            if (GetZoneEnum(action.getDB(), zone) ) {
                if (action.getDB()->TestValExDate(valexdate.c_str(), 
                            GetZoneValPeriod(action.getDB(), zone),
                            DefaultValExpInterval() , id) == false) // test validace expirace
                {
                    LOG(WARNING_LOG, "DomainUpdate: validity exp date is not valid %s", valexdate.c_str());
                    code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
                            ccReg::domain_ext_valDate, 1, REASON_MSG_VALEXPDATE_NOT_VALID);
                }

            } else {
                LOG(WARNING_LOG, "DomainUpdate: can not validity exp date %s", valexdate.c_str());
                code = action.setErrorReason(COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                        ccReg::domain_ext_valDate, 1, REASON_MSG_VALEXPDATE_NOT_USED);

            }
        }
        if (strlen(registrant_chg) != 0) {
            try {
                if (!code && testObjectHasState(action,id,FLAG_serverRegistrantChangeProhibited))
                {
                    LOG( WARNING_LOG, "registrant change %s is prohibited" , fqdn );
                    code = COMMAND_STATUS_PROHIBITS_OPERATION;
                }
            } catch (...) {
                code = COMMAND_FAILED;
            }
        }
        if (code == 0) {

            // BEGIN notifier
            // notify default contacts
            ntf.reset(new EPPNotifier(
                          disable_epp_notifier_, mm, action.getDB(),
                          action.getRegistrar(), id, regMan.get()));

            for (i = 0; i < admin_add.length(); i++)
                ntf->AddAdminNew(ac_add[i]); // notifier new ADMIN contact

            //  NSSET change  if  NULL value   nssetid = -1
            if (nssetid != 0) {
                ntf->AddNSSetTechByDomain(id); // notifier tech-c old nsset
                if (nssetid > 0)
                    ntf->AddNSSetTech(nssetid); // tech-c changed nsset if not null
            }

            //KeySet change if NULL valus      keysetid = -1
            if (keysetid != 0) {
                ntf->AddKeySetTechByDomain(id); //notifier tech-c old keyset
                if (keysetid > 0)
                    ntf->AddKeySetTech(keysetid); //tech-c changed keyset if not null
            }

            // change owner of domain send to new registrant
            if (contactid)
                ntf->AddRegistrantNew(contactid);

            // END notifier

            // begin update
            if (action.getDB()->ObjectUpdate(id, action.getRegistrar(), authInfo_chg) ) {

                if (nssetid || contactid || keysetid) // update domain table only if change
                {
                    // change record of domain
                    action.getDB()->UPDATE("DOMAIN");

                    if (nssetid > 0)
                        action.getDB()->SET("nsset", nssetid); // change nssetu
                    else if (nssetid == -1)
                        action.getDB()->SETNULL("nsset"); // delete nsset

                    if (keysetid > 0)
                        action.getDB()->SET("keyset", keysetid); // change keyset
                    else if (keysetid == -1)
                        action.getDB()->SETNULL("keyset"); // delete keyset

                    if (contactid)
                        action.getDB()->SET("registrant", contactid); // change owner

                    action.getDB()->WHEREID(id);
                    if ( !action.getDB()->EXEC() )
                        code = COMMAND_FAILED;
                }

                if (code == 0) {

                    // change validity exdate  extension
                    if (GetZoneEnum(action.getDB(), zone)) {
                        if (valexdate.length() > 0 || publish != ccReg::DISCL_EMPTY) {
                            action.getDB()->UPDATE("enumval");
                            if (valexdate.length() > 0) {
                                LOG(NOTICE_LOG, "change valExpDate %s", valexdate.c_str());
                                action.getDB()->SET("ExDate", valexdate.c_str());
                            }
                            if (publish == ccReg::DISCL_DISPLAY) {
                                LOG(NOTICE_LOG, "change publish flag to YES");
                                action.getDB()->SET("publish", true);
                            }
                            if (publish == ccReg::DISCL_HIDE) {
                                LOG(NOTICE_LOG, "change publish flag to NO");
                                action.getDB()->SET("publish", false);
                            }
                            action.getDB()->WHERE("domainID", id);

                            if ( !action.getDB()->EXEC() )
                                code = COMMAND_FAILED;
                        }
                    }

                    // REM temp-c (must be befor ADD admin-c because of uniqueness)
                    for (i = 0; i < tmpcontact_rem.length(); i++) {
                        if ( (adminid = getIdOfContact(action.getDB(), tmpcontact_rem[i],
                                restricted_handles_, lock_epp_commands_) )) {
                            LOG( NOTICE_LOG , "delete temp-c-c  -> %d [%s]" , tc_rem[i] , (const char * ) tmpcontact_rem[i] );
                            if ( !action.getDB()->DeleteFromTableMap("domain", id, tc_rem[i]) ) {
                                code = COMMAND_FAILED;
                                break;
                            }
                        }

                    }

                    // ADD admin-c
                    for (i = 0; i < admin_add.length(); i++) {

                        LOG( DEBUG_LOG, "DomainUpdate: add admin Contact %s id %d " , (const char *) admin_add[i] , ac_add[i] );
                        if ( !action.getDB()->AddContactMap("domain", id, ac_add[i]) ) {
                            code = COMMAND_FAILED;
                            break;
                        }

                    }

                    // REM admin-c
                    for (i = 0; i < admin_rem.length(); i++) {
                        if ( (adminid = getIdOfContact(action.getDB(), admin_rem[i]
                             , restricted_handles_, lock_epp_commands_) )) {
                            LOG( NOTICE_LOG , "delete admin  -> %d [%s]" , ac_rem[i] , (const char * ) admin_rem[i] );
                            if ( !action.getDB()->DeleteFromTableMap("domain", id, ac_rem[i]) ) {
                                code = COMMAND_FAILED;
                                break;
                            }
                        }

                    }

                    // save to the history on the end if is OK
                    if (code == 0)
                        if (action.getDB()->SaveDomainHistory(id, params.requestID))
                            code = COMMAND_OK; // set up successfully


                }

            }

            // notifier send messages
            if (code == COMMAND_OK)
                action.setNotifier(ntf.get()); // schedule message send

        }

    }

    // EPP exception
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("DomainUpdate");
    }

    return action.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    DomainCreate
 *
 * DESCRIPTION: creation of domain record
 *
 * PARAMETERS:  fqdn - domain identifier, its name
 *              registrant -  domain holder
 *              nsset -  nsset identifier
 *              keyset - keyset identifier
 *              period - period of domain validity in mounths
 *              AuthInfoPw  -  password
 *              admin - sequence of administration contacts
 *        OUT:  crDate - date of object creation
 *        OUT:  exDate - date of object expiration
 *              params - common EPP parametres
 *              ext - ExtensionList
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::DomainCreate(
        const char *fqdn,
        const char *Registrant,
        const char *nsset,
        const char *keyset,
        const char *AuthInfoPw,
        const ccReg::Period_str& period,
        const ccReg::AdminContact & admin,
        ccReg::timestamp_out crDate,
        ccReg::timestamp_out exDate,
        const ccReg::EppParams &params,
        const ccReg::ExtensionList & ext
        )
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    std::auto_ptr<EPPNotifier> ntf;
    std::string valexdate;
    ccReg::Disclose publish;
    char FQDN[164];
    int contactid, nssetid, adminid, id, keysetid;
    int zone =0;
    unsigned int i, j;
    std::vector<int> ad;
    int period_count;
    char periodStr[10];
    Fred::NameIdPair dConflict;
    Fred::Domain::CheckAvailType dType;
    short int code = 0;

    crDate = CORBA::string_dup("");
    exDate = CORBA::string_dup("");

    Database::Connection conn = wrapped_acquire(this);
    EPPAction action(this, params.loginID, EPP_DomainCreate, static_cast<const char*>(params.clTRID), params.XML, conn, params.requestID);

    ad.resize(admin.length());

    LOGGER(PACKAGE).notice(boost::format("DomainCreate: clientID -> %1% clTRID [%2%] fqdn  [%3%] ") %
            (int ) params.loginID % (const char*)params.clTRID % fqdn );
    LOGGER(PACKAGE).notice(boost::format("DomainCreate:  Registrant  [%1%]  nsset [%2%]  keyset [%3%] AuthInfoPw [%4%]") %
            Registrant % nsset % keyset % AuthInfoPw);

    //  period transform from structure to month
    // if in year
    if (period.unit == ccReg::unit_year) {
        period_count = period.count * 12;
        snprintf(periodStr, sizeof(periodStr), "y%d", period.count);
    }
    // if in month
    else if (period.unit == ccReg::unit_month) {
        period_count = period.count;
        snprintf(periodStr, sizeof(periodStr), "m%d", period.count);
    } else
        period_count = 0;

    LOG( NOTICE_LOG,
            "DomainCreate: period count %d unit %d period_count %d string [%s]" ,
            period.count , period.unit , period_count , periodStr);

    // parse enum.exdate extension for validitydate
    extractEnumDomainExtension(valexdate, publish, ext);

    try {
        std::auto_ptr<Fred::Zone::Manager> zm( Fred::Zone::Manager::create() );
        std::auto_ptr<Fred::Domain::Manager> dman( Fred::Domain::Manager::create(action.getDB(),zm.get()) );

        LOG( NOTICE_LOG , "Domain::checkAvail  fqdn [%s]" , (const char * ) fqdn );

        dType = dman->checkAvail( ( const char * ) fqdn , dConflict);
        LOG( NOTICE_LOG , "domain type %d" , dType );
        switch (dType) {
            case Fred::Domain::CA_INVALID_HANDLE:
            case Fred::Domain::CA_BAD_LENGHT:
                LOG( NOTICE_LOG , "bad format %s of fqdn" , (const char * ) fqdn );
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_fqdn, 1, REASON_MSG_BAD_FORMAT_FQDN);
                break;
            case Fred::Domain::CA_REGISTRED:
            case Fred::Domain::CA_CHILD_REGISTRED:
            case Fred::Domain::CA_PARENT_REGISTRED:
                LOG( WARNING_LOG, "domain  [%s] EXIST", fqdn );
                code = COMMAND_OBJECT_EXIST; // if is exist
                break;
            case Fred::Domain::CA_BLACKLIST: // black listed
                LOG( NOTICE_LOG , "blacklisted  %s" , (const char * ) fqdn );
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_fqdn, 1, REASON_MSG_BLACKLISTED_DOMAIN);
                break;
            case Fred::Domain::CA_AVAILABLE: // if is free
                // conver fqdn to lower case and get zone
                zone = getFQDN(action.getDB(), FQDN, fqdn);
                LOG( NOTICE_LOG , "domain %s avail zone %d" ,(const char * ) FQDN , zone );
                break;
            case Fred::Domain::CA_BAD_ZONE:
                // domain not in zone
                LOG( NOTICE_LOG , "NOn in zone not applicable %s" , (const char * ) fqdn );
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_fqdn, 1, REASON_MSG_NOT_APPLICABLE_DOMAIN);
                break;
        }

        if (dType == Fred::Domain::CA_AVAILABLE) {

            if (action.getDB()->TestRegistrarZone(action.getRegistrar(), zone) == false) {
                LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
                code = COMMAND_AUTHENTICATION_ERROR;
            } else {

                if (strlen(nsset) == 0) {
                    nssetid = 0; // domain can be create without nsset
                } else {
                    nssetid = getIdOfNSSet( action.getDB(), nsset
                            , restricted_handles_, lock_epp_commands_);
                    if (nssetid < 0) {
                        LOG(WARNING_LOG, "bad format of domain nsset [%s]", nsset);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::domain_nsset, 1, REASON_MSG_BAD_FORMAT_NSSET_HANDLE);
                    } else if (nssetid == 0) {
                        LOG(WARNING_LOG, "domain nsset not exist [%s]", nsset);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::domain_nsset, 1, REASON_MSG_NSSET_NOTEXIST);
                    }
                }
                // KeySet
                if (strlen(keyset) == 0) {
                    keysetid = 0;
                } else {
                    keysetid = getIdOfKeySet(action.getDB(), keyset
                                , restricted_handles_, lock_epp_commands_);
                    if (keysetid < 0) {
                        LOG(WARNING_LOG, "bad format of domain keyset [%s]", keyset);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::domain_keyset, 1, REASON_MSG_BAD_FORMAT_KEYSET_HANDLE);
                    } else if (keysetid == 0) {
                        LOG(WARNING_LOG, "domain keyset not exist [%s]", keyset);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::domain_keyset, 1, REASON_MSG_KEYSET_NOTEXIST);
                    }
                }

                //  owner of domain
                if ( (contactid = getIdOfContact(action.getDB(), Registrant
                        , restricted_handles_, lock_epp_commands_) ) <= 0) {
                    if (contactid < 0) {
                        LOG(WARNING_LOG, "bad format of registrant [%s]", Registrant);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::domain_registrant, 1,
                                REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
                    } else if (contactid == 0) {
                        LOG(WARNING_LOG, "domain registrant not exist [%s]", Registrant);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::domain_registrant, 1,
                                REASON_MSG_REGISTRANT_NOTEXIST);
                    }
                }

                // default period if not set from zone parametrs
                if (period_count == 0) {
                    period_count = GetZoneExPeriodMin(action.getDB(), zone);
                    LOG( NOTICE_LOG, "get defualt peridod %d month  for zone   %d ", period_count , zone );
                }

                // test period validity range and modulo
                switch (TestPeriodyInterval(period_count,
                            GetZoneExPeriodMin(action.getDB(), zone) , GetZoneExPeriodMax(action.getDB(), zone) ) ) {
                    case 2:
                        LOG( WARNING_LOG, "period %d interval ot of range MAX %d MIN %d" , period_count , GetZoneExPeriodMax(action.getDB(),  zone ) , GetZoneExPeriodMin(action.getDB(),  zone ) );
                        code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
                                ccReg::domain_period, 1, REASON_MSG_PERIOD_RANGE);
                        break;
                    case 1:
                        LOG( WARNING_LOG, "period %d  interval policy error MIN %d" , period_count , GetZoneExPeriodMin(action.getDB(),  zone ) );
                        code = action.setErrorReason(COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                                ccReg::domain_period, 1, REASON_MSG_PERIOD_POLICY);
                        break;

                }

                // test  validy date for enum domain
                if (valexdate.length() == 0) {
                    // for enum domain must set validity date
                    if (GetZoneEnum(action.getDB(), zone) ) {
                        LOG( WARNING_LOG, "DomainCreate: validity exp date MISSING" );
                        code = action.setErrorReason(COMMAND_PARAMETR_MISSING,
                                ccReg::domain_ext_valDate_missing, 0,
                                REASON_MSG_VALEXPDATE_REQUIRED);
                    }
                } else {
                    // Test for enum domain
                    if (GetZoneEnum(action.getDB(), zone) ) {
                        // test
                        if (action.getDB()->TestValExDate(valexdate.c_str(),
                                    GetZoneValPeriod(action.getDB(), zone) , DefaultValExpInterval() , 0)
                                == false) {
                            LOG(WARNING_LOG, "Validity exp date is not valid %s", valexdate.c_str());
                            code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
                                    ccReg::domain_ext_valDate, 1,
                                    REASON_MSG_VALEXPDATE_NOT_VALID);
                        }
                    } else {
                        LOG(WARNING_LOG, "Validity exp date %s not used", valexdate.c_str());
                        code = action.setErrorReason(COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                                ccReg::domain_ext_valDate, 1,
                                REASON_MSG_VALEXPDATE_NOT_USED);
                    }

                }

                // test   admin-c if set

                if (admin.length() > 0) {
                    // test
                    for (i = 0; i < admin.length(); i++) {
                        adminid = getIdOfContact( action.getDB(), admin[i]
                                    , restricted_handles_, lock_epp_commands_);
                        if (adminid < 0) {
                            LOG(WARNING_LOG, "bad format of contact %", (const char *)admin[i]);
                            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                    ccReg::domain_admin, i + 1,
                                    REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
                        } else if (adminid == 0) {
                            LOG(WARNING_LOG, "contact %s not exist", (const char *)admin[i]);
                            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                    ccReg::domain_admin, i + 1,
                                    REASON_MSG_ADMIN_NOTEXIST);
                        } else {
                            ad[i] = adminid;
                            for (j = 0; j < i; j ++) // test  duplicity
                            {
                                LOG( DEBUG_LOG , "admin comapare j %d adminid %d ad %d" , j , adminid , ad[j] );
                                if (ad[j] == adminid && ad[j] > 0) {
                                    ad[j] = 0;
                                    LOG(WARNING_LOG, "Contact [%s] duplicity", (const char *)admin[i]);
                                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                            ccReg::domain_admin, i, REASON_MSG_DUPLICITY_CONTACT);
                                }
                            }

                        }
                    }

                }

                if (code == 0) // if not error
                {

                    id= action.getDB()->CreateObject("D", action.getRegistrar(), FQDN, AuthInfoPw);
                    if (id<=0) {
                        if (id == 0) {
                            LOG( WARNING_LOG, "domain fqdn [%s] EXIST", fqdn );
                            code= COMMAND_OBJECT_EXIST;
                        } else {
                            LOG( WARNING_LOG, "Cannot insert [%s] into object_registry", fqdn );
                            code= COMMAND_FAILED;
                        }
                    } else {

                        std::string computed_exdate;
                        try {
                            /* compute expiration date from creation time - need
                             * proper conversion to local time then take only date
                             */
                            using namespace boost::posix_time;

                            ptime db_crdate = time_from_string(action.getDB()->GetValueFromTable("object_registry", "crdate", "id", id));
                            ptime local_crdate = boost::date_time::c_local_adjustor<ptime>::utc_to_local(db_crdate);
                            date local_exdate = (local_crdate + months(period_count)).date();
                            computed_exdate = to_iso_extended_string(local_exdate);

                            LOG(DEBUG_LOG, "db crdate: [%s] local create date: [%s] computed exdate: [%s]",
                                    to_simple_string(db_crdate).c_str(),
                                    to_simple_string(local_crdate).c_str(),
                                    computed_exdate.c_str());
                        }
                        catch (std::exception &ex) {
                            /* on error log and rethrow again - we can't create
                             * domain without expiration date
                             */
                            LOG(DEBUG_LOG, "time convert error: %s", ex.what());
                            throw;
                        }

                        action.getDB()->INSERT("DOMAIN");
                        action.getDB()->INTO("id");
                        action.getDB()->INTO("zone");
                        action.getDB()->INTO("Exdate");
                        action.getDB()->INTO("Registrant");
                        action.getDB()->INTO("nsset");
                        action.getDB()->INTO("keyset");

                        action.getDB()->VALUE(id);
                        action.getDB()->VALUE(zone);
                        action.getDB()->VALUE(computed_exdate.c_str());
                        //action.getDB()->VALUEPERIOD(period_count); // actual time plus interval of period in months
                        action.getDB()->VALUE(contactid);
                        if (nssetid == 0)
                            action.getDB()->VALUENULL(); // domain without  nsset write NULL value
                        else
                            action.getDB()->VALUE(nssetid);

                        if (keysetid == 0)
                            action.getDB()->VALUENULL();
                        else
                            action.getDB()->VALUE(keysetid);

                        if (action.getDB()->EXEC() ) {

                            // get local timestamp of created domain
                            CORBA::string_free(crDate);
                            crDate= CORBA::string_dup(action.getDB()->GetObjectCrDateTime(id) );

                            //  get local date of expiration
                            CORBA::string_free(exDate);
                            exDate = CORBA::string_dup(action.getDB()->GetDomainExDate(id) );

                            // save  enum domain   extension validity date
                            if (GetZoneEnum(action.getDB(), zone) && valexdate.length() > 0) {
                                action.getDB()->INSERT("enumval");
                                action.getDB()->VALUE(id);
                                action.getDB()->VALUE(valexdate.c_str());
                                if (publish == ccReg::DISCL_DISPLAY) {
                                    action.getDB()->VALUE(true);
                                }
                                if (publish == ccReg::DISCL_HIDE) {
                                    action.getDB()->VALUE(false);
                                }

                                if (action.getDB()->EXEC() == false)
                                    code = COMMAND_FAILED;
                            }

                            // insert   admin-c
                            for (i = 0; i < admin.length(); i++) {
                                LOG( DEBUG_LOG, "DomainCreate: add admin Contact %s id %d " , (const char *) admin[i] , ad[i] );
                                if ( !action.getDB()->AddContactMap("domain", id, ad[i]) ) {
                                    code = COMMAND_FAILED;
                                    break;
                                }

                            }
                            std::auto_ptr<Fred::Invoicing::Manager> invMan(
                                    Fred::Invoicing::Manager::create());
                            if (invMan->chargeDomainCreate(zone, action.getRegistrar(),
                                        id, Database::Date(std::string(exDate)), period_count)) {

                                if(!invMan->chargeDomainRenew(zone, action.getRegistrar(),
                                        id, Database::Date(std::string(exDate)), period_count)) {
                                    code = COMMAND_BILLING_FAILURE;
                                }

                                else if (action.getDB()->SaveDomainHistory(id, params.requestID)) {
                                    if (action.getDB()->SaveObjectCreate(id)) {
                                        code = COMMAND_OK;
                                    }
                                } else {
                                    code = COMMAND_FAILED;
                                }
                            } else {
                                code = COMMAND_BILLING_FAILURE;
                            }

                        } else
                            code = COMMAND_FAILED;

                            if (code == COMMAND_OK) // run notifier
                            {
                                ntf.reset(new EPPNotifier(
                                            disable_epp_notifier_,
                                            mm , action.getDB(), action.getRegistrar(), id ));
                                action.setNotifier(ntf.get());
                            }

                    }
                }

            }

        }
    }
    catch (...) {
        LOG( WARNING_LOG, "cannot run Fred::Domain::checkAvail");
        code=COMMAND_FAILED;
    }


    // EPP exception
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("DomainCreate");
    }

    return action.getRet()._retn();
}

/***********************************************************************
 *
 * FUNCTION:    DomainRenew
 *
 * DESCRIPTION: domain validity renewal of aperiod and
 *		save changes into history
 * PARAMETERS:  fqdn - domain name of nssetu
 *              curExpDate - date of domain expiration !!! time in a GMT format 00:00:00
 *              period - period of renewal in mounths
 *        OUT:  exDate - date and time of new domain expiration
 *              params - common EPP parametres
 *              ext - ExtensionList
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response *
ccReg_EPP_i::DomainRenew(const char *fqdn, const char* curExpDate,
        const ccReg::Period_str& period, ccReg::timestamp_out exDate,
        const ccReg::EppParams &params, const ccReg::ExtensionList & ext)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    std::auto_ptr<EPPNotifier> ntf;
    std::string valexdate;
    ccReg::Disclose publish;
    int id, zone;
    int period_count;
    char periodStr[10];
    short int code = 0;


    Database::Connection conn = wrapped_acquire(this);
    EPPAction action(this, params.loginID, EPP_DomainRenew, static_cast<const char*>(params.clTRID), params.XML, conn, params.requestID);

    // default
    exDate = CORBA::string_dup("");

    LOGGER(PACKAGE).notice(boost::format("DomainRenew: clientID -> %1% clTRID [%2%] fqdn  [%3%] curExpDate [%4%]") % (int ) params.loginID % (const char*)params.clTRID % fqdn % (const char *) curExpDate );

    //  period count
    if (period.unit == ccReg::unit_year) {
        period_count = period.count * 12;
        snprintf(periodStr, sizeof(periodStr), "y%d", period.count);
    } else if (period.unit == ccReg::unit_month) {
        period_count = period.count;
        snprintf(periodStr, sizeof(periodStr), "m%d", period.count);
    } else
        period_count = 0; // use default value

    LOG( NOTICE_LOG, "DomainRenew: period count %d unit %d period_count %d string [%s]" , period.count , period.unit , period_count , periodStr);

    // parse enum.ExDate extension
    extractEnumDomainExtension(valexdate, publish, ext);

    if ((id = getIdOfDomain(action.getDB(), fqdn, lock_epp_commands_
            , true, &zone) ) <= 0) {
        LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
        code=COMMAND_OBJECT_NOT_EXIST;
    }
    else  if (action.getDB()->TestRegistrarZone(action.getRegistrar(), zone) == false) {
        LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
        code = COMMAND_AUTHENTICATION_ERROR;
    }
    // test curent ExDate
    // there should be lock here for row with exdate
    // but there is already lock on object_registry row
    // (from getIdOfDomain) and so there cannot be any race condition
    else if (TestExDate(curExpDate, action.getDB()->GetDomainExDate(id)) == false) {
        LOG( WARNING_LOG, "curExpDate is not same as ExDate" );
        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                ccReg::domain_curExpDate, 1,
                REASON_MSG_CUREXPDATE_NOT_EXPDATE);
    } else {
        // set default renew  period from zone params
        if (period_count == 0) {
            period_count = GetZoneExPeriodMin(action.getDB(), zone);
            LOG( NOTICE_LOG, "get default peridod %d month  for zone   %d ", period_count , zone );
        }

        //  test period
        switch (TestPeriodyInterval(period_count,
                    GetZoneExPeriodMin(action.getDB(), zone) , GetZoneExPeriodMax(action.getDB(), zone) ) ) {
            case 2:
                LOG( WARNING_LOG, "period %d interval ot of range MAX %d MIN %d" , period_count , GetZoneExPeriodMax(action.getDB(),  zone ) , GetZoneExPeriodMin(action.getDB(),  zone ) );
                code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
                        ccReg::domain_period, 1,
                        REASON_MSG_PERIOD_RANGE);
                break;
            case 1:
                LOG( WARNING_LOG, "period %d  interval policy error MIN %d" , period_count , GetZoneExPeriodMin(action.getDB(),  zone ) );
                code = action.setErrorReason(COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                        ccReg::domain_period, 1,
                        REASON_MSG_PERIOD_POLICY);
                break;
            default:
                // count new  ExDate
                if (action.getDB()->CountExDate(id, period_count,
                            GetZoneExPeriodMax(action.getDB(), zone) ) == false) {
                    LOG( WARNING_LOG, "period %d ExDate out of range" , period_count );
                    code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
                            ccReg::domain_period, 1,
                            REASON_MSG_PERIOD_RANGE);
                }
                break;

        }

        // test validity Date for enum domain
        if (valexdate.length() > 0) {
            // Test for enum domain only
            if (GetZoneEnum(action.getDB(), zone) ) {
                if (action.getDB()->TestValExDate(valexdate.c_str(),
                            GetZoneValPeriod(action.getDB(), zone) , DefaultValExpInterval() , id)
                        == false) {
                    LOG(WARNING_LOG, "Validity exp date is not valid %s", valexdate.c_str());
                    code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
                            ccReg::domain_ext_valDate, 1,
                            REASON_MSG_VALEXPDATE_NOT_VALID);
                }

            } else {

                LOG(WARNING_LOG, "Can not validity exp date %s", valexdate.c_str());
                code = action.setErrorReason(COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                        ccReg::domain_ext_valDate, 1,
                        REASON_MSG_VALEXPDATE_NOT_USED);

            }
        }

        if (code == 0)// if not param error
        {
            // test client of the object
            if ( !action.getDB()->TestObjectClientID(id, action.getRegistrar()) ) {
                LOG( WARNING_LOG, "bad autorization not client of domain [%s]", fqdn );
                code = action.setErrorReason(COMMAND_AUTOR_ERROR,
                        ccReg::registrar_autor, 0,
                        REASON_MSG_REGISTRAR_AUTOR);
            }
            try {
                if (!code && (
                            testObjectHasState(action,id,FLAG_serverRenewProhibited) ||
                            testObjectHasState(action,id,FLAG_deleteCandidate)
                            )
                   )
                {
                    LOG( WARNING_LOG, "renew of object %s is prohibited" , fqdn );
                    code = COMMAND_STATUS_PROHIBITS_OPERATION;
                }
            } catch (...) {
                code = COMMAND_FAILED;
            }

            if (!code) {

                // change validity date for enum domain
                if (GetZoneEnum(action.getDB(), zone) ) {
                    if (valexdate.length() > 0 || publish != ccReg::DISCL_EMPTY) {
                        action.getDB()->UPDATE("enumval");
                        if (valexdate.length() > 0) {
                            LOG(NOTICE_LOG, "change valExpDate %s", valexdate.c_str());
                            action.getDB()->SET("ExDate", valexdate.c_str());
                        }
                        if (publish == ccReg::DISCL_DISPLAY) {
                            LOG(NOTICE_LOG, "change publish flag to YES");
                            action.getDB()->SET("publish", true);
                        }
                        if (publish == ccReg::DISCL_HIDE) {
                            LOG(NOTICE_LOG, "change publish flag to NO");
                            action.getDB()->SET("publish", false);
                        }
                        action.getDB()->WHERE("domainID", id);

                        if (action.getDB()->EXEC() == false)
                            code = COMMAND_FAILED;
                    }
                }

                if (code == 0) // if is OK OK
                {

                    // make Renew Domain count new Exdate in
                    if (action.getDB()->RenewExDate(id, period_count) ) {
                        //  return new Exdate as local date
                        CORBA::string_free(exDate);
                        exDate = CORBA::string_dup(action.getDB()->GetDomainExDate(id) );



                        std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());
                        if (invMan->chargeDomainRenew(zone, action.getRegistrar(),
                                    id, Database::Date(std::string(exDate)), period_count) == false ) {
                            code = COMMAND_BILLING_FAILURE;
                        } else if (action.getDB()->SaveDomainHistory(id, params.requestID)) {
                            code = COMMAND_OK;
                        }

                    } else
                        code = COMMAND_FAILED;
                }
            }
        }
    }
    if (code == COMMAND_OK) // run notifier
    {
        ntf.reset(new EPPNotifier(disable_epp_notifier_,mm , action.getDB(), action.getRegistrar() , id ));
        action.setNotifier(ntf.get());
    }
    // EPP exception
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("DomainRenew");
    }

    return action.getRet()._retn();
}

/*************************************************************
 *
 * Function:    KeySetInfo
 *
 *************************************************************/
ccReg::Response *
ccReg_EPP_i::KeySetInfo(
        const char *handle,
        ccReg::KeySet_out k,
        const ccReg::EppParams &params)
{
  Logging::Context::clear();
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
  ConnectionReleaser releaser;

    LOG(NOTICE_LOG, "KeySetInfo: clientID -> %llu clTRID [%s] handle [%s] ",
            params.loginID, static_cast<const char*>(params.clTRID), handle);

    EPPAction a(this, params.loginID, EPP_KeySetInfo, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    std::auto_ptr<Fred::KeySet::Manager> kman(
            Fred::KeySet::Manager::create(
                a.getDB(), restricted_handles_)
            );
    // first check handle for proper format
    if (!kman->checkHandleFormat(handle))
        a.failed(SetReasonKeySetHandle(
                    a.getErrors(), handle, a.getLang())
                );

    // load keyset by handle
    std::auto_ptr<Fred::KeySet::List> klist(kman->createList());

    Database::Filters::Union unionFilter;
    Database::Filters::KeySet *keyFilter = new Database::Filters::KeySetHistoryImpl();

    keyFilter->addHandle().setValue(std::string(handle));
    keyFilter->addDeleteTime().setNULL();
    unionFilter.addFilter(keyFilter);

    klist->reload(unionFilter);


    //klist->setHandleFilter(handle);
    // try {
        // klist->reload();
    // } catch (...) {
        // a.failedInternal("Cannot load keyset");
    // }
    if (klist->getCount() != 1)
        // failed because of non existence
        a.failed(COMMAND_OBJECT_NOT_EXIST);

    Fred::KeySet::KeySet *kss = klist->getKeySet(0);
    k = new ccReg::KeySet;

    //fill common object data
    k->ROID = CORBA::string_dup(kss->getROID().c_str());
    k->CrDate = CORBA::string_dup(formatTime(kss->getCreateDate()).c_str());
    k->UpDate = CORBA::string_dup(formatTime(kss->getUpdateDate()).c_str());
    k->TrDate = CORBA::string_dup(formatTime(kss->getTransferDate()).c_str());
    k->ClID = CORBA::string_dup(kss->getRegistrarHandle().c_str());
    k->CrID = CORBA::string_dup(kss->getCreateRegistrarHandle().c_str());
    k->UpID = CORBA::string_dup(kss->getUpdateRegistrarHandle().c_str());
    //authinfo is filled only if session registrar is also owner
    k->AuthInfoPw = CORBA::string_dup(
            a.getRegistrar() == (int)kss->getRegistrarId() ?
            kss->getAuthPw().c_str() : "");


    // states
    for (unsigned int i = 0; i < kss->getStatusCount(); i++) {
        Fred::TID stateId = kss->getStatusByIdx(i)->getStatusId();
        const Fred::StatusDesc *sd = regMan->getStatusDesc(stateId);
        if (!sd || !sd->getExternal())
            continue;
        k->stat.length(k->stat.length() + 1);
        k->stat[k->stat.length()-1].value =
            CORBA::string_dup(sd->getName().c_str());
        k->stat[k->stat.length()-1].text =
            CORBA::string_dup(sd->getDesc(
                        a.getLang() == LANG_CS ? "CS" : "EN").c_str()
                    );
    }

    if (!k->stat.length()) {
        const Fred::StatusDesc *sd = regMan->getStatusDesc(0);
        if (sd) {
            k->stat.length(1);
            k->stat[0].value = CORBA::string_dup(sd->getName().c_str());
            k->stat[0].text = CORBA::string_dup(
                    sd->getDesc(a.getLang() == LANG_CS ? "CS" : "EN").c_str());
        }
    }

    // keyset specific data
    k->handle = CORBA::string_dup(kss->getHandle().c_str());
    k->tech.length(kss->getAdminCount());
    for (unsigned int i = 0; i < kss->getAdminCount(); i++)
        k->tech[i] = CORBA::string_dup(kss->getAdminByIdx(i).c_str());


    // dsrecord
    k->dsrec.length(kss->getDSRecordCount());
    for (unsigned int i = 0; i < kss->getDSRecordCount(); i++) {
        const Fred::KeySet::DSRecord *dsr = kss->getDSRecordByIdx(i);
        k->dsrec[i].keyTag = dsr->getKeyTag();
        k->dsrec[i].alg = dsr->getAlg();
        k->dsrec[i].digestType = dsr->getDigestType();
        k->dsrec[i].digest = CORBA::string_dup(dsr->getDigest().c_str());
        k->dsrec[i].maxSigLife = dsr->getMaxSigLife();
    }

    // dnskey record
    k->dnsk.length(kss->getDNSKeyCount());
    for (unsigned int i = 0; i < kss->getDNSKeyCount(); i++) {
        const Fred::KeySet::DNSKey *dnsk = kss->getDNSKeyByIdx(i);
        k->dnsk[i].flags = dnsk->getFlags();
        k->dnsk[i].protocol = dnsk->getProtocol();
        k->dnsk[i].alg = dnsk->getAlg();
        k->dnsk[i].key = CORBA::string_dup(dnsk->getKey().c_str());
    }

    return a.getRet()._retn();
}

/*************************************************************
 *
 * Function:    KeySetDelete
 *
 *************************************************************/

ccReg::Response *
ccReg_EPP_i::KeySetDelete(
        const char *handle,
        const ccReg::EppParams &params)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;

    int                 id;
    std::auto_ptr<EPPNotifier> ntf;
    short int code = 0;

    EPPAction action(this, params.loginID, EPP_KeySetDelete, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    LOGGER(PACKAGE).notice( boost::format("KeySetDelete: clientID -> %1% clTRID [%2%] handle [%3%]") %
            (int)params.loginID % (const char*)params.clTRID % handle);

    id = getIdOfKeySet(action.getDB(), handle, restricted_handles_
            , lock_epp_commands_, true);
    if (id < 0) {
        LOG(WARNING_LOG, "bad format of keyset [%s]", handle);
        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                ccReg::keyset_handle, 1,
                REASON_MSG_BAD_FORMAT_KEYSET_HANDLE);
    } else if (id == 0) {
        LOG(WARNING_LOG, "KeySet handle [%s] NOT_EXISTS", handle);
        code = COMMAND_OBJECT_NOT_EXIST;
    }
    if (!code && !action.getDB()->TestObjectClientID(id, action.getRegistrar())) {
        LOG(WARNING_LOG, "bad authorisation not client of KeySet [%s]", handle);
        code = action.setErrorReason(COMMAND_AUTOR_ERROR,
                ccReg::registrar_autor, 0, REASON_MSG_REGISTRAR_AUTOR);
    }
    try {
        if (!code && (
                    testObjectHasState(action, id, FLAG_serverDeleteProhibited) ||
                    testObjectHasState(action, id, FLAG_serverUpdateProhibited) ||
                    testObjectHasState(action,id,FLAG_deleteCandidate)
                    )) {
            LOG(WARNING_LOG, "delete of object %s is prohibited", handle);
            code = COMMAND_STATUS_PROHIBITS_OPERATION;
        }
    } catch (...) {
        code = COMMAND_FAILED;
    }
    if (!code) {
        ntf.reset(new EPPNotifier(
                      disable_epp_notifier_,
                      mm,
                      action.getDB(),
                      action.getRegistrar(),
                      id));
        if (action.getDB()->TestKeySetRelations(id)) {
            LOG(WARNING_LOG, "KeySet can't be deleted - relations in db");
            code = COMMAND_PROHIBITS_OPERATION;
        } else {
            if (action.getDB()->SaveObjectDelete(id))
                if (action.getDB()->DeleteKeySetObject(id))
                    code = COMMAND_OK;
        }
        if (code == COMMAND_OK)
            action.setNotifier(ntf.get());
    }
    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }
    if (code == 0) {
        action.failedInternal("KeySetDelete");
    }
    return action.getRet()._retn();
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

/*************************************************************
 *
 * Function:    KeySetCreate
 *
 *************************************************************/
ccReg::Response *
ccReg_EPP_i::KeySetCreate(
        const char *handle,
        const char *authInfoPw,
        const ccReg::TechContact &tech,
        const ccReg::DSRecord &dsrec,
        const ccReg::DNSKey &dnsk,
        ccReg::timestamp_out crDate,
        const ccReg::EppParams &params)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
    ConnectionReleaser releaser;
    std::auto_ptr<EPPNotifier>  ntf;
    int                         id, techid, dsrecID;
    unsigned int                i, j;
    int                         *tch = NULL;
    short int                   code = 0;

    LOGGER(PACKAGE).notice( boost::format("KeySetCreate: clientID -> %1% clTRID [%2%] handle [%3%] authInfoPw [%4%]") %
            (int)params.loginID % (const char*)params.clTRID % handle % authInfoPw);

    crDate = CORBA::string_dup("");

    EPPAction action(this, params.loginID, EPP_KeySetCreate, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    std::auto_ptr<Fred::KeySet::Manager> keyMan(
            Fred::KeySet::Manager::create(
                action.getDB(),
                restricted_handles_)
            );
    if (tech.length() < 1) {
        LOG(WARNING_LOG, "KeySetCreate: not any tech contact ");
        code = action.setErrorReason(
                COMMAND_PARAMETR_MISSING,
                ccReg::keyset_tech,
                0,
                REASON_MSG_TECH_NOTEXIST);
    } else if (tech.length() > 10) {
        LOG(WARNING_LOG, "KeySetCreate: too many tech contacts (maximum is 10)");
        code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
                ccReg::keyset_tech, 0, REASON_MSG_TECHADMIN_LIMIT);
    } else if (dnsk.length() < 1) {
            LOG(WARNING_LOG, "KeySetCreate: not any DNSKey record");
            code = action.setErrorReason(COMMAND_PARAMETR_MISSING,
                    ccReg::keyset_dnskey, 0, REASON_MSG_NO_DNSKEY);
    } else if (dsrec.length() > 0) {
        LOG(WARNING_LOG, "KeySetCreate: too many ds-records (maximum is 0)");
        code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
                ccReg::keyset_dsrecord, 0, REASON_MSG_DSRECORD_LIMIT);
    } else if (dnsk.length() > 10) {
        LOG(WARNING_LOG, "KeySetCreate: too many dnskeys (maximum is 10)");
        code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
                ccReg::keyset_dnskey, 0, REASON_MSG_DNSKEY_LIMIT);
    }
    if (code == 0) {
        Fred::KeySet::Manager::CheckAvailType caType;

        tch = new int[tech.length()];

        try {
            Fred::NameIdPair nameId;
            caType = keyMan->checkAvail(handle, nameId);
            id = nameId.id;
        } catch (...) {
            caType = Fred::KeySet::Manager::CA_INVALID_HANDLE;
            id = -1;
        }

        if (id < 0 || caType == Fred::KeySet::Manager::CA_INVALID_HANDLE) {
            LOG(WARNING_LOG, "Bad format of keyset handle [%s]", handle);
            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                    ccReg::keyset_handle, 1, REASON_MSG_BAD_FORMAT_KEYSET_HANDLE);
        } else if (caType == Fred::KeySet::Manager::CA_REGISTRED) {
            LOG(WARNING_LOG, "KeySet handle [%s] EXISTS", handle);
            code = COMMAND_OBJECT_EXIST;
        } else if (caType == Fred::KeySet::Manager::CA_PROTECTED) {
            LOG(WARNING_LOG, "object [%s] in history period", handle);
            code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                    ccReg::keyset_handle, 1, REASON_MSG_PROTECTED_PERIOD);
        }
    }

    if (code == 0) {
        // test technical contact
        std::auto_ptr<Fred::Contact::Manager> cman(
                Fred::Contact::Manager::create(
                    action.getDB(),
                    restricted_handles_)
                );
        for (i = 0; i < tech.length(); i++) {
            Fred::Contact::Manager::CheckAvailType caType;
            try {
                Fred::NameIdPair nameId;
                caType = cman->checkAvail((const char *)tech[i], nameId);
                techid = nameId.id;
            } catch (...) {
                caType = Fred::Contact::Manager::CA_INVALID_HANDLE;
                techid = 0;
            }

            if (caType != Fred::Contact::Manager::CA_REGISTRED) {
                LOG(DEBUG_LOG, "Tech contact doesn't exist: %s",
                        (const char *)tech[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_tech, i + 1, REASON_MSG_TECH_NOTEXIST);
            }
            else {
                tch[i] = techid;
                //duplicity test
                for (j = 0; j < i; j++) {
                    LOG(DEBUG_LOG, "tech compare j %d techid %d and %d",
                            j, techid, tch[j]);
                    if (tch[j] == techid && tch[j] > 0) {
                        tch[j] = 0;
                        LOG(WARNING_LOG, "Contact [%s] duplicity",
                                (const char *)tech[i]);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::keyset_tech, i, REASON_MSG_DUPLICITY_CONTACT);
                    }
                }
            }
        }
    }

    // dsrecord digest type test
    if (code == 0) {
        // digest type must be 1 (sha-1) - see RFC 4034 for details:
        // http://rfc-ref.org/RFC-TEXTS/4034/kw-dnssec_digest_type.html
        for (int ii = 0; ii < (int)dsrec.length(); ii++) {
            if (dsrec[ii].digestType != 1) {
                LOG(WARNING_LOG,
                        "Digest is %d (must be 1)",
                        dsrec[ii].digestType);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_dsrecord, ii,
                        REASON_MSG_DSRECORD_BAD_DIGEST_TYPE);
                break;
            }
        }
    }
    // dsrecord digest length test
    if (code == 0) {
        // digest must be 40 characters length (because SHA-1 is used)
        for (int ii = 0; ii < (int)dsrec.length(); ii++) {
            if (strlen(dsrec[ii].digest) != 40) {
                LOG(WARNING_LOG,
                        "Digest length is %d char (must be 40)",
                        strlen(dsrec[ii].digest));
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_dsrecord, ii,
                        REASON_MSG_DSRECORD_BAD_DIGEST_LENGTH);
                break;
            }
        }
    }
    // dsrecord duplicity test
    if (code == 0) {
        if (dsrec.length() >= 2) {
            for (int ii = 0; ii < (int)dsrec.length(); ii++) {
                for (int jj = ii + 1; jj < (int)dsrec.length(); jj++) {
                    if (testDSRecordDuplicity(dsrec[ii], dsrec[jj])) {
                        LOG(WARNING_LOG,
                                "Found DSRecord duplicity: %d x %d (%d %d %d '%s' %d)",
                                ii, jj, dsrec[ii].keyTag, dsrec[ii].alg, dsrec[ii].digestType,
                                (const char *)dsrec[ii].digest, dsrec[ii].maxSigLife);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::keyset_dsrecord, jj, REASON_MSG_DUPLICITY_DSRECORD);
                        break;
                    }
                }
            }
        }
    }

    // dnskey flag field (must be 0, 256 or 267)
    // http://rfc-ref.org/RFC-TEXTS/4034/kw-flags_field.html
    if (code == 0) {
        for (int ii = 0; ii < (int)dnsk.length(); ii++) {
            if (!(dnsk[ii].flags == 0 || dnsk[ii].flags == 256 || dnsk[ii].flags == 257)) {
                LOG(WARNING_LOG,
                        "dnskey flag is %d (must be 0, 256 or 257)",
                        dnsk[ii].flags);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_dnskey, ii, REASON_MSG_DNSKEY_BAD_FLAGS);
                break;
            }
        }
    }
    // dnskey protocol field (must be 3)
    // http://rfc-ref.org/RFC-TEXTS/4034/kw-protocol_field.html
    if (code == 0) {
        for (int ii = 0; ii < (int)dnsk.length(); ii++) {
            if (dnsk[ii].protocol != 3) {
                LOG(WARNING_LOG,
                        "dnskey protocol is %d (must be 3)",
                        dnsk[ii].protocol);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_dnskey, ii, REASON_MSG_DNSKEY_BAD_PROTOCOL);
                break;
            }
        }
    }
    // Ticket #4432 - algorithm validity removed
    //
    // dnskey algorithm type (must be 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 252, 253, 254 or 255)
    // http://www.bind9.net/dns-sec-algorithm-numbers
    // http://rfc-ref.org/RFC-TEXTS/4034/kw-dnssec_algorithm_type.html
    // http://rfc-ref.org/RFC-TEXTS/4034/chapter7.html#d4e446172
    // if (code == 0) {
    //     for (int ii = 0; ii < (int)dnsk.length(); ii++) {
    //         if (!((dnsk[ii].alg >= 1 && dnsk[ii].alg <= 8) ||
    //                     (dnsk[ii].alg == 10) || (dnsk[ii].alg == 12) ||
    //                     (dnsk[ii].alg >= 252 && dnsk[ii].alg <=255))) {
    //             LOG(WARNING_LOG,
    //                     "dnskey algorithm is %d (must be 1,2,3,4,5,6,7,8,10,12,252,253,254 or 255)",
    //                     dnsk[ii].alg);
    //             code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
    //                     ccReg::keyset_dnskey, ii, REASON_MSG_DNSKEY_BAD_ALG);
    //             break;
    //         }
    //     }
    // }
    // test if key is valid base64 encoded string
    if (code == 0) {
        int ret1, ret2;
        for (int ii = 0; ii < (int)dnsk.length(); ii++) {
            if ((ret1 = isValidBase64((const char *)dnsk[ii].key, &ret2)) != BASE64_OK) {
                if (ret1 == BASE64_BAD_LENGTH) {
                    LOG(WARNING_LOG, "dnskey key length is wrong (must be dividable by 4)");
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::keyset_dnskey, ii, REASON_MSG_DNSKEY_BAD_KEY_LEN);
                } else if (ret1 == BASE64_BAD_CHAR) {
                    LOG(WARNING_LOG, "dnskey key contain invalid character '%c' at position %d",
                            ((const char *)dnsk[ii].key)[ret2], ret2);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::keyset_dnskey, ii, REASON_MSG_DNSKEY_BAD_KEY_CHAR);
                } else {
                    LOG(WARNING_LOG, "isValidBase64() return unknown value (%d)",
                            ret1);
                    code = COMMAND_FAILED;
                }
            }
        }
    }
    // dnskey duplicity test
    if (code == 0) {
        if (dnsk.length() >= 2) {
            for (int ii = 0; ii < (int)dnsk.length(); ii++) {
                for (int jj = ii + 1; jj < (int)dnsk.length(); jj++) {
                    if (testDNSKeyDuplicity(dnsk[ii], dnsk[jj])) {
                        LOG(WARNING_LOG,
                                "Found DSNKey duplicity: %d x %d (%d %d %d %s)",
                                ii, jj, dnsk[ii].flags, dnsk[ii].protocol,
                                dnsk[ii].alg, (const char *)dnsk[ii].key);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::keyset_dnskey, jj, REASON_MSG_DUPLICITY_DNSKEY);
                        break;
                    }
                }
            }
        }
    }

    // keyset creating
    if (code == 0) {
        // id = DBsql.CreateObject("K", regID, handle, authInfoPw);
        id = action.getDB()->CreateObject("K", action.getRegistrar(),
                handle, authInfoPw);
        if (id <= 0) {
            if (id == 0) {
                LOG(WARNING_LOG, "KeySet handle [%s] EXISTS", handle);
                code = COMMAND_OBJECT_EXIST;
            } else {
                LOG(WARNING_LOG, "Cannot insert [%s] into object_registry", handle);
                code = COMMAND_FAILED;
            }
        } else {
            action.getDB()->INSERT("keyset");
            action.getDB()->INTO("id");
            action.getDB()->VALUE(id);

            if (!action.getDB()->EXEC())
                code = COMMAND_FAILED;
            else {
                CORBA::string_free(crDate);
                crDate = CORBA::string_dup(action.getDB()->GetObjectCrDateTime(id));

                //insert all technical contact
                for (i = 0; i < tech.length(); i++) {
                    LOG(DEBUG_LOG, "KeySetCreate: add tech contact %s id %d",
                            (const char *)tech[i], tch[i]);
                    if (!action.getDB()->AddContactMap("keyset", id, tch[i])) {
                        code = COMMAND_FAILED;
                        break;
                    }
                }
                // insert dnskey(s)
                for (int ii = 0; ii < (int)dnsk.length(); ii++) {
                    char *key;
                    if ((key = removeWhitespaces((const char *)dnsk[ii].key)) == NULL) {
                        LOG(WARNING_LOG, "removeWhitespaces fails (memory problem)");
                        code = COMMAND_FAILED;
                        break;
                    }
                    LOG(NOTICE_LOG, "KeySetCreate: dnskey");

                    int dnskeyId = action.getDB()->GetSequenceID("dnskey");

                    action.getDB()->INSERT("dnskey");
                    action.getDB()->INTO("id");
                    action.getDB()->INTO("keysetid");
                    action.getDB()->INTO("flags");
                    action.getDB()->INTO("protocol");
                    action.getDB()->INTO("alg");
                    action.getDB()->INTO("key");

                    action.getDB()->VALUE(dnskeyId);
                    action.getDB()->VALUE(id);
                    action.getDB()->VALUE(dnsk[ii].flags);
                    action.getDB()->VALUE(dnsk[ii].protocol);
                    action.getDB()->VALUE(dnsk[ii].alg);
                    action.getDB()->VALUE(key);
                    if (!action.getDB()->EXEC()) {
                        code = COMMAND_FAILED;
                        free(key);
                        break;
                    }
                    free(key);
                }

                // insert DSRecord(s)
                for (i = 0; i < dsrec.length(); i++) {
                    LOG(NOTICE_LOG, "KeySetCreate: DSRecord");

                    dsrecID = action.getDB()->GetSequenceID("dsrecord");

                    //DSRecord information
                    action.getDB()->INSERT("DSRECORD");
                    action.getDB()->INTO("ID");
                    action.getDB()->INTO("KEYSETID");
                    action.getDB()->INTO("KEYTAG");
                    action.getDB()->INTO("ALG");
                    action.getDB()->INTO("DIGESTTYPE");
                    action.getDB()->INTO("DIGEST");
                    action.getDB()->INTO("MAXSIGLIFE");

                    action.getDB()->VALUE(dsrecID);
                    action.getDB()->VALUE(id);
                    action.getDB()->VALUE(dsrec[i].keyTag);
                    action.getDB()->VALUE(dsrec[i].alg);
                    action.getDB()->VALUE(dsrec[i].digestType);
                    action.getDB()->VVALUE(dsrec[i].digest);
                    if (dsrec[i].maxSigLife == -1)
                        action.getDB()->VALUENULL();
                    else
                        action.getDB()->VALUE(dsrec[i].maxSigLife);

                    if (!action.getDB()->EXEC()) {
                        code = COMMAND_FAILED;
                        break;
                    }
                }

                // save it to histrory if it's ok
                if (code != COMMAND_FAILED)
                    if (action.getDB()->SaveKeySetHistory(id, params.requestID))
                        if (action.getDB()->SaveObjectCreate(id))
                            code = COMMAND_OK;
            }

            if (code == COMMAND_OK) {
                // run notifier and send notify (suprisingly) message
                ntf.reset(new EPPNotifier(
                            disable_epp_notifier_,
                            mm,
                            action.getDB(),
                            action.getRegistrar(),
                            id)
                        );
                action.setNotifier(ntf.get());
            }
        }
    }

    delete []tch;

    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }

    if (code == 0) {
        action.failedInternal("KeySetCreate");
    }

    return action.getRet()._retn();
}

/*************************************************************
 *
 * Function:    KeySetUpdate
 *
 *************************************************************/
ccReg::Response *
ccReg_EPP_i::KeySetUpdate(
        const char *handle,
        const char *authInfo_chg,
        const ccReg::TechContact &tech_add,
        const ccReg::TechContact &tech_rem,
        const ccReg::DSRecord &dsrec_add,
        const ccReg::DSRecord &dsrec_rem,
        const ccReg::DNSKey &dnsk_add,
        const ccReg::DNSKey &dnsk_rem,
        const ccReg::EppParams &params)
{
    Logging::Context::clear();
    Logging::Context ctx("rifd");
    ConnectionReleaser releaser;

    std::auto_ptr<EPPNotifier> ntf;
    int keysetId, techId;

    int *techAdd = NULL;
    int *techRem = NULL;
    techAdd = new int[tech_add.length()];
    techRem = new int[tech_rem.length()];
    short int code = 0;

    for (int i = 0; i < (int)tech_add.length(); i++) {
        techAdd[i] = 0;
    }
    for (int i = 0; i < (int)tech_rem.length(); i++) {
        techRem[i] = 0;
    }

    EPPAction action(this, params.loginID, EPP_KeySetUpdate, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    LOGGER(PACKAGE).notice ( boost::format("KeySetUpdate: clientId -> %1% clTRID[%2%] handle[%3%] "
            "authInfo_chg[%4%] tech_add[%5%] tech_rem[%6%] dsrec_add[%7%] "
            "dsrec_rem[%8%] dnskey_add[%9%] dnskey_rem[%10%]") %
            (int)params.loginID % (const char*)params.clTRID % handle % authInfo_chg % tech_add.length() %
            tech_rem.length() % dsrec_add.length() % dsrec_rem.length() %
            dnsk_add.length() % dnsk_rem.length());

    std::auto_ptr<Fred::KeySet::Manager> kMan(
            Fred::KeySet::Manager::create(
                action.getDB(), restricted_handles_)
            );
    if ((keysetId = getIdOfKeySet(action.getDB(), handle, restricted_handles_
                    , lock_epp_commands_, true)) < 0) {
        LOG(WARNING_LOG, "bad format of keyset [%s]", handle);
        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                ccReg::keyset_handle, 1, REASON_MSG_BAD_FORMAT_KEYSET_HANDLE);
    } else if (keysetId == 0) {
        LOG(WARNING_LOG, "KeySet handle [%s] NOT_EXISTS", handle);
        code = COMMAND_OBJECT_NOT_EXIST;
    }
    // registrar of the object
    if (!code && !action.getDB()->TestObjectClientID(keysetId, action.getRegistrar())) {
        LOG(WARNING_LOG, "bad authorization not client of KeySet [%s]", handle);
        code = action.setErrorReason(COMMAND_AUTOR_ERROR,
                ccReg::registrar_autor, 0,
                REASON_MSG_REGISTRAR_AUTOR);
    }
    if (dsrec_add.length() > 0) {
       LOG(WARNING_LOG, "KeySetCreate: too many ds-records (maximum is 0)");
       code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
               ccReg::keyset_dsrecord, 0, REASON_MSG_DSRECORD_LIMIT);
    }
    try {
        if (!code && (testObjectHasState(action, keysetId, FLAG_serverUpdateProhibited) ||
                testObjectHasState(action,keysetId,FLAG_deleteCandidate))) {
            LOG(WARNING_LOG, "update of object %s is prohibited", handle);
            code = COMMAND_STATUS_PROHIBITS_OPERATION;
        }
    } catch (...) {
        code = COMMAND_FAILED;
    }

    ///////////////////////////////////////////////////////////
    //
    //  ** KeySetUpdate() - TESTS **
    //
    ///////////////////////////////////////////////////////////

    // duplicity test of dsrec_add in given parameters
    if (!code) {
        for (int ii = 0; ii < (int)dsrec_add.length(); ii++) {
            for (int jj = ii + 1; jj < (int)dsrec_add.length(); jj++) {
                if (testDSRecordDuplicity(dsrec_add[ii], dsrec_add[jj])) {
                    LOG(WARNING_LOG, "Found DSRecord add duplicity: 1:(%d %d %d '%s' %d) 2:(%d %d %d '%s' %d)",
                            dsrec_add[ii].keyTag, dsrec_add[ii].alg, dsrec_add[ii].digestType,
                            (const char *)dsrec_add[ii].digest, dsrec_add[ii].maxSigLife,
                            dsrec_add[jj].keyTag, dsrec_add[jj].alg, dsrec_add[jj].digestType,
                            (const char *)dsrec_add[jj].digest, dsrec_add[jj].maxSigLife);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::keyset_dsrecord_add, jj,
                            REASON_MSG_DUPLICITY_DSRECORD);
                    break;
                }
            }
            LOG(NOTICE_LOG, "ADD keyset DSRecord (%d %d %d '%s' %d)",
                    dsrec_add[ii].keyTag, dsrec_add[ii].alg, dsrec_add[ii].digestType,
                    (const char *)dsrec_add[ii].digest, dsrec_add[ii].maxSigLife);
        }
    }

    // duplicity test of dsrec_rem in given parameters
    if (!code) {
        for (int ii = 0; ii < (int)dsrec_rem.length(); ii++) {
            for (int jj = ii + 1; jj < (int)dsrec_rem.length(); jj++) {
                if (testDSRecordDuplicity(dsrec_rem[ii], dsrec_rem[jj])) {
                    LOG(WARNING_LOG, "Found DSRecord rem duplicity: 1:(%d %d %d '%s' %d) 2:(%d %d %d '%s' %d)",
                            dsrec_rem[ii].keyTag, dsrec_rem[ii].alg, dsrec_rem[ii].digestType,
                            (const char *)dsrec_rem[ii].digest, dsrec_rem[ii].maxSigLife,
                            dsrec_rem[jj].keyTag, dsrec_rem[jj].alg, dsrec_rem[jj].digestType,
                            (const char *)dsrec_rem[jj].digest, dsrec_rem[jj].maxSigLife);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::keyset_dsrecord_rem, jj,
                            REASON_MSG_DUPLICITY_DSRECORD);
                    break;
                }
            }
            LOG(NOTICE_LOG, "REM keyset DSRecord (%d %d %d '%s' %d)",
                    dsrec_rem[ii].keyTag, dsrec_rem[ii].alg, dsrec_rem[ii].digestType,
                    (const char *)dsrec_rem[ii].digest, dsrec_rem[ii].maxSigLife);
        }
    }

    // duplicity test for dnsk_add in given parameters
    if (!code) {
        for (int ii = 0; ii < (int)dnsk_add.length(); ii++) {
            for (int jj = ii + 1; jj < (int)dnsk_add.length(); jj++) {
                if (testDNSKeyDuplicity(dnsk_add[ii], dnsk_add[jj])) {
                    LOG(WARNING_LOG, "Found DNSKey add duplicity: %d x %d (%d %d %d %s)",
                            ii, jj, dnsk_add[ii].flags, dnsk_add[ii].protocol,
                            dnsk_add[ii].alg, (const char *)dnsk_add[ii].key);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::keyset_dnskey_add, jj,
                            REASON_MSG_DUPLICITY_DNSKEY);
                    break;
                }
            }
            LOG(NOTICE_LOG, "ADD keyset DNSKey (%d %d %d %s)",
                    dnsk_add[ii].flags, dnsk_add[ii].protocol,
                    dnsk_add[ii].alg, (const char *)dnsk_add[ii].key);
        }
    }

    // duplicity test for dnsk_rem in given parameters
    if (!code) {
        for (int ii = 0; ii < (int)dnsk_rem.length(); ii++) {
            for (int jj = ii + 1; jj < (int)dnsk_rem.length(); jj++) {
                if (testDNSKeyDuplicity(dnsk_rem[ii], dnsk_rem[jj])) {
                    LOG(WARNING_LOG, "Found DNSKey rem duplicity: %d x %d (%d %d %d %s)",
                            ii, jj, dnsk_rem[ii].flags, dnsk_rem[ii].protocol,
                            dnsk_rem[ii].alg, (const char *)dnsk_rem[ii].key);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::keyset_dnskey_rem, jj,
                            REASON_MSG_DUPLICITY_DNSKEY);
                    break;
                }
            }
            LOG(NOTICE_LOG, "REM keyset DNSKey (%d %d %d %s)",
                    dnsk_rem[ii].flags, dnsk_rem[ii].protocol,
                    dnsk_rem[ii].alg, (const char *)dnsk_rem[ii].key);
        }
    }

    // test dsrecord to add (if same exist for this keyset)
    if (!code) {
        for (int i = 0; i < (int)dsrec_add.length(); i++) {
            int id;
            id = action.getDB()->GetDSRecordId(
                    keysetId,
                    dsrec_add[i].keyTag,
                    dsrec_add[i].alg,
                    dsrec_add[i].digestType,
                    dsrec_add[i].digest,
                    dsrec_add[i].maxSigLife);
            // GetDSRecordId returns id number (equal or bigger than zero) if
            // record is found, otherwise returns -1 (record not found)
            if (id > 0) {
                LOG(WARNING_LOG, "Same DSRecord already exist for keyset [%d]: (%d %d %d '%s' %d) with id %d",
                        keysetId,
                        dsrec_add[i].keyTag,
                        dsrec_add[i].alg,
                        dsrec_add[i].digestType,
                        (const char *)dsrec_add[i].digest,
                        dsrec_add[i].maxSigLife,
                        id);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_dsrecord_add, i,
                        REASON_MSG_DSRECORD_EXIST);
                break;
            }
        }
    }

    // test dsrecord to rem (if this dsrecord exist for this keyset)
    if (!code) {
        for (int i = 0; i < (int)dsrec_rem.length(); i++) {
            int id;
            id = action.getDB()->GetDSRecordId(
                    keysetId,
                    dsrec_rem[i].keyTag,
                    dsrec_rem[i].alg,
                    dsrec_rem[i].digestType,
                    dsrec_rem[i].digest,
                    dsrec_rem[i].maxSigLife);
            if (id <= 0) {
                LOG(WARNING_LOG, "This DSRecord not exist for keyset [%d]: (%d %d %d '%s' %d)",
                        keysetId,
                        dsrec_rem[i].keyTag,
                        dsrec_rem[i].alg,
                        dsrec_rem[i].digestType,
                        (const char *)dsrec_rem[i].digest,
                        dsrec_rem[i].maxSigLife);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_dsrecord_rem, i,
                        REASON_MSG_DSRECORD_NOTEXIST);
                break;
            }
        }
    }

    // dnsk_add tests - if exact same exist for this keyset
    if (!code) {
        for (int ii = 0; ii < (int)dnsk_add.length(); ii++) {
            bool pass = true;
            int id;
            char *key;
            // keys are inserted into database without whitespaces
            if ((key = removeWhitespaces(dnsk_add[ii].key)) == NULL) {
                code = COMMAND_FAILED;
                LOG(WARNING_LOG, "removeWhitespaces failed");
                free(key);
                pass = false;
                break;
            }
            id = action.getDB()->GetDNSKeyId(
                    keysetId,
                    dnsk_add[ii].flags,
                    dnsk_add[ii].protocol,
                    dnsk_add[ii].alg,
                    key);
            free(key);
            if (id > 0) {
                LOG(WARNING_LOG, "Same DNSKey already exist for keyset [%d]: (%d %d %d '%s') with id %d",
                        keysetId, dnsk_add[ii].flags, dnsk_add[ii].protocol, dnsk_add[ii].alg,
                        (const char *)dnsk_add[ii].key);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_dnskey_add, ii,
                        REASON_MSG_DNSKEY_EXIST);
                pass = false;
                break;
            }
        }
    }

    // test dnskey to rem (if this dnskey exist for this keyset)
    if (!code) {
        for (int ii = 0; ii < (int)dnsk_rem.length(); ii++) {
            int id;
            char *key;
            // keys are inserted into database without whitespaces
            if ((key = removeWhitespaces(dnsk_rem[ii].key)) == NULL) {
                code = COMMAND_FAILED;
                LOG(WARNING_LOG, "removeWhitespaces failed");
                free(key);
                break;
            }
            id = action.getDB()->GetDNSKeyId(
                    keysetId,
                    dnsk_rem[ii].flags,
                    dnsk_rem[ii].protocol,
                    dnsk_rem[ii].alg,
                    key);
            //dnsk_rem[ii].key);
            if (id <= 0) {
                LOG(WARNING_LOG, "This DNSKey not exists for keyset [%d] (%d %d %d %s)",
                        keysetId,
                        dnsk_rem[ii].flags,
                        dnsk_rem[ii].protocol,
                        dnsk_rem[ii].alg,
                        (const char *)dnsk_rem[ii].key);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_dnskey_rem, ii,
                        REASON_MSG_DNSKEY_NOTEXIST);
                free(key);
                break;
            }
            free(key);
        }
    }

    // test maximum values - technical contacts
    if (!code) {
        if ((action.getDB()->GetKeySetContacts(keysetId) + tech_add.length() - tech_rem.length()) > 10) {
            LOG(WARNING_LOG, "KeySetUpdate: too may tech contacts (maximum is 10)"
                    "(existing:%d, to add:%d, to rem:%d)",
                    action.getDB()->GetKeySetContacts(keysetId),
                    tech_add.length(),
                    tech_rem.length());
            code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
                    ccReg::keyset_tech, 0, REASON_MSG_TECHADMIN_LIMIT);
        }
    }

    // test maximum values - dsrecords
    if (!code) {
        if ((action.getDB()->GetKeySetDSRecords(keysetId) + dsrec_add.length() - dsrec_rem.length()) > 10) {
            LOG(WARNING_LOG, "KeySetUpdate: too many ds-records (maximum is 10)"
                    "(existing:%d, to add:%d, to rem:%d)",
                    action.getDB()->GetKeySetDSRecords(keysetId),
                    dsrec_add.length(),
                    dsrec_rem.length());
            code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
                    ccReg::keyset_dsrecord, 0, REASON_MSG_DSRECORD_LIMIT);
        }
    }

    // test maximum values - dnskeys
    if (!code) {
        if ((action.getDB()->GetKeySetDNSKeys(keysetId) + dnsk_add.length() - dnsk_rem.length()) > 10) {
            LOG(WARNING_LOG, "KeySetUpdate: too many dnskeys (maximum is 10)"
                    "(existing:%d, to add:%d, to rem:%d)",
                    action.getDB()->GetKeySetDNSKeys(keysetId),
                    dnsk_add.length(),
                    dnsk_rem.length());
            code = action.setErrorReason(COMMAND_PARAMETR_RANGE_ERROR,
                    ccReg::keyset_dnskey, 0, REASON_MSG_DNSKEY_LIMIT);
        }
    }

    // test minimum value - there must be at least one from the pair dnskey-dsrecord
    if (!code) {
        if ((action.getDB()->GetKeySetDSRecords(keysetId) + action.getDB()->GetKeySetDNSKeys(keysetId) +
                    dsrec_add.length() + dnsk_add.length() - dsrec_rem.length() -
                    dnsk_rem.length()) <= 0) {
            LOG(WARNING_LOG, "KeySetUpdate: there must remain one DNSKey or one DSRecord");
            code = action.setErrorReason(COMMAND_FAILED,
                    ccReg::keyset_dnskey, 0, REASON_MSG_NO_DNSKEY_DSRECORD);
            code = COMMAND_FAILED;
        }
    }

    if (!code) {
        // test ADD tech-c
        for (int i = 0; i < (int)tech_add.length(); i++) {
            techId = getIdOfContact(action.getDB(), tech_add[i]
                         , restricted_handles_, lock_epp_commands_);
            if (techId < 0) {
                LOG(WARNING_LOG, "bad format of contact %s", (const char *)tech_add[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_tech_add, i + 1,
                        REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
            } else if (techId == 0) {
                LOG(WARNING_LOG, "Contact %s not exist", (const char *)tech_add[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_tech_add, i,
                        REASON_MSG_TECH_NOTEXIST);
            } else if (action.getDB()->CheckContactMap("keyset", keysetId, techId, 0)) {
                LOG(WARNING_LOG, "Tech contact [%s] exist in contact map table",
                        (const char *)tech_add[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_tech_add, i + 1,
                        REASON_MSG_TECH_EXIST);
            } else {
                techAdd[i] = techId;
                for (int j = 0; j < i; j++) {
                    if (techAdd[j] == techId && techAdd[j] > 0) {
                        techAdd[j] = 0;
                        LOG(WARNING_LOG, "Tech contact [%s] exist in contact map table",
                                (const char *)tech_add[i]);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::keyset_tech_add, i + 1,
                                REASON_MSG_TECH_EXIST);
                    }
                }
            }
            LOG(NOTICE_LOG, "ADD tech  techid -> %d [%s]",
                    techId, (const char *)tech_add[i]);
        }
    }

    // test REM tech-c
    if (!code) {
        for (int i = 0; i < (int)tech_rem.length(); i++) {
            techId = getIdOfContact(action.getDB(), tech_rem[i]
                         , restricted_handles_, lock_epp_commands_);
            if (techId < 0) {
                LOG(WARNING_LOG, "bad format of contact %s", (const char *)tech_rem[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_tech_rem, i + 1,
                        REASON_MSG_BAD_FORMAT_CONTACT_HANDLE);
            } else if (techId == 0) {
                LOG(WARNING_LOG, "Contact %s not exist", (const char *)tech_rem[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_tech_rem, i,
                        REASON_MSG_TECH_NOTEXIST);
            } else if (!action.getDB()->CheckContactMap("keyset", keysetId, techId, 0)) {
                LOG(WARNING_LOG, "Tech contact [%s] does exist in contact map table",
                        (const char *)tech_rem[i]);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_tech_rem, i + 1,
                        REASON_MSG_TECH_NOTEXIST);
            } else {
                techRem[i] = techId;
                for (int j = 0; j < i; j++) {
                    if (techRem[j] == techId && techRem[j] > 0) {
                        techRem[j] = 0;
                        LOG(WARNING_LOG, "Contact [%s] duplicity", (const char *)handle);
                        code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                                ccReg::keyset_tech_rem, i,
                                REASON_MSG_DUPLICITY_CONTACT);
                    }
                }
            }
            LOG(NOTICE_LOG, "REM tech  techid -> %d [%s]",
                    techId, (const char *)tech_rem[i]);
        }
    }

    // DSRecord member tests
    if (!code) {
        for (int i = 0; i < (int)dsrec_add.length(); i++) {
            // test digest type
            if (dsrec_add[i].digestType != 1) {
                LOG(WARNING_LOG, "Bad digest type: %d (must be 1)",
                        dsrec_add[i].digestType);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_dsrecord_add, i,
                        REASON_MSG_DSRECORD_BAD_DIGEST_TYPE);
                break;
            }
            // test digest length
            if (strlen(dsrec_add[i].digest) != 40) {
                LOG(WARNING_LOG, "Bad digest length: %d (must be 40)",
                        strlen(dsrec_add[i].digest));
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_dsrecord_add, i,
                        REASON_MSG_DSRECORD_BAD_DIGEST_LENGTH);
                break;
            }
        }
    }

    // DNSKey member tests
    if (!code) {
        for (int ii = 0; ii < (int)dnsk_add.length(); ii++) {
            // dnskey flag field test (must be 0, 256, 257)
            if (!(dnsk_add[ii].flags == 0 || dnsk_add[ii].flags == 256 || dnsk_add[ii].flags == 257)) {
                LOG(WARNING_LOG,
                        "dnskey flag is %d (must be 0, 256 or 257)",
                        dnsk_add[ii].flags);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_dnskey_add, ii,
                        REASON_MSG_DNSKEY_BAD_FLAGS);
                break;
            }
            // dnskey protocol test (must be 3)
            if (dnsk_add[ii].protocol != 3) {
                LOG(WARNING_LOG,
                        "dnskey protocol is %d (must be 3)",
                        dnsk_add[ii].protocol);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::keyset_dnskey_add, ii,
                        REASON_MSG_DNSKEY_BAD_PROTOCOL);
                break;
            }
            // Ticket #4432 - algorithm validity removed
            //
            // dnskey algorithm type test (must be 1,2,3,4,5,6,7,8,10,12,252,253,254,255)
            // if (!((dnsk_add[ii].alg >= 1 && dnsk_add[ii].alg <= 8) ||
            //             (dnsk_add[ii].alg == 10) || (dnsk_add[ii].alg == 12) ||
            //             (dnsk_add[ii].alg >= 252 && dnsk_add[ii].alg <= 255))) {
            //     LOG(WARNING_LOG,
            //             "dnskey algorithm is %d (must be 1,2,3,4,5,6,7,8,10,12,252,253,254 or 255)",
            //             dnsk_add[ii].alg);
            //     code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
            //             ccReg::keyset_dnskey_add, ii,
            //             REASON_MSG_DNSKEY_BAD_ALG);
            //     break;
            // }
            // dnskey key test - see isValidBase64 function for details
            int ret1, ret2;
            if ((ret1 = isValidBase64((const char *)dnsk_add[ii].key, &ret2)) != BASE64_OK) {
                if (ret1 == BASE64_BAD_LENGTH) {
                    LOG(WARNING_LOG, "dnskey key length is wrong (must be dividable by 4)");
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::keyset_dnskey_add, ii,
                            REASON_MSG_DNSKEY_BAD_KEY_LEN);
                } if (ret1 == BASE64_BAD_CHAR) {
                    LOG(WARNING_LOG, "dnskey key contain bad character '%c' at position %d",
                            ((const char *)dnsk_add[ii].key)[ret2], ret2);
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::keyset_dnskey_add, ii,
                            REASON_MSG_DNSKEY_BAD_KEY_CHAR);
                } else {
                    LOG(WARNING_LOG, "isValidBase64() return unknown value (%d)",
                            ret1);
                    code = COMMAND_FAILED;
                }
                break;
            }
        }

    }

    ///////////////////////////////////////////////////////
    //
    //  ** KeySetUpdate() - UPDATE **
    //
    ///////////////////////////////////////////////////////

    // if no errors occured run update
    if (code == 0) {
        if (action.getDB()->ObjectUpdate(keysetId, action.getRegistrar(), authInfo_chg)) {
            ntf.reset(new EPPNotifier(
                          disable_epp_notifier_,
                          mm, action.getDB(), action.getRegistrar(), keysetId, regMan.get()));

            for (int i = 0; i < (int)tech_add.length(); i++)
                ntf->AddTechNew(techAdd[i]);

            // adding tech contacts
            for (int i = 0; i < (int)tech_add.length(); i++) {
                LOG(NOTICE_LOG, "INSERT add techid -> %d [%s]",
                        techAdd[i], (const char *)tech_add[i]);
                if (!action.getDB()->AddContactMap("keyset", keysetId, techAdd[i])) {
                    code = COMMAND_FAILED;
                    break;
                }
            }

            // delete tech contacts
            for (int i = 0; i < (int)tech_rem.length(); i++) {
                LOG(NOTICE_LOG, "DELETE rem techid -> %d [%s]",
                        techRem[i], (const char *)tech_rem[i]);
                if (!action.getDB()->DeleteFromTableMap("keyset", keysetId, techRem[i])) {
                    code = COMMAND_FAILED;
                    break;
                }
            }

            // test for count of tech contacts after add&rem
            if (tech_rem.length() > 0) {
                int techNum = action.getDB()->GetKeySetContacts(keysetId);
                LOG(NOTICE_LOG, "KeySetUpdate: tech Contacts %d", techNum);
                if (techNum == 0) {
                    // mark all rem tech-c as param error
                    for (int i = 0; i < (int)tech_rem.length(); i++) {
                        code = action.setErrorReason(COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                                ccReg::keyset_tech_rem, i + 1,
                                REASON_MSG_CAN_NOT_REMOVE_TECH);
                    }
                }
            }

            // delete dsrecords
            for (int i = 0; i < (int)dsrec_rem.length(); i++) {
                LOG(NOTICE_LOG,
                        "KeySetUpdate: delete DSRecord (keytag:%d,alg:%d,"
                        "digestType:%d,digest:%s,maxSigLife:%d) from KeySet [%s]",
                        dsrec_rem[i].keyTag, dsrec_rem[i].alg, dsrec_rem[i].digestType,
                        (const char *)dsrec_rem[i].digest, dsrec_rem[i].maxSigLife,
                        (const char *)handle);
                int dsrecId = action.getDB()->GetDSRecordId(keysetId, dsrec_rem[i].keyTag,
                        dsrec_rem[i].alg, dsrec_rem[i].digestType,
                        (const char *)dsrec_rem[i].digest, dsrec_rem[i].maxSigLife);
                if (dsrecId == -1) {
                    code = COMMAND_FAILED;
                    break;
                }
                LOG(NOTICE_LOG,
                        "KeySetUpdate: delete DSRecord id found: %d",
                        dsrecId);
                if (!action.getDB()->DeleteFromTable("dsrecord", "id", dsrecId))
                    code = COMMAND_FAILED;
            }
            // delete dnskey(s)
            for (int ii = 0; ii < (int)dnsk_rem.length(); ii++) {
                char *key;
                // keys are inserted into database without whitespaces
                if ((key = removeWhitespaces(dnsk_rem[ii].key)) == NULL) {
                    code = COMMAND_FAILED;
                    LOG(WARNING_LOG, "removeWhitespaces failed");
                    free(key);
                    break;
                }
                LOG(NOTICE_LOG,
                        "KeySetUpdate: delete DNSKey (flags:%d,protocol:%d,"
                        "alg:%d,key:%s from keyset (handle:%d)",
                        dnsk_rem[ii].flags, dnsk_rem[ii].protocol,
                        dnsk_rem[ii].alg, (const char *)dnsk_rem[ii].key,
                        (const char *)handle);
                int dnskeyId = action.getDB()->GetDNSKeyId(keysetId, dnsk_rem[ii].flags,
                        dnsk_rem[ii].protocol, dnsk_rem[ii].alg,
                        key);
                if (dnskeyId == -1) {
                    code = COMMAND_FAILED;
                    free(key);
                    break;
                }
                LOG(NOTICE_LOG, "KeySetUpdate: delete DNSKey id found: %d",
                        dnskeyId);
                if (!action.getDB()->DeleteFromTable("dnskey", "id", dnskeyId)) {
                    LOG(WARNING_LOG, "during deleting dnskeys command failed");
                    code = COMMAND_FAILED;
                    free(key);
                    break;
                }
                free(key);
            }

            // add dsrecords
            for (int i = 0; i < (int)dsrec_add.length(); i++) {
                LOG(NOTICE_LOG,
                        "KeySetUpdate: add DSRecord (keyTag:%d,alg:%d,"
                        "digestType:%d,digest:%s,maxSigLife:%d) to KeySet [%s]",
                        dsrec_add[i].keyTag, dsrec_add[i].alg, dsrec_add[i].digestType,
                        (const char *)dsrec_add[i].digest, dsrec_add[i].maxSigLife,
                        (const char *)handle);
                int dsrecId = action.getDB()->GetSequenceID("dsrecord");

                action.getDB()->INSERT("dsrecord");
                action.getDB()->INTO("id");
                action.getDB()->INTO("keysetid");
                action.getDB()->INTO("keytag");
                action.getDB()->INTO("alg");
                action.getDB()->INTO("digesttype");
                action.getDB()->INTO("digest");
                action.getDB()->INTO("maxsiglife");
                action.getDB()->VALUE(dsrecId);
                action.getDB()->VALUE(keysetId);
                action.getDB()->VALUE(dsrec_add[i].keyTag);
                action.getDB()->VALUE(dsrec_add[i].alg);
                action.getDB()->VALUE(dsrec_add[i].digestType);
                action.getDB()->VALUE(dsrec_add[i].digest);
                if (dsrec_add[i].maxSigLife == -1)
                    action.getDB()->VALUENULL();
                else
                    action.getDB()->VALUE(dsrec_add[i].maxSigLife);
                if (!action.getDB()->EXEC()) {
                    code = COMMAND_FAILED;
                    break;
                }
            }

            // add dnskey(s)
            for (int ii = 0; ii < (int)dnsk_add.length(); ii++) {
                char *key;
                if ((key = removeWhitespaces((const char *)dnsk_add[ii].key)) == NULL) {
                    LOG(WARNING_LOG, "removeWhitespaces fails (memory problem)");
                    code = COMMAND_FAILED;
                    break;
                }
                LOG(NOTICE_LOG,
                        "KeySetUpdate: add DNSKey (flags:%d,protocol:%d,"
                        "alg:%d,key:%s to keyset (handle:%d)",
                        dnsk_add[ii].flags, dnsk_add[ii].protocol,
                        dnsk_add[ii].alg, (const char *)dnsk_add[ii].key,
                        (const char *)handle);
                int dnskeyId = action.getDB()->GetSequenceID("dnskey");
                action.getDB()->INSERT("dnskey");
                action.getDB()->INTO("id");
                action.getDB()->INTO("keysetid");
                action.getDB()->INTO("flags");
                action.getDB()->INTO("protocol");
                action.getDB()->INTO("alg");
                action.getDB()->INTO("key");
                action.getDB()->VALUE(dnskeyId);
                action.getDB()->VALUE(keysetId);
                action.getDB()->VALUE(dnsk_add[ii].flags);
                action.getDB()->VALUE(dnsk_add[ii].protocol);
                action.getDB()->VALUE(dnsk_add[ii].alg);
                action.getDB()->VALUE(key);
                if (!action.getDB()->EXEC()) {
                    code = COMMAND_FAILED;
                    free(key);
                    break;
                }
                free(key);
            }
            // save to history if no errors
            if (code == 0) {
                if (action.getDB()->SaveKeySetHistory(keysetId, params.requestID))
                    code = COMMAND_OK;
            }
            if (code == COMMAND_OK)
                action.setNotifier(ntf.get()); // schedule message send
        }
    }

    delete[] techAdd;
    delete[] techRem;

    if (code > COMMAND_EXCEPTION) {
        action.failed(code);
    }
    if (code == 0) {
        action.failedInternal("KeysetUpdate");
    }

    return action.getRet()._retn();
}

bool isbase64char (unsigned char c)
{
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9')
        || c == '+'
        || c == '/';
}

int
countTailPads(const char *key)
{
    const char *str;
    int count;
    str = strchr(key, '=');
    if (str == NULL) {
        return 0;
    }
    for (count = 0; *str != '\0'; str++, count++)
        ;
    return count;
}
/*
 * test if ``key'' contains equal character (``='') only at the end
 * return -1 if no ``='' character is present or if all ``='' characters
 * are at the end.
 * Otherwise return index of first invalid ``='' character.
 */
int
testPad(const char *key)
{
    const char *str;
    if ((str = strchr(key, '=')) == NULL) {
        return -1;
    }
    for ( ; *str != '\0'; str++) {
        if (*str != '=') {
            return str - key - 1;
        }
    }
    if (countTailPads(key) >= 3) {
        return strchr(key, '=') - key;
    }
    return -1;
}


char *
removeWhitespaces(const char *encoded)
{
    int len = 0;
    char *ret;
    int i, j;
    for (i = 0; i < (int)strlen(encoded); i++) {
        if (!isspace(encoded[i])) {
            len++;
        }
    }
    if ((ret = (char *)malloc(len + 1)) == NULL) {
        return NULL;
    }
    for (i = 0, j = 0; i < (int)strlen(encoded); i++) {
        if (!isspace(encoded[i])) {
            ret[j] = encoded[i];
            j++;
        }
    }
    ret[j] = '\0';
    return ret;
}

/*
 * decide if ``str'' is valid base64 encoded string
 * - there must be only valid characters in the string (a-z, A-Z, 0-9, + and /)
 * - at the end could be zero, one or two pad characters (=)
 * - string lenght (including padding) must be dividable by 4
 * for further details see RFC 4648 chapter 3
 */
int
isValidBase64(const char *str, int *ret)
{
    char *key;
    int i;
    int len;
    int ch;
    if ((key = removeWhitespaces(str)) == NULL) {
        return BASE64_UNKNOWN;
    }
    if ((*ret = testPad(key)) != -1) {
        free(key);
        (*ret)--;
        return BASE64_BAD_CHAR;
    }
    for (i = 0, len = 0; i < (int)strlen(key); i++) {
        ch = key[i];
        if (isbase64char(ch) || ch == '=') {
            len++;
        } else {
            free(key);
            *ret = i;
            return BASE64_BAD_CHAR;
        }
    }
    if (len % 4 == 0) {
        free(key);
        *ret = -1;
        return BASE64_OK;
    }
    *ret = len;
    free(key);
    return BASE64_BAD_LENGTH;
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
        case EPP_ListNSset:
            type=2;
            break;
        case EPP_ListDomain:
            type=3;
            break;
        case EPP_ListKeySet:
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

  return FullList( EPP_ListNSset , "NSSET" , "HANDLE" , nssets , params);
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
            EPP_ListKeySet, "KEYSET", "HANDLE", keysets, params);
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

  DBAutoPtr _db( new DB);
  bool db_connected_ = _db->OpenDatabase(database);
  DBSharedPtr DBsql = DBDisconnectPtr(_db.release());

  if ( (regID = GetRegistrarID(params.loginID) ))
    if (db_connected_) {

      if ( (DBsql->BeginAction(params.loginID, EPP_NSsetTest, static_cast<const char*>(params.clTRID), params.XML, params.requestID) )) {

        if ( (nssetid = getIdOfNSSet(DBsql, handle
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
        ServerInternalError("NSSetTest");
    }

  return ret._retn();
}

// function for send authinfo
ccReg::Response *
ccReg_EPP_i::ObjectSendAuthInfo(
        short act, const char * table, const char *fname, const char *name,
        const ccReg::EppParams &params)
{
    int zone;
    int id = 0;
    char FQDN[164];
    short int code = 0;

    EPPAction action(this, params.loginID, act, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    // Database::Manager db(new Database::ConnectionFactory(database));
    // std::auto_ptr<Database::Connection> conn;
    // try { conn.reset(db.getConnection()); } catch (...) {}

    LOG( NOTICE_LOG , "ObjectSendAuthInfo type %d  object [%s]  clientID -> %llu clTRID [%s] " , act , name , params.loginID , static_cast<const char*>(params.clTRID) );

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
        case EPP_NSSetSendAuthInfo:
            if ( (id = getIdOfNSSet(action.getDB(), name, restricted_handles_
                    , lock_epp_commands_) ) < 0) {
                LOG(WARNING_LOG, "bad format of nsset [%s]", name);
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::nsset_handle, 1,
                        REASON_MSG_BAD_FORMAT_NSSET_HANDLE);
            } else if (id == 0)
                code=COMMAND_OBJECT_NOT_EXIST;
            break;
        case EPP_DomainSendAuthInfo:
            if ( (zone = getFQDN(action.getDB(), FQDN, name) ) <= 0) {
                LOG(WARNING_LOG, "domain in zone %s", name);
                if (zone == 0) {
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::domain_fqdn, 1,
                            REASON_MSG_NOT_APPLICABLE_DOMAIN);
                } else if (zone < 0) {
                    code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                            ccReg::domain_fqdn, 1,
                            REASON_MSG_BAD_FORMAT_FQDN);
                }
            } else {
                if ( (id = action.getDB()->GetDomainID(FQDN, GetZoneEnum(action.getDB(), zone) ) ) == 0) {
                    LOG( WARNING_LOG , "domain [%s] NOT_EXIST" , name );
                    code= COMMAND_OBJECT_NOT_EXIST;
                }
            }
            break;
        case EPP_KeySetSendAuthInfo:
            if ((id = getIdOfKeySet(action.getDB(), name, restricted_handles_
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
                    regMan->getNSSetManager(),
                    regMan->getKeySetManager(),
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

  return ObjectSendAuthInfo( EPP_NSSetSendAuthInfo , "NSSET" , "handle" , handle , params);
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
            EPP_KeySetSendAuthInfo, "KEYSET",
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
    std::auto_ptr<Fred::NSSet::Manager> nssMan(
        Fred::NSSet::Manager::create(
            a.getDB(),zoneMan.get(),restricted_handles_
        )
    );
    std::auto_ptr<Fred::KeySet::Manager> keyMan(
            Fred::KeySet::Manager::create(
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
    std::auto_ptr<Fred::NSSet::Manager> nssMan(
        Fred::NSSet::Manager::create(
            a.getDB(),zoneMan.get(),restricted_handles_
        )
    );
    std::auto_ptr<Fred::KeySet::Manager> keyMan(
            Fred::KeySet::Manager::create(
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
