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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <corba/ccReg.hh>
#include "epp_impl.h"

#include "config.h"
// database functions
#include "old_utils/dbsql.h"
#include "db/manager.h"

// support function
#include "old_utils/util.h"

#include "action.h"    // code of the EPP operations
#include "response.h"  // errors code
#include "reason.h"  // reason messages code

// logger
#include "old_utils/log.h"

//config
#include "old_utils/conf.h"

// MailerManager is connected in constructor
#include "register/auth_info.h"
#include "register/domain.h"
#include "register/contact.h"
#include "register/nsset.h"
#include "register/keyset.h"
#include "register/info_buffer.h"
#include "register/poll.h"
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
  DB *db, Register::TID object_id, unsigned state_id)
    throw (Register::SQL_ERROR)
{
  std::stringstream sql;
  sql << "SELECT COUNT(*) FROM object_state " << "WHERE object_id="
      << object_id << " AND state_id=" << state_id << " AND valid_to ISNULL";
  if (!db->ExecSelect(sql.str().c_str()))
    throw Register::SQL_ERROR();
  return atoi(db->GetFieldValue(0, 0));
}

class EPPAction
{
  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  ccReg_EPP_i *epp;
  DB db;
  int regID;
  int clientID;
  int code; ///< needed for destructor where Response is invalidated
public:
  struct ACTION_START_ERROR
  {
  };
  EPPAction(
    ccReg_EPP_i *_epp, int _clientID, int action, const char *clTRID,
    const char *xml, ParsedAction *paction = NULL
  ) throw (ccReg::EPP::EppError) :
    ret(new ccReg::Response()), errors(new ccReg::Errors()), epp(_epp),
    regID(_epp->GetRegistrarID(_clientID)), clientID(_clientID)
  {
    Logging::Context::push(str(boost::format("action-%1%") % action));


    if (!db.OpenDatabase(epp->getDatabaseString()))
      epp->ServerInternalError("Cannot connect to DB");
    if (!db.BeginAction(clientID, action, clTRID, xml, paction)) {
      db.Disconnect();
      epp->ServerInternalError("Cannot beginAction");
    }
    if (!regID) {
      ret->code = COMMAND_MAX_SESSION_LIMIT;
      ccReg::Response_var& r(getRet());
      db.EndAction(r->code);
      db.Disconnect();
      epp->EppError(r->code, r->msg, r->svTRID, errors);
    }
    if (!db.BeginTransaction()) {
      db.EndAction(COMMAND_FAILED);
      db.Disconnect();
      epp->ServerInternalError("Cannot start transaction",
          CORBA::string_dup(db.GetsvTRID()) );
    }
    code = ret->code = COMMAND_OK;

    Logging::Context::push(str(boost::format("%1%") % db.GetsvTRID()));
  }
  ~EPPAction()
  {
    db.QuitTransaction(code);
    db.EndAction(code);
    db.Disconnect();

    Logging::Context::pop();
    Logging::Context::pop();
  }
  DB *getDB()
  {
    return &db;
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
    ret->svTRID = CORBA::string_dup(db.GetsvTRID());
    ret->msg = CORBA::string_dup(epp->GetErrorMessage(ret->code, getLang()));
    return ret;
  }
  ccReg::Errors_var& getErrors()
  {
    return errors;
  }
  void failed(
    int _code) throw (ccReg::EPP::EppError)
  {
    code = ret->code = _code;
    ccReg::Response_var& r(getRet());
    epp->EppError(r->code, r->msg, r->svTRID, errors);
  }
  void failedInternal(
    const char *msg) throw (ccReg::EPP::EppError)
  {
    code = ret->code = COMMAND_FAILED;
    epp->ServerInternalError(msg, db.GetsvTRID());
  }
  void NoMessage() throw (ccReg::EPP::NoMessages)
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
};

/// timestamp formatting function
static std::string formatTime(
  boost::posix_time::ptime tm)
{
  char buffer[100];
  convert_rfc3339_timestamp(buffer, to_iso_extended_string(tm).c_str());
  return buffer;
}

/// replace GetContactID
static long int getIdOfContact(
  DB *db, const char *handle, Config::Conf& c, bool lock = false)
{
  if (lock && !c.get<bool>("registry.lock_epp_commands")) lock = false;
  std::auto_ptr<Register::Contact::Manager>
      cman(Register::Contact::Manager::create(db, c.get<bool>("registry.restricted_handles")) );
  Register::Contact::Manager::CheckAvailType caType;
  long int ret = -1;
  try {
    Register::NameIdPair nameId;
    caType = cman->checkAvail(handle,nameId, lock);
    ret = nameId.id;
    if (caType == Register::Contact::Manager::CA_INVALID_HANDLE)
    ret = -1;
  } catch (...) {}
  return ret;
}

/// replace GetNSSetID
static long int getIdOfNSSet(
  DB *db, const char *handle, Config::Conf& c, bool lock = false)
{
  if (lock && !c.get<bool>("registry.lock_epp_commands")) lock = false;
  std::auto_ptr<Register::Zone::Manager>
      zman(Register::Zone::Manager::create(db) );
  std::auto_ptr<Register::NSSet::Manager> man(Register::NSSet::Manager::create(
      db, zman.get(), c.get<bool>("registry.restricted_handles")) );
  Register::NSSet::Manager::CheckAvailType caType;
  long int ret = -1;
  try {
    Register::NameIdPair nameId;
    caType = man->checkAvail(handle,nameId,lock);
    ret = nameId.id;
    if (caType == Register::NSSet::Manager::CA_INVALID_HANDLE)
    ret = -1;
  } catch (...) {}
  return ret;
}

/// replace GetKeySetID
static long int
getIdOfKeySet(DB *db, const char *handle, Config::Conf &c, bool lock = false)
{
    if (lock && !c.get<bool>("registry.lock_epp_commands"))
        lock = false;
    std::auto_ptr<Register::KeySet::Manager> man(
            Register::KeySet::Manager::create(db, c.get<bool>("registry.restricted_handles")));
    Register::KeySet::Manager::CheckAvailType caType;
    long int ret = -1;
    try {
        Register::NameIdPair nameId;
        caType = man->checkAvail(handle, nameId, lock);
        ret = nameId.id;
        if (caType == Register::KeySet::Manager::CA_INVALID_HANDLE)
            ret = -1;
    } catch (...) {}
    return ret;
}

/// replace GetDomainID
static long int getIdOfDomain(
  DB *db, const char *handle, Config::Conf& c, bool lock = false, int* zone = NULL)
{
  if (lock && !c.get<bool>("registry.lock_epp_commands")) lock = false;
  std::auto_ptr<Register::Zone::Manager> zm(
    Register::Zone::Manager::create(db)
  );
  std::auto_ptr<Register::Domain::Manager> dman(
    Register::Domain::Manager::create(db,zm.get())
  );
  Register::NameIdPair nameId;
  long int ret = -1;
  try {
    switch (dman->checkAvail(handle, nameId, lock)) {
      case Register::Domain::CA_REGISTRED :
        ret = nameId.id;
        break;
      case Register::Domain::CA_INVALID_HANDLE :
      case Register::Domain::CA_BAD_LENGHT :
      case Register::Domain::CA_BLACKLIST :
        ret = -2;
        break;
      case Register::Domain::CA_BAD_ZONE :
        ret = -1;
        break;
      case Register::Domain::CA_PARENT_REGISTRED :
      case Register::Domain::CA_CHILD_REGISTRED :
      case Register::Domain::CA_AVAILABLE :
        ret = 0;
        break;
    }
    const Register::Zone::Zone *z = zm->findZoneId(handle);
    if (zone && z) *zone = z->getId();
  } catch (...) {}
  return ret;
}

//
// Example implementational code for IDL interface ccReg::EPP
//
ccReg_EPP_i::ccReg_EPP_i(
  const std::string &_db, MailerManager *_mm, NameService *_ns, Config::Conf& _conf)
    throw (DB_CONNECT_FAILED) : database(_db),
                                mm(_mm),
                                ns(_ns),
                                conf(_conf),
                                zone(NULL),
                                testInfo(false)
{
  Logging::Context ctx("rifd");

  // objects are shared between threads!!!
  // init at the beginning and do not change
  if (!db.OpenDatabase(database)) {
    LOG(ALERT_LOG, "can not connect to DATABASE %s", database.c_str());
    throw DB_CONNECT_FAILED();
  }
  LOG(NOTICE_LOG, "successfully  connect to DATABASE %s", database.c_str());
  regMan.reset(Register::Manager::create(&db, false)); //TODO: replace 'false'
  regMan->initStates();
}
ccReg_EPP_i::~ccReg_EPP_i()
{
  Logging::Context ctx("rifd");

  delete CC;
  delete ReasonMsg;
  delete ErrorMsg;
  delete zone;
  delete [] session;

  db.Disconnect();
  LOG( ERROR_LOG, "EPP_i destructor");
}

// HANDLE EXCEPTIONS
void ccReg_EPP_i::ServerInternalError(
  const char *fce, const char *svTRID) throw (ccReg::EPP::EppError)
{
  LOG( ERROR_LOG ,"Internal errror in fce %s svTRID[%s] " , fce , svTRID );
  throw ccReg::EPP::EppError( COMMAND_FAILED , "" , svTRID , ccReg::Errors(0) );
}

void ccReg_EPP_i::EppError(
  short errCode, const char *msg, const char *svTRID, ccReg::Errors_var& errors)
    throw (ccReg::EPP::EppError)

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

void ccReg_EPP_i::CreateSession(
  int max, long wait)
{
  int i;

  LOG( DEBUG_LOG , "SESSION CREATE max %d wait %ld" , max , wait );

  session = new Session[max];
  numSession=0; //  number of the active session
  maxSession=max; // maximum number of sessions
  maxWaitClient=wait; // timeout
  for (i = 0; i < max; i ++) {
    session[i].clientID=0;
    session[i].registrarID=0;
    session[i].language =0;
    session[i].timestamp=0;
  }

}
// session manager
bool ccReg_EPP_i::LoginSession(
  long loginID, int registrarID, int language)
{
  int i;

  GarbageSesion();

  if (numSession < maxSession) {
    // count sessions for given registrar
    unsigned count = 0;
    for (i=0; i<maxSession; i++)
      if (session[i].registrarID == registrarID)
        count++;
    if (count >= conf.get<unsigned>("rifd.session_registrar_max")) {
      LOG( DEBUG_LOG , "SESSION max per registrar exceeded clientID %d registrarID %d lang %d" , loginID , registrarID , language );
      //
      return false;
    }
    // find first session to free
    for (i = 0; i < maxSession; i ++) {
      if (session[i].clientID== 0) {
        LOG( DEBUG_LOG , "SESSION  login  clientID %d registrarID %d lang %d" , loginID , registrarID , language );
        session[i].clientID=loginID;
        session[i].registrarID=registrarID;
        session[i].language = language;
        session[i].timestamp=( long long ) time(NULL);
        numSession++;
        LOG( DEBUG_LOG , "SESSION  num %d numSession %d timespatmp %lld" , i , numSession , session[i].timestamp );
        return true;
      }
    }
  } else {
    LOG( ERROR_LOG , "SESSION MAX_CLIENTS %d" , maxSession );
  }

  return false;
}

bool ccReg_EPP_i::LogoutSession(
  long loginID)
{
  int i;

  for (i = 0; i < maxSession; i ++) {
    if (session[i].clientID== loginID) {
      session[i].clientID=0;
      session[i].registrarID=0;
      session[i].language =0;
      session[i].timestamp=0;
      numSession--;
      LOG( DEBUG_LOG , "SESSION LOGOUT %d numSession %d" , i , numSession );
      return true;
    }
  }

  LOG( ERROR_LOG , "SESSION LOGOUT UNKNOWN loginID %d" , loginID );

  return false;
}

void ccReg_EPP_i::GarbageSesion()
{
  int i;
  long long t;

  LOG( DEBUG_LOG , "SESSION GARBAGE" );
  t = ( long long ) time(NULL);

  for (i = 0; i < maxSession; i ++) {

    if (session[i].clientID) {
      LOG( DEBUG_LOG , "SESSION  maxWait %lld time %lld timestamp session[%d].timestamp  %lld" , maxWaitClient , t , i , session[i].timestamp );

      // garbage collector
      // clear unused sessions
      if (t > session[i].timestamp + maxWaitClient) {
        LOG( DEBUG_LOG , "SESSION[%d] TIMEOUT %lld GARBAGE" , i , session[i].timestamp);
        session[i].clientID=0;
        session[i].registrarID=0;
        session[i].language =0;
        session[i].timestamp=0;
        numSession--;
      }
    }

  }

}

int ccReg_EPP_i::GetRegistrarID(
  int clientID)
{
  int regID=0;
  int i;

  LOG( DEBUG_LOG , "SESSION GetRegistrarID %d" , clientID );

  for (i = 0; i < maxSession; i ++) {

    if (session[i].clientID==clientID) {
      session[i].timestamp= ( long long ) time(NULL);
      LOG( DEBUG_LOG , "SESSION[%d] loginID %d -> regID %d" , i , clientID , session[i].registrarID );
      LOG( DEBUG_LOG , "SESSION[%d] TIMESTMAP %lld" , i , session[i].timestamp );
      regID = session[i].registrarID;
    }

  }

  return regID;
}

int ccReg_EPP_i::GetRegistrarLang(
  int clientID)
{
  int i;

  for (i = 0; i < numSession; i ++) {
    if (session[i].clientID==clientID) {
      LOG( DEBUG_LOG , "SESSION[%d]  loginID %d -> lang %d" , i , clientID , session[i].language );
      return session[i].language;
    }
  }

  return 0;
}

// test connection to database when server starting
bool ccReg_EPP_i::TestDatabaseConnect(
  const std::string& db)
{
  DB DBsql;

  // connection info
  database = db;

  if (DBsql.OpenDatabase(database) ) {
    LOG( NOTICE_LOG , "successfully  connect to DATABASE" );
    DBsql.Disconnect();
    return true;
  } else {
    LOG( ALERT_LOG , "can not connect to DATABASE" );
    return false;
  }

}

// Load table to memory for speed

int ccReg_EPP_i::LoadReasonMessages()
{
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

const char * ccReg_EPP_i::GetReasonMessage(
  int err, int lang)
{
  if (lang == LANG_CS)
    return ReasonMsg->GetMesg_CS(err).c_str();
  else
    return ReasonMsg->GetMesg(err).c_str();
}

const char * ccReg_EPP_i::GetErrorMessage(
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

short ccReg_EPP_i::SetReasonUnknowCC(
  ccReg::Errors_var& err, const char *value, int lang)
{
  LOG( WARNING_LOG, "Reason: unknown country code: %s" , value );
  return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::contact_cc, 1,
  REASON_MSG_COUNTRY_NOTEXIST, lang);
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

short ccReg_EPP_i::SetReasonDomainNSSet(
  ccReg::Errors_var& err, const char * nsset_handle, int nssetid, int lang)
{

  if (nssetid < 0) {
    LOG( WARNING_LOG, "bad format of domain nsset  [%s]" , nsset_handle );
    return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::domain_nsset, 1,
    REASON_MSG_BAD_FORMAT_NSSET_HANDLE, lang);
  } else if (nssetid == 0) {
    LOG( WARNING_LOG, " domain nsset not exist [%s]" , nsset_handle );
    return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::domain_nsset, 1,
    REASON_MSG_NSSET_NOTEXIST, lang);
  }

  return 0;
}

short int
ccReg_EPP_i::SetReasonDomainKeySet(
        ccReg::Errors_var &err,
        const char *keyset_handle,
        int keysetid,
        int lang)
{
    if (keysetid < 0) {
        LOG(WARNING_LOG, "bad format of domain keyset [%s]", keyset_handle);
        return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::domain_keyset,
                1, REASON_MSG_BAD_FORMAT_KEYSET_HANDLE, lang);
    } else if (keysetid == 0) {
        LOG(WARNING_LOG, "domain keyset not exist [%s]", keyset_handle);
        return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::domain_keyset,
                1, REASON_MSG_KEYSET_NOTEXIST, lang);
    }
    return 0;
}

short ccReg_EPP_i::SetReasonDomainRegistrant(
  ccReg::Errors_var& err, const char * contact_handle, int contactid, int lang)
{

  if (contactid < 0) {
    LOG( WARNING_LOG, "bad format of registrant  [%s]" , contact_handle );
    return SetErrorReason(err, COMMAND_PARAMETR_ERROR,
        ccReg::domain_registrant, 1, REASON_MSG_BAD_FORMAT_CONTACT_HANDLE, lang);
  } else if (contactid == 0) {
    LOG( WARNING_LOG, " domain registrant not exist [%s]" , contact_handle );
    return SetErrorReason(err, COMMAND_PARAMETR_ERROR,
        ccReg::domain_registrant, 1, REASON_MSG_REGISTRANT_NOTEXIST, lang);
  }

  return 0;
}

short ccReg_EPP_i::SetReasonProtectedPeriod(
  ccReg::Errors_var& err, const char *value, int lang, ccReg::ParamError param)
{
  LOG( WARNING_LOG, "object [%s] in history period" , value );
  return SetErrorReason(err, COMMAND_PARAMETR_ERROR, param, 1,
  REASON_MSG_PROTECTED_PERIOD, lang);
}

short ccReg_EPP_i::SetReasonContactMap(
  ccReg::Errors_var& err, ccReg::ParamError paramCode, const char *handle,
  int id, int lang, short position, bool tech_or_admin)
{

  if (id < 0) {
    LOG( WARNING_LOG, "bad format of Contact %s" , (const char *) handle );
    return SetErrorReason(err, COMMAND_PARAMETR_ERROR, paramCode, position +1,
    REASON_MSG_BAD_FORMAT_CONTACT_HANDLE, lang);
  } else if (id == 0) {
    LOG( WARNING_LOG, "Contact %s not exist" , (const char *) handle );
    if (tech_or_admin)
      return SetErrorReason(err, COMMAND_PARAMETR_ERROR, paramCode, position,
      REASON_MSG_TECH_NOTEXIST, lang);
    else
      return SetErrorReason(err, COMMAND_PARAMETR_ERROR, paramCode, position+1,
      REASON_MSG_ADMIN_NOTEXIST, lang);
  }

  return 0;
}

short ccReg_EPP_i::SetReasonContactDuplicity(
  ccReg::Errors_var& err, const char * handle, int lang, short position,
  ccReg::ParamError paramCode)
{
  LOG( WARNING_LOG, "Contact [%s] duplicity " , (const char *) handle );
  return SetErrorReason(err, COMMAND_PARAMETR_ERROR, paramCode, position,
  REASON_MSG_DUPLICITY_CONTACT, lang);
}

short ccReg_EPP_i::SetReasonNSSetTech(
  ccReg::Errors_var& err, const char * handle, int techID, int lang,
  short position)
{
  return SetReasonContactMap(err, ccReg::nsset_tech, handle, techID, lang,
      position, true);
}

short ccReg_EPP_i::SetReasonNSSetTechADD(
  ccReg::Errors_var& err, const char * handle, int techID, int lang,
  short position)
{
  return SetReasonContactMap(err, ccReg::nsset_tech_add, handle, techID, lang,
      position, true);
}

short ccReg_EPP_i::SetReasonNSSetTechREM(
  ccReg::Errors_var& err, const char * handle, int techID, int lang,
  short position)
{
  return SetReasonContactMap(err, ccReg::nsset_tech_rem, handle, techID, lang,
      position, true);
}

short int
ccReg_EPP_i::SetReasonKeySetTech(
        ccReg::Errors_var &err,
        const char *handle,
        int techID,
        int lang,
        short int position)
{
    return SetReasonContactMap(err, ccReg::keyset_tech, handle, techID,
            lang, position, true);
}

short int
ccReg_EPP_i::SetReasonKeySetTechADD(
        ccReg::Errors_var &err,
        const char *handle,
        int techID,
        int lang,
        short int position)
{
    return SetReasonContactMap(err, ccReg::keyset_tech_add, handle, techID,
            lang, position, true);
}

short int
ccReg_EPP_i::SetReasonKeySetTechREM(
        ccReg::Errors_var &err,
        const char *handle,
        int techID,
        int lang,
        short int position)
{
    return SetReasonContactMap(err, ccReg::keyset_tech_rem, handle, techID,
            lang, position, true);
}


short ccReg_EPP_i::SetReasonDomainAdmin(
  ccReg::Errors_var& err, const char * handle, int adminID, int lang,
  short position)
{
  return SetReasonContactMap(err, ccReg::domain_admin, handle, adminID, lang,
      position, false);
}

short ccReg_EPP_i::SetReasonDomainAdminADD(
  ccReg::Errors_var& err, const char * handle, int adminID, int lang,
  short position)
{
  return SetReasonContactMap(err, ccReg::domain_admin_add, handle, adminID,
      lang, position, false);
}

short ccReg_EPP_i::SetReasonDomainAdminREM(
  ccReg::Errors_var& err, const char * handle, int adminID, int lang,
  short position)
{
  return SetReasonContactMap(err, ccReg::domain_admin_rem, handle, adminID,
      lang, position, false);
}

short ccReg_EPP_i::SetReasonDomainTempCREM(
  ccReg::Errors_var& err, const char * handle, int adminID, int lang,
  short position)
{
  return SetReasonContactMap(err, ccReg::domain_tmpcontact, handle, adminID,
      lang, position, false);
}

short ccReg_EPP_i::SetReasonNSSetTechExistMap(
  ccReg::Errors_var& err, const char * handle, int lang, short position)
{
  LOG( WARNING_LOG, "Tech Contact [%s] exist in contact map table" , (const char *) handle );
  return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::nsset_tech_add,
      position +1, REASON_MSG_TECH_EXIST, lang);
}

short ccReg_EPP_i::SetReasonNSSetTechNotExistMap(
  ccReg::Errors_var& err, const char * handle, int lang, short position)
{
  LOG( WARNING_LOG, "Tech Contact [%s] notexist in contact map table" , (const char *) handle );
  return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::nsset_tech_rem,
      position+1, REASON_MSG_TECH_NOTEXIST, lang);
}

short int
ccReg_EPP_i::SetReasonKeySetTechExistMap(
        ccReg::Errors_var &err,
        const char *handle,
        int lang,
        short int position)
{
    LOG(WARNING_LOG, "Tech contact [%s] exist in contact map table",
            (const char *)handle);
    return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::keyset_tech_add,
            position+1, REASON_MSG_TECH_EXIST, lang);
}

short int
ccReg_EPP_i::SetReasonKeySetTechNotExistMap(
        ccReg::Errors_var &err,
        const char *handle,
        int lang,
        short int position)
{
    LOG(WARNING_LOG, "Tech contact [%s] does not exist in contact map table",
            (const char *)handle);
    return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::keyset_tech_rem,
            position+1, REASON_MSG_TECH_NOTEXIST, lang);
}

short ccReg_EPP_i::SetReasonDomainAdminExistMap(
  ccReg::Errors_var& err, const char * handle, int lang, short position)
{
  LOG( WARNING_LOG, "Admin Contact [%s] exist in contact map table" , (const char *) handle );
  return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::domain_admin_add,
      position+1, REASON_MSG_ADMIN_EXIST, lang);
}

short ccReg_EPP_i::SetReasonDomainAdminNotExistMap(
  ccReg::Errors_var& err, const char * handle, int lang, short position)
{
  LOG( WARNING_LOG, "Admin Contact [%s] notexist in contact map table" , (const char *) handle );
  return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::domain_admin_rem,
      position+1, REASON_MSG_ADMIN_NOTEXIST, lang);
}

short ccReg_EPP_i::SetReasonDomainTempCNotExistMap(
  ccReg::Errors_var& err, const char * handle, int lang, short position)
{
  LOG( WARNING_LOG, "Temp Contact [%s] notexist in contact map table" , (const char *) handle );
  return SetErrorReason(err, COMMAND_PARAMETR_ERROR, ccReg::domain_tmpcontact,
      position+1, REASON_MSG_ADMIN_NOTEXIST, lang);
}

short
ccReg_EPP_i::SetReasonWrongRegistrar(
        ccReg::Errors_var &err,
        int registrarId,
        int lang)
{
    return SetErrorReason(
            err,
            COMMAND_AUTOR_ERROR,
            ccReg::registrar_autor,
            0,
            REASON_MSG_REGISTRAR_AUTOR,
            lang);
}

// load country code table  enum_country from database
int ccReg_EPP_i::LoadCountryCode()
{
  Logging::Context ctx("rifd");

  DB DBsql;
  int i, rows;

  if (DBsql.OpenDatabase(database) ) {
    rows=0;
    if (DBsql.ExecSelect("SELECT id FROM enum_country order by id;") ) {
      rows = DBsql.GetSelectRows();
      CC = new CountryCode( rows );
      for (i = 0; i < rows; i ++)
        CC->AddCode(DBsql.GetFieldValue(i, 0) );
      DBsql.FreeSelect();
    }

    DBsql.Disconnect();
  } else
    return -1;

  return rows;
}

bool ccReg_EPP_i::TestCountryCode(
  const char *cc)
{
  LOG( NOTICE_LOG , "CCREG:: TestCountryCode  [%s]" , cc );

  // if not country code
  if (strlen(cc) == 0)
    return true;
  else {
    if (strlen(cc) == 2) // must by two counry code
    {
      LOG( NOTICE_LOG , "TestCountryCode [%s]" , cc);
      return CC->TestCountryCode(cc);
    } else
      return false;
  }

}

// get version of the server and actual time
char* ccReg_EPP_i::version(
  ccReg::timestamp_out datetime)
{
  Logging::Context ctx("rifd");

  time_t t;
  char dateStr[MAX_DATE+1];

  LOG( NOTICE_LOG, "get version %s BUILD %s %s", VERSION, __DATE__, __TIME__);

  // return  actual time (local time)
  t = time(NULL);
  get_rfc3339_timestamp(t, dateStr, false);
  datetime = CORBA::string_dup(dateStr);

  // XXX there was memory leak in server.cc (main:277): free() added there
  return strdup("DSDng");
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

// ZONE manager to load zones into memory

int ccReg_EPP_i::loadZones() // load zones
{
  Logging::Context ctx("rifd");

  int rows=0, i;
  DB DBsql;

  LOG( NOTICE_LOG, "LOAD zones" );
  zone = new ccReg::Zones;

  if (DBsql.OpenDatabase(database) ) {

    if (DBsql.ExecSelect("select * from zone order by length(fqdn) desc") ) {
      rows = DBsql.GetSelectRows();
      max_zone = rows;

      LOG( NOTICE_LOG, "Max zone  -> %d " , max_zone );

      zone->length(rows);

      for (i = 0; i < rows; i ++) {
        (*zone)[i].id=atoi(DBsql.GetFieldValueName("id", i));
        (*zone)[i].fqdn
            =CORBA::string_dup(DBsql.GetFieldValueName("fqdn", i) );
        (*zone)[i].ex_period_min= atoi(DBsql.GetFieldValueName(
            "ex_period_min", i));
        (*zone)[i].ex_period_max= atoi(DBsql.GetFieldValueName(
            "ex_period_max", i));
        (*zone)[i].val_period
            = atoi(DBsql.GetFieldValueName("val_period", i));
        (*zone)[i].dots_max= atoi(DBsql.GetFieldValueName("dots_max", i) );
        (*zone)[i].enum_zone
            = DBsql.GetFieldBooleanValueName("enum_zone", i);
        LOG( NOTICE_LOG, "Get ZONE %d fqdn [%s] ex_period_min %d ex_period_max %d val_period %d dots_max %d  enum_zone %d" , i+1 ,
            ( char *) (*zone)[i].fqdn , (*zone)[i].ex_period_min , (*zone)[i].ex_period_max , (*zone)[i].val_period , (*zone)[i].dots_max , (*zone)[i].enum_zone );
      }

      DBsql.FreeSelect();
    }

    DBsql.Disconnect();
  }

  if (rows == 0)
    zone->length(rows);

  return rows;
}

unsigned int ccReg_EPP_i::GetZoneLength()
{
  return zone->length() ;
}
unsigned int ccReg_EPP_i::GetZoneID(
  unsigned int z)
{
  if (z < zone->length() )
    return (*zone)[z].id;
  else
    return 0;
}

// ZONE parameters
int ccReg_EPP_i::GetZoneExPeriodMin(
  int id)
{
  unsigned int z;

  for (z = 0; z < zone->length() ; z ++) {
    if ((*zone)[z].id == id)
      return (*zone)[z].ex_period_min;
  }

  return 0;
}

int ccReg_EPP_i::GetZoneExPeriodMax(
  int id)
{
  unsigned int z;

  for (z = 0; z < zone->length() ; z ++) {
    if ((*zone)[z].id == id)
      return (*zone)[z].ex_period_max;
  }

  return 0;
}

int ccReg_EPP_i::GetZoneValPeriod(
  int id)
{
  unsigned int z;

  for (z = 0; z < zone->length() ; z ++) {
    if ((*zone)[z].id == id)
      return (*zone)[z].val_period;
  }

  return 0;
}

bool ccReg_EPP_i::GetZoneEnum(
  int id)
{
  unsigned int z;

  for (z = 0; z < zone->length() ; z ++) {
    if ((*zone)[z].id == id)
      return (*zone)[z].enum_zone;
  }

  return false;
}

int ccReg_EPP_i::GetZoneDotsMax(
  int id)
{
  unsigned int z;

  for (z = 0; z < zone->length() ; z ++) {
    if ((*zone)[z].id == id)
      return (*zone)[z].dots_max;
  }

  return 0;
}

const char * ccReg_EPP_i::GetZoneFQDN(
  int id)
{
  unsigned int z;
  for (z = 0; z < zone->length() ; z ++) {
    if ((*zone)[z].id == id)
      return (char *) (*zone)[z].fqdn ;
  }

  return "";
}

int ccReg_EPP_i::getZone(
  const char *fqdn)
{
  return getZZ(fqdn, true);
}

int ccReg_EPP_i::getZoneMax(
  const char *fqdn)
{
  return getZZ(fqdn, false);
}

int ccReg_EPP_i::getZZ(
  const char *fqdn, bool compare)
{
  int max, i;
  int len, slen, l;

  max = (int ) zone->length();

  len = strlen(fqdn);

  for (i = 0; i < max; i ++) {

    slen = strlen( (char *) (*zone)[i].fqdn );
    l = len - slen;

    if (l > 0) {
      if (fqdn[l-1] == '.') // case less compare
      {
        if (compare) {
          if (strncasecmp(fqdn+l, (char *) (*zone)[i].fqdn , slen) == 0)
            return (*zone)[i].id; // place into zone, return ID
        } else
          return l -1; // return end of name
      }
    }

  }

  // v return end of domain without dot
  if (compare == false) {
    for (l = len-1; l > 0; l --) {
      if (fqdn[l] == '.')
        return l-1; //  return end of name
    }
  }

  return 0;
}

bool ccReg_EPP_i::testFQDN(
  const char *fqdn)
{
  Logging::Context ctx("rifd");

  char FQDN[164];

  if (getFQDN(FQDN, fqdn) > 0)
    return true;
  else
    return false;
}

int ccReg_EPP_i::getFQDN(
  char *FQDN, const char *fqdn)
{
  int i, len, max;
  int z;
  int dot=0, dot_max;
  bool en;
  z = getZone(fqdn);
  max = getZoneMax(fqdn); // return the end

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
  dot_max = GetZoneDotsMax(z);

  if (dot > dot_max) {
    LOG( LOG_DEBUG , "too much %d dots max %d" , dot , dot_max );
    return -1;
  }

  en = GetZoneEnum(z);

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
  CORBA::Long clientID)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  LOG( DEBUG_LOG , "SESSION CLOSED by clientID %ld, Calling SESSION LOGOUT", clientID );
  LogoutSession(clientID);
}

