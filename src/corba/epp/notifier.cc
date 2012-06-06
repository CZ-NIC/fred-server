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

#include <vector>
#include <string>
#include <sstream>

#include "notifier.h"
#include "old_utils/log.h"
#include "log/logger.h"

// mailer manager
#include "fredlib/mailer.h"
#include "fredlib/contact_diff.h"
#include "fredlib/domain_diff.h"
#include "fredlib/nsset_diff.h"
#include "fredlib/keyset_diff.h"


//#include "notifier_changes.h"



#define MAX_SQLSTRING 512

EPPNotifier::EPPNotifier(bool _disable, 
                         MailerManager *mailManager, 
                         DBSharedPtr  db_disconnect_guard,
                         unsigned int regid, 
                         unsigned int objectid,
                         Fred::Manager *_rm)
{
  disable = _disable;
  mm=mailManager;
  db=db_disconnect_guard;
  enum_action=db->GetEPPAction(); // id of the EPP operation
  objectID=objectid;
  registrarID=regid;

  rm_ = _rm;

  messages_ready_ = false;

  LOG( DEBUG_LOG ,"EPPNotifier:  object %d  enum_action %d regID %d " , objectID , enum_action , registrarID );

  LOG( DEBUG_LOG ,"EPPNotifier: add default contacts");

  AddDefault(); // add default contacts
}

EPPNotifier::~EPPNotifier()
{
  enum_action=0;
  objectID=0;
  registrarID=0;
}

bool EPPNotifier::Send()
{
  if (!notify.size() || disable)
    return true;

  try {
    constructMessages();
  
    if (mm->checkEmailList(emails)) {
      LOG(DEBUG_LOG, "EPPNotifier: TO: %s" , emails.c_str() );
    }
    else {
      LOG(DEBUG_LOG, "EPPNotifier: TO: no valid email address found - not sending");
      return false;
    }

    //#6547 send only if there are some changes
    //"changes" key is added in constructMessages()
    Fred::Mailer::Parameters::const_iterator params_changes_it
        = params.find("changes");
    if((params_changes_it != params.end())
        && (params_changes_it->second.compare("0") == 0))
    {
        LOG(DEBUG_LOG, "EPPNotifier: update request - no changes found - not sending");
        return false;
    }
    else
    {
        // mailer manager send emails
        mm->sendEmail("", emails, "", getTemplate(), params, handles, attach);
    }



  }
  catch (...) {
    LOGGER(PACKAGE).error(boost::format("EPPNotifier: notification for '%1%' failed! "
                "(object_id=%2% action=%3% registrar_id=%4%)")
                % emails % objectID % enum_action % registrarID);
    return false;
  }

  return true;
}

/* Ticket #1622 */
void EPPNotifier::constructMessages() {
  /* check if messages have been constructed already */
  if (messages_ready_)
    return;

  /* construct messages */
  unsigned int i, num;
  short type, mod;
  unsigned int cID;

  // 4 parameters  type of the object name  ticket svTRID and  handle of  registrar
  params["ticket"] = db->GetsvTRID();
  std::stringstream registrarInfo;
  registrarInfo << db->GetValueFromTable("registrar", "name", "id", registrarID);
  registrarInfo << " ("
                << db->GetValueFromTable("registrar", "url", "id",registrarID)
                << ")";
  params["registrar"] = registrarInfo.str(); // registrar name and url
  params["handle"] = db->GetValueFromTable("object_registry", "name", "id", objectID); // name of the object
  params["type"] = db->GetValueFromTable("object_registry", "type", "id", objectID);   // type 1 contact 2 nsset 3 domain 4 keyset

  if (rm_ && (enum_action == EPP_ContactUpdate ||
              enum_action == EPP_DomainUpdate  ||
              enum_action == EPP_NSsetUpdate   ||
              enum_action == EPP_KeySetUpdate)) {
    try {
      Fred::ChangesMap values;

      if (enum_action == EPP_ContactUpdate) {
          values = Fred::Contact::diff_last_history(rm_->getContactManager(), objectID);
      }
      else if (enum_action == EPP_DomainUpdate) {
          values = Fred::Domain::diff_last_history(rm_->getDomainManager(), objectID);
      }
      else if (enum_action == EPP_NSsetUpdate) {
          values = Fred::NSSet::diff_last_history(rm_->getNSSetManager(), objectID);
      }
      else if (enum_action == EPP_KeySetUpdate) {
          values = Fred::KeySet::diff_last_history(rm_->getKeySetManager(), objectID);
      }

      params["changes"] = values.size() > 0 ? "1" : "0";

      Fred::ChangesMap::const_iterator it = values.begin();
      for (; it != values.end(); ++it) {
        params["changes." + it->first] = "1";
        params["changes." + it->first + ".old"] = it->second.first;
        params["changes." + it->first + ".new"] = it->second.second;
      }
    }
    catch (Database::Exception &ex) {
        LOGGER(PACKAGE).error(boost::format("EPPNotifier: update changes - database error => %1%") % ex.what());
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("EPPNotifier: update changes (%1%)") % _ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("EPPNotifier: update changes - unknown exception");
    }
  }

  LOG( DEBUG_LOG ,"EPPNotifier: Send object %d  enum_action %d regID %d ticket %s" , objectID , enum_action , registrarID , db->GetsvTRID() );

  num = notify.size();
  for (i = 0; i < num; i ++) {
    cID = notify[i].contactID;
    type = notify[i].type;
    mod = notify[i].modify;

    std::string objectName = db->GetValueFromTable("object_registry", "name", "id", cID);
    LOG(DEBUG_LOG, "EPPNotifier: sendTo %s %s contactID %d [%s]",
        GetContactType(type), GetContactModify(mod), cID, objectName.c_str());

    std::string cEmail = db->GetValueFromTable("contact", "email", "id", cID);
    std::string cNotifyEmail = db->GetValueFromTable("contact", "notifyemail", "id", cID);
    LOG(DEBUG_LOG, "EPPNotifier: email %s notifyEmail %s ",
        cEmail.c_str(), cNotifyEmail.c_str());

    if (!cNotifyEmail.empty()) {
      if (!emails.empty())
        emails += ", ";
      emails += cNotifyEmail;
    }
    emails += " " + extraEmails;
  }

  messages_ready_ = true;
}

