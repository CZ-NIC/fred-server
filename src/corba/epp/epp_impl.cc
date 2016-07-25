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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "src/corba/EPP.hh"
#include "epp_impl.h"

#include "src/corba/connection_releaser.h"

#include "util/optional_value.h"
#include "src/fredlib/notifier/enqueue_notification.h"

#include "config.h"

// database functions
#include "src/old_utils/dbsql.h"

// support function
#include "src/old_utils/util.h"

#include "action.h"    // code of the EPP operations
#include "response.h"  // errors code
#include "reason.h"    // reason messages code

// logger
#include "src/old_utils/log.h"

// MailerManager is connected in constructor
#include "src/fredlib/common_diff.h"
#include "src/fredlib/domain.h"
#include "src/fredlib/contact.h"
#include "src/fredlib/nsset.h"
#include "src/fredlib/keyset.h"
#include "src/fredlib/info_buffer.h"
#include "src/fredlib/poll.h"
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

#include "src/epp/nsset/nsset_check.h"
#include "src/epp/nsset/nsset_info.h"
#include "src/epp/nsset/nsset_delete.h"
#include "src/epp/nsset/nsset_create.h"
#include "src/epp/nsset/nsset_update.h"
#include "src/epp/nsset/nsset_transfer.h"

#include "src/epp/contact/contact_update.h"
#include "src/epp/contact/contact_create.h"
#include "src/epp/contact/contact_info.h"
#include "src/epp/contact/contact_delete.h"
#include "src/epp/contact/contact_transfer.h"
#include "src/epp/contact/post_contact_update_hooks.h"

#include "src/epp/keyset/localized_info.h"
#include "src/epp/keyset/localized_create.h"
#include "src/epp/keyset/localized_update.h"
#include "src/epp/keyset/localized_check.h"
#include "src/epp/keyset/localized_delete.h"
#include "src/epp/keyset/localized_transfer.h"

#include "src/epp/response.h"
#include "src/epp/reason.h"
#include "src/epp/param.h"
#include "src/epp/session_lang.h"
#include "src/epp/get_registrar_session_data.h"
#include "src/epp/registrar_session_data.h"
#include "src/epp/request_params.h"
#include "src/epp/localization.h"
#include "src/epp/disclose_policy.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/corba/epp/corba_conversions.h"
#include "src/corba/epp/epp_legacy_compatibility.h"
#include "util/util.h"

#define FLAG_serverDeleteProhibited 1
#define FLAG_serverRenewProhibited 2
#define FLAG_serverTransferProhibited 3
#define FLAG_serverUpdateProhibited 4
#define FLAG_serverRegistrantChangeProhibited 18
#define FLAG_deleteCandidate 17

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