/***********************************************************************
 *
 * FUNCTION:    SaveOutXML
 *
 * DESCRIPTION: save exit XML according to server generated
 *              transaction ID
 *
 * PARAMETERS:  svTRID - client transaction number
 *              XML - xml exit string from mod_eppd
 *
 * RETURNED:    true if success save
 *
 ***********************************************************************/

CORBA::Boolean ccReg_EPP_i::SaveOutXML(
  const char* svTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("%1%") % svTRID));

  DB DBsql;
  int ok;

  if (DBsql.OpenDatabase(database) ) {
    if (DBsql.BeginTransaction() ) {

      if (DBsql.SaveXMLout(svTRID, XML) )
        ok=CMD_OK;
      else
        ok=0;

      DBsql.QuitTransaction(ok);
    }

    DBsql.Disconnect();
  } else
    ServerInternalError("SaveOutXML");

  // default
  return true;
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
 *              errCode - save error report from client into table action
 *
 * RETURNED:    svTRID and errCode msg
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::GetTransaction(
  CORBA::Short errCode, CORBA::Long clientID, const char* clTRID,
  const ccReg::XmlErrors& errorCodes, ccReg::ErrorStrings_out errStrings)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  DB DBsql;
  ccReg::Response_var ret;
  ret = new ccReg::Response;
  int i, len;

  LOG( NOTICE_LOG, "GetTransaction: clientID -> %d clTRID [%s] ", (int ) clientID, clTRID );
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
      if (DBsql.BeginAction(clientID, EPP_UnknowAction, clTRID, "")) {
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
 *              clTRID - transaction client number
 *              clientID - client identification
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::PollAcknowledgement(
  const char* msgID, CORBA::Short& count, CORBA::String_out newmsgID,
  CORBA::Long clientID, const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  LOG(
      NOTICE_LOG,
      "PollAcknowledgement: clientID -> %d clTRID [%s] msgID -> %s",
      (int) clientID, clTRID, msgID
  );
  // start EPP action - this will handle all init stuff
  EPPAction a(this, clientID, EPP_PollAcknowledgement, clTRID, XML);
  try {
    std::auto_ptr<Register::Poll::Manager> pollMan(
        Register::Poll::Manager::create(a.getDB())
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
  catch (Register::NOT_FOUND) {
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
 *              clTRID - transaction client number
 *              clientID - client identification
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::PollRequest(
  CORBA::String_out msgID, CORBA::Short& count, ccReg::timestamp_out qDate,
  ccReg::PollType& type, CORBA::Any_OUT_arg msg, CORBA::Long clientID,
  const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  LOG(
      NOTICE_LOG,
      "PollRequest: clientID -> %d clTRID [%s]", (int ) clientID, clTRID
  );
  // start EPP action - this will handle all init stuff
  EPPAction a(this, clientID, EPP_PollResponse, clTRID, XML);
  Register::Poll::Message *m= NULL;
  try {
    std::auto_ptr<Register::Poll::Manager> pollMan(
        Register::Poll::Manager::create(a.getDB())
    );
    // fill count
    count = pollMan->getMessageCount(a.getRegistrar());
    if (!count) a.NoMessage(); // throw exception NoMessage
    m = pollMan->getNextMessage(a.getRegistrar());
  }
  catch (ccReg::EPP::NoMessages) {throw;}
  catch (...) {a.failedInternal("Connection problems");}
  if (!m)
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
  Register::Poll::MessageEventReg *mer =
      dynamic_cast<Register::Poll::MessageEventReg *>(m);
  if (mer) {
    switch (m->getType()) {
      case Register::Poll::MT_TRANSFER_CONTACT:
        type = ccReg::polltype_transfer_contact;
        break;
      case Register::Poll::MT_TRANSFER_NSSET:
        type = ccReg::polltype_transfer_nsset;
        break;
      case Register::Poll::MT_TRANSFER_DOMAIN:
        type = ccReg::polltype_transfer_domain;
        break;
      case Register::Poll::MT_TRANSFER_KEYSET:
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
  Register::Poll::MessageEvent *me =
      dynamic_cast<Register::Poll::MessageEvent *>(m);
  if (me) {
    switch (m->getType()) {
      case Register::Poll::MT_DELETE_CONTACT:
        type = ccReg::polltype_delete_contact;
        break;
      case Register::Poll::MT_DELETE_NSSET:
        type = ccReg::polltype_delete_nsset;
        break;
      case Register::Poll::MT_DELETE_DOMAIN:
        type = ccReg::polltype_delete_domain;
        break;
      case Register::Poll::MT_DELETE_KEYSET:
        type = ccReg::polltype_delete_keyset;
        break;
      case Register::Poll::MT_IMP_EXPIRATION:
        type = ccReg::polltype_impexpiration;
        break;
      case Register::Poll::MT_EXPIRATION:
        type = ccReg::polltype_expiration;
        break;
      case Register::Poll::MT_IMP_VALIDATION:
        type = ccReg::polltype_impvalidation;
        break;
      case Register::Poll::MT_VALIDATION:
        type = ccReg::polltype_validation;
        break;
      case Register::Poll::MT_OUTZONE:
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
  Register::Poll::MessageLowCredit *mlc =
      dynamic_cast<Register::Poll::MessageLowCredit *>(m);
  if (mlc) {
    type = ccReg::polltype_lowcredit;
    ccReg::PollMsg_LowCredit *hdm = new ccReg::PollMsg_LowCredit;
    hdm->zone = CORBA::string_dup(mlc->getZone().c_str());
    hdm->limit = mlc->getLimit();
    hdm->credit = mlc->getCredit();
    *msg <<= hdm;
    return a.getRet()._retn();
  }
  Register::Poll::MessageTechCheck *mtc =
      dynamic_cast<Register::Poll::MessageTechCheck *>(m);
  if (mtc) {
    type = ccReg::polltype_techcheck;
    ccReg::PollMsg_Techcheck *hdm = new ccReg::PollMsg_Techcheck;
    hdm->handle = CORBA::string_dup(mtc->getHandle().c_str());
    hdm->fqdns.length(mtc->getFQDNS().size());
    for (unsigned i=0; i<mtc->getFQDNS().size(); i++)
      hdm->fqdns[i] = mtc->getFQDNS()[i].c_str();
    hdm->tests.length(mtc->getTests().size());
    for (unsigned i=0; i<mtc->getTests().size(); i++) {
      Register::Poll::MessageTechCheckItem *test = mtc->getTests()[i];
      hdm->tests[i].testname = CORBA::string_dup(test->getTestname().c_str());
      hdm->tests[i].status = test->getStatus();
      hdm->tests[i].note = CORBA::string_dup(test->getNote().c_str());
    }
    *msg <<= hdm;
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
 * PARAMETERS:  clientID - id of connected client
 *              clTRID - transaction client number
 *        OUT:  credit - credit amount in haler
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::ClientCredit(
  ccReg::ZoneCredit_out credit, CORBA::Long clientID, const char* clTRID,
  const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  DB DBsql;
  ccReg::Response_var ret;
  int regID;
  long price;
  unsigned int z, seq, zoneID;

  ret = new ccReg::Response;
  ret->code = 0;

  credit = new ccReg::ZoneCredit;
  credit->length(0);
  seq=0;

  LOG( NOTICE_LOG, "ClientCredit: clientID -> %d clTRID [%s]", (int ) clientID, clTRID );

  if ( (regID = GetRegistrarID(clientID) ))
    if (DBsql.OpenDatabase(database) ) {
      if ( (DBsql.BeginAction(clientID, EPP_ClientCredit, clTRID, XML) )) {

        for (z = 0; z < GetZoneLength() ; z ++) {
          zoneID = GetZoneID(z);
          // credit of the registrar
          price = DBsql.GetRegistrarCredit(regID, zoneID);

          //  return all not depend on            if( price >  0)
          {
            credit->length(seq+1);
            credit[seq].price = price;
            credit[seq].zone_fqdn = CORBA::string_dup(GetZoneFQDN(zoneID) );
            seq++;
          }

        }

        ret->code = COMMAND_OK;

        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
      }

      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
    }

  if (ret->code == 0)
    ServerInternalError("ClientCredit");

  return ret._retn();
}

/***********************************************************************
 *
 * FUNCTION:    ClientLogout
 *
 * DESCRIPTION: client logout for record into table login
 *              about logout date
 * PARAMETERS:  clientID - connected client id
 *              clTRID - transaction client number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::ClientLogout(
  CORBA::Long clientID, const char *clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  DB DBsql;
  ccReg::Response_var ret;
  int lang=0; // default


  ret = new ccReg::Response;
  ret->code = 0;

  LOG( NOTICE_LOG, "ClientLogout: clientID -> %d clTRID [%s]", (int ) clientID, clTRID );

  if (DBsql.OpenDatabase(database) ) {
    if (DBsql.BeginAction(clientID, EPP_ClientLogout, clTRID, XML) ) {

      DBsql.UPDATE("Login");
      DBsql.SET("logoutdate", "now");
      DBsql.SET("logouttrid", clTRID);
      DBsql.WHEREID(clientID);

      if (DBsql.EXEC() ) {
        lang = GetRegistrarLang(clientID); // remember lang of the client before logout

        LogoutSession(clientID); // logout session
        ret->code = COMMAND_LOGOUT; // succesfully logout
      }

      ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
    }

    // return messages
    ret->msg =CORBA::string_dup(GetErrorMessage(ret->code, lang) );

    DBsql.Disconnect();
  }

  if (ret->code == 0)
    ServerInternalError("ClientLogout");

  return ret._retn();
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
 *        OUT:  clientID - connected client id
 *              clTRID - transaction client number
 *              certID - certificate fingerprint
 *              language - communication language of client en or cs empty value = en
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::ClientLogin(
  const char *ClID, const char *passwd, const char *newpass,
  const char *clTRID, const char* XML, CORBA::Long & clientID,
  const char *certID, ccReg::Languages lang)
{
  Logging::Context ctx("rifd");

  DB DBsql;
  int regID=0, id=0;
  int language=0;
  ccReg::Response_var ret;
  ret = new ccReg::Response;

  // default
  ret->code = 0;
  clientID = 0;

  LOG( NOTICE_LOG, "ClientLogin: username-> [%s] clTRID [%s] passwd [%s]  newpass [%s] ", ClID, clTRID, passwd, newpass );
  LOG( NOTICE_LOG, "ClientLogin:  certID  [%s] language  [%d] ", certID, lang );

  if (DBsql.OpenDatabase(database) ) {

    // get ID of registrar by handle
    if ( (regID = DBsql.GetNumericFromTable("REGISTRAR", "id", "handle",
        ( char * ) ClID ) ) == 0) {
      LOG( NOTICE_LOG, "bad username [%s]", ClID );
      // bad username
      ret->code = COMMAND_AUTH_ERROR;
    } else

    // test password and certificate fingerprint in the table RegistrarACL
    if ( !DBsql.TestRegistrarACL(regID, passwd, certID) ) {
      LOG( NOTICE_LOG, "password [%s]  or certID [%s]  not accept", passwd , certID );
      ret->code = COMMAND_AUTH_ERROR;
    } else

    if (DBsql.BeginTransaction() ) {
      id = DBsql.GetSequenceID("login"); // get sequence ID from login table

      // write to table
      DBsql.INSERT("Login");
      DBsql.INTO("id");
      DBsql.INTO("registrarid");
      DBsql.INTO("logintrid");
      DBsql.VALUE(id);
      DBsql.VALUE(regID);
      DBsql.VALUE(clTRID);

      if (DBsql.EXEC() ) // if sucess write
      {
        clientID = id;

        LOG( NOTICE_LOG, "GET clientID  -> %d", (int ) clientID );

        // change language
        if (lang == ccReg::CS) {
          LOG( NOTICE_LOG, "SET LANG to CS" );

          DBsql.UPDATE("Login");
          DBsql.SSET("lang", "cs");
          DBsql.WHEREID(clientID);
          language=1;
          if (DBsql.EXEC() == false)
            ret->code = COMMAND_FAILED; // if failed
        }

        // change password if set new
        if (strlen(newpass) ) {
          LOG( NOTICE_LOG, "change password  [%s]  to  newpass [%s] ", passwd, newpass );

          DBsql.UPDATE("REGISTRARACL");
          DBsql.SET("password", newpass);
          DBsql.WHERE("registrarid", regID);

          if (DBsql.EXEC() == false)
            ret->code = COMMAND_FAILED; // if failed
        }

        if (ret->code == 0) {
          if (LoginSession(clientID, regID, language) )
            ret->code = COMMAND_OK;
          else {
            clientID=0; //  not login
            ret->code =COMMAND_MAX_SESSION_LIMIT; // maximal limit of connection sessions
          }
        }
      }

      // end of transaction
      DBsql.QuitTransaction(ret->code);
    }

    // write  to table action aand return  svTRID
    if (DBsql.BeginAction(clientID, EPP_ClientLogin, clTRID, XML) ) {
      ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );

      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );
    } else
      ServerInternalError("ClientLogin");

    DBsql.Disconnect();
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
 *        OUT:  a - (1) object doesn't exist and it is free
 *                  (0) object is already established
 *              clientID - connected client id
 *              clTRID - client transaction number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::ObjectCheck(
  short act, const char * table, const char *fname, const ccReg::Check& chck,
  ccReg::CheckResp_out a, CORBA::Long clientID, const char* clTRID,
  const char* XML)
{
  DB DBsql;
  ccReg::Response_var ret;
  unsigned long i, len;

  Register::NameIdPair caConflict;
  Register::Domain::CheckAvailType caType;
  Register::Contact::Manager::CheckAvailType cType;
  Register::NSSet::Manager::CheckAvailType nType;
  Register::KeySet::Manager::CheckAvailType kType;

  ret = new ccReg::Response;

  a = new ccReg::CheckResp;

  ret->code=0;

  len = chck.length();
  a->length(len);

  ParsedAction paction;
  for (unsigned i=0; i<len; i++)
    paction.add((unsigned)1,(const char *)chck[i]);

  LOG( NOTICE_LOG , "OBJECT %d  Check: clientID -> %d clTRID [%s] " , act , (int ) clientID , clTRID );

  if (DBsql.OpenDatabase(database) ) {

    if (DBsql.BeginAction(clientID, act, clTRID, XML, &paction) ) {

      for (i = 0; i < len; i ++) {
        switch (act) {
          case EPP_ContactCheck:
            try {
              std::auto_ptr<Register::Contact::Manager> cman( Register::Contact::Manager::create(&DBsql,conf.get<bool>("registry.restricted_handles")) );

              LOG( NOTICE_LOG , "contact checkAvail handle [%s]" , (const char * ) chck[i] );

              cType = cman->checkAvail( ( const char * ) chck[i] , caConflict );
              LOG( NOTICE_LOG , "contact type %d" , cType );
              switch (cType) {
                case Register::Contact::Manager::CA_INVALID_HANDLE:
                  a[i].avail = ccReg::BadFormat;
                  a[i].reason
                      = CORBA::string_dup(GetReasonMessage( REASON_MSG_INVALID_FORMAT , GetRegistrarLang( clientID )));
                  LOG( NOTICE_LOG , "bad format %s of contact handle" , (const char * ) chck[i] );
                  break;
                case Register::Contact::Manager::CA_REGISTRED:
                  a[i].avail = ccReg::Exist;
                  a[i].reason
                      = CORBA::string_dup(GetReasonMessage( REASON_MSG_REGISTRED , GetRegistrarLang( clientID )) );
                  LOG( NOTICE_LOG , "contact %s exist not Avail" , (const char * ) chck[i] );
                  break;

                case Register::Contact::Manager::CA_PROTECTED:
                  a[i].avail = ccReg::DelPeriod;
                  a[i].reason
                      = CORBA::string_dup(GetReasonMessage( REASON_MSG_PROTECTED_PERIOD , GetRegistrarLang( clientID )) ); // v ochrane lhute
                  LOG( NOTICE_LOG , "contact %s in delete period" ,(const char * ) chck[i] );
                  break;
                case Register::Contact::Manager::CA_FREE:
                  a[i].avail = ccReg::NotExist;
                  a[i].reason = CORBA::string_dup(""); // free
                  LOG( NOTICE_LOG , "contact %s not exist  Avail" ,(const char * ) chck[i] );
                  break;
              }
            }
            catch (...) {
              LOG( WARNING_LOG, "cannot run Register::Contact::checkAvail");
              ret->code=COMMAND_FAILED;
            }
            break;

          case EPP_KeySetCheck:
            try {
                std::auto_ptr<Register::KeySet::Manager> kman(
                        Register::KeySet::Manager::create(
                            &DBsql, conf.get<bool>("registry.restricted_handles")));
                LOG(NOTICE_LOG, "keyset checkAvail handle [%s]",
                        (const char *)chck[i]);

                kType = kman->checkAvail((const char *)chck[i], caConflict);
                LOG(NOTICE_LOG, "keyset check type %d", kType);
                switch (kType) {
                    case Register::KeySet::Manager::CA_INVALID_HANDLE:
                        a[i].avail = ccReg::BadFormat;
                        a[i].reason = CORBA::string_dup(
                                GetReasonMessage(
                                    REASON_MSG_INVALID_FORMAT,
                                    GetRegistrarLang(clientID))
                                );
                        LOG(NOTICE_LOG, "bad format %s of keyset handle",
                                (const char *)chck[i]);
                        break;
                    case Register::KeySet::Manager::CA_REGISTRED:
                        a[i].avail = ccReg::Exist;
                        a[i].reason = CORBA::string_dup(
                                GetReasonMessage(
                                    REASON_MSG_REGISTRED,
                                    GetRegistrarLang(clientID))
                                );
                        LOG(NOTICE_LOG, "keyset %s exist not avail",
                                (const char *)chck[i]);
                        break;
                    case Register::KeySet::Manager::CA_PROTECTED:
                        a[i].avail = ccReg::DelPeriod;
                        a[i].reason = CORBA::string_dup(
                                GetReasonMessage(
                                    REASON_MSG_PROTECTED_PERIOD,
                                    GetRegistrarLang(clientID))
                                );
                        LOG(NOTICE_LOG, "keyset %s in delete period",
                                (const char *)chck[i]);
                        break;
                    case Register::KeySet::Manager::CA_FREE:
                        a[i].avail = ccReg::NotExist;
                        a[i].reason = CORBA::string_dup(""); //free
                        LOG(NOTICE_LOG, "keyset %s not exist Available",
                                (const char *)chck[i]);
                        break;
                }
            }
            catch (...) {
                LOG(WARNING_LOG, "cannot run Register::Contact::checkAvail");
                ret->code = COMMAND_FAILED;
            }
            break;

          case EPP_NSsetCheck:

            try {
              std::auto_ptr<Register::Zone::Manager> zman( Register::Zone::Manager::create(&DBsql) );
              std::auto_ptr<Register::NSSet::Manager> nman( Register::NSSet::Manager::create(&DBsql,zman.get(),conf.get<bool>("registry.restricted_handles")) );

              LOG( NOTICE_LOG , "nsset checkAvail handle [%s]" , (const char * ) chck[i] );

              nType = nman->checkAvail( ( const char * ) chck[i] , caConflict );
              LOG( NOTICE_LOG , "nsset check type %d" , nType );
              switch (nType) {
                case Register::NSSet::Manager::CA_INVALID_HANDLE:
                  a[i].avail = ccReg::BadFormat;
                  a[i].reason
                      = CORBA::string_dup(GetReasonMessage( REASON_MSG_INVALID_FORMAT , GetRegistrarLang( clientID )));
                  LOG( NOTICE_LOG , "bad format %s of nsset handle" , (const char * ) chck[i] );
                  break;
                case Register::NSSet::Manager::CA_REGISTRED:
                  a[i].avail = ccReg::Exist;
                  a[i].reason
                      = CORBA::string_dup(GetReasonMessage( REASON_MSG_REGISTRED , GetRegistrarLang( clientID )) );
                  LOG( NOTICE_LOG , "nsset %s exist not Avail" , (const char * ) chck[i] );
                  break;

                case Register::NSSet::Manager::CA_PROTECTED:
                  a[i].avail = ccReg::DelPeriod;
                  a[i].reason
                      = CORBA::string_dup(GetReasonMessage( REASON_MSG_PROTECTED_PERIOD , GetRegistrarLang( clientID )) ); // v ochrane lhute
                  LOG( NOTICE_LOG , "nsset %s in delete period" ,(const char * ) chck[i] );
                  break;
                case Register::NSSet::Manager::CA_FREE:
                  a[i].avail = ccReg::NotExist;
                  a[i].reason = CORBA::string_dup("");
                  LOG( NOTICE_LOG , "nsset %s not exist  Avail" ,(const char * ) chck[i] );
                  break;
              }
            }
            catch (...) {
              LOG( WARNING_LOG, "cannot run Register::NSSet::checkAvail");
              ret->code=COMMAND_FAILED;
            }


            break;

          case EPP_DomainCheck:

            try {
              std::auto_ptr<Register::Zone::Manager> zm( Register::Zone::Manager::create(&DBsql) );
              std::auto_ptr<Register::Domain::Manager> dman( Register::Domain::Manager::create(&DBsql,zm.get()) );

              LOG( NOTICE_LOG , "domain checkAvail fqdn [%s]" , (const char * ) chck[i] );

              caType = dman->checkAvail( ( const char * ) chck[i] , caConflict);
              LOG( NOTICE_LOG , "domain type %d" , caType );
              switch (caType) {
                case Register::Domain::CA_INVALID_HANDLE:
                case Register::Domain::CA_BAD_LENGHT:
                  a[i].avail = ccReg::BadFormat;
                  a[i].reason
                      = CORBA::string_dup(GetReasonMessage(REASON_MSG_INVALID_FORMAT , GetRegistrarLang( clientID )) );
                  LOG( NOTICE_LOG , "bad format %s of fqdn" , (const char * ) chck[i] );
                  break;
                case Register::Domain::CA_REGISTRED:
                case Register::Domain::CA_CHILD_REGISTRED:
                case Register::Domain::CA_PARENT_REGISTRED:
                  a[i].avail = ccReg::Exist;
                  a[i].reason
                      = CORBA::string_dup(GetReasonMessage( REASON_MSG_REGISTRED , GetRegistrarLang( clientID )) );
                  LOG( NOTICE_LOG , "domain %s exist not Avail" , (const char * ) chck[i] );
                  break;
                case Register::Domain::CA_BLACKLIST:
                  a[i].avail = ccReg::BlackList;
                  a[i].reason
                      = CORBA::string_dup(GetReasonMessage( REASON_MSG_BLACKLISTED_DOMAIN , GetRegistrarLang( clientID )) );
                  LOG( NOTICE_LOG , "blacklisted  %s" , (const char * ) chck[i] );
                  break;
                case Register::Domain::CA_AVAILABLE:
                  a[i].avail = ccReg::NotExist;
                  a[i].reason = CORBA::string_dup(""); // free
                  LOG( NOTICE_LOG , "domain %s not exist  Avail" ,(const char * ) chck[i] );
                  break;
                case Register::Domain::CA_BAD_ZONE:
                  a[i].avail = ccReg::NotApplicable; // unusable domain isn't in zone
                  a[i].reason
                      = CORBA::string_dup(GetReasonMessage(REASON_MSG_NOT_APPLICABLE_DOMAIN , GetRegistrarLang( clientID )) );
                  LOG( NOTICE_LOG , "not applicable %s" , (const char * ) chck[i] );
                  break;
              }

              /*
               #      CA_INVALID_HANDLE, ///< bad formed handle
               #      CA_BAD_ZONE, ///< domain outside of register
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
              LOG( WARNING_LOG, "cannot run Register::Domain::checkAvail");
              ret->code=COMMAND_FAILED;
            }
            break;

        }
      }

      // command OK
      if (ret->code == 0)
        ret->code=COMMAND_OK; // OK not errors


      ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) ) ;
    }

    ret->msg = CORBA::string_dup(GetErrorMessage(ret->code,
        GetRegistrarLang(clientID) ) ) ;

    DBsql.Disconnect();
  }

  if (ret->code == 0)
    ServerInternalError("ObjectCheck");

  return ret._retn();
}

ccReg::Response* ccReg_EPP_i::ContactCheck(
  const ccReg::Check& handle, ccReg::CheckResp_out a, CORBA::Long clientID,
  const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  return ObjectCheck( EPP_ContactCheck , "CONTACT" , "handle" , handle , a , clientID , clTRID , XML);
}

ccReg::Response* ccReg_EPP_i::NSSetCheck(
  const ccReg::Check& handle, ccReg::CheckResp_out a, CORBA::Long clientID,
  const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  return ObjectCheck( EPP_NSsetCheck , "NSSET" , "handle" , handle , a , clientID , clTRID , XML);
}

ccReg::Response* ccReg_EPP_i::DomainCheck(
  const ccReg::Check& fqdn, ccReg::CheckResp_out a, CORBA::Long clientID,
  const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  return ObjectCheck( EPP_DomainCheck , "DOMAIN" , "fqdn" , fqdn , a , clientID , clTRID , XML);
}

ccReg::Response *
ccReg_EPP_i::KeySetCheck(
        const ccReg::Check &handle,
        ccReg::CheckResp_out a,
        CORBA::Long clientID,
        const char *clTRID,
        const char *XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

    return ObjectCheck(
            EPP_KeySetCheck, "KEYSET", "handle", handle,
            a, clientID, clTRID, XML);
}

/***********************************************************************
 *
 * FUNCTION:    ContactInfo
 *
 * DESCRIPTION: returns detailed information about contact
 *              empty value if contact doesn't exist
 * PARAMETERS:  handle - contact identifier
 *        OUT:  c - contact structure detailed description
 *              clientID - connected client id
 *              clTRID - client transaction number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::ContactInfo(
  const char* handle, ccReg::Contact_out c, CORBA::Long clientID,
  const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  LOG(
      NOTICE_LOG ,
      "ContactInfo: clientID -> %d clTRID [%s] handle [%s] ",
      (int) clientID, clTRID, handle
  );
  ParsedAction paction;
  paction.add(1,(const char*)handle);
  // start EPP action - this will handle all init stuff
  EPPAction a(this, clientID, EPP_ContactInfo, clTRID, XML, &paction);
  // initialize managers for contact manipulation
  std::auto_ptr<Register::Contact::Manager>
      cman(Register::Contact::Manager::create(a.getDB(),
          conf.get<bool>("registry.restricted_handles")) );
  // first check handle for proper format
  if (!cman->checkHandleFormat(handle))
    // failure in handle check, throw exception
    a.failed(SetReasonContactHandle(a.getErrors(), handle, a.getLang()));
  // now load contact by handle
  std::auto_ptr<Register::Contact::List> clist(cman->createList());
  clist->setHandleFilter(handle);
  try {clist->reload();}
  catch (...) {a.failedInternal("Cannot load contacts");}
  if (clist->getCount() != 1)
    // failer because non existance, throw exception
    a.failed(COMMAND_OBJECT_NOT_EXIST);
  // start filling output contact structure
  Register::Contact::Contact *con = clist->getContact(0);
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
    Register::TID stateId = con->getStatusByIdx(i)->getStatusId();
    const Register::StatusDesc* sd = regMan->getStatusDesc(stateId);
    if (!sd || !sd->getExternal())
      continue;
    c->stat.length(c->stat.length()+1);
    c->stat[c->stat.length()-1].value = CORBA::string_dup(sd->getName().c_str() );
    c->stat[c->stat.length()-1].text = CORBA::string_dup(sd->getDesc(
        a.getLang() == LANG_CS ? "CS" : "EN"
    ).c_str());
  }
  if (!c->stat.length()) {
    const Register::StatusDesc* sd = regMan->getStatusDesc(0);
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
 *              clientID - connected client id
 *              clTRID - client transaction number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::ContactDelete(
  const char* handle, CORBA::Long clientID, const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  std::auto_ptr<EPPNotifier> ntf;
  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  DB DBsql;
  int regID, id;

  ret = new ccReg::Response;
  errors = new ccReg::Errors;

  ret->code=0;
  errors->length(0);

  ParsedAction paction;
  paction.add(1,(const char *)handle);

  LOG( NOTICE_LOG , "ContactDelete: clientID -> %d clTRID [%s] handle [%s] " , (int ) clientID , clTRID , handle );

  if ( (regID = GetRegistrarID(clientID) ))

    if (DBsql.OpenDatabase(database) ) {
      if ( (DBsql.BeginAction(clientID, EPP_ContactDelete, clTRID, XML, &paction) )) {
        if (DBsql.BeginTransaction() ) {

          // lock
          id = getIdOfContact(&DBsql, handle, conf, true);

          if (id < 0)
            ret->code= SetReasonContactHandle(errors, handle,
                GetRegistrarLang(clientID) );
          else if (id ==0) {
            LOG( WARNING_LOG, "contact handle [%s] NOT_EXIST", handle );
            ret->code= COMMAND_OBJECT_NOT_EXIST;
          }
          if (!ret->code && !DBsql.TestObjectClientID(id, regID) ) //  if registrar is not client of the object
          {
            LOG( WARNING_LOG, "bad autorization not  creator of handle [%s]", handle );
            ret->code = SetReasonWrongRegistrar(errors, regID, GetRegistrarLang(clientID));
          }
          try {
            if (!ret->code && (
                  testObjectHasState(&DBsql,id,FLAG_serverDeleteProhibited) ||
                  testObjectHasState(&DBsql,id,FLAG_serverUpdateProhibited)
               ))
            {
              LOG( WARNING_LOG, "delete of object %s is prohibited" , handle );
              ret->code = COMMAND_STATUS_PROHIBITS_OPERATION;
            }
          } catch (...) {
            ret->code = COMMAND_FAILED;
          }
          if (!ret->code) {

            ntf.reset(new EPPNotifier(conf.get<bool>("registry.disable_epp_notifier"),mm , &DBsql, regID , id )); // notifier maneger before delete

            // test to  table  domain domain_contact_map and nsset_contact_map for relations
            if (DBsql.TestContactRelations(id) ) // can not be deleted
            {
              LOG( WARNING_LOG, "test contact handle [%s] relations: PROHIBITS_OPERATION", handle );
              ret->code = COMMAND_PROHIBITS_OPERATION;
            } else {
              if (DBsql.SaveObjectDelete(id) ) // save to delete object object_registry.ErDate
              {
                if (DBsql.DeleteContactObject(id) )
                  ret->code = COMMAND_OK; // if deleted successfully
              }

            }

            if (ret->code == COMMAND_OK)
              ntf->Send(); // run notifier

          }

          // end of transaction  commit or  rollback
          DBsql.QuitTransaction(ret->code);
        }

        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
        ParsedAction paction;
        paction.add(1,(const char *)handle);
      }

      ret->msg = CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
    }

  // EPP exception
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  if (ret->code == 0)
    ServerInternalError("ContactDelete");

  return ret._retn();
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
 *              clientID - connected client id
 *              clTRID - client transaction number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::ContactUpdate(
  const char *handle, const ccReg::ContactChange & c, CORBA::Long clientID,
  const char *clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  DB DBsql;
  std::auto_ptr<EPPNotifier> ntf;
  int regID, id;
  int s, snum;
  char streetStr[10];

  ret = new ccReg::Response;
  errors = new ccReg::Errors;

  ret->code = 0;
  errors->length(0);

  ParsedAction paction;
  paction.add(1,(const char*)handle);

  LOG( NOTICE_LOG, "ContactUpdate: clientID -> %d clTRID [%s] handle [%s] ", (int ) clientID, clTRID, handle );
  LOG( NOTICE_LOG, "Discloseflag %d: Disclose Name %d Org %d Add %d Tel %d Fax %d Email %d VAT %d Ident %d NotifyEmail %d" , c.DiscloseFlag ,
      c.DiscloseName , c.DiscloseOrganization , c.DiscloseAddress , c.DiscloseTelephone , c.DiscloseFax , c.DiscloseEmail, c.DiscloseVAT, c.DiscloseIdent, c.DiscloseNotifyEmail );

  if ( (regID = GetRegistrarID(clientID) ))

    if (DBsql.OpenDatabase(database) ) {

      if ( (DBsql.BeginAction(clientID, EPP_ContactUpdate, clTRID, XML, &paction) )) {

        if (DBsql.BeginTransaction() ) {
          // get ID with locking record
          id = getIdOfContact(&DBsql, handle, conf, true);
          // for notification to old notify address, this address must be
          // discovered before change happen
          std::string oldNotifyEmail;
          if (strlen(c.NotifyEmail) && !conf.get<bool>("registry.disable_epp_notifier"))
            oldNotifyEmail = DBsql.GetValueFromTable(
              "contact", "notifyemail", "id", id
            );
          if (id < 0)
            ret->code= SetReasonContactHandle(errors, handle,
                GetRegistrarLang(clientID) );
          else if (id ==0) {
            LOG( WARNING_LOG, "contact handle [%s] NOT_EXIST", handle );
            ret->code= COMMAND_OBJECT_NOT_EXIST;
          }
          if (!ret->code && !DBsql.TestObjectClientID(id, regID) ) {
            LOG( WARNING_LOG, "bad autorization not  client of contact [%s]", handle );
            ret->code = SetReasonWrongRegistrar(errors, regID, GetRegistrarLang(clientID));
          }
          try {
            if (!ret->code && testObjectHasState(&DBsql,id,FLAG_serverUpdateProhibited))
            {
              LOG( WARNING_LOG, "update of object %s is prohibited" , handle );
              ret->code = COMMAND_STATUS_PROHIBITS_OPERATION;
            }
          } catch (...) {
            ret->code = COMMAND_FAILED;
          }
          if (!ret->code) {
            if ( !TestCountryCode(c.CC) )
              ret->code = SetReasonUnknowCC(errors, c.CC,
                  GetRegistrarLang(clientID) );
            else if (DBsql.ObjectUpdate(id, regID, c.AuthInfoPw) ) // update OBJECT table
            {

              // begin update
              DBsql.UPDATE("Contact");

              DBsql.SET("Name", c.Name);
              DBsql.SET("Organization", c.Organization);
              // whole adrress must be updated at once
              // it's not allowed to update only part of it
              if (strlen(c.City) || strlen(c.CC)) {
                snum = c.Streets.length();

                for (s = 0; s < 3; s ++) {
                  sprintf(streetStr, "Street%d", s +1);
                  if (s < snum)
                    DBsql.SET(streetStr, c.Streets[s]);
                  else
                    DBsql.SETNULL(streetStr);
                }

                DBsql.SET("City", c.City);
                if (strlen(c.StateOrProvince))
                  DBsql.SET("StateOrProvince", c.StateOrProvince);
                else
                  DBsql.SETNULL("StateOrProvince");
                if (strlen(c.PostalCode))
                  DBsql.SET("PostalCode", c.PostalCode);
                else
                  DBsql.SETNULL("PostalCode");
                DBsql.SET("Country", c.CC);
              }
              DBsql.SET("Telephone", c.Telephone);
              DBsql.SET("Fax", c.Fax);
              DBsql.SET("Email", c.Email);
              DBsql.SET("NotifyEmail", c.NotifyEmail);
              DBsql.SET("VAT", c.VAT);
              DBsql.SET("SSN", c.ident);
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
                DBsql.SET("SSNtype", identtype); // type ssn
              }

              //  Disclose parameters flags translate
              DBsql.SETBOOL("DiscloseName", update_DISCLOSE(c.DiscloseName,
                  c.DiscloseFlag) );
              DBsql.SETBOOL("DiscloseOrganization", update_DISCLOSE(
                  c.DiscloseOrganization, c.DiscloseFlag) );
              DBsql.SETBOOL("DiscloseAddress", update_DISCLOSE(
                  c.DiscloseAddress, c.DiscloseFlag) );
              DBsql.SETBOOL("DiscloseTelephone", update_DISCLOSE(
                  c.DiscloseTelephone, c.DiscloseFlag) );
              DBsql.SETBOOL("DiscloseFax", update_DISCLOSE(c.DiscloseFax,
                  c.DiscloseFlag) );
              DBsql.SETBOOL("DiscloseEmail", update_DISCLOSE(c.DiscloseEmail,
                  c.DiscloseFlag) );
              DBsql.SETBOOL("DiscloseVAT", update_DISCLOSE(c.DiscloseVAT,
                  c.DiscloseFlag) );
              DBsql.SETBOOL("DiscloseIdent", update_DISCLOSE(c.DiscloseIdent,
                  c.DiscloseFlag) );
              DBsql.SETBOOL("DiscloseNotifyEmail", update_DISCLOSE(
                  c.DiscloseNotifyEmail, c.DiscloseFlag) );

              // the end of UPDATE SQL
              DBsql.WHEREID(id);

              // make update and save to history
              if (DBsql.EXEC() )
                if (DBsql.SaveContactHistory(id) )
                  ret->code = COMMAND_OK;

            }
          }
          if (ret->code == COMMAND_OK) // run notifier
          {
            ntf.reset(new EPPNotifier(conf.get<bool>("registry.disable_epp_notifier"),mm , &DBsql, regID , id ));
            ntf->addExtraEmails(oldNotifyEmail);
            ntf->Send(); // send messages with objectID
          }

          DBsql.QuitTransaction(ret->code);
        }

        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
      }

      ret->msg = CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
    }

  // EPP exception
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  if (ret->code == 0)
    ServerInternalError("ContactUpdate");

  return ret._retn();
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
 *              clientID - id of connected client
 *              clTRID - client transaction number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::ContactCreate(
  const char *handle, const ccReg::ContactChange & c,
  ccReg::timestamp_out crDate, CORBA::Long clientID, const char *clTRID,
  const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  std::auto_ptr<EPPNotifier> ntf;
  DB DBsql;
  ccReg::Response_var ret;
  ccReg::Errors_var errors;

  int regID, id;
  int s, snum;
  char streetStr[10];

  // default
  ret = new ccReg::Response;
  errors = new ccReg::Errors;

  ret->code = 0;
  errors->length( 0);

  ParsedAction paction;
  paction.add(1,(const char*)handle);

  crDate = CORBA::string_dup("");

  LOG( NOTICE_LOG, "ContactCreate: clientID -> %d clTRID [%s] handle [%s]", (int ) clientID, clTRID, handle );
  LOG( NOTICE_LOG, "Discloseflag %d: Disclose Name %d Org %d Add %d Tel %d Fax %d Email %d VAT %d Ident %d NotifyEmail %d" , c.DiscloseFlag ,
      c.DiscloseName , c.DiscloseOrganization , c.DiscloseAddress , c.DiscloseTelephone , c.DiscloseFax , c.DiscloseEmail, c.DiscloseVAT, c.DiscloseIdent, c.DiscloseNotifyEmail);

  if ( (regID = GetRegistrarID(clientID) ))

    if (DBsql.OpenDatabase(database) ) {
      if ( (DBsql.BeginAction(clientID, EPP_ContactCreate, clTRID, XML, &paction) )) {
        Register::Contact::Manager::CheckAvailType caType;
        try {
          std::auto_ptr<Register::Contact::Manager> cman(
              Register::Contact::Manager::create(&DBsql,conf.get<bool>("registry.restricted_handles"))
          );
          Register::NameIdPair nameId;
          caType = cman->checkAvail(handle,nameId);
          id = nameId.id;
        }
        catch (...) {
          id = -1;
          caType = Register::Contact::Manager::CA_INVALID_HANDLE;
        }
        if (id<0 || caType == Register::Contact::Manager::CA_INVALID_HANDLE)
          ret->code= SetReasonContactHandle(errors, handle,
              GetRegistrarLang(clientID) );
        else if (caType == Register::Contact::Manager::CA_REGISTRED) {
          LOG( WARNING_LOG, "contact handle [%s] EXIST", handle );
          ret->code= COMMAND_OBJECT_EXIST;
        } else if (caType == Register::Contact::Manager::CA_PROTECTED)
          ret->code= SetReasonProtectedPeriod(errors, handle,
              GetRegistrarLang(clientID) );
        else if (DBsql.BeginTransaction() ) {
          // test  if country code  is valid
          if ( !TestCountryCode(c.CC) )
            ret->code=SetReasonUnknowCC(errors, c.CC,
                GetRegistrarLang(clientID) );
          else {
            // create object generate ROID
            id= DBsql.CreateObject("C", regID, handle, c.AuthInfoPw);
            if (id<=0) {
              if (id == 0) {
                LOG( WARNING_LOG, "contact handle [%s] EXIST", handle );
                ret->code= COMMAND_OBJECT_EXIST;
              } else {
                LOG( WARNING_LOG, "Cannot insert [%s] into object_registry", handle );
                ret->code= COMMAND_FAILED;
              }

            } else {

              DBsql.INSERT("CONTACT");
              DBsql.INTO("id");

              DBsql.INTOVAL("Name", c.Name);
              DBsql.INTOVAL("Organization", c.Organization);

              // insert streets
              snum = c.Streets.length();
              for (s = 0; s < snum; s ++) {
                sprintf(streetStr, "Street%d", s +1);
                DBsql.INTOVAL(streetStr, c.Streets[s]);
              }

              DBsql.INTOVAL("City", c.City);
              DBsql.INTOVAL("StateOrProvince", c.StateOrProvince);
              DBsql.INTOVAL("PostalCode", c.PostalCode);
              DBsql.INTOVAL("Country", c.CC);
              DBsql.INTOVAL("Telephone", c.Telephone);
              DBsql.INTOVAL("Fax", c.Fax);
              DBsql.INTOVAL("Email", c.Email);
              DBsql.INTOVAL("NotifyEmail", c.NotifyEmail);
              DBsql.INTOVAL("VAT", c.VAT);
              DBsql.INTOVAL("SSN", c.ident);
              if (c.identtype != ccReg::EMPTY)
                DBsql.INTO("SSNtype");

              // disclose are write true or false
              DBsql.INTO("DiscloseName");
              DBsql.INTO("DiscloseOrganization");
              DBsql.INTO("DiscloseAddress");
              DBsql.INTO("DiscloseTelephone");
              DBsql.INTO("DiscloseFax");
              DBsql.INTO("DiscloseEmail");
              DBsql.INTO("DiscloseVAT");
              DBsql.INTO("DiscloseIdent");
              DBsql.INTO("DiscloseNotifyEmail");

              DBsql.VALUE(id);

              DBsql.VAL(c.Name);
              DBsql.VAL(c.Organization);
              snum = c.Streets.length();
              for (s = 0; s < snum; s ++) {
                sprintf(streetStr, "Street%d", s +1);
                DBsql.VAL(c.Streets[s]);
              }

              DBsql.VAL(c.City);
              DBsql.VAL(c.StateOrProvince);
              DBsql.VAL(c.PostalCode);
              DBsql.VAL(c.CC);
              DBsql.VAL(c.Telephone);
              DBsql.VAL(c.Fax);
              DBsql.VAL(c.Email);
              DBsql.VAL(c.NotifyEmail);
              DBsql.VAL(c.VAT);
              DBsql.VAL(c.ident);
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
                DBsql.VALUE(identtype);
              }

              // insert DiscloseFlag by a  DefaultPolicy of server
              DBsql.VALUE(setvalue_DISCLOSE(c.DiscloseName, c.DiscloseFlag) );
              DBsql.VALUE(setvalue_DISCLOSE(c.DiscloseOrganization,
                  c.DiscloseFlag) );
              DBsql.VALUE(setvalue_DISCLOSE(c.DiscloseAddress, c.DiscloseFlag) );
              DBsql.VALUE(setvalue_DISCLOSE(c.DiscloseTelephone, c.DiscloseFlag) );
              DBsql.VALUE(setvalue_DISCLOSE(c.DiscloseFax, c.DiscloseFlag) );
              DBsql.VALUE(setvalue_DISCLOSE(c.DiscloseEmail, c.DiscloseFlag) );
              DBsql.VALUE(setvalue_DISCLOSE(c.DiscloseVAT, c.DiscloseFlag) );
              DBsql.VALUE(setvalue_DISCLOSE(c.DiscloseIdent, c.DiscloseFlag) );
              DBsql.VALUE(setvalue_DISCLOSE(c.DiscloseNotifyEmail,
                  c.DiscloseFlag) );

              // if is inserted
              if (DBsql.EXEC() ) {
                // get local timestamp of created  object
                CORBA::string_free(crDate);
                crDate= CORBA::string_dup(DBsql.GetObjectCrDateTime(id) );
                if (DBsql.SaveContactHistory(id) ) // save history
                  if (DBsql.SaveObjectCreate(id) )
                    ret->code = COMMAND_OK; // if saved
              }
            }
          }

          if (ret->code == COMMAND_OK) // run notifier
          {
            ntf.reset(new EPPNotifier(conf.get<bool>("registry.disable_epp_notifier"),mm , &DBsql, regID , id ));
            ntf->Send(); // send message with  objectID
          }

          DBsql.QuitTransaction(ret->code);

        }
        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
      }

      ret->msg = CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );
      DBsql.Disconnect();
    }

  // EPP exception
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  if (ret->code == 0)
    ServerInternalError("ContactCreate");

  return ret._retn();
}

/***********************************************************************
 *
 * FUNCTION:    ObjectTransfer
 *
 * DESCRIPTION: contact transfer from original into new registrar
 *              and saving change into history
 * PARAMETERS:  handle - contact identifier
 *              authInfo - password  authentication
 *              clientID - id of connected client
 *              clTRID - client transaction number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::ObjectTransfer(
  short act, const char*table, const char*fname, const char* name,
  const char* authInfo, CORBA::Long clientID, const char* clTRID,
  const char* XML)
{
  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  DB DBsql;
  std::auto_ptr<EPPNotifier> ntf;
  char pass[PASS_LEN+1];
  int regID, oldregID;
  int type = 0;
  int id = 0;

  ret = new ccReg::Response;
  errors = new ccReg::Errors;

  ret->code=0;
  errors->length(0);

  ParsedAction paction;
  paction.add(1,(const char*)name);

  LOG( NOTICE_LOG , "ObjectContact: act %d  clientID -> %d clTRID [%s] object [%s] authInfo [%s] " , act , (int ) clientID , clTRID , name , authInfo );
  if ( (regID = GetRegistrarID(clientID) ))
    if (DBsql.OpenDatabase(database) ) {

      if ( (DBsql.BeginAction(clientID, act, clTRID, XML, &paction) )) {

        if (DBsql.BeginTransaction()) {

          int zone = 0; // for domain zone check
          switch (act) {
            case EPP_ContactTransfer:
              if ( (id = getIdOfContact(&DBsql, name, conf, true)) < 0)
                ret->code= SetReasonContactHandle(errors, name,
                    GetRegistrarLang(clientID) );
              else if (id == 0)
                ret->code=COMMAND_OBJECT_NOT_EXIST;
              break;

            case EPP_NSsetTransfer:
              if ( (id = getIdOfNSSet(&DBsql, name, conf, true) ) < 0)
                ret->code=SetReasonNSSetHandle(errors, name,
                    GetRegistrarLang(clientID) );
              else if (id == 0)
                ret->code=COMMAND_OBJECT_NOT_EXIST;
              break;

            case EPP_KeySetTransfer:
              if ((id = getIdOfKeySet(&DBsql, name, conf, true)) < 0)
                  ret->code = SetReasonKeySetHandle(
                          errors, name, GetRegistrarLang(clientID));
              else if (id == 0)
                  ret->code = COMMAND_OBJECT_NOT_EXIST;
              break;

            case EPP_DomainTransfer:
              if ( (id = getIdOfDomain(&DBsql, name, conf, true, &zone) ) < 0)
                ret->code=SetReasonDomainFQDN(errors, name, id == -1,
                    GetRegistrarLang(clientID) );
              else if (id == 0)
                ret->code=COMMAND_OBJECT_NOT_EXIST;
              if (DBsql.TestRegistrarZone(regID, zone) == false) {
                 LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
                 ret->code = COMMAND_AUTHENTICATION_ERROR;
               }
               break;
            default:
              ret->code = COMMAND_PARAMETR_ERROR;
          }

          if (!ret->code) {


            // transfer can not be run by existing client
            if (DBsql.TestObjectClientID(id, regID)
                && !DBsql.GetRegistrarSystem(regID) ) {
              LOG( WARNING_LOG, "client can not transfer  object %s" , name
              );
              ret->code = COMMAND_NOT_ELIGIBLE_FOR_TRANSFER;
            }

            try {
              if (!ret->code && testObjectHasState(&DBsql,id,FLAG_serverTransferProhibited))
              {
                LOG( WARNING_LOG, "transfer of object %s is prohibited" , name );
                ret->code = COMMAND_STATUS_PROHIBITS_OPERATION;
              }
            } catch (...) {
              ret->code = COMMAND_FAILED;
            }
            if (!ret->code) {

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
              ret->code = COMMAND_AUTOR_ERROR;
              if (!DBsql.ExecSelect(sql.str().c_str())) {
                LOG( WARNING_LOG , "autorization failed - sql error");
              } else {
                for (unsigned i=0; i < (unsigned)DBsql.GetSelectRows(); i++) {
                  if (!strcmp(DBsql.GetFieldValue(i, 0), (char *)authInfo)) {
                    ret->code = 0;
                    break;
                  }
                }
                DBsql.FreeSelect();
              }
              if (ret->code) {
                LOG( WARNING_LOG , "autorization failed");
              } else {

                //  get ID of old registrant
                oldregID
                    = DBsql.GetNumericFromTable("object", "clid", "id", id);

                // after transfer generete new  authinfo
                random_pass(pass);

                // change registrant
                DBsql.UPDATE("OBJECT");
                DBsql.SSET("TrDate", "now"); // tr timestamp now
                DBsql.SSET("AuthInfoPw", pass);
                DBsql.SET("ClID", regID);
                DBsql.WHEREID(id);

                // IF ok save to history
                if (DBsql.EXEC() ) {
                  switch (act) {
                    case EPP_ContactTransfer:
                      type=1;
                      if (DBsql.SaveContactHistory(id) )
                        ret->code = COMMAND_OK;
                      break;
                    case EPP_NSsetTransfer:
                      type=2;
                      if (DBsql.SaveNSSetHistory(id) )
                        ret->code = COMMAND_OK;
                      break;
                    case EPP_DomainTransfer:
                      type =3;
                      if (DBsql.SaveDomainHistory(id) )
                        ret->code = COMMAND_OK;
                      break;
                    case EPP_KeySetTransfer:
                      type = 4;
                      if (DBsql.SaveKeySetHistory(id))
                          ret->code = COMMAND_OK;
                      break;
                  }

                  if (ret->code == COMMAND_OK) {
                    try {
                      std::auto_ptr<Register::Poll::Manager> pollMan(
                          Register::Poll::Manager::create(&DBsql)
                      );
                      pollMan->createActionMessage(
                          oldregID,
                          type == 1 ? Register::Poll::MT_TRANSFER_CONTACT :
                          type == 2 ? Register::Poll::MT_TRANSFER_NSSET :
                          type == 3 ? Register::Poll::MT_TRANSFER_DOMAIN :
                          Register::Poll::MT_TRANSFER_KEYSET,
                          id
                      );
                    } catch (...) {ret->code = COMMAND_FAILED;}
                  }
                }

              }
            }

            if (ret->code == COMMAND_OK) // run notifier
            {
              ntf.reset(new EPPNotifier(conf.get<bool>("registry.disable_epp_notifier"),mm , &DBsql, regID , id ));
              ntf->Send();
            }

          }
          DBsql.QuitTransaction(ret->code);
        } //test code before begin tr

        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) ) ;
      }

      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
    }

  // EPP exception
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  if (ret->code == 0)
    ServerInternalError("ObjectTransfer");

  return ret._retn();
}

ccReg::Response* ccReg_EPP_i::ContactTransfer(
  const char* handle, const char* authInfo, CORBA::Long clientID,
  const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  return ObjectTransfer( EPP_ContactTransfer , "CONTACT" , "handle" , handle, authInfo, clientID, clTRID , XML);
}

ccReg::Response* ccReg_EPP_i::NSSetTransfer(
  const char* handle, const char* authInfo, CORBA::Long clientID,
  const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  return ObjectTransfer( EPP_NSsetTransfer , "NSSET" , "handle" , handle, authInfo, clientID, clTRID , XML);
}

ccReg::Response* ccReg_EPP_i::DomainTransfer(
  const char* fqdn, const char* authInfo, CORBA::Long clientID,
  const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  return ObjectTransfer( EPP_DomainTransfer , "DOMAIN" , "fqdn" , fqdn, authInfo, clientID, clTRID , XML);
}

ccReg::Response *
ccReg_EPP_i::KeySetTransfer(
        const char *handle,
        const char *authInfo,
        CORBA::Long clientID,
        const char *clTRID,
        const char *XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

    return ObjectTransfer(EPP_KeySetTransfer, "KEYSET", "handle", handle,
            authInfo, clientID, clTRID, XML);
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
 *              clientID - id of connected client
 *              clTRID - client transaction number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::NSSetInfo(
  const char* handle, ccReg::NSSet_out n, CORBA::Long clientID,
  const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  LOG(
      NOTICE_LOG,
      "NSSetInfo: clientID -> %d clTRID [%s] handle [%s] ",
      (int) clientID, clTRID, handle
  );
  ParsedAction paction;
  paction.add(1,(const char*)handle);
  // start EPP action - this will handle all init stuff
  EPPAction a(this, clientID, EPP_NSsetInfo, clTRID, XML, &paction);
  // initialize managers for nsset manipulation
  std::auto_ptr<Register::Zone::Manager>
      zman(Register::Zone::Manager::create(a.getDB()) );
  std::auto_ptr<Register::NSSet::Manager>
      nman(Register::NSSet::Manager::create(a.getDB(), zman.get(),
          conf.get<bool>("registry.restricted_handles") ) );
  // first check handle for proper format
  if (!nman->checkHandleFormat(handle))
    // failure in handle check, throw exception
    a.failed(SetReasonNSSetHandle(a.getErrors(), handle, a.getLang()));
  // now load nsset by handle
  std::auto_ptr<Register::NSSet::List> nlist(nman->createList());
  nlist->setHandleFilter(handle);
  try {nlist->reload();}
  catch (...) {a.failedInternal("Cannot load nsset");}
  if (nlist->getCount() != 1)
    // failer because non existance, throw exception
    a.failed(COMMAND_OBJECT_NOT_EXIST);
  // start filling output nsset structure
  Register::NSSet::NSSet *nss = nlist->getNSSet(0);
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
    Register::TID stateId = nss->getStatusByIdx(i)->getStatusId();
    const Register::StatusDesc* sd = regMan->getStatusDesc(stateId);
    if (!sd || !sd->getExternal())
      continue;
    n->stat.length(n->stat.length()+1);
    n->stat[n->stat.length()-1].value = CORBA::string_dup(sd->getName().c_str() );
    n->stat[n->stat.length()-1].text = CORBA::string_dup(sd->getDesc(
        a.getLang() == LANG_CS ? "CS" : "EN"
    ).c_str());
  }
  if (!n->stat.length()) {
    const Register::StatusDesc* sd = regMan->getStatusDesc(0);
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
    const Register::NSSet::Host *h = nss->getHostByIdx(i);
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
 *              clientID - id of connected client
 *              clTRID - client transaction number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::NSSetDelete(
  const char* handle, CORBA::Long clientID, const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  DB DBsql;
  std::auto_ptr<EPPNotifier> ntf;
  int regID, id;

  ret = new ccReg::Response;
  errors = new ccReg::Errors;

  ret->code=0;
  errors->length(0);

  ParsedAction paction;
  paction.add(1,(const char*)handle);

  LOG( NOTICE_LOG , "NSSetDelete: clientID -> %d clTRID [%s] handle [%s] " , (int ) clientID , clTRID , handle );

  if ( (regID = GetRegistrarID(clientID) ))

    if (DBsql.OpenDatabase(database) ) {

      if ( (DBsql.BeginAction(clientID, EPP_NSsetDelete, clTRID, XML, &paction) )) {

        if (DBsql.BeginTransaction() ) {
          // lock row
          id = getIdOfNSSet(&DBsql, handle, conf, true);
          if (id < 0)
            ret->code = SetReasonNSSetHandle(errors, handle,
              GetRegistrarLang(clientID) );
          else if (id == 0) {
            LOG( WARNING_LOG, "nsset handle [%s] NOT_EXIST", handle );
            ret->code = COMMAND_OBJECT_NOT_EXIST;
          }
          if (!ret->code &&  !DBsql.TestObjectClientID(id, regID) ) // if not client od the object
          {
            LOG( WARNING_LOG, "bad autorization not client of nsset [%s]", handle );
            ret->code = SetReasonWrongRegistrar(errors, regID, GetRegistrarLang(clientID));
          }
          try {
            if (!ret->code && (
                  testObjectHasState(&DBsql,id,FLAG_serverDeleteProhibited) ||
                  testObjectHasState(&DBsql,id,FLAG_serverUpdateProhibited)
                ))
            {
              LOG( WARNING_LOG, "delete of object %s is prohibited" , handle );
              ret->code = COMMAND_STATUS_PROHIBITS_OPERATION;
            }
          } catch (...) {
            ret->code = COMMAND_FAILED;
          }
          if (!ret->code) {
            // create notifier
            ntf.reset(new EPPNotifier(conf.get<bool>("registry.disable_epp_notifier"),mm , &DBsql, regID , id ));

            // test to  table domain if relations to nsset
            if (DBsql.TestNSSetRelations(id) ) //  can not be delete
            {
              LOG( WARNING_LOG, "database relations" );
              ret->code = COMMAND_PROHIBITS_OPERATION;
            } else {
              if (DBsql.SaveObjectDelete(id) ) // save to delete object
              {
                if (DBsql.DeleteNSSetObject(id) )
                  ret->code = COMMAND_OK; // if is OK
              }
            }

            if (ret->code == COMMAND_OK)
              ntf->Send(); // send messages by notifier
          }

          DBsql.QuitTransaction(ret->code);
        }

        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
      }

      ret->msg = CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
    }

  // EPP exception
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  if (ret->code == 0)
    ServerInternalError("NSSetDelete");

  return ret._retn();
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
 *              clientID - id of connected client
 *              clTRID - transaction client number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::NSSetCreate(
  const char *handle, const char *authInfoPw, const ccReg::TechContact & tech,
  const ccReg::DNSHost & dns, CORBA::Short level, ccReg::timestamp_out crDate,
  CORBA::Long clientID, const char *clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  DB DBsql;
  std::auto_ptr<EPPNotifier> ntf;
  char NAME[256]; // to upper case of name of DNS hosts
  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  int regID, id, techid, hostID;
  unsigned int i, j, l;
  short inetNum;
  int *tch= NULL;

  LOG( NOTICE_LOG, "NSSetCreate: clientID -> %d clTRID [%s] handle [%s]  authInfoPw [%s]", (int ) clientID, clTRID, handle , authInfoPw );
  LOG( NOTICE_LOG, "NSSetCreate: tech check level %d tech num %d" , (int) level , (int) tech.length() );

  ret = new ccReg::Response;
  errors = new ccReg::Errors;

  // defaults
  ret->code = 0;
  errors->length( 0);

  ParsedAction paction;
  paction.add(1,(const char*)handle);

  crDate = CORBA::string_dup("");

  if ((regID = GetRegistrarID(clientID))) {

    if (DBsql.OpenDatabase(database) ) {
      std::auto_ptr<Register::Zone::Manager> zman(
        Register::Zone::Manager::create(&DBsql));
      std::auto_ptr<Register::NSSet::Manager> nman(
        Register::NSSet::Manager::create(&DBsql,zman.get(),conf.get<bool>("registry.restricted_handles")));

      if ( (DBsql.BeginAction(clientID, EPP_NSsetCreate, clTRID, XML, &paction) )) {
        if (DBsql.BeginTransaction() ) {
          if (tech.length() < 1) {
            LOG( WARNING_LOG, "NSSetCreate: not any tech Contact " );
            ret->code = SetErrorReason(errors, COMMAND_PARAMETR_MISSING,
                ccReg::nsset_tech, 0, REASON_MSG_TECH_NOTEXIST,
                GetRegistrarLang(clientID) );
          } else if (tech.length() > 9) {
              LOG(WARNING_LOG, "NSSetCreate: too many tech contacts (max is 9)");
              ret->code = SetErrorReason(errors,
                      COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                      ccReg::nsset_tech, 0,
                      REASON_MSG_TECHADMIN_LIMIT,
                      GetRegistrarLang(clientID));
          } else if (dns.length() < 2) {
            //  minimal two dns hosts
              if (dns.length() == 1) {
              LOG( WARNING_LOG, "NSSetCreate: minimal two dns host create one %s" , (const char *) dns[0].fqdn );
              ret->code = SetErrorReason(errors,
                      COMMAND_PARAMETR_VALUE_POLICY_ERROR, ccReg::nsset_dns_name, 1,
                      REASON_MSG_MIN_TWO_DNS_SERVER, GetRegistrarLang(clientID) );
            } else {
              LOG( WARNING_LOG, "NSSetCreate: minimal two dns DNS hosts" );
              ret->code = SetErrorReason(errors, COMMAND_PARAMETR_MISSING,
                  ccReg::nsset_dns_name, 0, REASON_MSG_MIN_TWO_DNS_SERVER,
                  GetRegistrarLang(clientID) );
            }

          } else if (dns.length() > 9) {
              LOG(WARNING_LOG, "NSSetCreate: too many dns hosts (maximum is 9)");
              ret->code = SetErrorReason(errors,
                      COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                      ccReg::nsset_dns_name, 0,
                      REASON_MSG_NSSET_LIMIT,
                      GetRegistrarLang(clientID));
          }
          if (ret->code == 0) {
            Register::NSSet::Manager::CheckAvailType caType;

            tch = new int[tech.length()];

            try {
              Register::NameIdPair nameId;
              caType = nman->checkAvail(handle,nameId);
              id = nameId.id;
            }
            catch (...) {
              caType = Register::NSSet::Manager::CA_INVALID_HANDLE;
              id = -1;
            }

            if (id<0 || caType == Register::NSSet::Manager::CA_INVALID_HANDLE)
              ret->code = SetReasonNSSetHandle(errors, handle,
                  GetRegistrarLang(clientID) );
            else if (caType == Register::NSSet::Manager::CA_REGISTRED) {
              LOG( WARNING_LOG, "nsset handle [%s] EXIST", handle );
              ret->code = COMMAND_OBJECT_EXIST;
            } else if (caType == Register::NSSet::Manager::CA_PROTECTED)
              ret->code = SetReasonProtectedPeriod(errors, handle,
                  GetRegistrarLang(clientID), ccReg::nsset_handle);
          }
          if (ret->code == 0) {
            // test tech-c
            std::auto_ptr<Register::Contact::Manager>
                cman(Register::Contact::Manager::create(&DBsql,
                    conf.get<bool>("registry.restricted_handles")) );
            for (i = 0; i < tech.length() ; i++) {
              Register::Contact::Manager::CheckAvailType caType;
              try {
                Register::NameIdPair nameId;
                caType = cman->checkAvail((const char *)tech[i],nameId);
                techid = nameId.id;
              } catch (...) {
                caType = Register::Contact::Manager::CA_INVALID_HANDLE;
                techid = 0;
              }

              if (caType != Register::Contact::Manager::CA_REGISTRED)
                ret->code = SetReasonNSSetTech(errors, tech[i], techid,
                    GetRegistrarLang(clientID) , i);
              else {
                tch[i] = techid;
                for (j = 0; j < i; j ++) // test duplicity
                {
                  LOG( DEBUG_LOG , "tech compare j %d techid %d ad %d" , j , techid , tch[j] );
                  if (tch[j] == techid && tch[j] > 0) {
                    tch[j]= 0;
                    ret->code = SetReasonContactDuplicity(errors, tech[i],
                        GetRegistrarLang(clientID) , i, ccReg::nsset_tech);
                  }
                }

              }
            }
          }

          if (ret->code == 0) {

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
                      ret->code =SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                          ccReg::nsset_dns_addr, inetNum+j+1,
                          REASON_MSG_DUPLICITY_DNS_ADDRESS,
                          GetRegistrarLang(clientID) );
                    }
                  }

                } else {
                  LOG( WARNING_LOG, "NSSetCreate: bad host address %s " , (const char *) dns[i].inet[j] );
                  ret->code =SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                      ccReg::nsset_dns_addr, inetNum+j+1,
                      REASON_MSG_BAD_IP_ADDRESS, GetRegistrarLang(clientID) );
                }

              }

              // test DNS hosts
              unsigned hostnameTest = nman->checkHostname(
                  (const char *)dns[i].fqdn, dns[i].inet.length() > 0);
              if (hostnameTest == 1) {

                LOG( WARNING_LOG, "NSSetCreate: bad host name %s " , (const char *) dns[i].fqdn );
                ret->code = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                    ccReg::nsset_dns_name, i+1, REASON_MSG_BAD_DNS_NAME,
                    GetRegistrarLang(clientID) );
              } else {
                LOG( NOTICE_LOG , "NSSetCreate: test DNS Host %s", (const char *) dns[i].fqdn );
                convert_hostname(NAME, dns[i].fqdn);

                // not in defined zones and exist record of ip address
                if (hostnameTest == 2) {

                  for (j = 0; j < dns[i].inet.length() ; j ++) {

                    LOG( WARNING_LOG, "NSSetCreate:  ipaddr  glue not allowed %s " , (const char *) dns[i].inet[j] );
                    ret->code = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                        ccReg::nsset_dns_addr, inetNum+j+1,
                        REASON_MSG_IP_GLUE_NOT_ALLOWED,
                        GetRegistrarLang(clientID) );
                  }

                }

                // test to duplicity of nameservers
                for (l = 0; l < i; l ++) {
                  char PREV_NAME[256]; // to upper case of name of DNS hosts
                  convert_hostname(PREV_NAME, dns[l].fqdn);
                  if (strcmp(NAME, PREV_NAME) == 0) {
                    LOG( WARNING_LOG, "NSSetCreate: duplicity host name %s " , (const char *) NAME );
                    ret->code = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                        ccReg::nsset_dns_name, i+1, REASON_MSG_DNS_NAME_EXIST,
                        GetRegistrarLang(clientID) );
                  }
                }

              }

              inetNum+= dns[i].inet.length(); //  InetNum counter  for return errors
            } // end of cycle

            LOG( DEBUG_LOG , "NSSetCreate: ret->code %d" , ret->code );

          }

          if (ret->code == 0) {

            id = DBsql.CreateObject("N", regID, handle, authInfoPw);
            if (id<=0) {
              if (id==0) {
                LOG( WARNING_LOG, "nsset handle [%s] EXIST", handle );
                ret->code= COMMAND_OBJECT_EXIST;
              } else {
                LOG( WARNING_LOG, "Cannot insert [%s] into object_registry", handle );
                ret->code= COMMAND_FAILED;
              }
            } else {

              if (level<0)
                level = conf.get<unsigned>("registry.nsset_level");
              // write to nsset table
              DBsql.INSERT("NSSET");
              DBsql.INTO("id");
              if (level >= 0)
                DBsql.INTO("checklevel");
              DBsql.VALUE(id);
              if (level >= 0)
                DBsql.VALUE(level);

              // nsset first
              if ( !DBsql.EXEC() )
                ret->code = COMMAND_FAILED;
              else {

                // get local timestamp with timezone of created object
                CORBA::string_free(crDate);
                crDate= CORBA::string_dup(DBsql.GetObjectCrDateTime(id) );

                // insert all tech-c
                for (i = 0; i < tech.length() ; i++) {
                  LOG( DEBUG_LOG, "NSSetCreate: add tech Contact %s id %d " , (const char *) tech[i] , tch[i]);
                  if ( !DBsql.AddContactMap("nsset", id, tch[i]) ) {
                    ret->code = COMMAND_FAILED;
                    break;
                  }
                }

                // insert all DNS hosts


                for (i = 0; i < dns.length() ; i++) {

                  // convert host name to lower case
                  LOG( NOTICE_LOG , "NSSetCreate: DNS Host %s ", (const char *) dns[i].fqdn );
                  convert_hostname(NAME, dns[i].fqdn);

                  // ID  sequence
                  hostID = DBsql.GetSequenceID("host");

                  // HOST  informations
                  DBsql.INSERT("HOST");
                  DBsql.INTO("ID");
                  DBsql.INTO("NSSETID");
                  DBsql.INTO("fqdn");
                  DBsql.VALUE(hostID);
                  DBsql.VALUE(id);
                  DBsql.VVALUE(NAME);
                  if (DBsql.EXEC() ) {

                    // save ip address of host
                    for (j = 0; j < dns[i].inet.length(); j++) {
                      LOG( NOTICE_LOG , "NSSetCreate: IP address hostID  %d [%s] ", hostID , (const char *) dns[i].inet[j] );

                      // HOST_IPADDR insert IP address of DNS host
                      DBsql.INSERT("HOST_IPADDR_map");
                      DBsql.INTO("HOSTID");
                      DBsql.INTO("NSSETID");
                      DBsql.INTO("ipaddr");
                      DBsql.VALUE(hostID);
                      DBsql.VALUE(id); // write nssetID
                      DBsql.VVALUE(dns[i].inet[j]);

                      if (DBsql.EXEC() == false) {
                        ret->code = COMMAND_FAILED;
                        break;
                      }

                    }

                  } else {
                    ret->code = COMMAND_FAILED;
                    break;
                  }

                } // end of host cycle


                //  save to  historie if is OK
                if (ret->code != COMMAND_FAILED)
                  if (DBsql.SaveNSSetHistory(id) )
                    if (DBsql.SaveObjectCreate(id) )
                      ret->code = COMMAND_OK;
              } //


              if (ret->code == COMMAND_OK) // run notifier
              {
                ntf.reset(new EPPNotifier(conf.get<bool>("registry.disable_epp_notifier"),mm , &DBsql, regID , id ));
                ntf->Send(); //send messages
              }

            }
          }

          delete[] tch;

          DBsql.QuitTransaction(ret->code);
        }

        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
      }

      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
    }

  }

  // EPP exception
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  if (ret->code == 0)
    ServerInternalError("NSSetCreate");

  return ret._retn();
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
 *              clientID - client connected id
 *              clTRID - client transaction number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::NSSetUpdate(
  const char* handle, const char* authInfo_chg, const ccReg::DNSHost& dns_add,
  const ccReg::DNSHost& dns_rem, const ccReg::TechContact& tech_add,
  const ccReg::TechContact& tech_rem, CORBA::Short level, CORBA::Long clientID,
  const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  std::auto_ptr<EPPNotifier> ntf;
  DB DBsql;
  char NAME[256], REM_NAME[256];
  int regID, nssetID, techid, hostID;
  unsigned int i, j, k, l;
  short inetNum;
  int *tch_add, *tch_rem;
  int hostNum, techNum;
  bool findRem; // test for change DNS hosts
  ret = new ccReg::Response;
  errors = new ccReg::Errors;

  tch_add = new int[ tech_add.length() ];
  tch_rem = new int[ tech_add.length() ];

  ret->code=0;
  errors->length(0);

  ParsedAction paction;
  paction.add(1,(const char*)handle);

  LOG( NOTICE_LOG , "NSSetUpdate: clientID -> %d clTRID [%s] handle [%s] authInfo_chg  [%s] " , (int ) clientID , clTRID , handle , authInfo_chg);
  LOG( NOTICE_LOG, "NSSetUpdate: tech check level %d" , (int) level );

  if ( (regID = GetRegistrarID(clientID) ))

    if (DBsql.OpenDatabase(database) ) {
      std::auto_ptr<Register::Zone::Manager> zman(
        Register::Zone::Manager::create(&DBsql));
      std::auto_ptr<Register::NSSet::Manager> nman(
        Register::NSSet::Manager::create(&DBsql,zman.get(),conf.get<bool>("registry.restricted_handles")));

      if ( (DBsql.BeginAction(clientID, EPP_NSsetUpdate, clTRID, XML, &paction) )) {

        if (DBsql.BeginTransaction() ) {
          if ( (nssetID = getIdOfNSSet(&DBsql, handle, conf, true) ) < 0)
            ret->code = SetReasonNSSetHandle(errors, handle,
                GetRegistrarLang(clientID) );
          else if (nssetID == 0) {
            LOG( WARNING_LOG, "nsset handle [%s] NOT_EXIST", handle );
            ret->code = COMMAND_OBJECT_NOT_EXIST;
          }
          // registrar of the object
          if (!ret->code && !DBsql.TestObjectClientID(nssetID, regID) ) {
            LOG( WARNING_LOG, "bad autorization not  client of nsset [%s]", handle );
            ret->code = SetReasonWrongRegistrar(errors, regID, GetRegistrarLang(clientID));
          }
          try {
            if (!ret->code && testObjectHasState(&DBsql,nssetID,FLAG_serverUpdateProhibited))
            {
              LOG( WARNING_LOG, "update of object %s is prohibited" , handle );
              ret->code = COMMAND_STATUS_PROHIBITS_OPERATION;
            }
          } catch (...) {
            ret->code = COMMAND_FAILED;
          }

          if (!ret->code) {

            // test  ADD tech-c
            for (i = 0; i < tech_add.length(); i++) {
              if ( (techid = getIdOfContact(&DBsql, tech_add[i], conf) ) <= 0)
                ret->code = SetReasonNSSetTechADD(errors, tech_add[i], techid,
                    GetRegistrarLang(clientID) , i);
              else if (DBsql.CheckContactMap("nsset", nssetID, techid, 0) )
                ret->code = SetReasonNSSetTechExistMap(errors, tech_add[i],
                    GetRegistrarLang(clientID) , i);
              else {
                tch_add[i] = techid;
                for (j = 0; j < i; j ++)
                  // duplicity test
                  if (tch_add[j] == techid && tch_add[j] > 0) {
                    tch_add[j] = 0;
                    ret->code = SetReasonContactDuplicity(errors, tech_add[i],
                        GetRegistrarLang(clientID) , i, ccReg::nsset_tech_add);
                  }
              }

              LOG( NOTICE_LOG , "ADD  tech  techid ->%d [%s]" , techid , (const char *) tech_add[i] );
            }

            // test REM tech-c
            for (i = 0; i < tech_rem.length(); i++) {

              if ( (techid = getIdOfContact(&DBsql, tech_rem[i], conf) ) <= 0)
                ret->code = SetReasonNSSetTechREM(errors, tech_rem[i], techid,
                    GetRegistrarLang(clientID) , i);
              else if ( !DBsql.CheckContactMap("nsset", nssetID, techid, 0) )
                ret->code = SetReasonNSSetTechNotExistMap(errors, tech_rem[i],
                    GetRegistrarLang(clientID) , i);
              else {
                tch_rem[i] = techid;
                for (j = 0; j < i; j ++)
                  // test  duplicity
                  if (tch_rem[j] == techid && tch_rem[j] > 0) {
                    tch_rem[j] = 0;
                    ret->code = SetReasonContactDuplicity(errors, tech_rem[i],
                        GetRegistrarLang(clientID) , i, ccReg::nsset_tech_rem);
                  }
              }
              LOG( NOTICE_LOG , "REM  tech  techid ->%d [%s]" , techid , (const char *) tech_rem[i] );

            }

            // ADD DNS HOSTS  and TEST IP address and name of the  DNS HOST
            for (i = 0, inetNum =0; i < dns_add.length(); i++) {

              /// test DNS host
              if (nman->checkHostname((const char *)dns_add[i].fqdn, false)) {
                LOG( WARNING_LOG, "NSSetUpdate: bad add host name %s " , (const char *) dns_add[i].fqdn );
                ret->code = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                    ccReg::nsset_dns_name_add, i+1, REASON_MSG_BAD_DNS_NAME,
                    GetRegistrarLang(clientID) );
              } else {
                LOG( NOTICE_LOG , "NSSetUpdate: add dns [%s]" , (const char * ) dns_add[i].fqdn );

                convert_hostname(NAME, dns_add[i].fqdn); // convert to lower case
                // HOST is not in defined zone and contain ip address
                if (getZone(dns_add[i].fqdn) == 0
                    && (int ) dns_add[i].inet.length() > 0) {
                  for (j = 0; j < dns_add[i].inet.length() ; j ++) {
                    LOG( WARNING_LOG, "NSSetUpdate:  ipaddr  glue not allowed %s " , (const char *) dns_add[i].inet[j] );
                    ret->code = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                        ccReg::nsset_dns_addr, inetNum+j+1,
                        REASON_MSG_IP_GLUE_NOT_ALLOWED,
                        GetRegistrarLang(clientID) );
                  }
                } else {

                  if (DBsql.GetHostID(NAME, nssetID) ) // already exist can not add
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
                      ret->code = SetErrorReason(errors,
                      COMMAND_PARAMETR_ERROR, ccReg::nsset_dns_name_add, i+1,
                      REASON_MSG_DNS_NAME_EXIST, GetRegistrarLang(clientID) );
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
                      ret->code = SetErrorReason(errors,
                      COMMAND_PARAMETR_ERROR, ccReg::nsset_dns_addr, inetNum +j
                          +1, REASON_MSG_DUPLICITY_DNS_ADDRESS,
                          GetRegistrarLang(clientID) );
                    }
                  }

                } else // not valid IP address
                {
                  LOG( WARNING_LOG, "NSSetUpdate: bad add host address %s " , (const char *) dns_add[i].inet[j] );
                  ret->code = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                      ccReg::nsset_dns_addr, inetNum+j+1,
                      REASON_MSG_BAD_IP_ADDRESS, GetRegistrarLang(clientID) );
                }
              }

              inetNum+= dns_add[i].inet.length(); //  count  InetNum for errors

              // test to duplicity of added nameservers
              for (l = 0; l < i; l ++) {
                char PREV_NAME[256]; // to upper case of name of DNS hosts
                convert_hostname(PREV_NAME, dns_add[l].fqdn);
                if (strcmp(NAME, PREV_NAME) == 0) {
                  LOG( WARNING_LOG, "NSSetUpdate:  host name %s duplicate" , (const char *) dns_add[i].fqdn );
                  ret->code = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                      ccReg::nsset_dns_name_add, i+1,
                      REASON_MSG_DNS_NAME_EXIST, GetRegistrarLang(clientID) );
                }
              }

            } // end of cycle


            // test for DNS HOSTS to REMOVE if is valid format and if is exist in the table
            for (i = 0; i < dns_rem.length(); i++) {
              LOG( NOTICE_LOG , "NSSetUpdate:  delete  host  [%s] " , (const char *) dns_rem[i].fqdn );

              if (nman->checkHostname((const char *)dns_rem[i].fqdn, false)) {
                LOG( WARNING_LOG, "NSSetUpdate: bad rem host name %s " , (const char *) dns_rem[i].fqdn );
                ret->code = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                    ccReg::nsset_dns_name_rem, i+1, REASON_MSG_BAD_DNS_NAME,
                    GetRegistrarLang(clientID) );
              } else {
                convert_hostname(NAME, dns_rem[i].fqdn);
                if ( (hostID = DBsql.GetHostID(NAME, nssetID) ) == 0) {
                  LOG( WARNING_LOG, "NSSetUpdate:  host  [%s] not in table" , (const char *) dns_rem[i].fqdn );
                  ret->code
                      = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                          ccReg::nsset_dns_name_rem, i+1,
                          REASON_MSG_DNS_NAME_NOTEXIST,
                          GetRegistrarLang(clientID) );
                }
              }

              // test to duplicity of removing nameservers
              for (l = 0; l < i; l ++) {
                char PREV_NAME[256]; // to upper case of name of DNS hosts
                convert_hostname(PREV_NAME, dns_rem[l].fqdn);
                if (strcmp(NAME, PREV_NAME) == 0) {
                  LOG( WARNING_LOG, "NSSetUpdate:  host name %s duplicate" , (const char *) dns_rem[i].fqdn );
                  ret->code
                      = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                          ccReg::nsset_dns_name_rem, i+1,
                          REASON_MSG_DNS_NAME_NOTEXIST,
                          GetRegistrarLang(clientID) );
                }
              }

            }

            // if not any errors in the parametrs run update
            if (ret->code == 0)
              if (DBsql.ObjectUpdate(nssetID, regID, authInfo_chg) ) {

                // notifier
                ntf.reset(new EPPNotifier(conf.get<bool>("registry.disable_epp_notifier"),mm , &DBsql, regID , nssetID ));
                //  add to current tech-c added tech-c
                for (i = 0; i < tech_add.length(); i++)
                  ntf->AddTechNew(tch_add[i]);

                ntf->Send(); // send messages to all


                // update tech level
                if (level >= 0) {
                  LOG( NOTICE_LOG, "update nsset check level %d ", (int ) level );
                  DBsql.UPDATE("nsset");
                  DBsql.SET("checklevel", level);
                  DBsql.WHERE("id", nssetID);
                  if (DBsql.EXEC() == false)
                    ret->code = COMMAND_FAILED;
                }

                //-------- TECH contacts

                // add tech contacts
                for (i = 0; i < tech_add.length(); i++) {

                  LOG( NOTICE_LOG , "INSERT add techid ->%d [%s]" , tch_add[i] , (const char *) tech_add[i] );
                  if ( !DBsql.AddContactMap("nsset", nssetID, tch_add[i]) ) {
                    ret->code = COMMAND_FAILED;
                    break;
                  }

                }

                // delete  tech contacts
                for (i = 0; i < tech_rem.length(); i++) {

                  LOG( NOTICE_LOG , "DELETE rem techid ->%d [%s]" , tch_rem[i] , (const char *) tech_rem[i] );
                  if ( !DBsql.DeleteFromTableMap("nsset", nssetID, tch_rem[i]) ) {
                    ret->code = COMMAND_FAILED;
                    break;
                  }

                }

                //--------- TEST for numer of tech-c after  ADD & REM
                // only if the tech-c remove
                if (tech_rem.length() > 0) {
                  techNum = DBsql.GetNSSetContacts(nssetID);
                  LOG(NOTICE_LOG, "NSSetUpdate: tech Contact  %d" , techNum );

                  if (techNum == 0) // can not be nsset without tech-c
                  {

                    // marked all  REM tech-c as param  errors
                    for (i = 0; i < tech_rem.length(); i++) {
                      ret->code = SetErrorReason(errors,
                      COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                          ccReg::nsset_tech_rem, i+1,
                          REASON_MSG_CAN_NOT_REMOVE_TECH,
                          GetRegistrarLang(clientID) );
                    }
                  }
                }

                // delete DNS HOSTY  first step
                for (i = 0; i < dns_rem.length(); i++) {
                  LOG( NOTICE_LOG , "NSSetUpdate:  delete  host  [%s] " , (const char *) dns_rem[i].fqdn );

                  convert_hostname(NAME, dns_rem[i].fqdn);
                  hostID = DBsql.GetHostID(NAME, nssetID);
                  LOG( NOTICE_LOG , "DELETE  hostID %d" , hostID );
                  if ( !DBsql.DeleteFromTable("HOST", "id", hostID) )
                    ret->code = COMMAND_FAILED;
                  else if ( !DBsql.DeleteFromTable("HOST_IPADDR_map", "hostID",
                      hostID) )
                    ret->code = COMMAND_FAILED;
                }

                //-------- add  DNS HOSTs second step

                for (i = 0; i < dns_add.length(); i++) {
                  // to lowe case
                  convert_hostname(NAME, dns_add[i].fqdn);

                  // hostID from sequence
                  hostID = DBsql.GetSequenceID("host");

                  // HOST information
                  DBsql.INSERT("HOST");
                  DBsql.INTO("ID");
                  DBsql.INTO("nssetid");
                  DBsql.INTO("fqdn");
                  DBsql.VALUE(hostID);
                  DBsql.VALUE(nssetID); // add nssetID
                  DBsql.VALUE(NAME);
                  if (DBsql.EXEC()) // add all IP address
                  {

                    for (j = 0; j < dns_add[i].inet.length(); j++) {
                      LOG( NOTICE_LOG , "insert  IP address hostID  %d [%s] ", hostID , (const char *) dns_add[i].inet[j] );

                      // insert ipaddr with hostID and nssetID
                      DBsql.INSERT("HOST_IPADDR_map");
                      DBsql.INTO("HOSTID");
                      DBsql.INTO("NSSETID");
                      DBsql.INTO("ipaddr");
                      DBsql.VALUE(hostID);
                      DBsql.VALUE(nssetID);
                      DBsql.VVALUE(dns_add[i].inet[j]);

                      // if failed
                      if (DBsql.EXEC() == false) {
                        ret->code = COMMAND_FAILED;
                        break;
                      }

                    }

                  } else {
                    ret->code = COMMAND_FAILED;
                    break;
                  } // if add host failed


                }

                //------- TEST number DNS host after REM & ADD
                //  only if the add or rem
                if (dns_rem.length() > 0 || dns_add.length() > 0) {
                  hostNum = DBsql.GetNSSetHosts(nssetID);
                  LOG(NOTICE_LOG, "NSSetUpdate:  hostNum %d" , hostNum );

                  if (hostNum < 2) //  minimal two DNS
                  {
                    for (i = 0; i < dns_rem.length(); i++) {
                      // marked all  REM DNS hots as param error
                      ret->code = SetErrorReason(errors,
                      COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                          ccReg::nsset_dns_name_rem, i+1,
                          REASON_MSG_CAN_NOT_REM_DNS,
                          GetRegistrarLang(clientID) );
                    }
                  }

                  if (hostNum > 9) // maximal number
                  {
                    for (i = 0; i < dns_add.length(); i++) {
                      // marked all ADD dns host  as param error
                      ret->code = SetErrorReason(errors,
                      COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                          ccReg::nsset_dns_name_add, i +1,
                          REASON_MSG_CAN_NOT_ADD_DNS,
                          GetRegistrarLang(clientID) );
                    }
                  }

                }

                // save to history if not errors
                if (ret->code == 0)
                  if (DBsql.SaveNSSetHistory(nssetID) )
                    ret->code = COMMAND_OK; // set up successfully as default


                if (ret->code == COMMAND_OK)
                  ntf->Send(); // send messages if is OK


              }

          }
          DBsql.QuitTransaction(ret->code);
        }
        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
      }

      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
    }

  // free mem
  delete[] tch_add;
  delete[] tch_rem;

  // EPP exception
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  if (ret->code == 0)
    ServerInternalError("NSSetUpdate");

  return ret._retn();
}

/***********************************************************************
 *
 * FUNCTION:    DomainInfo
 *
 * DESCRIPTION: return detailed information about domain
 *              empty value if domain doesn't exists
 * PARAMETERS:  fqdn - domain identifier its name
 *        OUT:  d - domain structure detailed description
 *              clientID - client id
 *              clTRID - transaction client number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::DomainInfo(
  const char* fqdn, ccReg::Domain_out d, CORBA::Long clientID,
  const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  LOG(
      NOTICE_LOG, "DomainInfo: clientID -> %d clTRID [%s] fqdn  [%s] ",
      (int) clientID, clTRID, fqdn
  );
  ParsedAction paction;
  paction.add(1,(const char*)fqdn);
  // start EPP action - this will handle all init stuff
  EPPAction a(this, clientID, EPP_DomainInfo, clTRID, XML, &paction);
  // initialize managers for domain manipulation
  std::auto_ptr<Register::Zone::Manager>
      zman(Register::Zone::Manager::create(a.getDB()) );
  std::auto_ptr<Register::Domain::Manager>
      dman(Register::Domain::Manager::create(a.getDB(), zman.get()) );
  // first check handle for proper format
  Register::Domain::CheckAvailType caType = dman->checkHandle(fqdn);
  if (caType != Register::Domain::CA_AVAILABLE) {
    // failure in FQDN check, throw exception
    a.failed(SetReasonDomainFQDN(a.getErrors(), fqdn, caType
        != Register::Domain::CA_BAD_ZONE ? -1 : 0, a.getLang() ));
  }
  // now load domain by fqdn
  std::auto_ptr<Register::Domain::List> dlist(dman->createList());
  dlist->setFQDNFilter(fqdn);
  try {dlist->reload();}
  catch (...) {a.failedInternal("Cannot load domains");}
  if (dlist->getCount() != 1)
    // failer because non existance, throw exception
    a.failed(COMMAND_OBJECT_NOT_EXIST);
  // start filling output domain structure
  Register::Domain::Domain *dom = dlist->getDomain(0);
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
    Register::TID stateId = dom->getStatusByIdx(i)->getStatusId();
    const Register::StatusDesc* sd = regMan->getStatusDesc(stateId);
    if (!sd || !sd->getExternal())
      continue;
    d->stat.length(d->stat.length()+1);
    d->stat[d->stat.length()-1].value = CORBA::string_dup(sd->getName().c_str() );
    d->stat[d->stat.length()-1].text = CORBA::string_dup(sd->getDesc(
        a.getLang() == LANG_CS ? "CS" : "EN"
    ).c_str());
  }
  if (!d->stat.length()) {
    const Register::StatusDesc* sd = regMan->getStatusDesc(0);
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
      && zman->findZoneId(fqdn)->isEnumZone();
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
 *              clientID - client id
 *              clTRID - transaction client number
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::DomainDelete(
  const char* fqdn, CORBA::Long clientID, const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  DB DBsql;
  std::auto_ptr<EPPNotifier> ntf;
  int regID, id, zone;
  ret = new ccReg::Response;
  errors = new ccReg::Errors;

  // default
  ret->code=0;
  errors->length(0);

  ParsedAction paction;
  paction.add(1,(const char*)fqdn);

  LOG( NOTICE_LOG , "DomainDelete: clientID -> %d clTRID [%s] fqdn  [%s] " , (int ) clientID , clTRID , fqdn );

  if ( (regID = GetRegistrarID(clientID) ))

    if (DBsql.OpenDatabase(database) ) {

      if ( (DBsql.BeginAction(clientID, EPP_DomainDelete, clTRID, XML, &paction) )) {

        if (DBsql.BeginTransaction() ) {
          if ( (id = getIdOfDomain(&DBsql, fqdn, conf, true, &zone) ) < 0)
            ret->code=SetReasonDomainFQDN(errors, fqdn, id == -1,
                GetRegistrarLang(clientID) );
          else if (id == 0) {
            LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
            ret->code=COMMAND_OBJECT_NOT_EXIST;
          }
          else if (DBsql.TestRegistrarZone(regID, zone) == false) {
            LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
            ret->code = COMMAND_AUTHENTICATION_ERROR;
          }
          else if ( !DBsql.TestObjectClientID(id, regID) ) {
            LOG( WARNING_LOG, "bad autorization not client of fqdn [%s]", fqdn );
            ret->code = SetReasonWrongRegistrar(errors, regID, GetRegistrarLang(clientID));
          }
          try {
            if (!ret->code && (
                 testObjectHasState(&DBsql,id,FLAG_serverDeleteProhibited) ||
                 testObjectHasState(&DBsql,id,FLAG_serverUpdateProhibited)
               ))
            {
              LOG( WARNING_LOG, "delete of object %s is prohibited" , fqdn );
              ret->code = COMMAND_STATUS_PROHIBITS_OPERATION;
            }
          } catch (...) {
            ret->code = COMMAND_FAILED;
          }
          if (!ret->code) {
                // run notifier
                ntf.reset(new EPPNotifier(conf.get<bool>("registry.disable_epp_notifier"),mm , &DBsql, regID , id ));

                if (DBsql.SaveObjectDelete(id) ) //save object as delete
                {
                  if (DBsql.DeleteDomainObject(id) )
                    ret->code = COMMAND_OK; // if succesfully deleted
                }
                if (ret->code == COMMAND_OK)
                  ntf->Send(); // if is ok send messages
          }

            DBsql.QuitTransaction(ret->code);
          }

        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
      }

      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
    }
  // EPP exception
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  if (ret->code == 0)
    ServerInternalError("DomainDelete");

  return ret._retn();
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
 *              clientID - client id
 *              clTRID - transaction client number
 *              ext - ExtensionList
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/
ccReg::Response * ccReg_EPP_i::DomainUpdate(
  const char *fqdn, const char *registrant_chg, const char *authInfo_chg,
  const char *nsset_chg, const char *keyset_chg,
  const ccReg::AdminContact & admin_add, const ccReg::AdminContact & admin_rem,
  const ccReg::AdminContact& tmpcontact_rem, CORBA::Long clientID,
  const char *clTRID, const char* XML, const ccReg::ExtensionList & ext)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  std::auto_ptr<EPPNotifier> ntf;
  DB DBsql;
  char valexpiryDate[MAX_DATE+1];
  int regID = 0, id, nssetid, contactid, adminid, keysetid;
  int seq, zone;
  std::vector<int> ac_add, ac_rem, tc_rem;
  unsigned int i, j;

  ret = new ccReg::Response;
  errors = new ccReg::Errors;

  seq=0;
  ret->code = 0;
  errors->length( 0);

  ParsedAction paction;
  paction.add(1,(const char*)fqdn);

  LOG( NOTICE_LOG, "DomainUpdate: clientID -> %d clTRID [%s] fqdn  [%s] , registrant_chg  [%s] authInfo_chg [%s]  nsset_chg [%s] keyset_chg[%s] ext.length %ld",
      (int ) clientID, clTRID, fqdn, registrant_chg, authInfo_chg, nsset_chg , keyset_chg, (long)ext.length() );

  ac_add.resize(admin_add.length());
  ac_rem.resize(admin_rem.length());
  tc_rem.resize(tmpcontact_rem.length());

  // parse enum.Exdate extension
  GetValExpDateFromExtension(valexpiryDate, ext);

  if ( (regID = GetRegistrarID(clientID) ))
      LOG(NOTICE_LOG, "registrarID: %d", regID);

    if (DBsql.OpenDatabase(database) ) {

      if ( (DBsql.BeginAction(clientID, EPP_DomainUpdate, clTRID, XML, &paction) )) {

        if (DBsql.BeginTransaction()) {
          if ( (id = getIdOfDomain(&DBsql, fqdn, conf, true, &zone) ) < 0)
            ret->code=SetReasonDomainFQDN(errors, fqdn, id == -1,
                GetRegistrarLang(clientID) );
          else if (id == 0) {
            LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
            ret->code=COMMAND_OBJECT_NOT_EXIST;
          }
          else if (DBsql.TestRegistrarZone(regID, zone) == false) // test registrar autority to the zone
          {
            LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
            ret->code = COMMAND_AUTHENTICATION_ERROR;
          }
          // if not client of the domain
          else if ( !DBsql.TestObjectClientID(id, regID) ) {
            LOG( WARNING_LOG, "bad autorization not client of domain [%s]", fqdn );
            ret->code = SetReasonWrongRegistrar(errors, regID, GetRegistrarLang(clientID));
          }

          if (!ret->code) {
              try {
                  if (!ret->code && testObjectHasState(&DBsql,id,FLAG_serverUpdateProhibited)) {
                      LOG( WARNING_LOG, "update of object %s is prohibited" , fqdn );
                      ret->code = COMMAND_STATUS_PROHIBITS_OPERATION;
                  }
              } catch (...) {
                  ret->code = COMMAND_FAILED;
              }
          }

          if ( !ret->code) {

            // test  ADD admin-c
            for (i = 0; i < admin_add.length(); i++) {
              LOG( NOTICE_LOG , "admin ADD contact %s" , (const char *) admin_add[i] );
              if ( (adminid = getIdOfContact(&DBsql, admin_add[i], conf) ) <= 0)
                ret->code = SetReasonDomainAdminADD(errors, admin_add[i],
                    adminid, GetRegistrarLang(clientID) , i);
              else {
                if (DBsql.CheckContactMap("domain", id, adminid, 1) )
                  ret->code = SetReasonDomainAdminExistMap(errors,
                      admin_add[i], GetRegistrarLang(clientID) , i);
                else {
                  ac_add[i] = adminid;
                  for (j = 0; j < i; j ++)
                    // test  duplicity
                    if (ac_add[j] == adminid && ac_add[j] > 0) {
                      ac_add[j] = 0;
                      ret->code = SetReasonContactDuplicity(errors,
                          admin_add[i], GetRegistrarLang(clientID) , i,
                          ccReg::domain_admin_add);
                    }
                }
                // admin cannot be added if there is equivalent id in temp-c
                if (DBsql.CheckContactMap("domain", id, adminid, 2)) {
                  // exception is when in this command there is schedulet remove of thet temp-c
                  std::string adminHandle = (const char *)admin_add[i];
                  bool tmpcFound = false;
                  for (unsigned ti=0; ti<tmpcontact_rem.length(); ti++) {
                    if (adminHandle == (const char *)tmpcontact_rem[ti]) {
                      tmpcFound = true;
                      break;
                    }
                  }
                  if (!tmpcFound)
                    ret->code = SetReasonDomainAdminExistMap(errors,
                        admin_add[i], GetRegistrarLang(clientID) , i);
                }
              }
            }

            // test REM admin-c
            for (i = 0; i < admin_rem.length(); i++) {
              LOG( NOTICE_LOG , "admin REM contact %s" , (const char *) admin_rem[i] );
              if ( (adminid = getIdOfContact(&DBsql, admin_rem[i], conf) ) <= 0)
                ret->code = SetReasonDomainAdminREM(errors, admin_rem[i],
                    adminid, GetRegistrarLang(clientID) , i);
              else {
                if ( !DBsql.CheckContactMap("domain", id, adminid, 1) )
                  ret->code = SetReasonDomainAdminNotExistMap(errors,
                      admin_rem[i], GetRegistrarLang(clientID) , i);
                else {

                  ac_rem[i] = adminid;
                  for (j = 0; j < i; j ++)
                    // test  duplicity
                    if (ac_rem[j] == adminid && ac_rem[j] > 0) {
                      ac_rem[j] = 0;
                      ret->code = SetReasonContactDuplicity(errors,
                          admin_rem[i], GetRegistrarLang(clientID) , i,
                          ccReg::domain_admin_rem);
                    }

                }
              }
            }

            // test REM temp-c
            for (i = 0; i < tmpcontact_rem.length(); i++) {
              LOG( NOTICE_LOG , "temp REM contact %s" , (const char *) tmpcontact_rem[i] );
              if ( (adminid = getIdOfContact(&DBsql, tmpcontact_rem[i], conf) )
                  <= 0)
                ret->code = SetReasonDomainTempCREM(errors, tmpcontact_rem[i],
                    adminid, GetRegistrarLang(clientID) , i);
              else {
                if ( !DBsql.CheckContactMap("domain", id, adminid, 2) )
                  ret->code = SetReasonDomainTempCNotExistMap(errors,
                      tmpcontact_rem[i], GetRegistrarLang(clientID) , i);
                else {

                  tc_rem[i] = adminid;
                  for (j = 0; j < i; j ++)
                    // test  duplicity
                    if (tc_rem[j] == adminid && ac_rem[j] > 0) {
                      tc_rem[j] = 0;
                      ret->code = SetReasonContactDuplicity(errors,
                          tmpcontact_rem[i], GetRegistrarLang(clientID) , i,
                          ccReg::domain_admin_rem);
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
                if ( (nssetid = getIdOfNSSet(&DBsql, nsset_chg, conf) ) <= 0)
                  ret->code = SetReasonDomainNSSet(errors, nsset_chg, nssetid,
                      GetRegistrarLang(clientID) );
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
                    if ((keysetid = getIdOfKeySet(&DBsql, keyset_chg, conf)) <= 0)
                        ret->code = SetReasonDomainKeySet(errors, keyset_chg,
                                keysetid, GetRegistrarLang(clientID));
                }
            }

            //  owner of domain
            if (strlen(registrant_chg) == 0)
              contactid = 0; // not change owner
            else if ( (contactid = getIdOfContact(&DBsql, registrant_chg, conf) )
                <= 0)
              ret->code = SetReasonDomainRegistrant(errors, registrant_chg,
                  contactid, GetRegistrarLang(clientID) );

            if (strlen(valexpiryDate) ) {
              // Test for  enum domain
              if (GetZoneEnum(zone) ) {
                if (DBsql.TestValExDate(valexpiryDate, GetZoneValPeriod(zone) ,
                    DefaultValExpInterval() , id) == false) // test validace expirace
                {
                  LOG( WARNING_LOG, "DomainUpdate:  validity exp date is not valid %s" , valexpiryDate );
                  ret->code = SetErrorReason(errors,
                      COMMAND_PARAMETR_RANGE_ERROR, ccReg::domain_ext_valDate,
                      1,
                      REASON_MSG_VALEXPDATE_NOT_VALID,
                      GetRegistrarLang(clientID) );
                }

              } else {

                LOG( WARNING_LOG, "DomainUpdate: can not  validity exp date %s" , valexpiryDate );
                ret->code = SetErrorReason(errors,
                COMMAND_PARAMETR_VALUE_POLICY_ERROR, ccReg::domain_ext_valDate,
                    1, REASON_MSG_VALEXPDATE_NOT_USED,
                    GetRegistrarLang(clientID) );

              }
            }
            if (strlen(registrant_chg) != 0) {
              try {
                if (!ret->code && testObjectHasState(&DBsql,id,FLAG_serverRegistrantChangeProhibited))
                {
                  LOG( WARNING_LOG, "registrant change %s is prohibited" , fqdn );
                  ret->code = COMMAND_STATUS_PROHIBITS_OPERATION;
                }
              } catch (...) {
                ret->code = COMMAND_FAILED;
              }
            }
            if (ret->code == 0) {

              // BEGIN notifier
              // notify default contacts
              ntf.reset(new EPPNotifier(conf.get<bool>("registry.disable_epp_notifier"),mm , &DBsql, regID , id ));

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
              if (DBsql.ObjectUpdate(id, regID, authInfo_chg) ) {

                if (nssetid || contactid || keysetid) // update domain table only if change
                {
                  // change record of domain
                  DBsql.UPDATE("DOMAIN");

                  if (nssetid > 0)
                    DBsql.SET("nsset", nssetid); // change nssetu
                  else if (nssetid == -1)
                    DBsql.SETNULL("nsset"); // delete nsset

                  if (keysetid > 0)
                      DBsql.SET("keyset", keysetid); // change keyset
                  else if (keysetid == -1)
                      DBsql.SETNULL("keyset"); // delete keyset

                  if (contactid)
                    DBsql.SET("registrant", contactid); // change owner

                  DBsql.WHEREID(id);
                  if ( !DBsql.EXEC() )
                    ret->code = COMMAND_FAILED;
                }

                if (ret->code == 0) {

                  // change validity exdate  extension
                  if (GetZoneEnum(zone) && strlen(valexpiryDate) > 0) {
                    LOG( NOTICE_LOG, "change valExpDate %s ", valexpiryDate );
                    DBsql.UPDATE("enumval");
                    DBsql.SET("ExDate", valexpiryDate);
                    DBsql.WHERE("domainID", id);

                    if ( !DBsql.EXEC() )
                      ret->code = COMMAND_FAILED;
                  }

                  // REM temp-c (must be befor ADD admin-c because of uniqueness)
                  for (i = 0; i < tmpcontact_rem.length(); i++) {
                    if ( (adminid = getIdOfContact(&DBsql, tmpcontact_rem[i],
                        conf) )) {
                      LOG( NOTICE_LOG , "delete temp-c-c  -> %d [%s]" , tc_rem[i] , (const char * ) tmpcontact_rem[i] );
                      if ( !DBsql.DeleteFromTableMap("domain", id, tc_rem[i]) ) {
                        ret->code = COMMAND_FAILED;
                        break;
                      }
                    }

                  }

                  // ADD admin-c
                  for (i = 0; i < admin_add.length(); i++) {

                    LOG( DEBUG_LOG, "DomainUpdate: add admin Contact %s id %d " , (const char *) admin_add[i] , ac_add[i] );
                    if ( !DBsql.AddContactMap("domain", id, ac_add[i]) ) {
                      ret->code = COMMAND_FAILED;
                      break;
                    }

                  }

                  // REM admin-c
                  for (i = 0; i < admin_rem.length(); i++) {
                    if ( (adminid = getIdOfContact(&DBsql, admin_rem[i], conf) )) {
                      LOG( NOTICE_LOG , "delete admin  -> %d [%s]" , ac_rem[i] , (const char * ) admin_rem[i] );
                      if ( !DBsql.DeleteFromTableMap("domain", id, ac_rem[i]) ) {
                        ret->code = COMMAND_FAILED;
                        break;
                      }
                    }

                  }

                  // save to the history on the end if is OK
                  if (ret->code == 0)
                    if (DBsql.SaveDomainHistory(id) )
                      ret->code = COMMAND_OK; // set up successfully


                }

              }

              // notifier send messages
              if (ret->code == COMMAND_OK)
                ntf->Send();

            }

          }
          DBsql.QuitTransaction(ret->code);
        }

        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
      }

      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
    }

  // EPP exception
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  if (ret->code == 0)
    ServerInternalError("DomainUpdate");

  return ret._retn();
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
 *              clientID - client id
 *              clTRID - transaction client number
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
        CORBA::Long clientID,
        const char *clTRID,
        const char* XML,
        const ccReg::ExtensionList & ext
        )
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  DB DBsql;
  std::auto_ptr<EPPNotifier> ntf;
  char valexpiryDate[MAX_DATE+1];
  char FQDN[164];
  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  int contactid, regID, nssetid, adminid, id, keysetid;
  int zone =0;
  unsigned int i, j;
  std::vector<int> ad;
  int period_count;
  char periodStr[10];
  Register::NameIdPair dConflict;
  Register::Domain::CheckAvailType dType;

  ret = new ccReg::Response;
  errors = new ccReg::Errors;

  // default
  ret->code = 0;
  errors->length();

  ParsedAction paction;
  paction.add(1,(const char*)fqdn);

  crDate = CORBA::string_dup("");
  exDate = CORBA::string_dup("");

  ad.resize(admin.length());

  LOG( NOTICE_LOG, "DomainCreate: clientID -> %d clTRID [%s] fqdn  [%s] ",
          (int ) clientID, clTRID, fqdn );
  LOG( NOTICE_LOG,
          "DomainCreate:  Registrant  [%s]  nsset [%s]  keyset [%s] AuthInfoPw [%s]",
          Registrant, nsset, keyset, AuthInfoPw);

  //  period transform from structure to month
  // if in year
  if (period.unit == ccReg::unit_year) {
    period_count = period.count * 12;
    sprintf(periodStr, "y%d", period.count);
  }
  // if in month
  else if (period.unit == ccReg::unit_month) {
    period_count = period.count;
    sprintf(periodStr, "m%d", period.count);
  } else
    period_count = 0;

  LOG( NOTICE_LOG,
          "DomainCreate: period count %d unit %d period_count %d string [%s]" ,
          period.count , period.unit , period_count , periodStr);

  // parse enum.exdate extension for validitydate
  GetValExpDateFromExtension(valexpiryDate, ext);

  if ( (regID = GetRegistrarID(clientID) ))

    if (DBsql.OpenDatabase(database) ) {

      if ( (DBsql.BeginAction(clientID, EPP_DomainCreate, clTRID, XML, &paction) )) {

        if (DBsql.BeginTransaction() ) {

          // check domain FQDN by registrar lib

          try {
            std::auto_ptr<Register::Zone::Manager> zm( Register::Zone::Manager::create(&DBsql) );
            std::auto_ptr<Register::Domain::Manager> dman( Register::Domain::Manager::create(&DBsql,zm.get()) );

            LOG( NOTICE_LOG , "Domain::checkAvail  fqdn [%s]" , (const char * ) fqdn );

            dType = dman->checkAvail( ( const char * ) fqdn , dConflict);
            LOG( NOTICE_LOG , "domain type %d" , dType );
            switch (dType) {
              case Register::Domain::CA_INVALID_HANDLE:
              case Register::Domain::CA_BAD_LENGHT:
                LOG( NOTICE_LOG , "bad format %s of fqdn" , (const char * ) fqdn );
                ret->code = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                    ccReg::domain_fqdn, 1, REASON_MSG_BAD_FORMAT_FQDN,
                    GetRegistrarLang(clientID));
                break;
              case Register::Domain::CA_REGISTRED:
              case Register::Domain::CA_CHILD_REGISTRED:
              case Register::Domain::CA_PARENT_REGISTRED:
                LOG( WARNING_LOG, "domain  [%s] EXIST", fqdn );
                ret->code = COMMAND_OBJECT_EXIST; // if is exist
                break;
              case Register::Domain::CA_BLACKLIST: // black listed
                LOG( NOTICE_LOG , "blacklisted  %s" , (const char * ) fqdn );
                ret->code = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                    ccReg::domain_fqdn, 1, REASON_MSG_BLACKLISTED_DOMAIN,
                    GetRegistrarLang(clientID) );
                break;
              case Register::Domain::CA_AVAILABLE: // if is free
                // conver fqdn to lower case and get zone
                zone = getFQDN(FQDN, fqdn);
                LOG( NOTICE_LOG , "domain %s avail zone %d" ,(const char * ) FQDN , zone );
                break;
              case Register::Domain::CA_BAD_ZONE:
                // domain not in zone
                LOG( NOTICE_LOG , "NOn in zone not applicable %s" , (const char * ) fqdn );
                ret->code = SetErrorReason(errors, COMMAND_PARAMETR_ERROR,
                    ccReg::domain_fqdn, 1, REASON_MSG_NOT_APPLICABLE_DOMAIN,
                    GetRegistrarLang(clientID) );
                break;
            }

            if (dType == Register::Domain::CA_AVAILABLE) {

              if (DBsql.TestRegistrarZone(regID, zone) == false) {
                LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
                ret->code = COMMAND_AUTHENTICATION_ERROR;
              } else {

                if (strlen(nsset) == 0)
                  nssetid = 0; // domain can be create without nsset
                else if ( (nssetid = getIdOfNSSet( &DBsql, nsset, conf) ) <= 0)
                  ret->code = SetReasonDomainNSSet(errors, nsset, nssetid,
                      GetRegistrarLang(clientID) );

                // KeySet
                if (strlen(keyset) == 0)
                    keysetid = 0;
                else if ((keysetid = getIdOfKeySet(&DBsql, keyset, conf)) <= 0)
                    ret->code = SetReasonDomainKeySet(errors, keyset, keysetid,
                            GetRegistrarLang(clientID));

                //  owner of domain
                if ( (contactid = getIdOfContact(&DBsql, Registrant, conf) ) <= 0)
                  ret->code = SetReasonDomainRegistrant(errors, Registrant,
                      contactid, GetRegistrarLang(clientID) );

                // default period if not set from zone parametrs
                if (period_count == 0) {
                  period_count = GetZoneExPeriodMin(zone);
                  LOG( NOTICE_LOG, "get defualt peridod %d month  for zone   %d ", period_count , zone );
                }

                // test period validity range and modulo
                switch (TestPeriodyInterval(period_count,
                    GetZoneExPeriodMin(zone) , GetZoneExPeriodMax(zone) ) ) {
                  case 2:
                    LOG( WARNING_LOG, "period %d interval ot of range MAX %d MIN %d" , period_count , GetZoneExPeriodMax( zone ) , GetZoneExPeriodMin( zone ) );
                    ret->code = SetErrorReason(errors,
                    COMMAND_PARAMETR_RANGE_ERROR, ccReg::domain_period, 1,
                    REASON_MSG_PERIOD_RANGE, GetRegistrarLang(clientID) );
                    break;
                  case 1:
                    LOG( WARNING_LOG, "period %d  interval policy error MIN %d" , period_count , GetZoneExPeriodMin( zone ) );
                    ret->code = SetErrorReason(errors,
                    COMMAND_PARAMETR_VALUE_POLICY_ERROR, ccReg::domain_period, 1,
                        REASON_MSG_PERIOD_POLICY, GetRegistrarLang(clientID) );
                    break;

                }

                // test  validy date for enum domain
                if (strlen(valexpiryDate) == 0) {
                  // for enum domain must set validity date
                  if (GetZoneEnum(zone) ) {
                    LOG( WARNING_LOG, "DomainCreate: validity exp date MISSING" );

                    ret->code = SetErrorReason(errors, COMMAND_PARAMETR_MISSING,
                        ccReg::domain_ext_valDate_missing, 0,
                        REASON_MSG_VALEXPDATE_REQUIRED,
                        GetRegistrarLang(clientID) );
                  }
                } else {
                  // Test for enum domain
                  if (GetZoneEnum(zone) ) {
                    // test
                    if (DBsql.TestValExDate(valexpiryDate,
                        GetZoneValPeriod(zone) , DefaultValExpInterval() , 0)
                        == false) {
                      LOG( WARNING_LOG, "Validity exp date is not valid %s" , valexpiryDate );
                      ret->code = SetErrorReason(errors,
                      COMMAND_PARAMETR_RANGE_ERROR, ccReg::domain_ext_valDate, 1,
                          REASON_MSG_VALEXPDATE_NOT_VALID,
                          GetRegistrarLang(clientID) );
                    }

                  } else {
                    LOG( WARNING_LOG, "Validity exp date %s not user" , valexpiryDate );
                    ret->code = SetErrorReason(errors,
                    COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                        ccReg::domain_ext_valDate, 1,
                        REASON_MSG_VALEXPDATE_NOT_USED,
                        GetRegistrarLang(clientID) );

                  }

                }

                // test   admin-c if set

                if (admin.length() > 0) {
                  // test
                  for (i = 0; i < admin.length(); i++) {
                    if ( (adminid = getIdOfContact( &DBsql, admin[i], conf) )
                        <= 0)
                      ret->code = SetReasonDomainAdmin(errors, admin[i], adminid,
                          GetRegistrarLang(clientID) , i);
                    else {
                      ad[i] = adminid;
                      for (j = 0; j < i; j ++) // test  duplicity
                      {
                        LOG( DEBUG_LOG , "admin comapare j %d adminid %d ad %d" , j , adminid , ad[j] );
                        if (ad[j] == adminid && ad[j] > 0) {
                          ad[j] = 0;
                          ret->code
                              = SetReasonContactDuplicity(errors, admin[i],
                                  GetRegistrarLang(clientID) , i,
                                  ccReg::domain_admin);
                        }
                      }

                    }
                  }

                }

                if (ret->code == 0) // if not error
                {

                  id= DBsql.CreateObject("D", regID, FQDN, AuthInfoPw);
                  if (id<=0) {
                    if (id == 0) {
                      LOG( WARNING_LOG, "domain fqdn [%s] EXIST", fqdn );
                      ret->code= COMMAND_OBJECT_EXIST;
                    } else {
                      LOG( WARNING_LOG, "Cannot insert [%s] into object_registry", fqdn );
                      ret->code= COMMAND_FAILED;
                    }
                  } else {

                    std::string computed_exdate;
                    try {
                      /* compute expiration date from creation time - need
                       * proper conversion to local time then take only date
                       */
                      using namespace boost::posix_time;

                      ptime db_crdate = time_from_string(DBsql.GetValueFromTable("object_registry", "crdate", "id", id));
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

                    DBsql.INSERT("DOMAIN");
                    DBsql.INTO("id");
                    DBsql.INTO("zone");
                    DBsql.INTO("Exdate");
                    DBsql.INTO("Registrant");
                    DBsql.INTO("nsset");
                    DBsql.INTO("keyset");

                    DBsql.VALUE(id);
                    DBsql.VALUE(zone);
                    DBsql.VALUE(computed_exdate.c_str());
                    //DBsql.VALUEPERIOD(period_count); // actual time plus interval of period in months
                    DBsql.VALUE(contactid);
                    if (nssetid == 0)
                      DBsql.VALUENULL(); // domain without  nsset write NULL value
                    else
                      DBsql.VALUE(nssetid);

                    if (keysetid == 0)
                        DBsql.VALUENULL();
                    else
                        DBsql.VALUE(keysetid);

                    if (DBsql.EXEC() ) {

                      // get local timestamp of created domain
                      CORBA::string_free(crDate);
                      crDate= CORBA::string_dup(DBsql.GetObjectCrDateTime(id) );

                      //  get local date of expiration
                      CORBA::string_free(exDate);
                      exDate = CORBA::string_dup(DBsql.GetDomainExDate(id) );

                      // save  enum domain   extension validity date
                      if (GetZoneEnum(zone) && strlen(valexpiryDate) > 0) {
                        DBsql.INSERT("enumval");
                        DBsql.VALUE(id);
                        DBsql.VALUE(valexpiryDate);
                        if (DBsql.EXEC() == false)
                          ret->code = COMMAND_FAILED;
                      }

                      // insert   admin-c
                      for (i = 0; i < admin.length(); i++) {
                        LOG( DEBUG_LOG, "DomainCreate: add admin Contact %s id %d " , (const char *) admin[i] , ad[i] );
                        if ( !DBsql.AddContactMap("domain", id, ad[i]) ) {
                          ret->code = COMMAND_FAILED;
                          break;
                        }

                      }

                      //  billing credit from and save for invoicing
                      // first operation billing domain-create
                      if (DBsql.BillingCreateDomain(regID, zone, id) == false)
                        ret->code = COMMAND_BILLING_FAILURE;
                      else
                      // next operation billing  domain-renew save Exdate
                      if (DBsql.BillingRenewDomain(regID, zone, id, period_count,
                          exDate) == false)
                        ret->code = COMMAND_BILLING_FAILURE;
                      else if (DBsql.SaveDomainHistory(id) ) // if is ok save to history
                        if (DBsql.SaveObjectCreate(id) )
                          ret->code = COMMAND_OK;

                    } else
                      ret->code = COMMAND_FAILED;

                    if (ret->code == COMMAND_OK) // run notifier
                    {
                      ntf.reset(new EPPNotifier(conf.get<bool>("registry.disable_epp_notifier"),mm , &DBsql, regID , id ));
                      ntf->Send(); // send messages
                    }

                  }
                }

              }

            }
          }
          catch (...) {
            LOG( WARNING_LOG, "cannot run Register::Domain::checkAvail");
            ret->code=COMMAND_FAILED;
          }


          // if ret->code == COMMAND_OK commit transaction
          DBsql.QuitTransaction(ret->code);
        }

        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
      }

      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
    }

  // EPP exception
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  if (ret->code == 0)
    ServerInternalError("DomainCreate");

  return ret._retn();
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
 *              clientID - connected client id
 *              clTRID - transaction client number
 *              ext - ExtensionList
 *
 * RETURNED:    svTRID and errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::DomainRenew(
  const char *fqdn, const char* curExpDate, const ccReg::Period_str& period,
  ccReg::timestamp_out exDate, CORBA::Long clientID, const char *clTRID,
  const char* XML, const ccReg::ExtensionList & ext)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  DB DBsql;
  std::auto_ptr<EPPNotifier> ntf;
  char valexpiryDate[MAX_DATE+1];
  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  int regID, id, zone;
  int period_count;
  char periodStr[10];

  ret = new ccReg::Response;
  errors = new ccReg::Errors;

  ParsedAction paction;
  paction.add(1,(const char*)fqdn);

  // default
  exDate = CORBA::string_dup("");

  // default
  ret->code = 0;
  errors->length( 0);

  LOG( NOTICE_LOG, "DomainRenew: clientID -> %d clTRID [%s] fqdn  [%s] curExpDate [%s]", (int ) clientID, clTRID, fqdn , (const char *) curExpDate );

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
  GetValExpDateFromExtension(valexpiryDate, ext);

  if ( (regID = GetRegistrarID(clientID) )) {

    if (DBsql.OpenDatabase(database)) {
      if (DBsql.BeginAction(clientID, EPP_DomainRenew, clTRID, XML, &paction)) {
        if (DBsql.BeginTransaction()) {
          if ((id = getIdOfDomain(&DBsql, fqdn, conf, true, &zone) ) < 0)
            ret->code=SetReasonDomainFQDN(errors, fqdn, id == -1,
                GetRegistrarLang(clientID) );
          else if (id == 0) {
            LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
            ret->code=COMMAND_OBJECT_NOT_EXIST;
          }
          else  if (DBsql.TestRegistrarZone(regID, zone) == false) {
              LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
              ret->code = COMMAND_AUTHENTICATION_ERROR;
          }
          // test curent ExDate
          // there should be lock here for row with exdate
          // but there is already lock on object_registry row
          // (from getIdOfDomain) and so there cannot be any race condition
          else if (TestExDate(curExpDate, DBsql.GetDomainExDate(id)) == false) {
            LOG( WARNING_LOG, "curExpDate is not same as ExDate" );
            ret->code = SetErrorReason(
              errors, COMMAND_PARAMETR_ERROR,
              ccReg::domain_curExpDate, 1,
              REASON_MSG_CUREXPDATE_NOT_EXPDATE,
              GetRegistrarLang(clientID)
            );
          } else {
                // set default renew  period from zone params
                if (period_count == 0) {
                  period_count = GetZoneExPeriodMin(zone);
                  LOG( NOTICE_LOG, "get default peridod %d month  for zone   %d ", period_count , zone );
                }

                //  test period
                switch (TestPeriodyInterval(period_count,
                    GetZoneExPeriodMin(zone) , GetZoneExPeriodMax(zone) ) ) {
                  case 2:
                    LOG( WARNING_LOG, "period %d interval ot of range MAX %d MIN %d" , period_count , GetZoneExPeriodMax( zone ) , GetZoneExPeriodMin( zone ) );
                    ret->code = SetErrorReason(errors,
                    COMMAND_PARAMETR_RANGE_ERROR, ccReg::domain_period, 1,
                    REASON_MSG_PERIOD_RANGE, GetRegistrarLang(clientID) );
                    break;
                  case 1:
                    LOG( WARNING_LOG, "period %d  interval policy error MIN %d" , period_count , GetZoneExPeriodMin( zone ) );
                    ret->code
                        = SetErrorReason(errors,
                        COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                            ccReg::domain_period, 1, REASON_MSG_PERIOD_POLICY,
                            GetRegistrarLang(clientID) );
                    break;
                  default:
                    // count new  ExDate
                    if (DBsql.CountExDate(id, period_count,
                        GetZoneExPeriodMax(zone) ) == false) {
                      LOG( WARNING_LOG, "period %d ExDate out of range" , period_count );
                      ret->code = SetErrorReason(errors,
                      COMMAND_PARAMETR_RANGE_ERROR, ccReg::domain_period, 1,
                      REASON_MSG_PERIOD_RANGE, GetRegistrarLang(clientID) );
                    }
                    break;

                }

                // test validity Date for enum domain
                if (strlen(valexpiryDate) ) {
                  // Test for enum domain only
                  if (GetZoneEnum(zone) ) {
                    if (DBsql.TestValExDate(valexpiryDate,
                        GetZoneValPeriod(zone) , DefaultValExpInterval() , id)
                        == false) {
                      LOG( WARNING_LOG, "Validity exp date is not valid %s" , valexpiryDate );
                      ret->code = SetErrorReason(errors,
                      COMMAND_PARAMETR_RANGE_ERROR, ccReg::domain_ext_valDate,
                          1,
                          REASON_MSG_VALEXPDATE_NOT_VALID,
                          GetRegistrarLang(clientID) );
                    }

                  } else {

                    LOG( WARNING_LOG, "Can not  validity exp date %s" , valexpiryDate );
                    ret->code = SetErrorReason(errors,
                    COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                        ccReg::domain_ext_valDate, 1,
                        REASON_MSG_VALEXPDATE_NOT_USED,
                        GetRegistrarLang(clientID) );

                  }
                }

                if (ret->code == 0)// if not param error
                {
                  // test client of the object
                  if ( !DBsql.TestObjectClientID(id, regID) ) {
                    LOG( WARNING_LOG, "bad autorization not client of domain [%s]", fqdn );
                    ret->code = SetReasonWrongRegistrar(errors, regID, GetRegistrarLang(clientID));
                  }
                  try {
                    if (!ret->code && (
                            testObjectHasState(&DBsql,id,FLAG_serverRenewProhibited) ||
                            testObjectHasState(&DBsql,id,FLAG_deleteCandidate)
                        )
                    )
                    {
                      LOG( WARNING_LOG, "renew of object %s is prohibited" , fqdn );
                      ret->code = COMMAND_STATUS_PROHIBITS_OPERATION;
                    }
                  } catch (...) {
                    ret->code = COMMAND_FAILED;
                  }

                  if (!ret->code) {

                    // change validity date for enum domain
                    if (GetZoneEnum(zone) ) {
                      if (strlen(valexpiryDate) > 0) {
                        LOG( NOTICE_LOG, "change valExpDate %s ", valexpiryDate );

                        DBsql.UPDATE("enumval");
                        DBsql.SET("ExDate", valexpiryDate);
                        DBsql.WHERE("domainID", id);

                        if (DBsql.EXEC() == false)
                          ret->code = COMMAND_FAILED;
                      }
                    }

                    if (ret->code == 0) // if is OK OK
                    {

                      // make Renew Domain count new Exdate in
                      if (DBsql.RenewExDate(id, period_count) ) {
                        //  return new Exdate as local date
                        CORBA::string_free(exDate);
                        exDate = CORBA::string_dup(DBsql.GetDomainExDate(id) );

                        // billing credit operation domain-renew
                        if (DBsql.BillingRenewDomain(regID, zone, id,
                            period_count, exDate) == false)
                          ret->code = COMMAND_BILLING_FAILURE;
                        else if (DBsql.SaveDomainHistory(id) )
                          ret->code = COMMAND_OK;

                      } else
                        ret->code = COMMAND_FAILED;
                    }
                  }
                }
              }
            if (ret->code == COMMAND_OK) // run notifier
            {
              ntf.reset(new EPPNotifier(conf.get<bool>("registry.disable_epp_notifier"),mm , &DBsql, regID , id ));
              ntf->Send(); // send mesages to default contats
            }
            DBsql.QuitTransaction(ret->code);

          }
        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) );
      }

      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
    }

  }
  // EPP exception
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  if (ret->code == 0)
    ServerInternalError("DomainRenew");

  return ret._retn();
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
        CORBA::Long clientID,
        const char *clTRID,
        const char *XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

    LOG(NOTICE_LOG, "KeySetInfo: clientID -> %d clTRID [%s] handle [%s] ",
            (int)clientID, clTRID, handle);
    ParsedAction paction;
    paction.add(1, (const char *)handle);

    EPPAction a(this, clientID, EPP_KeySetInfo, clTRID, XML, &paction);

    std::auto_ptr<Register::KeySet::Manager> kman(
            Register::KeySet::Manager::create(
                a.getDB(), conf.get<bool>("registry.restricted_handles"))
            );
    // first check handle for proper format
    if (!kman->checkHandleFormat(handle))
        a.failed(SetReasonKeySetHandle(
                    a.getErrors(), handle, a.getLang())
                );

    // load keyset by handle
    std::auto_ptr<Register::KeySet::List> klist(kman->createList());

    Database::Filters::Union unionFilter;
    Database::Filters::KeySet *keyFilter = new Database::Filters::KeySetHistoryImpl();

    keyFilter->addHandle().setValue(std::string(handle));
    keyFilter->addDeleteTime().setNULL();
    unionFilter.addFilter(keyFilter);
    Database::Manager *dbman = new Database::Manager(database);

    klist->reload(unionFilter, dbman);

    //klist->setHandleFilter(handle);
    // try {
        // klist->reload();
    // } catch (...) {
        // a.failedInternal("Cannot load keyset");
    // }
    if (klist->getCount() != 1)
        // failed because of non existence
        a.failed(COMMAND_OBJECT_NOT_EXIST);

    Register::KeySet::KeySet *kss = klist->getKeySet(0);
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
        Register::TID stateId = kss->getStatusByIdx(i)->getStatusId();
        const Register::StatusDesc *sd = regMan->getStatusDesc(stateId);
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
        const Register::StatusDesc *sd = regMan->getStatusDesc(0);
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
        const Register::KeySet::DSRecord *dsr = kss->getDSRecordByIdx(i);
        k->dsrec[i].keyTag = dsr->getKeyTag();
        k->dsrec[i].alg = dsr->getAlg();
        k->dsrec[i].digestType = dsr->getDigestType();
        k->dsrec[i].digest = CORBA::string_dup(dsr->getDigest().c_str());
        k->dsrec[i].maxSigLife = dsr->getMaxSigLife();
    }

    // dnskey record
    k->dnsk.length(kss->getDNSKeyCount());
    for (unsigned int i = 0; i < kss->getDNSKeyCount(); i++) {
        const Register::KeySet::DNSKey *dnsk = kss->getDNSKeyByIdx(i);
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
        CORBA::Long clientID,
        const char *clTRID,
        const char *XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

    ccReg::Response_var ret;
    ccReg::Errors_var   errors;
    DB                  DBsql;
    int                 regID, id;
    std::auto_ptr<EPPNotifier> ntf;

    ret = new ccReg::Response;
    errors = new ccReg::Errors;

    ret->code = 0;
    errors->length(0);

    ParsedAction paction;
    paction.add(1, (const char *)handle);

    LOG(NOTICE_LOG, "KeySetDelete: clientID -> %d clTRID [%s] handle [%s]",
            (int)clientID, clTRID, handle);

    if ((regID = GetRegistrarID(clientID))) {
        if (DBsql.OpenDatabase(database)) {
            if (DBsql.BeginAction(clientID, EPP_KeySetDelete, clTRID, XML, &paction)) {
                if (DBsql.BeginTransaction()) {
                    id = getIdOfKeySet(&DBsql, handle, conf, true);
                    if (id < 0)
                        ret->code = SetReasonKeySetHandle(
                                errors,
                                handle,
                                GetRegistrarLang(clientID));
                    else if (id == 0) {
                        LOG(WARNING_LOG, "KeySet handle [%s] NOT_EXISTS", handle);
                        ret->code = COMMAND_OBJECT_NOT_EXIST;
                    }
                    if (!ret->code && !DBsql.TestObjectClientID(id, regID)) {
                        LOG(WARNING_LOG, "bad authorisation not client of KeySet [%s]", handle);
                        ret->code = SetReasonWrongRegistrar(errors, regID, GetRegistrarLang(clientID));
                    }
                    try {
                        if (!ret->code && (
                                    testObjectHasState(&DBsql, id, FLAG_serverDeleteProhibited) ||
                                    testObjectHasState(&DBsql, id, FLAG_serverUpdateProhibited)
                                    )) {
                            LOG(WARNING_LOG, "delete of object %s is prohibited", handle);
                            ret->code = COMMAND_STATUS_PROHIBITS_OPERATION;
                        }
                    } catch (...) {
                        ret->code = COMMAND_FAILED;
                    }
                    if (!ret->code) {
                        //create notifier
                        ntf.reset(new EPPNotifier(
                                    conf.get<bool>("registry.disable_epp_notifier"),
                                    mm,
                                    &DBsql,
                                    regID,
                                    id));
                        if (DBsql.TestKeySetRelations(id)) {
                            LOG(WARNING_LOG, "KeySet can't be deleted - relations in db");
                            ret->code = COMMAND_PROHIBITS_OPERATION;
                        } else {
                            if (DBsql.SaveObjectDelete(id))
                                if (DBsql.DeleteKeySetObject(id))
                                    ret->code = COMMAND_OK;
                        }
                        if (ret->code == COMMAND_OK)
                            ntf->Send();
                    }
                    DBsql.QuitTransaction(ret->code);
                }
                ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code));
            }
            ret->msg = CORBA::string_dup(
                    GetErrorMessage(
                        ret->code,
                        GetRegistrarLang(clientID))
                    );
            DBsql.Disconnect();
        }
    }
    if (ret->code > COMMAND_EXCEPTION)
        EppError(ret->code, ret->msg, ret->svTRID, errors);
    if (ret->code == 0)
        ServerInternalError("KeySetDelete");
    return ret._retn();
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
        CORBA::Long clientID,
        const char *clTRID,
        const char *XML)
{
    Logging::Context ctx("rifd");
    Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

    DB                          DBsql;
    std::auto_ptr<EPPNotifier>  ntf;
    ccReg::Response_var         ret;
    ccReg::Errors_var           errors;
    int                         regID, id, techid, dsrecID;
    unsigned int                i, j;
    int                         *tch = NULL;

    LOG(NOTICE_LOG, "KeySetCreate: clientID -> %d clTRID [%s] handle [%s] authInfoPw [%s]",
            (int)clientID, clTRID, handle, authInfoPw);

    ret = new ccReg::Response;
    errors = new ccReg::Errors;

    ret->code = 0;
    errors->length(0);

    ParsedAction paction;
    paction.add(1, (const char *)handle);

    crDate = CORBA::string_dup("");

    if ((regID = GetRegistrarID(clientID))) {
        if (DBsql.OpenDatabase(database)) {
            std::auto_ptr<Register::KeySet::Manager> keyMan(
                    Register::KeySet::Manager::create(
                        &DBsql,
                        conf.get<bool>("registry.restricted_handles"))
                    );
            if ((DBsql.BeginAction(
                            clientID,
                            EPP_KeySetCreate,
                            clTRID,
                            XML,
                            &paction))) {
                if (DBsql.BeginTransaction()) {
                    if (tech.length() < 1) {
                        LOG(WARNING_LOG, "KeySetCreate: not any tech contact ");
                        ret->code = SetErrorReason(
                                errors,
                                COMMAND_PARAMETR_MISSING,
                                ccReg::keyset_tech,
                                0,
                                REASON_MSG_TECH_NOTEXIST,
                                GetRegistrarLang(clientID));
                    } else if (tech.length() > 10) {
                        LOG(WARNING_LOG, "KeySetCreate: too many tech contacts (maximum is 10)");
                        ret->code = SetErrorReason(
                                errors,
                                COMMAND_PARAMETR_RANGE_ERROR,
                                ccReg::keyset_tech,
                                0,
                                REASON_MSG_TECHADMIN_LIMIT,
                                GetRegistrarLang(clientID));
                    } else if (dsrec.length() < 1 && dnsk.length() < 1) {
                        if (dsrec.length() < 1) {
                            LOG(WARNING_LOG, "KeySetCreate: not any DS record");
                            ret->code = SetErrorReason(
                                    errors,
                                    COMMAND_PARAMETR_MISSING,
                                    ccReg::keyset_dsrecord,
                                    0,
                                    REASON_MSG_NO_DSRECORD,
                                    GetRegistrarLang(clientID));
                        }
                        if (dnsk.length() < 1) {
                            LOG(WARNING_LOG, "KeySetCreate: not any DNSKey record");
                            ret->code = SetErrorReason(
                                    errors,
                                    COMMAND_PARAMETR_MISSING,
                                    ccReg::keyset_dnskey,
                                    0,
                                    REASON_MSG_NO_DNSKEY,
                                    GetRegistrarLang(clientID));
                        }
                    } else if (dsrec.length() > 10) {
                        LOG(WARNING_LOG, "KeySetCreate: too many ds-records (maximum is 10)");
                        ret->code = SetErrorReason(
                                errors,
                                COMMAND_PARAMETR_RANGE_ERROR,
                                ccReg::keyset_dsrecord,
                                0,
                                REASON_MSG_DSRECORD_LIMIT,
                                GetRegistrarLang(clientID));
                    } else if (dnsk.length() > 10) {
                        LOG(WARNING_LOG, "KeySetCreate: too many dnskeys (maximum is 10)");
                        ret->code = SetErrorReason(
                                errors,
                                COMMAND_PARAMETR_RANGE_ERROR,
                                ccReg::keyset_dnskey,
                                0,
                                REASON_MSG_DNSKEY_LIMIT,
                                GetRegistrarLang(clientID));
                    }
                    if (ret->code == 0) {
                        Register::KeySet::Manager::CheckAvailType caType;

                        tch = new int[tech.length()];

                        try {
                            Register::NameIdPair nameId;
                            caType = keyMan->checkAvail(handle, nameId);
                            id = nameId.id;
                        } catch (...) {
                            caType = Register::KeySet::Manager::CA_INVALID_HANDLE;
                            id = -1;
                        }

                        if (id < 0 || caType == Register::KeySet::Manager::CA_INVALID_HANDLE)
                            ret->code = SetReasonKeySetHandle(
                                    errors, handle, GetRegistrarLang(clientID));
                        else if (caType == Register::KeySet::Manager::CA_REGISTRED) {
                            LOG(WARNING_LOG, "KeySet handle [%s] EXISTS", handle);
                            ret->code = COMMAND_OBJECT_EXIST;
                        } else if (caType == Register::KeySet::Manager::CA_PROTECTED)
                            ret->code = SetReasonProtectedPeriod(
                                    errors,
                                    handle,
                                    GetRegistrarLang(clientID),
                                    ccReg::keyset_handle);
                    }

                    if (ret->code == 0) {
                        // test technical contact
                        std::auto_ptr<Register::Contact::Manager> cman(
                                Register::Contact::Manager::create(
                                    &DBsql,
                                    conf.get<bool>("registry.restricted_handles"))
                                );
                        for (i = 0; i < tech.length(); i++) {
                            Register::Contact::Manager::CheckAvailType caType;
                            try {
                                Register::NameIdPair nameId;
                                caType = cman->checkAvail((const char *)tech[i], nameId);
                                techid = nameId.id;
                            } catch (...) {
                                caType = Register::Contact::Manager::CA_INVALID_HANDLE;
                                techid = 0;
                            }

                            if (caType != Register::Contact::Manager::CA_REGISTRED) {
                                LOG(DEBUG_LOG, "Tech contact doesn't exist: %s",
                                        CORBA::string_dup(tech[i]));
                                ret->code = SetReasonKeySetTech(
                                        errors,
                                        tech[i],
                                        techid,
                                        GetRegistrarLang(clientID),
                                        i);
                            }
                            else {
                                tch[i] = techid;
                                //duplicity test
                                for (j = 0; j < i; j++) {
                                    LOG(DEBUG_LOG, "tech compare j %d techid %d and %d",
                                            j, techid, tch[j]);
                                    if (tch[j] == techid && tch[j] > 0) {
                                        tch[j] = 0;
                                        ret->code = SetReasonContactDuplicity(
                                                errors,
                                                tech[i],
                                                GetRegistrarLang(clientID),
                                                i,
                                                ccReg::keyset_tech);
                                    }
                                }
                            }
                        }
                    }

                    // test if dsrecord already exists in database
                    /*
                    if (ret->code == 0) {
                        for (int ii = 0; ii < (int)dsrec.length(); ii++) {
                            int id = DBsql.GetDSRecordId(
                                    dsrec[ii].keyTag,
                                    dsrec[ii].alg,
                                    dsrec[ii].digestType,
                                    dsrec[ii].digest,
                                    dsrec[ii].maxSigLife);
                            if (id > 0) {
                                LOG(WARNING_LOG,
                                        "DSRecord already exist, cannot create it: id(%d) (%d %d %d '%s' %d)",
                                        id, dsrec[ii].keyTag, dsrec[ii].alg, dsrec[ii].digestType,
                                        CORBA::string_dup(dsrec[ii].digest), dsrec[ii].maxSigLife);
                                ret->code = SetErrorReason(
                                        errors,
                                        COMMAND_PARAMETR_ERROR,
                                        ccReg::keyset_dsrecord,
                                        ii,
                                        REASON_MSG_DUPLICITY_DSRECORD,
                                        GetRegistrarLang(clientID));
                                break;
                            }
                        }
                    }
                    */

                    // dsrecord digest type test
                    if (ret->code == 0) {
                        // digest type must be 1 (sha-1) - see RFC 4034 for details:
                        // http://rfc-ref.org/RFC-TEXTS/4034/kw-dnssec_digest_type.html
                        for (int ii = 0; ii < (int)dsrec.length(); ii++) {
                            if (dsrec[ii].digestType != 1) {
                                LOG(WARNING_LOG,
                                        "Digest is %d (must be 1)",
                                        dsrec[ii].digestType);
                                ret->code = SetErrorReason(
                                        errors,
                                        COMMAND_PARAMETR_ERROR,
                                        ccReg::keyset_dsrecord,
                                        ii,
                                        REASON_MSG_DSRECORD_BAD_DIGEST_TYPE,
                                        GetRegistrarLang(clientID)
                                        );
                                break;
                            }
                        }
                    }
                    // dsrecord digest length test
                    if (ret->code == 0) {
                        // digest must be 40 characters length (because SHA-1 is used)
                        for (int ii = 0; ii < (int)dsrec.length(); ii++) {
                            if (strlen(dsrec[ii].digest) != 40) {
                                LOG(WARNING_LOG,
                                        "Digest length is %d char (must be 40)",
                                        strlen(dsrec[ii].digest));
                                ret->code = SetErrorReason(
                                        errors,
                                        COMMAND_PARAMETR_ERROR,
                                        ccReg::keyset_dsrecord,
                                        ii,
                                        REASON_MSG_DSRECORD_BAD_DIGEST_LENGTH,
                                        GetRegistrarLang(clientID)
                                        );
                                break;
                            }
                        }
                    }
                    // dsrecord duplicity test
                    if (ret->code == 0) {
                        if (dsrec.length() >= 2) {
                            for (int ii = 0; ii < (int)dsrec.length(); ii++) {
                                for (int jj = ii + 1; jj < (int)dsrec.length(); jj++) {
                                    if (testDSRecordDuplicity(dsrec[ii], dsrec[jj])) {
                                        LOG(WARNING_LOG,
                                                "Found DSRecord duplicity: %d x %d (%d %d %d '%s' %s)",
                                                ii, jj, dsrec[ii].keyTag, dsrec[ii].alg, dsrec[ii].digestType,
                                                CORBA::string_dup(dsrec[ii].digest), dsrec[ii].maxSigLife);
                                        ret->code = SetErrorReason(
                                                errors,
                                                COMMAND_PARAMETR_ERROR,
                                                ccReg::keyset_dsrecord,
                                                jj,
                                                REASON_MSG_DUPLICITY_DSRECORD,
                                                GetRegistrarLang(clientID));
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // dnskey flag field (must be 0, 256 or 267)
                    // http://rfc-ref.org/RFC-TEXTS/4034/kw-flags_field.html
                    if (ret->code == 0) {
                        for (int ii = 0; ii < (int)dnsk.length(); ii++) {
                            if (!(dnsk[ii].flags == 0 || dnsk[ii].flags == 256 || dnsk[ii].flags == 257)) {
                                LOG(WARNING_LOG,
                                        "dnskey flag is %d (must be 0, 256 or 257)",
                                        dnsk[ii].flags);
                                ret->code = SetErrorReason(
                                        errors,
                                        COMMAND_PARAMETR_ERROR,
                                        ccReg::keyset_dnskey,
                                        ii,
                                        REASON_MSG_DNSKEY_BAD_FLAGS,
                                        GetRegistrarLang(clientID));
                                break;
                            }
                        }
                    }
                    // dnskey protocol field (must be 3)
                    // http://rfc-ref.org/RFC-TEXTS/4034/kw-protocol_field.html
                    if (ret->code == 0) {
                        for (int ii = 0; ii < (int)dnsk.length(); ii++) {
                            if (dnsk[ii].protocol != 3) {
                                LOG(WARNING_LOG,
                                        "dnskey protocol is %d (must be 3)",
                                        dnsk[ii].protocol);
                                ret->code = SetErrorReason(
                                        errors,
                                        COMMAND_PARAMETR_ERROR,
                                        ccReg::keyset_dnskey,
                                        ii,
                                        REASON_MSG_DNSKEY_BAD_PROTOCOL,
                                        GetRegistrarLang(clientID));
                                break;
                            }
                        }
                    }
                    // dnskey algorithm type (must be 1, 2, 3, 4, 5, 252, 253, 254 or 255)
                    // http://rfc-ref.org/RFC-TEXTS/4034/kw-dnssec_algorithm_type.html
                    // http://rfc-ref.org/RFC-TEXTS/4034/chapter7.html#d4e446172
                    if (ret->code == 0) {
                        for (int ii = 0; ii < (int)dnsk.length(); ii++) {
                            if (!((dnsk[ii].alg >= 1 && dnsk[ii].alg <= 5) ||
                                        (dnsk[ii].alg >= 252 && dnsk[ii].alg <=255))) {
                                LOG(WARNING_LOG,
                                        "dnskey algorithm is %d (must be 1,2,3,4,5,252,253,254 or 255)",
                                        dnsk[ii].alg);
                                ret->code = SetErrorReason(
                                        errors,
                                        COMMAND_PARAMETR_ERROR,
                                        ccReg::keyset_dnskey,
                                        ii,
                                        REASON_MSG_DNSKEY_BAD_ALG,
                                        GetRegistrarLang(clientID));
                                break;
                            }
                        }
                    }
                    // test if key is valid base64 encoded string
                    if (ret->code == 0) {
                        int ret1, ret2;
                        for (int ii = 0; ii < (int)dnsk.length(); ii++) {
                            if ((ret1 = isValidBase64((const char *)dnsk[ii].key, &ret2)) != BASE64_OK) {
                                if (ret1 == BASE64_BAD_LENGTH) {
                                    LOG(WARNING_LOG, "dnskey key length is wrong (must be dividable by 4)");
                                    ret->code = SetErrorReason(
                                            errors,
                                            COMMAND_PARAMETR_ERROR,
                                            ccReg::keyset_dnskey,
                                            ii,
                                            REASON_MSG_DNSKEY_BAD_KEY_LEN,
                                            GetRegistrarLang(clientID));
                                } else if (ret1 == BASE64_BAD_CHAR) {
                                    LOG(WARNING_LOG, "dnskey key contain invalid character '%c' at position %d",
                                            ((const char *)dnsk[ii].key)[ret2], ret2);
                                    ret->code = SetErrorReason(
                                            errors,
                                            COMMAND_PARAMETR_ERROR,
                                            ccReg::keyset_dnskey,
                                            ii,
                                            REASON_MSG_DNSKEY_BAD_KEY_CHAR,
                                            GetRegistrarLang(clientID));
                                } else {
                                    LOG(WARNING_LOG, "isValidBase64() return unknown value (%d)",
                                            ret1);
                                    ret->code = COMMAND_FAILED;
                                }
                            }
                        }
                    }
                    // dnskey duplicity test
                    if (ret->code == 0) {
                        if (dnsk.length() >= 2) {
                            for (int ii = 0; ii < (int)dnsk.length(); ii++) {
                                for (int jj = ii + 1; jj < (int)dnsk.length(); jj++) {
                                    if (testDNSKeyDuplicity(dnsk[ii], dnsk[jj])) {
                                        LOG(WARNING_LOG,
                                                "Found DSNKey duplicity: %d x %d (%d %d %d %s)",
                                                ii, jj, dnsk[ii].flags, dnsk[ii].protocol,
                                                dnsk[ii].alg, CORBA::string_dup(dnsk[ii].key));
                                        ret->code = SetErrorReason(
                                                errors,
                                                COMMAND_PARAMETR_ERROR,
                                                ccReg::keyset_dnskey,
                                                jj,
                                                REASON_MSG_DUPLICITY_DNSKEY,
                                                GetRegistrarLang(clientID));
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // keyset creating
                    if (ret->code == 0) {
                        id = DBsql.CreateObject("K", regID, handle, authInfoPw);
                        if (id <= 0) {
                            if (id == 0) {
                                LOG(WARNING_LOG, "KeySet handle [%s] EXISTS", handle);
                                ret->code = COMMAND_OBJECT_EXIST;
                            } else {
                                LOG(WARNING_LOG, "Cannot insert [%s] into object_registry", handle);
                                ret->code = COMMAND_FAILED;
                            }
                        } else {
                            DBsql.INSERT("keyset");
                            DBsql.INTO("id");
                            DBsql.VALUE(id);

                            if (!DBsql.EXEC())
                                ret->code = COMMAND_FAILED;
                            else {
                                CORBA::string_free(crDate);
                                crDate = CORBA::string_dup(DBsql.GetObjectCrDateTime(id));

                                //insert all technical contact
                                for (i = 0; i < tech.length(); i++) {
                                    LOG(DEBUG_LOG, "KeySetCreate: add tech contact %s id %d",
                                            (const char *)tech[i], tch[i]);
                                    if (!DBsql.AddContactMap("keyset", id, tch[i])) {
                                        ret->code = COMMAND_FAILED;
                                        break;
                                    }
                                }
                                // insert dnskey(s)
                                for (int ii = 0; ii < (int)dnsk.length(); ii++) {
                                    char *key;
                                    if ((key = removeWhitespaces((const char *)dnsk[ii].key)) == NULL) {
                                        LOG(WARNING_LOG, "removeWhitespaces fails (memory problem)");
                                        ret->code = COMMAND_FAILED;
                                        break;
                                    }
                                    LOG(NOTICE_LOG, "KeySetCreate: dnskey");

                                    int dnskeyId = DBsql.GetSequenceID("dnskey");

                                    DBsql.INSERT("dnskey");
                                    DBsql.INTO("id");
                                    DBsql.INTO("keysetid");
                                    DBsql.INTO("flags");
                                    DBsql.INTO("protocol");
                                    DBsql.INTO("alg");
                                    DBsql.INTO("key");

                                    DBsql.VALUE(dnskeyId);
                                    DBsql.VALUE(id);
                                    DBsql.VALUE(dnsk[ii].flags);
                                    DBsql.VALUE(dnsk[ii].protocol);
                                    DBsql.VALUE(dnsk[ii].alg);
                                    DBsql.VALUE(key);
                                    if (!DBsql.EXEC()) {
                                        ret->code = COMMAND_FAILED;
                                        free(key);
                                        break;
                                    }
                                    free(key);
                                }

                                // insert DSRecord(s)
                                for (i = 0; i < dsrec.length(); i++) {
                                    LOG(NOTICE_LOG, "KeySetCreate: DSRecord");

                                    dsrecID = DBsql.GetSequenceID("dsrecord");

                                    //DSRecord information
                                    DBsql.INSERT("DSRECORD");
                                    DBsql.INTO("ID");
                                    DBsql.INTO("KEYSETID");
                                    DBsql.INTO("KEYTAG");
                                    DBsql.INTO("ALG");
                                    DBsql.INTO("DIGESTTYPE");
                                    DBsql.INTO("DIGEST");
                                    DBsql.INTO("MAXSIGLIFE");

                                    DBsql.VALUE(dsrecID);
                                    DBsql.VALUE(id);
                                    DBsql.VALUE(dsrec[i].keyTag);
                                    DBsql.VALUE(dsrec[i].alg);
                                    DBsql.VALUE(dsrec[i].digestType);
                                    DBsql.VVALUE(dsrec[i].digest);
                                    if (dsrec[i].maxSigLife == -1)
                                        DBsql.VALUENULL();
                                    else
                                        DBsql.VALUE(dsrec[i].maxSigLife);

                                    if (!DBsql.EXEC()) {
                                        ret->code = COMMAND_FAILED;
                                        break;
                                    }
                                }

                                // save it to histrory if it's ok
                                if (ret->code != COMMAND_FAILED)
                                    if (DBsql.SaveKeySetHistory(id))
                                        if (DBsql.SaveObjectCreate(id))
                                            ret->code = COMMAND_OK;
                            }

                            if (ret->code == COMMAND_OK) {
                                // run notifier and send notify (suprisingly) message
                                ntf.reset(new EPPNotifier(
                                            conf.get<bool>("registry.disable_epp_notifier"),
                                            mm,
                                            &DBsql,
                                            regID,
                                            id)
                                        );
                                ntf->Send();
                            }
                        }
                    }

                    delete []tch;
                    DBsql.QuitTransaction(ret->code);
                }
                ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code));
            }
            ret->msg = CORBA::string_dup(GetErrorMessage(
                        ret->code,
                        GetRegistrarLang(clientID))
                    );

            DBsql.Disconnect();
        }
    }

    if (ret->code > COMMAND_EXCEPTION)
        EppError(ret->code, ret->msg, ret->svTRID, errors);

    if (ret->code == 0)
        ServerInternalError("KeySetCreate");

    return ret._retn();
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
        CORBA::Long clientId,
        const char *clTRID,
        const char *XML)
{
    Logging::Context ctx("rifd");

    std::auto_ptr<EPPNotifier> ntf;
    DB DbSql;
    int regId, keysetId, techId;

    ccReg::Response_var ret = new ccReg::Response;
    ccReg::Errors_var errors = new ccReg::Errors;

    ret->code = 0;
    errors->length(0);

    int *techAdd = NULL;
    int *techRem = NULL;
    techAdd = new int[tech_add.length()];
    techRem = new int[tech_rem.length()];

    for (int i = 0; i < (int)tech_add.length(); i++) {
        techAdd[i] = 0;
    }
    for (int i = 0; i < (int)tech_rem.length(); i++) {
        techRem[i] = 0;
    }

    ParsedAction paction;
    paction.add(1, (const char *)handle);

    LOG(NOTICE_LOG, "KeySetUpdate: clientId -> %d clTRID[%s] handle[%s] "
            "authInfo_chg[%s] tech_add[%d] tech_rem[%d] dsrec_add[%d] "
            "dsrec_rem[%d] dnskey_add[%d] dnskey_rem[%d]",
            (int)clientId, clTRID, handle, authInfo_chg, tech_add.length(),
            tech_rem.length(), dsrec_add.length(), dsrec_rem.length(),
            dnsk_add.length(), dnsk_rem.length());

    if ((regId = GetRegistrarID(clientId))) {
        if (DbSql.OpenDatabase(database)) {
            std::auto_ptr<Register::KeySet::Manager> kMan(
                    Register::KeySet::Manager::create(
                        &DbSql, conf.get<bool>("registry.restricted_handles"))
                    );
            if ((DbSql.BeginAction(clientId, EPP_KeySetUpdate, clTRID, XML, &paction))) {
                if (DbSql.BeginTransaction()) {
                    if ((keysetId = getIdOfKeySet(&DbSql, handle, conf, true)) < 0)
                        ret->code = SetReasonKeySetHandle(
                                errors, handle, GetRegistrarLang(clientId));
                    else if (keysetId == 0) {
                        LOG(WARNING_LOG, "KeySet handle [%s] NOT_EXISTS", handle);
                        ret->code = COMMAND_OBJECT_NOT_EXIST;
                    }
                    // registrar of the object
                    if (!ret->code && !DbSql.TestObjectClientID(keysetId, regId)) {
                        LOG(WARNING_LOG, "bad authorization not client of KeySet [%s]", handle);
                        ret->code = SetReasonWrongRegistrar(errors, regId, GetRegistrarLang(clientId));
                    }
                    try {
                        if (!ret->code && testObjectHasState(&DbSql, keysetId, FLAG_serverUpdateProhibited)) {
                            LOG(WARNING_LOG, "update of object %s is prohibited", handle);
                            ret->code = COMMAND_STATUS_PROHIBITS_OPERATION;
                        }
                    } catch (...) {
                        ret->code = COMMAND_FAILED;
                    }

                    ///////////////////////////////////////////////////////////
                    //
                    //  ** KeySetUpdate() - TESTS **
                    //
                    ///////////////////////////////////////////////////////////

                    // duplicity test of dsrec_add in given parameters
                    if (!ret->code) {
                        for (int ii = 0; ii < (int)dsrec_add.length(); ii++) {
                            for (int jj = ii + 1; jj < (int)dsrec_add.length(); jj++) {
                                if (testDSRecordDuplicity(dsrec_add[ii], dsrec_add[jj])) {
                                    LOG(WARNING_LOG, "Found DSRecord add duplicity: 1:(%d %d %d '%s' %d) 2:(%d %d %d '%s' %d)",
                                            dsrec_add[ii].keyTag, dsrec_add[ii].alg, dsrec_add[ii].digestType,
                                            (const char *)dsrec_add[ii].digest, dsrec_add[ii].maxSigLife,
                                            dsrec_add[jj].keyTag, dsrec_add[jj].alg, dsrec_add[jj].digestType,
                                            (const char *)dsrec_add[jj].digest, dsrec_add[jj].maxSigLife);
                                    ret->code = SetErrorReason(
                                            errors,
                                            COMMAND_PARAMETR_ERROR,
                                            ccReg::keyset_dsrecord_add,
                                            jj,
                                            REASON_MSG_DUPLICITY_DSRECORD,
                                            GetRegistrarLang(clientId));
                                    break;
                                }
                            }
                            LOG(NOTICE_LOG, "ADD keyset DSRecord (%d %d %d '%s' %d)",
                                    dsrec_add[ii].keyTag, dsrec_add[ii].alg, dsrec_add[ii].digestType,
                                    (const char *)dsrec_add[ii].digest, dsrec_add[ii].maxSigLife);
                        }
                    }

                    // duplicity test of dsrec_rem in given parameters
                    if (!ret->code) {
                        for (int ii = 0; ii < (int)dsrec_rem.length(); ii++) {
                            for (int jj = ii + 1; jj < (int)dsrec_rem.length(); jj++) {
                                if (testDSRecordDuplicity(dsrec_rem[ii], dsrec_rem[jj])) {
                                    LOG(WARNING_LOG, "Found DSRecord rem duplicity: 1:(%d %d %d '%s' %d) 2:(%d %d %d '%s' %d)",
                                            dsrec_rem[ii].keyTag, dsrec_rem[ii].alg, dsrec_rem[ii].digestType,
                                            (const char *)dsrec_rem[ii].digest, dsrec_rem[ii].maxSigLife,
                                            dsrec_rem[jj].keyTag, dsrec_rem[jj].alg, dsrec_rem[jj].digestType,
                                            (const char *)dsrec_rem[jj].digest, dsrec_rem[jj].maxSigLife);
                                    ret->code = SetErrorReason(
                                            errors,
                                            COMMAND_PARAMETR_ERROR,
                                            ccReg::keyset_dsrecord_rem,
                                            jj,
                                            REASON_MSG_DUPLICITY_DSRECORD,
                                            GetRegistrarLang(clientId));
                                    break;
                                }
                            }
                            LOG(NOTICE_LOG, "REM keyset DSRecord (%d %d %d '%s' %d)",
                                    dsrec_rem[ii].keyTag, dsrec_rem[ii].alg, dsrec_rem[ii].digestType,
                                    (const char *)dsrec_rem[ii].digest, dsrec_rem[ii].maxSigLife);
                        }
                    }

                    // duplicity test for dnsk_add in given parameters
                    if (!ret->code) {
                        for (int ii = 0; ii < (int)dnsk_add.length(); ii++) {
                            for (int jj = ii + 1; jj < (int)dnsk_add.length(); jj++) {
                                if (testDNSKeyDuplicity(dnsk_add[ii], dnsk_add[jj])) {
                                    LOG(WARNING_LOG, "Found DNSKey add duplicity: %d x %d (%d %d %d %s)",
                                            ii, jj, dnsk_add[ii].flags, dnsk_add[ii].protocol,
                                            dnsk_add[ii].alg, (const char *)dnsk_add[ii].key);
                                    ret->code = SetErrorReason(
                                            errors,
                                            COMMAND_PARAMETR_ERROR,
                                            ccReg::keyset_dnskey_add,
                                            jj,
                                            REASON_MSG_DUPLICITY_DNSKEY,
                                            GetRegistrarLang(clientId));
                                    break;
                                }
                            }
                            LOG(NOTICE_LOG, "ADD keyset DNSKey (%d %d %d %s)",
                                    dnsk_add[ii].flags, dnsk_add[ii].protocol,
                                    dnsk_add[ii].alg, (const char *)dnsk_add[ii].key);
                        }
                    }

                    // duplicity test for dnsk_rem in given parameters
                    if (!ret->code) {
                        for (int ii = 0; ii < (int)dnsk_rem.length(); ii++) {
                            for (int jj = ii + 1; jj < (int)dnsk_rem.length(); jj++) {
                                if (testDNSKeyDuplicity(dnsk_rem[ii], dnsk_rem[jj])) {
                                    LOG(WARNING_LOG, "Found DNSKey rem duplicity: %d x %d (%d %d %d %s)",
                                            ii, jj, dnsk_rem[ii].flags, dnsk_rem[ii].protocol,
                                            dnsk_rem[ii].alg, (const char *)dnsk_rem[ii].key);
                                    ret->code = SetErrorReason(
                                            errors,
                                            COMMAND_PARAMETR_ERROR,
                                            ccReg::keyset_dnskey_rem,
                                            jj,
                                            REASON_MSG_DUPLICITY_DNSKEY,
                                            GetRegistrarLang(clientId));
                                    break;
                                }
                            }
                            LOG(NOTICE_LOG, "REM keyset DNSKey (%d %d %d %s)",
                                    dnsk_rem[ii].flags, dnsk_rem[ii].protocol,
                                    dnsk_rem[ii].alg, (const char *)dnsk_rem[ii].key);
                        }
                    }

                    // test dsrecord to add (if same exist for this keyset)
                    if (!ret->code) {
                        for (int i = 0; i < (int)dsrec_add.length(); i++) {
                            int id;
                            id = DbSql.GetDSRecordId(
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
                                ret->code = SetErrorReason(
                                        errors,
                                        COMMAND_PARAMETR_ERROR,
                                        ccReg::keyset_dsrecord_add,
                                        i,
                                        REASON_MSG_DSRECORD_EXIST,
                                        GetRegistrarLang(clientId));
                                break;
                            }
                        }
                    }
                    
                    // test dsrecord to rem (if this dsrecord exist for this keyset)
                    if (!ret->code) {
                        for (int i = 0; i < (int)dsrec_rem.length(); i++) {
                            int id;
                            id = DbSql.GetDSRecordId(
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
                                ret->code = SetErrorReason(
                                        errors,
                                        COMMAND_PARAMETR_ERROR,
                                        ccReg::keyset_dsrecord_rem,
                                        i,
                                        REASON_MSG_DSRECORD_NOTEXIST,
                                        GetRegistrarLang(clientId));
                                break;
                            }
                        }
                    }

                    // dnsk_add tests - if exact same exist for this keyset
                    if (!ret->code) {
                        for (int ii = 0; ii < (int)dnsk_add.length(); ii++) {
                            bool pass = true;
                            int id;
                            char *key;
                            // keys are inserted into database without whitespaces
                            if ((key = removeWhitespaces(dnsk_add[ii].key)) == NULL) {
                                ret->code = COMMAND_FAILED;
                                LOG(WARNING_LOG, "removeWhitespaces failed");
                                free(key);
                                pass = false;
                                break;
                            }
                            id = DbSql.GetDNSKeyId(
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
                                ret->code = SetErrorReason(
                                        errors,
                                        COMMAND_PARAMETR_ERROR,
                                        ccReg::keyset_dnskey_add,
                                        ii,
                                        REASON_MSG_DNSKEY_EXIST,
                                        GetRegistrarLang(clientId));
                                pass = false;
                                break;
                            }
                        }
                    }

                    // test dnskey to rem (if this dnskey exist for this keyset)
                    if (!ret->code) {
                        for (int ii = 0; ii < (int)dnsk_rem.length(); ii++) {
                            int id;
                            char *key;
                            // keys are inserted into database without whitespaces
                            if ((key = removeWhitespaces(dnsk_rem[ii].key)) == NULL) {
                                ret->code = COMMAND_FAILED;
                                LOG(WARNING_LOG, "removeWhitespaces failed");
                                free(key);
                                break;
                            }
                            id = DbSql.GetDNSKeyId(
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
                                ret->code = SetErrorReason(
                                        errors,
                                        COMMAND_PARAMETR_ERROR,
                                        ccReg::keyset_dnskey_rem,
                                        ii,
                                        REASON_MSG_DNSKEY_NOTEXIST,
                                        GetRegistrarLang(clientId));
                                free(key);
                                break;
                            }
                            free(key);
                        }
                    }

                    // test maximum values - technical contacts
                    if (!ret->code) {
                        if ((DbSql.GetKeySetContacts(keysetId) + tech_add.length() - tech_rem.length()) > 10) {
                            LOG(WARNING_LOG, "KeySetUpdate: too may tech contacts (maximum is 10)"
                                    "(existing:%d, to add:%d, to rem:%d)",
                                    DbSql.GetKeySetContacts(keysetId),
                                    tech_add.length(),
                                    tech_rem.length());
                            ret->code = SetErrorReason(
                                    errors,
                                    COMMAND_PARAMETR_RANGE_ERROR,
                                    ccReg::keyset_tech,
                                    0,
                                    REASON_MSG_TECHADMIN_LIMIT,
                                    GetRegistrarLang(clientId));
                        }
                    }

                    // test maximum values - dsrecords
                    if (!ret->code) {
                        if ((DbSql.GetKeySetDSRecords(keysetId) + dsrec_add.length() - dsrec_rem.length()) > 10) {
                            LOG(WARNING_LOG, "KeySetUpdate: too many ds-records (maximum is 10)"
                                    "(existing:%d, to add:%d, to rem:%d)",
                                    DbSql.GetKeySetDSRecords(keysetId),
                                    dsrec_add.length(),
                                    dsrec_rem.length());
                            ret->code = SetErrorReason(
                                    errors,
                                    COMMAND_PARAMETR_RANGE_ERROR,
                                    ccReg::keyset_dsrecord,
                                    0,
                                    REASON_MSG_DSRECORD_LIMIT,
                                    GetRegistrarLang(clientId));
                        }
                    }

                    // test maximum values - dnskeys
                    if (!ret->code) {
                        if ((DbSql.GetKeySetDNSKeys(keysetId) + dnsk_add.length() - dnsk_rem.length()) > 10) {
                            LOG(WARNING_LOG, "KeySetUpdate: too many dnskeys (maximum is 10)"
                                    "(existing:%d, to add:%d, to rem:%d)",
                                    DbSql.GetKeySetDNSKeys(keysetId),
                                    dnsk_add.length(),
                                    dnsk_rem.length());
                            ret->code = SetErrorReason(
                                    errors,
                                    COMMAND_PARAMETR_RANGE_ERROR,
                                    ccReg::keyset_dnskey,
                                    0,
                                    REASON_MSG_DNSKEY_LIMIT,
                                    GetRegistrarLang(clientId));
                        }
                    }

                    // test minimum value - there must be at least one from the pair dnskey-dsrecord
                    if (!ret->code) {
                        if ((DbSql.GetKeySetDSRecords(keysetId) + DbSql.GetKeySetDNSKeys(keysetId) +
                                    dsrec_add.length() + dnsk_add.length() - dsrec_rem.length() -
                                    dnsk_rem.length()) <= 0) {
                            LOG(WARNING_LOG, "KeySetUpdate: there must remain one DNSKey or one DSRecord");
                            ret->code = SetErrorReason(
                                    errors,
                                    COMMAND_FAILED,
                                    ccReg::keyset_dnskey,
                                    0,
                                    REASON_MSG_NO_DNSKEY_DSRECORD,
                                    GetRegistrarLang(clientId));
                            ret->code = COMMAND_FAILED;
                        }
                    }

                    if (!ret->code) {
                        // test ADD tech-c
                        for (int i = 0; i < (int)tech_add.length(); i++) {
                            if ((techId = getIdOfContact(&DbSql, tech_add[i], conf)) <= 0)
                                ret->code = SetReasonKeySetTechADD(
                                        errors,
                                        tech_add[i],
                                        techId,
                                        GetRegistrarLang(clientId),
                                        i);
                            else if (DbSql.CheckContactMap("keyset", keysetId, techId, 0))
                                ret->code = SetReasonKeySetTechExistMap(
                                        errors,
                                        tech_add[i],
                                        GetRegistrarLang(clientId),
                                        i);
                            else {
                                techAdd[i] = techId;
                                for (int j = 0; j < i; j++) {
                                    if (techAdd[j] == techId && techAdd[j] > 0) {
                                        techAdd[j] = 0;
                                        ret->code = SetReasonContactDuplicity(
                                                errors,
                                                tech_add[i],
                                                GetRegistrarLang(clientId),
                                                i,
                                                ccReg::keyset_tech_add);
                                    }
                                }
                            }
                            LOG(NOTICE_LOG, "ADD tech  techid -> %d [%s]",
                                    techId, (const char *)tech_add[i]);
                        }

                        // test REM tech-c
                        if (!ret->code) {
                            for (int i = 0; i < (int)tech_rem.length(); i++) {
                                if ((techId = getIdOfContact(&DbSql, tech_rem[i], conf)) <= 0)
                                    ret->code = SetReasonKeySetTechREM(
                                            errors,
                                            tech_rem[i],
                                            techId,
                                            GetRegistrarLang(clientId),
                                            i);
                                else if (!DbSql.CheckContactMap("keyset", keysetId, techId, 0))
                                    ret->code = SetReasonKeySetTechNotExistMap(
                                            errors,
                                            tech_rem[i],
                                            GetRegistrarLang(clientId),
                                            i);
                                else {
                                    techRem[i] = techId;
                                    for (int j = 0; j < i; j++) {
                                        if (techRem[j] == techId && techRem[j] > 0) {
                                            techRem[j] = 0;
                                            ret->code = SetReasonContactDuplicity(
                                                    errors,
                                                    tech_rem[i],
                                                    GetRegistrarLang(clientId),
                                                    i,
                                                    ccReg::keyset_tech_rem);
                                        }
                                    }
                                }
                                LOG(NOTICE_LOG, "REM tech  techid -> %d [%s]",
                                        techId, (const char *)tech_rem[i]);
                            }
                        }

                        // DSRecord member tests
                        if (!ret->code) {
                            for (int i = 0; i < (int)dsrec_add.length(); i++) {
                                // test digest type
                                if (dsrec_add[i].digestType != 1) {
                                    LOG(WARNING_LOG, "Bad digest type: %d (must be 1)",
                                            dsrec_add[i].digestType);
                                    ret->code = SetErrorReason(
                                            errors,
                                            COMMAND_PARAMETR_ERROR,
                                            ccReg::keyset_dsrecord_add,
                                            i,
                                            REASON_MSG_DSRECORD_BAD_DIGEST_TYPE,
                                            GetRegistrarLang(clientId)
                                            );
                                    break;
                                }
                                // test digest length
                                if (strlen(dsrec_add[i].digest) != 40) {
                                    LOG(WARNING_LOG, "Bad digest length: %d (must be 40)",
                                            strlen(dsrec_add[i].digest));
                                    ret->code = SetErrorReason(
                                            errors,
                                            COMMAND_PARAMETR_ERROR,
                                            ccReg::keyset_dsrecord_add,
                                            i,
                                            REASON_MSG_DSRECORD_BAD_DIGEST_LENGTH,
                                            GetRegistrarLang(clientId)
                                            );
                                    break;
                                }
                            }
                        }

                        // DNSKey member tests
                        if (!ret->code) {
                            for (int ii = 0; ii < (int)dnsk_add.length(); ii++) {
                                // dnskey flag field test (must be 0, 256, 257)
                                if (!(dnsk_add[ii].flags == 0 || dnsk_add[ii].flags == 256 || dnsk_add[ii].flags == 257)) {
                                    LOG(WARNING_LOG,
                                            "dnskey flag is %d (must be 0, 256 or 257)",
                                            dnsk_add[ii].flags);
                                    ret->code = SetErrorReason(
                                            errors,
                                            COMMAND_PARAMETR_ERROR,
                                            ccReg::keyset_dnskey_add,
                                            ii,
                                            REASON_MSG_DNSKEY_BAD_FLAGS,
                                            GetRegistrarLang(clientId));
                                    break;
                                }
                                // dnskey protocol test (must be 3)
                                if (dnsk_add[ii].protocol != 3) {
                                    LOG(WARNING_LOG,
                                            "dnskey protocol is %d (must be 3)",
                                            dnsk_add[ii].protocol);
                                    ret->code = SetErrorReason(
                                            errors,
                                            COMMAND_PARAMETR_ERROR,
                                            ccReg::keyset_dnskey_add,
                                            ii,
                                            REASON_MSG_DNSKEY_BAD_PROTOCOL,
                                            GetRegistrarLang(clientId));
                                    break;
                                }
                                // dnskey algorithm type test (must be 1,2,3,4,5,252,253,254,255)
                                if (!((dnsk_add[ii].alg >= 1 && dnsk_add[ii].alg <= 5) ||
                                            (dnsk_add[ii].alg >= 252 && dnsk_add[ii].alg <= 255))) {
                                    LOG(WARNING_LOG,
                                            "dnskey algorithm is %d (must be 1,2,3,4,5,252,253,254 or 255)",
                                            dnsk_add[ii].alg);
                                    ret->code = SetErrorReason(
                                            errors,
                                            COMMAND_PARAMETR_ERROR,
                                            ccReg::keyset_dnskey_add,
                                            ii,
                                            REASON_MSG_DNSKEY_BAD_ALG,
                                            GetRegistrarLang(clientId));
                                    break;
                                }
                                // dnskey key test - see isValidBase64 function for details
                                int ret1, ret2;
                                if ((ret1 = isValidBase64((const char *)dnsk_add[ii].key, &ret2)) != BASE64_OK) {
                                    if (ret1 == BASE64_BAD_LENGTH) {
                                        LOG(WARNING_LOG, "dnskey key length is wrong (must be dividable by 4)");
                                        ret->code = SetErrorReason(
                                                errors,
                                                COMMAND_PARAMETR_ERROR,
                                                ccReg::keyset_dnskey_add,
                                                ii,
                                                REASON_MSG_DNSKEY_BAD_KEY_LEN,
                                                GetRegistrarLang(clientId));
                                    } if (ret1 == BASE64_BAD_CHAR) {
                                        LOG(WARNING_LOG, "dnskey key contain bad character '%c' at position %d",
                                                ((const char *)dnsk_add[ii].key)[ret2], ret2);
                                        ret->code = SetErrorReason(
                                                errors,
                                                COMMAND_PARAMETR_ERROR,
                                                ccReg::keyset_dnskey_add,
                                                ii,
                                                REASON_MSG_DNSKEY_BAD_KEY_CHAR,
                                                GetRegistrarLang(clientId));
                                    } else {
                                        LOG(WARNING_LOG, "isValidBase64() return unknown value (%d)",
                                                ret1);
                                        ret->code = COMMAND_FAILED;
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
                        if (ret->code == 0) {
                            if (DbSql.ObjectUpdate(keysetId, regId, authInfo_chg)) {
                                ntf.reset(new EPPNotifier(
                                            conf.get<bool>("registry.disable_epp_notifier"), mm, &DbSql, regId, keysetId));

                                for (int i = 0; i < (int)tech_add.length(); i++)
                                    ntf->AddTechNew(techAdd[i]);

                                ntf->Send(); // send message to all technical contacts

                                // adding tech contacts
                                for (int i = 0; i < (int)tech_add.length(); i++) {
                                    LOG(NOTICE_LOG, "INSERT add techid -> %d [%s]",
                                            techAdd[i], (const char *)tech_add[i]);
                                    if (!DbSql.AddContactMap("keyset", keysetId, techAdd[i])) {
                                        ret->code = COMMAND_FAILED;
                                        break;
                                    }
                                }

                                // delete tech contacts
                                for (int i = 0; i < (int)tech_rem.length(); i++) {
                                    LOG(NOTICE_LOG, "DELETE rem techid -> %d [%s]",
                                            techRem[i], (const char *)tech_rem[i]);
                                    if (!DbSql.DeleteFromTableMap("keyset", keysetId, techRem[i])) {
                                        ret->code = COMMAND_FAILED;
                                        break;
                                    }
                                }

                                // test for count of tech contacts after add&rem
                                if (tech_rem.length() > 0) {
                                    int techNum = DbSql.GetKeySetContacts(keysetId);
                                    LOG(NOTICE_LOG, "KeySetUpdate: tech Contacts %d", techNum);
                                    if (techNum == 0) {
                                        // mark all rem tech-c as param error
                                        for (int i = 0; i < (int)tech_rem.length(); i++) {
                                            ret->code = SetErrorReason(
                                                    errors,
                                                    COMMAND_PARAMETR_VALUE_POLICY_ERROR,
                                                    ccReg::keyset_tech_rem,
                                                    i + 1,
                                                    REASON_MSG_CAN_NOT_REMOVE_TECH,
                                                    GetRegistrarLang(clientId));
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
                                    int dsrecId = DbSql.GetDSRecordId(keysetId, dsrec_rem[i].keyTag,
                                            dsrec_rem[i].alg, dsrec_rem[i].digestType,
                                            (const char *)dsrec_rem[i].digest, dsrec_rem[i].maxSigLife);
                                    if (dsrecId == -1) {
                                        ret->code = COMMAND_FAILED;
                                        break;
                                    }
                                    LOG(NOTICE_LOG,
                                            "KeySetUpdate: delete DSRecord id found: %d",
                                            dsrecId);
                                    if (!DbSql.DeleteFromTable("dsrecord", "id", dsrecId))
                                        ret->code = COMMAND_FAILED;
                                }
                                // delete dnskey(s)
                                for (int ii = 0; ii < (int)dnsk_rem.length(); ii++) {
                                    char *key;
                                    // keys are inserted into database without whitespaces
                                    if ((key = removeWhitespaces(dnsk_rem[ii].key)) == NULL) {
                                        ret->code = COMMAND_FAILED;
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
                                    int dnskeyId = DbSql.GetDNSKeyId(keysetId, dnsk_rem[ii].flags,
                                            dnsk_rem[ii].protocol, dnsk_rem[ii].alg,
                                            key);
                                    if (dnskeyId == -1) {
                                        ret->code = COMMAND_FAILED;
                                        free(key);
                                        break;
                                    }
                                    LOG(NOTICE_LOG, "KeySetUpdate: delete DNSKey id found: %d",
                                            dnskeyId);
                                    if (!DbSql.DeleteFromTable("dnskey", "id", dnskeyId)) {
                                        LOG(WARNING_LOG, "during deleting dnskeys command failed");
                                        ret->code = COMMAND_FAILED;
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
                                    int dsrecId = DbSql.GetSequenceID("dsrecord");

                                    DbSql.INSERT("dsrecord");
                                    DbSql.INTO("id");
                                    DbSql.INTO("keysetid");
                                    DbSql.INTO("keytag");
                                    DbSql.INTO("alg");
                                    DbSql.INTO("digesttype");
                                    DbSql.INTO("digest");
                                    DbSql.INTO("maxsiglife");
                                    DbSql.VALUE(dsrecId);
                                    DbSql.VALUE(keysetId);
                                    DbSql.VALUE(dsrec_add[i].keyTag);
                                    DbSql.VALUE(dsrec_add[i].alg);
                                    DbSql.VALUE(dsrec_add[i].digestType);
                                    DbSql.VALUE(dsrec_add[i].digest);
                                    if (dsrec_add[i].maxSigLife == -1)
                                        DbSql.VALUENULL();
                                    else
                                        DbSql.VALUE(dsrec_add[i].maxSigLife);
                                    if (!DbSql.EXEC()) {
                                        ret->code = COMMAND_FAILED;
                                        break;
                                    }
                                }

                                // add dnskey(s)
                                for (int ii = 0; ii < (int)dnsk_add.length(); ii++) {
                                    char *key;
                                    if ((key = removeWhitespaces((const char *)dnsk_add[ii].key)) == NULL) {
                                        LOG(WARNING_LOG, "removeWhitespaces fails (memory problem)");
                                        ret->code = COMMAND_FAILED;
                                        break;
                                    }
                                    LOG(NOTICE_LOG,
                                            "KeySetUpdate: add DNSKey (flags:%d,protocol:%d,"
                                            "alg:%d,key:%s to keyset (handle:%d)",
                                            dnsk_add[ii].flags, dnsk_add[ii].protocol,
                                            dnsk_add[ii].alg, (const char *)dnsk_add[ii].key,
                                            (const char *)handle);
                                    int dnskeyId = DbSql.GetSequenceID("dnskey");
                                    DbSql.INSERT("dnskey");
                                    DbSql.INTO("id");
                                    DbSql.INTO("keysetid");
                                    DbSql.INTO("flags");
                                    DbSql.INTO("protocol");
                                    DbSql.INTO("alg");
                                    DbSql.INTO("key");
                                    DbSql.VALUE(dnskeyId);
                                    DbSql.VALUE(keysetId);
                                    DbSql.VALUE(dnsk_add[ii].flags);
                                    DbSql.VALUE(dnsk_add[ii].protocol);
                                    DbSql.VALUE(dnsk_add[ii].alg);
                                    DbSql.VALUE(key);
                                    if (!DbSql.EXEC()) {
                                        ret->code = COMMAND_FAILED;
                                        free(key);
                                        break;
                                    }
                                    free(key);
                                }
                                // save to history if no errors
                                if (ret->code == 0) {
                                    if (DbSql.SaveKeySetHistory(keysetId))
                                        ret->code = COMMAND_OK;
                                }
                                if (ret->code == COMMAND_OK)
                                    ntf->Send();
                            }
                        }
                    }
                    DbSql.QuitTransaction(ret->code);
                }
                ret->svTRID = CORBA::string_dup(DbSql.EndAction(ret->code));
            }
            ret->msg = CORBA::string_dup(
                    GetErrorMessage(ret->code, GetRegistrarLang(clientId)));
            DbSql.Disconnect();
        }
    }

    delete[] techAdd;
    delete[] techRem;

    if (ret->code > COMMAND_EXCEPTION)
        EppError(ret->code, ret->msg, ret->svTRID, errors);
    if (ret->code == 0)
        ServerInternalError("KeySetUpdate");

    return ret._retn();
}

#define isbase64char(c)     ((c) >= 'a' && (c) <= 'z' || (c) >= 'A' && (c) <= 'Z' \
        || (c) >= '0' && (c) <= '9' || (c) == '+' || (c) == '/')

int
countTailPads(const char *key)
{
    char *str;
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
    char *str;
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
ccReg::Response* ccReg_EPP_i::FullList(
  short act, const char *table, const char *fname, ccReg::Lists_out list,
  CORBA::Long clientID, const char* clTRID, const char* XML)
{
  DB DBsql;
  int rows =0, i;
  ccReg::Response_var ret;
  int regID;
  int type;
  char sqlString[128];

  ret = new ccReg::Response;

  // default
  ret->code =0; // default

  list = new ccReg::Lists;

  LOG( NOTICE_LOG , "LIST %d  clientID -> %d clTRID [%s] " , act , (int ) clientID , clTRID );

  if ( (regID = GetRegistrarID(clientID) ))

    if (DBsql.OpenDatabase(database) ) {

      if ( (DBsql.BeginAction(clientID, act, clTRID, XML) )) {

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
        sprintf(sqlString,
            "SELECT obr.name FROM  object_registry obr, object o "
              "WHERE obr.id=o.id AND o.clid=%d AND obr.type=%d", regID, type);

        if (DBsql.ExecSelect(sqlString) ) {
          rows = DBsql.GetSelectRows();

          LOG( NOTICE_LOG, "Full List: %s  num -> %d ClID %d", table , rows , regID );
          list->length(rows);

          for (i = 0; i < rows; i ++) {
            (*list)[i]=CORBA::string_dup(DBsql.GetFieldValue(i, 0) );
          }

          DBsql.FreeSelect();
        }

        // command OK
        if (ret->code == 0)
          ret->code=COMMAND_OK; // if is OK

        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) ) ;
      }

      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
    }

  if (ret->code == 0)
    ServerInternalError("FullList");

  return ret._retn();
}

ccReg::Response* ccReg_EPP_i::ContactList(
  ccReg::Lists_out contacts, CORBA::Long clientID, const char* clTRID,
  const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  return FullList( EPP_ListContact , "CONTACT" , "HANDLE" , contacts , clientID, clTRID, XML);
}

ccReg::Response* ccReg_EPP_i::NSSetList(
  ccReg::Lists_out nssets, CORBA::Long clientID, const char* clTRID,
  const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  return FullList( EPP_ListNSset , "NSSET" , "HANDLE" , nssets , clientID, clTRID, XML);
}

ccReg::Response* ccReg_EPP_i::DomainList(
  ccReg::Lists_out domains, CORBA::Long clientID, const char* clTRID,
  const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  return FullList( EPP_ListDomain , "DOMAIN" , "fqdn" , domains , clientID, clTRID, XML);
}

ccReg::Response *
ccReg_EPP_i::KeySetList(
        ccReg::Lists_out keysets,
        CORBA::Long clientID,
        const char *clTRID,
        const char *XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

    return FullList(
            EPP_ListKeySet, "KEYSET", "HANDLE", keysets, clientID, clTRID, XML);
}

// function for run nsset tests
ccReg::Response* ccReg_EPP_i::nssetTest(
  const char* handle, CORBA::Short level, const ccReg::Lists& fqdns,
  CORBA::Long clientID, const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  DB DBsql;
  ccReg::Response_var ret = new ccReg::Response;
  int regID;
  int nssetid;
  bool internalError = false;

  LOG( NOTICE_LOG , "nssetTest nsset %s  clientID -> %d clTRID [%s] \n" , handle, (int ) clientID , clTRID );

  if ( (regID = GetRegistrarID(clientID) ))
    if (DBsql.OpenDatabase(database) ) {

      if ( (DBsql.BeginAction(clientID, EPP_NSsetTest, clTRID, XML) )) {

        if ( (nssetid = getIdOfNSSet(&DBsql, handle, conf) > 0 ))// TODO   ret->code =  SetReasonNSSetHandle( errors  , nssetid , GetRegistrarLang( clientID ) );
        {
          std::stringstream strid;
          strid << regID;
          std::string regHandle = DBsql.GetValueFromTable("registrar",
              "handle", "id", strid.str().c_str() );
          ret->code=COMMAND_OK;
          TechCheckManager tc(ns);
          TechCheckManager::FQDNList ifqdns;
          for (unsigned i=0; i<fqdns.length(); i++)
            ifqdns.push_back((const char *)fqdns[i]);
          try {
            tc.checkFromRegistrar(regHandle,handle,level,ifqdns,clTRID);
          }
          catch (TechCheckManager::INTERNAL_ERROR) {
            LOG(ERROR_LOG,"Tech check internal error nsset [%s] clientID -> %d clTRID [%s] " , handle , (int ) clientID , clTRID );
            internalError = true;
          }
          catch (TechCheckManager::REGISTRAR_NOT_FOUND) {
            LOG(ERROR_LOG,"Tech check reg not found nsset [%s] clientID -> %d clTRID [%s] " , handle , (int ) clientID , clTRID );
            internalError = true;
          }
          catch (TechCheckManager::NSSET_NOT_FOUND) {
            LOG(ERROR_LOG,"Tech check nsset not found nset [%s] clientID -> %d clTRID [%s] " , handle , (int ) clientID , clTRID );
            internalError = true;
          }
        } else {
          LOG( WARNING_LOG, "nsset handle [%s] NOT_EXIST", handle );
          ret->code = COMMAND_OBJECT_NOT_EXIST;
        }

        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) ) ;
      }

      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );

      DBsql.Disconnect();
      if (internalError)
        ServerInternalError("NSSetTest");
    }

  return ret._retn();
}

// function for send authinfo
ccReg::Response* ccReg_EPP_i::ObjectSendAuthInfo(
  short act, const char * table, const char *fname, const char *name, CORBA::Long clientID,
  const char* clTRID, const char* XML)
{
  DB DBsql;
  int zone;
  int id = 0;
  ccReg::Response_var ret;
  ccReg::Errors_var errors;
  char FQDN[164];
  int regID;
  ret = new ccReg::Response;

  // default
  errors = new ccReg::Errors;
  errors->length( 0);
  ret->code =0; // default

  ParsedAction paction;
  paction.add(1,(const char*)name);
  Database::Manager db(database);
  std::auto_ptr<Database::Connection> conn;
  try { conn.reset(db.getConnection()); } catch (...) {}

  //LOG( NOTICE_LOG , "ObjectSendAuthInfo type %d  object [%s]  clientID -> %d clTRID [%s] " , act , name , (int ) clientID , clTRID );

  if ( (regID = GetRegistrarID(clientID) ))
    if (DBsql.OpenDatabase(database) && conn.get()) {
      if ( (DBsql.BeginAction(clientID, act, clTRID, XML, &paction) )) {
        switch (act) {
          case EPP_ContactSendAuthInfo:
            if ( (id = getIdOfContact(&DBsql, name, conf) ) < 0)
              ret->code= SetReasonContactHandle(errors, name,
                  GetRegistrarLang(clientID) );
            else if (id == 0)
              ret->code=COMMAND_OBJECT_NOT_EXIST;
            break;
          case EPP_NSSetSendAuthInfo:
            if ( (id = getIdOfNSSet(&DBsql, name, conf) ) < 0)
              ret->code=SetReasonNSSetHandle(errors, name,
                  GetRegistrarLang(clientID) );
            else if (id == 0)
              ret->code=COMMAND_OBJECT_NOT_EXIST;
            break;
          case EPP_DomainSendAuthInfo:
            if ( (zone = getFQDN(FQDN, name) ) <= 0)
              ret->code=SetReasonDomainFQDN(errors, name, zone,
                  GetRegistrarLang(clientID) );
            else {
              if ( (id = DBsql.GetDomainID(FQDN, GetZoneEnum(zone) ) ) == 0) {
                LOG( WARNING_LOG , "domain [%s] NOT_EXIST" , name );
                ret->code= COMMAND_OBJECT_NOT_EXIST;
              }
            }
            break;
          case EPP_KeySetSendAuthInfo:
            if ((id = getIdOfKeySet(&DBsql, name, conf)) < 0)
                ret->code = SetReasonKeySetHandle(
                        errors, name, GetRegistrarLang(clientID));
            else if (id == 0)
                ret->code = COMMAND_OBJECT_NOT_EXIST;
            break;
        }
        if (ret->code == 0) {
          std::auto_ptr<Register::Document::Manager> doc_manager(
            Register::Document::Manager::create(
              conf.get<std::string>("registry.docgen_path"),
              conf.get<std::string>("registry.docgen_template_path"),
              conf.get<std::string>("registry.fileclient_path"),
              ns->getHostName()
            )
          );
          std::auto_ptr<Register::PublicRequest::Manager> request_manager(
            Register::PublicRequest::Manager::create(
              &db,
              regMan->getDomainManager(),
              regMan->getContactManager(),
              regMan->getNSSetManager(),
              regMan->getKeySetManager(),
              mm,
              doc_manager.get()
            )
          );
          try {
            LOG(
              NOTICE_LOG , "createRequest objectID %d actionID %d" ,
              id,DBsql.GetActionID()
            );
            Register::PublicRequest::PublicRequest *new_request =
              request_manager->createRequest(
                Register::PublicRequest::PRT_AUTHINFO_AUTO_RIF,conn.get()
              );
            new_request->setEppActionId(DBsql.GetActionID());
            new_request->addObject(Register::PublicRequest::OID(id));
            if (!new_request->check()) {
              LOG(WARNING_LOG, "authinfo request for %s is prohibited",name);
              ret->code = COMMAND_STATUS_PROHIBITS_OPERATION;
            } else {
              ret->code=COMMAND_OK;
              new_request->save(conn.get());
            }
          } catch (...) {
            LOG( WARNING_LOG, "cannot create and process request");
            ret->code=COMMAND_FAILED;
          }
        }
        ret->svTRID = CORBA::string_dup(DBsql.EndAction(ret->code) ) ;
      }
      ret->msg =CORBA::string_dup(GetErrorMessage(ret->code,
          GetRegistrarLang(clientID) ) );
      DBsql.Disconnect();
    }

  // EPP exception
  if (ret->code > COMMAND_EXCEPTION)
    EppError(ret->code, ret->msg, ret->svTRID, errors);

  if (ret->code == 0)
    ServerInternalError("ObjectSendAuthInfo");

  return ret._retn();
}

ccReg::Response* ccReg_EPP_i::domainSendAuthInfo(
  const char* fqdn, CORBA::Long clientID, const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  return ObjectSendAuthInfo( EPP_DomainSendAuthInfo , "DOMAIN" , "fqdn" , fqdn , clientID , clTRID, XML);
}
ccReg::Response* ccReg_EPP_i::contactSendAuthInfo(
  const char* handle, CORBA::Long clientID, const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  return ObjectSendAuthInfo( EPP_ContactSendAuthInfo , "CONTACT" , "handle" , handle , clientID , clTRID, XML);
}
ccReg::Response* ccReg_EPP_i::nssetSendAuthInfo(
  const char* handle, CORBA::Long clientID, const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  return ObjectSendAuthInfo( EPP_NSSetSendAuthInfo , "NSSET" , "handle" , handle , clientID , clTRID, XML);
}

ccReg::Response *
ccReg_EPP_i::keysetSendAuthInfo(
        const char *handle,
        CORBA::Long clientID,
        const char *clTRID,
        const char *XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

    return ObjectSendAuthInfo(
            EPP_KeySetSendAuthInfo, "KEYSET",
            "handle", handle, clientID, clTRID, XML);
}

ccReg::Response* ccReg_EPP_i::info(
  ccReg::InfoType type, const char* handle, CORBA::Long& count,
  CORBA::Long clientID, const char* clTRID, const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  LOG(
      NOTICE_LOG,
      "Info: clientID -> %d clTRID [%s]", (int ) clientID, clTRID
  );
  // start EPP action - this will handle all init stuff
  EPPAction a(this, clientID, EPP_Info, clTRID, XML);
  try {
    std::auto_ptr<Register::Zone::Manager> zoneMan(
        Register::Zone::Manager::create(a.getDB())
    );
    std::auto_ptr<Register::Domain::Manager> domMan(
        Register::Domain::Manager::create(a.getDB(), zoneMan.get())
    );
    std::auto_ptr<Register::Contact::Manager> conMan(
        Register::Contact::Manager::create(a.getDB(),conf.get<bool>("registry.restricted_handles"))
    );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
        Register::NSSet::Manager::create(
            a.getDB(),zoneMan.get(),conf.get<bool>("registry.restricted_handles")
        )
    );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                a.getDB(), conf.get<bool>("registry.restricted_handles")
                )
            );
    std::auto_ptr<Register::InfoBuffer::Manager> infoBufMan(
        Register::InfoBuffer::Manager::create(
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
        Register::InfoBuffer::T_LIST_CONTACTS :
        type == ccReg::IT_LIST_DOMAINS ?
        Register::InfoBuffer::T_LIST_DOMAINS :
        type == ccReg::IT_LIST_NSSETS ?
        Register::InfoBuffer::T_LIST_NSSETS :
        type == ccReg::IT_LIST_KEYSETS ?
        Register::InfoBuffer::T_LIST_KEYSETS :
        type == ccReg::IT_DOMAINS_BY_NSSET ?
        Register::InfoBuffer::T_DOMAINS_BY_NSSET :
        type == ccReg::IT_DOMAINS_BY_CONTACT ?
        Register::InfoBuffer::T_DOMAINS_BY_CONTACT :
        type == ccReg::IT_NSSETS_BY_CONTACT ?
        Register::InfoBuffer::T_NSSETS_BY_CONTACT :
        type == ccReg::IT_NSSETS_BY_NS ?
        Register::InfoBuffer::T_NSSETS_BY_NS :
        type == ccReg::IT_DOMAINS_BY_KEYSET ?
        Register::InfoBuffer::T_DOMAINS_BY_KEYSET :
        Register::InfoBuffer::T_KEYSETS_BY_CONTACT,
        handle
    );
  }
  catch (...) {a.failedInternal("Connection problems");}
  return a.getRet()._retn();
}

ccReg::Response* ccReg_EPP_i::getInfoResults(
  ccReg::Lists_out handles, CORBA::Long clientID, const char* clTRID,
  const char* XML)
{
  Logging::Context ctx("rifd");
  Logging::Context ctx2(str(boost::format("clid-%1%") % clientID));

  LOG(
      NOTICE_LOG,
      "getResults: clientID -> %d clTRID [%s]", (int ) clientID, clTRID
  );
  // start EPP action - this will handle all init stuff
  EPPAction a(this, clientID, EPP_GetInfoResults, clTRID, XML);
  try {
    std::auto_ptr<Register::Zone::Manager> zoneMan(
        Register::Zone::Manager::create(a.getDB())
    );
    std::auto_ptr<Register::Domain::Manager> domMan(
        Register::Domain::Manager::create(a.getDB(), zoneMan.get())
    );
    std::auto_ptr<Register::Contact::Manager> conMan(
        Register::Contact::Manager::create(a.getDB(),conf.get<bool>("registry.restricted_handles"))
    );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
        Register::NSSet::Manager::create(
            a.getDB(),zoneMan.get(),conf.get<bool>("registry.restricted_handles")
        )
    );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                a.getDB(), conf.get<bool>("registry.restricted_handles")
                )
            );
    std::auto_ptr<Register::InfoBuffer::Manager> infoBufMan(
        Register::InfoBuffer::Manager::create(
            a.getDB(),
            domMan.get(),
            nssMan.get(),
            conMan.get(),
            keyMan.get()
        )
    );
    std::auto_ptr<Register::InfoBuffer::Chunk> chunk(
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