void EPPNotifier::AddContactID(
  unsigned int contactID, short type, short mod)
{
  NotifyST n;

  LOG( DEBUG_LOG ,"EPPNotifier: AddContactID %d typ[%d] mod[%d]" , contactID , type , mod );
  n.contactID=contactID;
  n.type=type;
  n.modify=mod;
  notify.push_back(n);
}

// addall  tech contact of  nsset  linked with domain with  domainID
void EPPNotifier::AddNSSetTechByDomain(
  unsigned int domainID)
{
  // SQL select by NSSETID
  AddNSSetTech(db->GetNumericFromTable("domain", "nsset", "id", domainID) );
}

void EPPNotifier::AddNSSetTech(
  unsigned int nssetID)
{
  int i, num;
  char sqlString[128];

  snprintf(sqlString, sizeof(sqlString), 
      "SELECT  contactid  from nsset_contact_map where nssetid=%d", nssetID);

  if (db->ExecSelect(sqlString) ) {
    num = db->GetSelectRows();
    for (i = 0; i < num; i ++)
      AddContactID(atoi(db->GetFieldValue(i, 0) ) , TECH_CONTACT, 0);
    db->FreeSelect();
  }

}

//add all tech contact of keyset linked with domain (identified by domainID)
void
EPPNotifier::AddKeySetTechByDomain(unsigned int domainID)
{
    AddKeySetTech(db->GetNumericFromTable("domain", "keyset", "id", domainID));
}
void
EPPNotifier::AddKeySetTech(unsigned int keysetID)
{
    int i, num;
    char sqlString[128];

    snprintf(sqlString, sizeof(sqlString),
            "SELECT contactid FROM keyset_contact_map WHERE keysetid=%d",
            keysetID);

    if (db->ExecSelect(sqlString)) {
        num = db->GetSelectRows();
        for (i = 0; i < num; i++)
            AddContactID(atoi(db->GetFieldValue(i, 0)), KEY_CONTACT, 0);
        db->FreeSelect();
    }
}


void EPPNotifier::AddDomainAdmin(
  unsigned int domainID)
{
  int i, num;
  char sqlString[128];

  /* Ticket #6684 - remove temp-c notification */
  snprintf(sqlString, sizeof(sqlString),
      "SELECT  contactid  from domain_contact_map where domainid=%d and role = 1", domainID);

  if (db->ExecSelect(sqlString) ) {
    num = db->GetSelectRows();
    for (i = 0; i < num; i ++)
      AddContactID(atoi(db->GetFieldValue(i, 0) ) , ADMIN_CONTACT, 0);
    db->FreeSelect();
  }

}

// add owner of domain
void EPPNotifier::AddDomainRegistrant(
  unsigned int domainID)
{
  unsigned int contactID;

  contactID = db->GetNumericFromTable("domain", "registrant", "id", domainID);
  LOG( DEBUG_LOG ,"EPPNotifier: AddDomainRegistrant domainID %d contactID %d" , domainID , contactID);

  AddContactID(contactID, REGISTRANT_CONTACT, 0);
}