static std::string formatTime(const boost::posix_time::ptime& tm) {
  char buffer[100];
  convert_rfc3339_timestamp(buffer, sizeof(buffer), boost::posix_time::to_iso_extended_string(tm).c_str());
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
    , bool allow_idn, bool lock = false, int* zone = NULL)
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
    switch (dman->checkAvail(handle, nameId, allow_idn, lock )) {
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


/*
 * common function to copy domain data to corba structure
 *
 * \param a        action data (transaction and registrar info - authinfo fill check)
 * \param regMan   registry manager - to get object states description
 * \param d        destination corba domain structure to fill data into
 * \param dom      source domain data
 */
void corba_domain_data_copy(
        EPPAction &a,
        const Fred::Manager * const regMan,
        ccReg::Domain *d,
        const Fred::Domain::Domain * const dom)
{
  std::auto_ptr<Fred::Zone::Manager>
      zman(Fred::Zone::Manager::create() );
  std::auto_ptr<Fred::Domain::Manager>
      dman(Fred::Domain::Manager::create(a.getDB(), zman.get()) );

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
      && zman->findApplicableZone(dom->getFQDN())->isEnumZone();
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
}


/*
 * common function to copy nsset data to corba structure
 *
 * \param a        action data (transaction and registrar info - authinfo fill check)
 * \param regMan   registry manager - to get object states description
 * \param n        destination corba nsset structure to fill data into
 * \param nss      source nsset data
 */
void corba_nsset_data_copy(
        EPPAction &a,
        const Fred::Manager * const regMan,
        ccReg::NSSet *n,
        const Fred::NSSet::NSSet * const nss)
{
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
}


/*
 * common function to copy keyset data to corba structure
 *
 * \param a        action data (transaction and registrar info - authinfo fill check)
 * \param regMan   registry manager - to get object states description
 * \param n        destination corba keyset structure to fill data into
 * \param nss      source keyset data
 */
void corba_keyset_data_copy(
        EPPAction &a,
        const Fred::Manager * const regMan,
        ccReg::KeySet *k,
        const Fred::KeySet::KeySet * const kss)
{
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
    , bool rifd_epp_operations_charging
    , bool _allow_idn
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
    , docgen_path_(docgen_path)
    , docgen_template_path_(docgen_template_path)
    , fileclient_path_(fileclient_path)
    , disable_epp_notifier_cltrid_prefix_(disable_epp_notifier_cltrid_prefix)
    , rifd_session_max_(rifd_session_max)
    , rifd_session_timeout_(rifd_session_timeout)
    , rifd_session_registrar_max_(rifd_session_registrar_max)
    , rifd_epp_update_domain_keyset_clear_(rifd_epp_update_domain_keyset_clear)
    , rifd_epp_operations_charging_(rifd_epp_operations_charging),
    allow_idn_(_allow_idn),
    epp_update_contact_enqueue_check_(epp_update_contact_enqueue_check),
    db_disconnect_guard_(),
    regMan(),
    epp_sessions(rifd_session_max, rifd_session_registrar_max, rifd_session_timeout),
    ErrorMsg(),
    ReasonMsg(),
    CC(),
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

bool ccReg_EPP_i::idn_allowed(EPPAction& action) const {
    DBSharedPtr db = action.getDB();
    if (!db.get()) {
        throw Fred::SQL_ERROR();
    }

    // system registrar has IDN always allowed
    if (db->GetRegistrarSystem(action.getRegistrar())) {
        return true;
    } else {
        return this->allow_idn_;
    }
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
  const char* msgID, CORBA::ULongLong& count, CORBA::String_out newmsgID,
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
  CORBA::String_out msgID, CORBA::ULongLong& count, ccReg::timestamp_out qDate,
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
      case Fred::Poll::MT_IDLE_DELETE_CONTACT:
      case Fred::Poll::MT_DELETE_CONTACT:
        type = ccReg::polltype_delete_contact;
        break;
      case Fred::Poll::MT_IDLE_DELETE_NSSET:
        type = ccReg::polltype_delete_nsset;
        break;
      case Fred::Poll::MT_IDLE_DELETE_DOMAIN:
      case Fred::Poll::MT_DELETE_DOMAIN:
        type = ccReg::polltype_delete_domain;
        break;
      case Fred::Poll::MT_IDLE_DELETE_KEYSET:
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
    hdm->limit = CORBA::string_dup((mlc->getLimit()).get_string(".2f").c_str());
    hdm->credit = CORBA::string_dup((mlc->getCredit()).get_string(".2f").c_str());
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
  Fred::Poll::MessageUpdateObject *muo =
      dynamic_cast<Fred::Poll::MessageUpdateObject*>(m.get());
  if (muo)
  {
      switch (muo->getType())
      {
          case Fred::Poll::MT_UPDATE_DOMAIN:
              type = ccReg::polltype_update_domain;
              break;
          case Fred::Poll::MT_UPDATE_NSSET:
              type = ccReg::polltype_update_nsset;
              break;
          case Fred::Poll::MT_UPDATE_KEYSET:
              type = ccReg::polltype_update_keyset;
              break;
      }
      ccReg::PollMsg_Update *hdm = new ccReg::PollMsg_Update;
      hdm->opTRID = CORBA::string_dup(muo->getOpTRID().c_str());
      hdm->pollID = muo->getId();
      *msg <<= hdm;
      LOGGER(PACKAGE).debug("poll message update_domain packed");
      return a.getRet()._retn();
  }
  a.failedInternal("Invalid message structure");
  // previous command throw exception in any case so this code
  // will never be called
  return NULL;
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
        CORBA::ULongLong _poll_id,
        ccReg::Domain_out _old_data,
        ccReg::Domain_out _new_data,
        const ccReg::EppParams &params)
{
    try {
        Logging::Context::clear();
        Logging::Context ctx("rifd");
        Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
        Logging::Context ctx3("poll-req-update-domain-details");
        ConnectionReleaser releaser;

        LOGGER(PACKAGE).debug(boost::format("poll_id=%1%") % _poll_id);

        EPPAction a(this, params.loginID, EPP_PollResponse, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

        _old_data = new ccReg::Domain;
        _new_data = new ccReg::Domain;

        Database::Connection conn = Database::Manager::acquire();
        Database::Result hids = conn.exec_params(
                "SELECT h1.id as old_hid, h1.next as new_hid"
                " FROM poll_eppaction pea"
                " JOIN domain_history dh ON dh.historyid = pea.objid"
                " JOIN history h1 ON h1.next = pea.objid"
                " WHERE pea.msgid = $1::bigint",
                Database::query_param_list(_poll_id));

        if (hids.size() != 1) {
            throw std::runtime_error("unable to get poll message data");
        }

        std::auto_ptr<Fred::Manager> rmgr(Fred::Manager::create(a.getDB(), false));
        rmgr->initStates();
        std::auto_ptr<const Fred::Domain::Domain> old_data = Fred::get_object_by_hid<
            Fred::Domain::Domain, Fred::Domain::Manager, Fred::Domain::List, Database::Filters::DomainHistoryImpl>(
                    rmgr->getDomainManager(), static_cast<unsigned long long>(hids[0][0]));
        std::auto_ptr<const Fred::Domain::Domain> new_data = Fred::get_object_by_hid<
            Fred::Domain::Domain, Fred::Domain::Manager, Fred::Domain::List, Database::Filters::DomainHistoryImpl>(
                    rmgr->getDomainManager(), static_cast<unsigned long long>(hids[0][1]));

        corba_domain_data_copy(a, rmgr.get(), _old_data, old_data.get());
        corba_domain_data_copy(a, rmgr.get(), _new_data, new_data.get());

        return;
    }
    catch (std::exception &ex)
    {
        LOGGER(PACKAGE).error(ex.what());
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown error");
    }
    this->ServerInternalError(">> PollRequestGetUpdateDomainDetails - failed internal");
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
        CORBA::ULongLong _poll_id,
        ccReg::NSSet_out _old_data,
        ccReg::NSSet_out _new_data,
        const ccReg::EppParams &params)
{
    try {
        Logging::Context::clear();
        Logging::Context ctx("rifd");
        Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
        Logging::Context ctx3("poll-req-update-nsset-details");
        ConnectionReleaser releaser;

        LOGGER(PACKAGE).debug(boost::format("poll_id=%1%") % _poll_id);

        EPPAction a(this, params.loginID, EPP_PollResponse, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

        _old_data = new ccReg::NSSet;
        _new_data = new ccReg::NSSet;

        Database::Connection conn = Database::Manager::acquire();
        Database::Result hids = conn.exec_params(
                "SELECT h1.id as old_hid, h1.next as new_hid"
                " FROM poll_eppaction pea"
                " JOIN nsset_history dh ON dh.historyid = pea.objid"
                " JOIN history h1 ON h1.next = pea.objid"
                " WHERE pea.msgid = $1::bigint",
                Database::query_param_list(_poll_id));

        if (hids.size() != 1) {
            throw std::runtime_error("unable to get poll message data");
        }

        std::auto_ptr<Fred::Manager> rmgr(Fred::Manager::create(a.getDB(), false));
        rmgr->initStates();
        std::auto_ptr<const Fred::NSSet::NSSet> old_data = Fred::get_object_by_hid<
            Fred::NSSet::NSSet, Fred::NSSet::Manager, Fred::NSSet::List, Database::Filters::NSSetHistoryImpl>(
                    rmgr->getNSSetManager(), static_cast<unsigned long long>(hids[0][0]));
        std::auto_ptr<const Fred::NSSet::NSSet> new_data = Fred::get_object_by_hid<
            Fred::NSSet::NSSet, Fred::NSSet::Manager, Fred::NSSet::List, Database::Filters::NSSetHistoryImpl>(
                    rmgr->getNSSetManager(), static_cast<unsigned long long>(hids[0][1]));

        corba_nsset_data_copy(a, rmgr.get(), _old_data, old_data.get());
        corba_nsset_data_copy(a, rmgr.get(), _new_data, new_data.get());

        return;
    }
    catch (std::exception &ex)
    {
        LOGGER(PACKAGE).error(ex.what());
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown error");
    }
    this->ServerInternalError(">> PollRequestGetUpdateNSSetDetails - failed internal");
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
        CORBA::ULongLong _poll_id,
        ccReg::KeySet_out _old_data,
        ccReg::KeySet_out _new_data,
        const ccReg::EppParams &params)
{
    try {
        Logging::Context::clear();
        Logging::Context ctx("rifd");
        Logging::Context ctx2(str(boost::format("clid-%1%") % params.loginID));
        Logging::Context ctx3("poll-req-update-keyset-details");
        ConnectionReleaser releaser;

        LOGGER(PACKAGE).debug(boost::format("poll_id=%1%") % _poll_id);

        EPPAction a(this, params.loginID, EPP_PollResponse, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

        _old_data = new ccReg::KeySet;
        _new_data = new ccReg::KeySet;

        Database::Connection conn = Database::Manager::acquire();
        Database::Result hids = conn.exec_params(
                "SELECT h1.id as old_hid, h1.next as new_hid"
                " FROM poll_eppaction pea"
                " JOIN keyset_history dh ON dh.historyid = pea.objid"
                " JOIN history h1 ON h1.next = pea.objid"
                " WHERE pea.msgid = $1::bigint",
                Database::query_param_list(_poll_id));

        if (hids.size() != 1) {
            throw std::runtime_error("unable to get poll message data");
        }

        std::auto_ptr<Fred::Manager> rmgr(Fred::Manager::create(a.getDB(), false));
        rmgr->initStates();
        std::auto_ptr<const Fred::KeySet::KeySet> old_data = Fred::get_object_by_hid<
            Fred::KeySet::KeySet, Fred::KeySet::Manager, Fred::KeySet::List, Database::Filters::KeySetHistoryImpl>(
                    rmgr->getKeySetManager(), static_cast<unsigned long long>(hids[0][0]));
        std::auto_ptr<const Fred::KeySet::KeySet> new_data = Fred::get_object_by_hid<
            Fred::KeySet::KeySet, Fred::KeySet::Manager, Fred::KeySet::List, Database::Filters::KeySetHistoryImpl>(
                    rmgr->getKeySetManager(), static_cast<unsigned long long>(hids[0][1]));

        corba_keyset_data_copy(a, rmgr.get(), _old_data, old_data.get());
        corba_keyset_data_copy(a, rmgr.get(), _new_data, new_data.get());

        return;
    }
    catch (std::exception &ex)
    {
        LOGGER(PACKAGE).error(ex.what());
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown error");
    }
    this->ServerInternalError(">> PollRequestGetUpdateKeySetDetails - failed internal");
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
                        out_clientID = epp_sessions.login_session(regID, language);

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

                    caType = dman->checkAvail( ( const char * ) chck[i] , caConflict, idn_allowed(action) );
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
    const ccReg::Check& _handles_to_be_checked,
    ccReg::CheckResp_out _check_results,
    const ccReg::EppParams& _epp_params
) {
    const std::string server_transaction_handle = Util::make_svtrid(_epp_params.requestID);
    try {
        /* output data must be ordered exactly the same */
        const std::vector<std::string> handles_to_be_checked = Corba::unwrap_handle_sequence_to_string_vector(_handles_to_be_checked);
        const Epp::RequestParams request_params = Corba::unwrap_EppParams(_epp_params);
        const Epp::RegistrarSessionData session_data = Epp::get_registrar_session_data(epp_sessions, request_params.session_id);

        const Epp::LocalizedCheckContactResponse response = Epp::contact_check(
            std::set<std::string>( handles_to_be_checked.begin(), handles_to_be_checked.end() ),
            session_data.registrar_id,
            session_data.language,
            server_transaction_handle
        );

        ccReg::CheckResp_var check_results = new ccReg::CheckResp(
            Corba::wrap_localized_check_info(
                handles_to_be_checked,
                response.contact_statuses
            )
        );

        ccReg::Response_var return_value = new ccReg::Response( Corba::wrap_response(response.ok_response, server_transaction_handle) );

        /* No exception shall be thrown from here onwards. */

        _check_results = check_results._retn();
        return return_value._retn();

    } catch(const Epp::LocalizedFailResponse& e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::NSSetCheck(
  const ccReg::Check& _handles_to_be_checked, ccReg::CheckResp_out _check_results, const ccReg::EppParams &_epp_params)
{

    const std::string server_transaction_handle = Util::make_svtrid(_epp_params.requestID);
    try {
        /* output data must be ordered exactly the same */
        const std::vector<std::string> handles_to_be_checked = Corba::unwrap_handle_sequence_to_string_vector(_handles_to_be_checked);
        const Epp::RequestParams request_params = Corba::unwrap_epp_request_params(_epp_params);
        const Epp::RegistrarSessionData session_data = Epp::get_registrar_session_data(epp_sessions, request_params.session_id);

        const Epp::LocalizedCheckNssetResponse response = Epp::nsset_check(
                    std::set<std::string>( handles_to_be_checked.begin(), handles_to_be_checked.end() ),
                    session_data.registrar_id,
                    session_data.language,
                    server_transaction_handle
                );

        ccReg::CheckResp_var check_results = new ccReg::CheckResp(
            Corba::wrap_localized_check_info(
                handles_to_be_checked,
                response.nsset_statuses
            )
        );

        ccReg::Response_var return_value = new ccReg::Response( Corba::wrap_response(response.ok_response, server_transaction_handle) );

        /* No exception shall be thrown from here onwards. */

        _check_results = check_results._retn();
        return return_value._retn();

    } catch(const Epp::LocalizedFailResponse& e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
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

ccReg::Response* ccReg_EPP_i::KeySetCheck(
    const ccReg::Check& _handles_to_be_checked,
    ccReg::CheckResp_out _check_results,
    const ccReg::EppParams& _epp_params)
{
    const Epp::RequestParams epp_request_params = Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();
    try {
        const Epp::RegistrarSessionData session_data =
            Epp::get_registrar_session_data(this->epp_sessions, epp_request_params.session_id);

        const std::vector< std::string > handles_to_be_checked = Corba::unwrap_handle_sequence_to_string_vector(_handles_to_be_checked);
        const Epp::KeySet::Localized::HandlesCheck localized_response = Epp::KeySet::Localized::check(
            std::set< std::string >(handles_to_be_checked.begin(), handles_to_be_checked.end()),
            session_data.registrar_id,
            session_data.language,
            server_transaction_handle);

        ccReg::CheckResp_var check_results = new ccReg::CheckResp;
        Corba::wrap_Epp_KeySet_Localized_HandlesCheck_Results(handles_to_be_checked, localized_response.results, check_results);

        ccReg::Response_var return_value =
            new ccReg::Response(Corba::wrap_response(localized_response.ok_response, server_transaction_handle));

        /* No exception shall be thrown from here onwards. */
        _check_results = check_results._retn();
        return return_value._retn();
    }
    catch (const Epp::LocalizedFailResponse &e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::ContactInfo(
    const char* const _handle,
    ccReg::Contact_out _info_result,
    const ccReg::EppParams& _epp_params
) {
    const std::string server_transaction_handle = Util::make_svtrid( _epp_params.requestID );
    try {

        const Epp::RegistrarSessionData session_data = Epp::get_registrar_session_data(
            epp_sessions,
            Corba::unwrap_EppParams(_epp_params).session_id
        );

        const Epp::LocalizedInfoContactResponse response = Epp::contact_info(
            Corba::unwrap_string(_handle),
            session_data.registrar_id,
            session_data.language,
            server_transaction_handle
        );

        ccReg::Contact_var info_result = new ccReg::Contact;
        Corba::wrap_LocalizedContactInfoOutputData(response.payload, info_result.inout());
        ccReg::Response_var return_value = new ccReg::Response( Corba::wrap_response(response.ok_response, server_transaction_handle) );

        /* No exception shall be thrown from here onwards. */

        _info_result = info_result._retn();
        return return_value._retn();

    } catch(const Epp::LocalizedFailResponse& e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::ContactDelete(
    const char* const _handle,
    const ccReg::EppParams& _epp_params
) {
    const std::string server_transaction_handle = Util::make_svtrid( _epp_params.requestID );
    try {
        const Epp::RequestParams request_params = Corba::unwrap_EppParams(_epp_params);
        const Epp::RegistrarSessionData session_data = Epp::get_registrar_session_data(epp_sessions, request_params.session_id);

        const Epp::LocalizedSuccessResponse response = Epp::contact_delete(
            Corba::unwrap_string(_handle),
            session_data.registrar_id,
            session_data.language,
            server_transaction_handle,
            request_params.client_transaction_id,
            disable_epp_notifier_,
            disable_epp_notifier_cltrid_prefix_
        );

        return new ccReg::Response( Corba::wrap_response(response, server_transaction_handle) );

    } catch(const Epp::LocalizedFailResponse& e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::ContactUpdate(
    const char* const _handle,
    const ccReg::ContactChange& _data_change,
    const ccReg::EppParams& _epp_params
) {
    const std::string server_transaction_handle = Util::make_svtrid( _epp_params.requestID );
    try {
        const Epp::RequestParams request_params = Corba::unwrap_EppParams(_epp_params);
        const Epp::RegistrarSessionData session_data = Epp::get_registrar_session_data(epp_sessions, request_params.session_id);

        Epp::ContactChange contact_update_data;
        Corba::unwrap_ContactChange(_data_change, contact_update_data);
        const Epp::LocalizedSuccessResponse response = Epp::contact_update(
            Corba::unwrap_string(_handle),
            contact_update_data,
            session_data.registrar_id,
            request_params.log_request_id.get_value_or(0),
            epp_update_contact_enqueue_check_,
            session_data.language,
            server_transaction_handle,
            request_params.client_transaction_id,
            disable_epp_notifier_,
            disable_epp_notifier_cltrid_prefix_);

        return new ccReg::Response(Corba::wrap_response(response, server_transaction_handle));

    } catch(const Epp::LocalizedFailResponse& e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
}

ccReg::Response * ccReg_EPP_i::ContactCreate(
    const char* const _handle,
    const ccReg::ContactChange& _contact_data,
    ccReg::timestamp_out _create_time,
    const ccReg::EppParams& _epp_params
) {
    const std::string server_transaction_handle = Util::make_svtrid( _epp_params.requestID );
    try {
        const Epp::RequestParams request_params = Corba::unwrap_EppParams(_epp_params);
        const Epp::RegistrarSessionData session_data = Epp::get_registrar_session_data(epp_sessions, request_params.session_id);

        Epp::ContactChange contact_create_data;
        Corba::unwrap_ContactChange(_contact_data, contact_create_data);
        const Epp::LocalizedCreateContactResponse response = contact_create(
            Corba::unwrap_string(_handle),
            contact_create_data,
            session_data.registrar_id,
            request_params.log_request_id.get_value_or(0),
            session_data.language,
            server_transaction_handle,
            request_params.client_transaction_id,
            disable_epp_notifier_,
            disable_epp_notifier_cltrid_prefix_
        );

        ccReg::timestamp_var create_time = Corba::wrap_string_to_corba_string( formatTime( response.crdate ) );
        ccReg::Response_var return_value = new ccReg::Response( Corba::wrap_response(response.ok_response, server_transaction_handle) );

        /* No exception shall be thrown from here onwards. */

        _create_time = create_time._retn();
        return return_value._retn();

    } catch(const Epp::LocalizedFailResponse& e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
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
                    , idn_allowed(action), true, &zone ) ) <= 0) {
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
            action.set_notification_params(id,Notification::transferred, disable_epp_notifier_);
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
    const char* const _handle,
    const char* const _auth_info,
    const ccReg::EppParams& _epp_params
) {

    const std::string server_transaction_handle = Util::make_svtrid( _epp_params.requestID );
    try {

        const Epp::RequestParams request_params = Corba::unwrap_EppParams(_epp_params);
        const Epp::RegistrarSessionData session_data = Epp::get_registrar_session_data(epp_sessions, request_params.session_id);

        return new ccReg::Response(
            Corba::wrap_response(
                contact_transfer(
                    Corba::unwrap_string(_handle),
                    Corba::unwrap_string(_auth_info),
                    session_data.registrar_id,
                    request_params.log_request_id.get_value_or(0),
                    session_data.language,
                    server_transaction_handle,
                    request_params.client_transaction_id,
                    disable_epp_notifier_,
                    disable_epp_notifier_cltrid_prefix_
                ),
                server_transaction_handle
            )
        );

    } catch(const Epp::LocalizedFailResponse& e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
}

ccReg::Response* ccReg_EPP_i::NSSetTransfer(
  const char* _handle, const char* _auth_info, const ccReg::EppParams &_epp_params)
{
    const std::string server_transaction_handle = Util::make_svtrid( _epp_params.requestID );
    try {

        const Epp::RequestParams request_params = Corba::unwrap_epp_request_params(_epp_params);
        const Epp::RegistrarSessionData session_data = Epp::get_registrar_session_data(epp_sessions, request_params.session_id);

        return new ccReg::Response(
            Corba::wrap_response(
                nsset_transfer(
                    Corba::unwrap_string(_handle),
                    Corba::unwrap_string(_auth_info),
                    session_data.registrar_id,
                    request_params.log_request_id,
                    session_data.language,
                    server_transaction_handle,
                    request_params.client_transaction_id,
                    disable_epp_notifier_cltrid_prefix_
                ),
                server_transaction_handle
            )
        );

    } catch(const Epp::LocalizedFailResponse& e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
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

ccReg::Response*
ccReg_EPP_i::KeySetTransfer(
        const char *_keyset_handle,
        const char *_auth_info,
        const ccReg::EppParams &_epp_params)
{
    const Epp::RequestParams epp_request_params = Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();
    try {
        const Epp::RegistrarSessionData session_data =
            Epp::get_registrar_session_data(this->epp_sessions, epp_request_params.session_id);

        const std::string keyset_handle = Corba::unwrap_string_from_const_char_ptr(_keyset_handle);
        const std::string auth_info = Corba::unwrap_string_from_const_char_ptr(_auth_info);
        const Epp::LocalizedSuccessResponse response = Epp::KeySet::Localized::transfer(
            keyset_handle,
            auth_info,
            session_data.registrar_id,
            epp_request_params.log_request_id,
            session_data.language,
            server_transaction_handle,
            epp_request_params.client_transaction_id,
            disable_epp_notifier_,
            disable_epp_notifier_cltrid_prefix_);

        ccReg::Response_var return_value = new ccReg::Response;
        Corba::wrap_Epp_LocalizedSuccessResponse(response,
                                                 server_transaction_handle,
                                                 return_value);

        /* No exception shall be thrown from here onwards. */
        return return_value._retn();
    }
    catch (const Epp::LocalizedFailResponse &e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
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
  const char* _handle, ccReg::NSSet_out _info_result,
  const ccReg::EppParams &_epp_params)
{
    const std::string server_transaction_handle = Util::make_svtrid( _epp_params.requestID );
    try {

        const Epp::RegistrarSessionData session_data = Epp::get_registrar_session_data(
            epp_sessions,
            Corba::unwrap_epp_request_params(_epp_params).session_id
        );

        const Epp::LocalizedInfoNssetResponse response = Epp::nsset_info(
            Corba::unwrap_string(_handle),
            session_data.registrar_id,
            session_data.language,
            server_transaction_handle
        );

        ccReg::NSSet_var info_result = new ccReg::NSSet( Corba::wrap_localized_info_nsset(response.payload) );
        ccReg::Response_var return_value = new ccReg::Response( Corba::wrap_response(response.ok_response, server_transaction_handle) );

        /* No exception shall be thrown from here onwards. */

        _info_result = info_result._retn();
        return return_value._retn();

    } catch(const Epp::LocalizedFailResponse& e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
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
  const char* _handle, const ccReg::EppParams &_epp_params)
{
    const std::string server_transaction_handle = Util::make_svtrid( _epp_params.requestID );
    try {
        const Epp::RequestParams request_params = Corba::unwrap_epp_request_params(_epp_params);
        const Epp::RegistrarSessionData session_data = Epp::get_registrar_session_data(epp_sessions, request_params.session_id);

        const Epp::LocalizedSuccessResponse response = Epp::nsset_delete(
            Corba::unwrap_string(_handle),
            session_data.registrar_id,
            session_data.language,
            server_transaction_handle,
            request_params.client_transaction_id,
            disable_epp_notifier_cltrid_prefix_
        );

        return new ccReg::Response( Corba::wrap_response(response, server_transaction_handle) );

    } catch(const Epp::LocalizedFailResponse& e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
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
  const char *_handle, const char *authInfoPw, const ccReg::TechContact & tech,
  const ccReg::DNSHost & dns, CORBA::Short level, ccReg::timestamp_out _create_time,
  const ccReg::EppParams &_epp_params)
{
    const std::string server_transaction_handle = Util::make_svtrid( _epp_params.requestID );
    try {
        const Epp::RequestParams request_params = Corba::unwrap_epp_request_params(_epp_params);
        const Epp::RegistrarSessionData session_data = Epp::get_registrar_session_data(epp_sessions, request_params.session_id);

        const Epp::LocalizedCreateNssetResponse response = nsset_create(
            Epp::NssetCreateInputData(
                Corba::unwrap_string_from_const_char_ptr(_handle),
                Corba::unwrap_string_from_const_char_ptr(authInfoPw),
                Corba::unwrap_ccreg_dnshosts_to_vector_dnshosts(dns),
                Corba::unwrap_ccreg_techcontacts_to_vector_string(tech),
                Corba::unwrap_tech_check_level_create(level, this->nsset_level_)
            ),
            session_data.registrar_id,
            request_params.log_request_id,
            session_data.language,
            server_transaction_handle,
            request_params.client_transaction_id,
            disable_epp_notifier_cltrid_prefix_
        );

        ccReg::timestamp_var create_time = Corba::wrap_string_to_corba_string( formatTime( response.crdate ) );
        ccReg::Response_var return_value = new ccReg::Response( Corba::wrap_response(response.ok_response, server_transaction_handle) );

        /* No exception shall be thrown from here onwards. */

        _create_time = create_time._retn();
        return return_value._retn();

    } catch(const Epp::LocalizedFailResponse& e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
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
ccReg_EPP_i::NSSetUpdate(const char* _handle, const char* authInfo_chg,
        const ccReg::DNSHost& dns_add, const ccReg::DNSHost& dns_rem,
        const ccReg::TechContact& tech_add, const ccReg::TechContact& tech_rem,
        CORBA::Short level, const ccReg::EppParams &_epp_params)
{
    const std::string server_transaction_handle = Util::make_svtrid( _epp_params.requestID );
    try {
        const Epp::RequestParams request_params = Corba::unwrap_epp_request_params(_epp_params);
        const Epp::RegistrarSessionData session_data = Epp::get_registrar_session_data(epp_sessions, request_params.session_id);
        const Epp::LocalizedSuccessResponse response = Epp::nsset_update(
                Epp::NssetUpdateInputData(
                    Corba::unwrap_string_from_const_char_ptr(_handle),
                    Corba::convert_corba_string_change(authInfo_chg),
                    Corba::unwrap_ccreg_dnshosts_to_vector_dnshosts(dns_add),
                    Corba::unwrap_ccreg_dnshosts_to_vector_dnshosts(dns_rem),
                    Corba::unwrap_ccreg_techcontacts_to_vector_string(tech_add),
                    Corba::unwrap_ccreg_techcontacts_to_vector_string(tech_rem),
                    Corba::unwrap_tech_check_level_update(level)
                ),
            session_data.registrar_id,
            request_params.log_request_id,
            epp_update_contact_enqueue_check_,
            session_data.language,
            server_transaction_handle,
            request_params.client_transaction_id,
            disable_epp_notifier_cltrid_prefix_
        );

        return new ccReg::Response( Corba::wrap_response(response, server_transaction_handle) );

    } catch(const Epp::LocalizedFailResponse& e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
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

  Fred::Domain::CheckAvailType caType = dman->checkHandle(fqdn, idn_allowed(a));
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
  corba_domain_data_copy(a, regMan.get(), d, dom);
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

    int id, zone;
    short int code = 0;

    EPPAction action(this, params.loginID, EPP_DomainDelete, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    LOGGER(PACKAGE).notice(boost::format("DomainDelete: clientID -> %1% clTRID [%2%] fqdn  [%3%] ") % (int ) params.loginID % static_cast<const char*>(params.clTRID) % fqdn );

    if ( (id = getIdOfDomain(action.getDB(), fqdn, lock_epp_commands_
            , idn_allowed(action), true,  &zone) ) <= 0) {
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
        if (action.getDB()->SaveObjectDelete(id) ) //save object as delete
        {
            if (action.getDB()->DeleteDomainObject(id) )
                code = COMMAND_OK; // if succesfully deleted
        }
        if (code == COMMAND_OK)
        {
            action.set_notification_params(id,Notification::deleted, disable_epp_notifier_);
        }
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

    std::string valexdate;
    ccReg::Disclose publish;
    int id, nssetid, contactid, adminid, keysetid;
    int zone;
    std::vector<int> ac_add, ac_rem, tc_rem;
    unsigned int i, j;
    short int code = 0;

    EPPAction action(this, params.loginID, EPP_DomainUpdate, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

    LOGGER(PACKAGE).notice(boost::format ("DomainUpdate: clientID -> %1% clTRID [%2%] fqdn  [%3%] registrant_chg  [%4%] authInfo_chg [%5%]  nsset_chg [%6%] keyset_chg[%7%] ext.length %8%") %
            (int ) params.loginID % (const char*)params.clTRID % fqdn % registrant_chg % authInfo_chg % nsset_chg % keyset_chg % (long)ext.length() );

    ac_add.resize(admin_add.length());
    ac_rem.resize(admin_rem.length());
    tc_rem.resize(tmpcontact_rem.length());

    // parse enum.Exdate extension
    extractEnumDomainExtension(valexdate, publish, ext);

    if ( (id = getIdOfDomain(action.getDB(), fqdn, lock_epp_commands_
            , idn_allowed(action), true, &zone) ) <= 0) {
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
            {
                action.set_notification_params(id,Notification::updated, disable_epp_notifier_);
            }

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

    std::string valexdate;
    ccReg::Disclose publish;
    std::string FQDN(fqdn);
    boost::to_lower(FQDN);

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

    EPPAction action(this, params.loginID, EPP_DomainCreate, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

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

        dType = dman->checkAvail( FQDN , dConflict, idn_allowed(action));
        const Fred::Zone::Zone* temp_zone = NULL;

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
            case Fred::Domain::CA_AVAILABLE: // if fqdn has valid format
                temp_zone = zm->findApplicableZone(FQDN);
                if(temp_zone != NULL) {
                    zone = temp_zone->getId();
                } else {
                    zone = -1;
                }
                temp_zone = NULL;

                LOG( NOTICE_LOG , "domain %s avail zone %d" ,(const char * ) FQDN.c_str(), zone );
                break;
            case Fred::Domain::CA_BAD_ZONE:
                // domain not in zone
                LOG( NOTICE_LOG , "NOn in zone not applicable %s" , (const char * ) FQDN.c_str() );
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

                    id= action.getDB()->CreateObject("D", action.getRegistrar(), FQDN.c_str(), AuthInfoPw);
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

                            if(rifd_epp_operations_charging_)
                            {
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

                            }
                            else if (action.getDB()->SaveDomainHistory(id, params.requestID)) {
                                if (action.getDB()->SaveObjectCreate(id)) {
                                    code = COMMAND_OK;
                                }
                            } else {
                                code = COMMAND_FAILED;
                            }


                        } else
                            code = COMMAND_FAILED;

                            if (code == COMMAND_OK) // run notifier
                            {
                                action.set_notification_params(id,Notification::created, disable_epp_notifier_);
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

    std::string valexdate;
    ccReg::Disclose publish;
    int id, zone;
    int period_count;
    char periodStr[10];
    short int code = 0;


    EPPAction action(this, params.loginID, EPP_DomainRenew, static_cast<const char*>(params.clTRID), params.XML, params.requestID);

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
            , idn_allowed(action), true, &zone) ) <= 0) {
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

                        if(rifd_epp_operations_charging_)
                        {
                            std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());
                            if (invMan->chargeDomainRenew(zone, action.getRegistrar(),
                                        id, Database::Date(std::string(exDate)), period_count) == false ) {
                                code = COMMAND_BILLING_FAILURE;
                            } else if (action.getDB()->SaveDomainHistory(id, params.requestID)) {
                                code = COMMAND_OK;
                            }
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
        action.set_notification_params(id,Notification::renewed, disable_epp_notifier_);
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
ccReg::Response*
ccReg_EPP_i::KeySetInfo(
        const char* const _keyset_handle,
        ccReg::KeySet_out _keyset_info,
        const ccReg::EppParams &_epp_params)
{
    const Epp::RequestParams epp_request_params = Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();
    try {
        const Epp::RegistrarSessionData session_data =
            Epp::get_registrar_session_data(this->epp_sessions, epp_request_params.session_id);

        const std::string keyset_handle = Corba::unwrap_string_from_const_char_ptr(_keyset_handle);
        const Epp::KeySet::Localized::InfoResult info_result =
            Epp::KeySet::Localized::info(keyset_handle,
                                         session_data.registrar_id,
                                         session_data.language,
                                         server_transaction_handle);

        ccReg::KeySet_var keyset = new ccReg::KeySet;
        Corba::wrap_Epp_KeySet_Localized_InfoData(info_result.data, keyset);

        ccReg::Response_var return_value = new ccReg::Response;
        Corba::wrap_Epp_LocalizedSuccessResponse(info_result.response,
                                                 server_transaction_handle,
                                                 return_value);

        /* No exception shall be thrown from here onwards. */
        _keyset_info = keyset._retn();
        return return_value._retn();
    }
    catch (const Epp::LocalizedFailResponse &e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
}

/*************************************************************
 *
 * Function:    KeySetDelete
 *
 *************************************************************/

ccReg::Response*
ccReg_EPP_i::KeySetDelete(
        const char *_keyset_handle,
        const ccReg::EppParams &_epp_params)
{
    const Epp::RequestParams epp_request_params = Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();
    try {
        const Epp::RegistrarSessionData session_data =
            Epp::get_registrar_session_data(this->epp_sessions, epp_request_params.session_id);

        const std::string keyset_handle = Corba::unwrap_string_from_const_char_ptr(_keyset_handle);
        const Epp::LocalizedSuccessResponse response = Epp::KeySet::localized_delete(
            keyset_handle,
            session_data.registrar_id,
            session_data.language,
            server_transaction_handle,
            epp_request_params.client_transaction_id,
            disable_epp_notifier_,
            disable_epp_notifier_cltrid_prefix_);

        ccReg::Response_var return_value = new ccReg::Response;
        Corba::wrap_Epp_LocalizedSuccessResponse(response,
                                                 server_transaction_handle,
                                                 return_value);

        /* No exception shall be thrown from here onwards. */
        return return_value._retn();
    }
    catch (const Epp::LocalizedFailResponse &e) {
        throw Corba::wrap_error(e, server_transaction_handle);
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

/*************************************************************
 *
 * Function:    KeySetCreate
 *
 *************************************************************/
ccReg::Response *
ccReg_EPP_i::KeySetCreate(
        const char *_keyset_handle,
        const char *_auth_info_pw,
        const ccReg::TechContact &_tech_contacts,
        const ccReg::DSRecord &_ds_records,
        const ccReg::DNSKey &_dns_keys,
        ccReg::timestamp_out _crdate,
        const ccReg::EppParams &_epp_params)
{
    const Epp::RequestParams epp_request_params = Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();
    try {
        const Epp::RegistrarSessionData session_data =
            Epp::get_registrar_session_data(this->epp_sessions, epp_request_params.session_id);

        const std::string keyset_handle =
            Corba::unwrap_string_from_const_char_ptr(_keyset_handle);
        const std::string auth_info_pw_value =
            Corba::unwrap_string_from_const_char_ptr(_auth_info_pw);
        const Optional< std::string > auth_info_pw = auth_info_pw_value.empty() ? Optional< std::string >()
                                                                                : auth_info_pw_value;
        const std::vector< std::string > tech_contacts =
            Corba::unwrap_TechContact_to_vector_string(_tech_contacts);
        const std::vector< Epp::KeySet::DsRecord > ds_records =
            Corba::unwrap_ccReg_DSRecord_to_vector_Epp_KeySet_DsRecord(_ds_records);
        const std::vector< Epp::KeySet::DnsKey > dns_keys =
            Corba::unwrap_ccReg_DNSKey_to_vector_Epp_KeySet_DnsKey(_dns_keys);

        const Epp::KeySet::Localized::ResponseOfCreate response = Epp::KeySet::Localized::create(
            keyset_handle,
            auth_info_pw,
            tech_contacts,
            ds_records,
            dns_keys,
            session_data.registrar_id,
            epp_request_params.log_request_id,
            session_data.language,
            server_transaction_handle,
            epp_request_params.client_transaction_id,
            disable_epp_notifier_,
            disable_epp_notifier_cltrid_prefix_);

        ccReg::timestamp_var create_time = formatTime(response.crdate).c_str();
        ccReg::Response_var return_value = new ccReg::Response;
        Corba::wrap_Epp_LocalizedSuccessResponse(response.ok_response,
                                                 server_transaction_handle,
                                                 return_value);

        /* No exception shall be thrown from here onwards. */
        _crdate = create_time._retn();
        return return_value._retn();
    }
    catch (const Epp::LocalizedFailResponse &e) {
        throw Corba::wrap_error(e, server_transaction_handle);
    }
}

/*************************************************************
 *
 * Function:    KeySetUpdate
 *
 *************************************************************/
ccReg::Response *
ccReg_EPP_i::KeySetUpdate(
        const char *_keyset_handle,
        const char *_auth_info_pw,
        const ccReg::TechContact &_tech_contacts_add,
        const ccReg::TechContact &_tech_contacts_rem,
        const ccReg::DSRecord &_ds_records_add,
        const ccReg::DSRecord &_ds_records_rem,
        const ccReg::DNSKey &_dns_keys_add,
        const ccReg::DNSKey &_dns_keys_rem,
        const ccReg::EppParams &_epp_params)
{
    const Epp::RequestParams epp_request_params = Corba::unwrap_EppParams(_epp_params);
    const std::string server_transaction_handle = epp_request_params.get_server_transaction_handle();
    try {
        const Epp::RegistrarSessionData session_data =
            Epp::get_registrar_session_data(this->epp_sessions, epp_request_params.session_id);

        const std::string keyset_handle =
            Corba::unwrap_string_from_const_char_ptr(_keyset_handle);
        const Optional< std::string > auth_info_pw =
            Corba::unwrap_string_for_change_to_Optional_string(_auth_info_pw);
        const std::vector< std::string > tech_contacts_add =
            Corba::unwrap_TechContact_to_vector_string(_tech_contacts_add);
        const std::vector< std::string > tech_contacts_rem =
            Corba::unwrap_TechContact_to_vector_string(_tech_contacts_rem);
        const std::vector< Epp::KeySet::DsRecord > ds_records_add =
            Corba::unwrap_ccReg_DSRecord_to_vector_Epp_KeySet_DsRecord(_ds_records_add);
        const std::vector< Epp::KeySet::DsRecord > ds_records_rem =
            Corba::unwrap_ccReg_DSRecord_to_vector_Epp_KeySet_DsRecord(_ds_records_rem);
        const std::vector< Epp::KeySet::DnsKey > dns_keys_add =
            Corba::unwrap_ccReg_DNSKey_to_vector_Epp_KeySet_DnsKey(_dns_keys_add);
        const std::vector< Epp::KeySet::DnsKey > dns_keys_rem =
            Corba::unwrap_ccReg_DNSKey_to_vector_Epp_KeySet_DnsKey(_dns_keys_rem);

        const Epp::LocalizedSuccessResponse response = Epp::KeySet::Localized::update(
            keyset_handle,
            auth_info_pw,
            tech_contacts_add,
            tech_contacts_rem,
            ds_records_add,
            ds_records_rem,
            dns_keys_add,
            dns_keys_rem,
            session_data.registrar_id,
            epp_request_params.log_request_id,
            session_data.language,
            server_transaction_handle,
            epp_request_params.client_transaction_id,
            disable_epp_notifier_,
            disable_epp_notifier_cltrid_prefix_);

        ccReg::Response_var return_value = new ccReg::Response;
        Corba::wrap_Epp_LocalizedSuccessResponse(response,
                                                 server_transaction_handle,
                                                 return_value);

        /* No exception shall be thrown from here onwards. */
        return return_value._retn();
    }
    catch (const Epp::LocalizedFailResponse &e) {
        throw Corba::wrap_error(e, server_transaction_handle);
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

  Database::Connection conn = Database::Manager::acquire();
  DBSharedPtr DBsql (new DB(conn));

  if ( (regID = GetRegistrarID(params.loginID) ))
    {

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
    int zone = 0;
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

    const Fred::Zone::Zone* temp_zone = NULL;

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

            // static check of fqdn format (no db)
            try {
                zm->parseDomainName(FQDN, dev_null, idn_allowed(action));
            } catch(Fred::Zone::INVALID_DOMAIN_NAME&) {
                zone = -1;
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_fqdn, 1,
                        REASON_MSG_BAD_FORMAT_FQDN);
            }

            // if fqdn is ok (see catch block above) ...
            if(zone != -1) {
                temp_zone = zm->findApplicableZone(FQDN);
            }
            // no zone was found for given fqdn
            if(temp_zone == NULL) {
                zone = 0;
                code = action.setErrorReason(COMMAND_PARAMETR_ERROR,
                        ccReg::domain_fqdn, 1,
                        REASON_MSG_NOT_APPLICABLE_DOMAIN);

            // bingo, we got the zone
            } else {
                zone = temp_zone->getId();
                // safety measure
                temp_zone = NULL;
                LOG(WARNING_LOG, "domain in zone %s", name);
            }

            // we still don't know whether the name is actually in register
            if(zone > 0) {
                if ( (id = action.getDB()->GetDomainID(FQDN.c_str(), GetZoneEnum(action.getDB(), zone) ) ) == 0) {
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
