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

#include "notify.h"
#include "old_utils/dbsql.h"
#include "old_utils/log.h"
#include "sql.h"
#include <sstream>

namespace Register
{
  namespace Notify
  {
    // Local transction needed for proper on commit handling of TEMP table
    struct LocalTransaction {
      DB *db;
      bool closed;
      LocalTransaction(DB *_db) : db(_db), closed(false)
      {
        (void)db->BeginTransaction();
      }
      ~LocalTransaction()
      {
        if (!closed)
          db->RollbackTransaction();
      }
      void commit()
      {
        db->CommitTransaction();
        closed = true;
      }
    };

    class ManagerImpl : virtual public Manager
    {
      DB *db;
      Mailer::Manager *mm;
      Contact::Manager *cm;
      NSSet::Manager *nm;
      KeySet::Manager *km;
      Domain::Manager *dm;
      Document::Manager *docm;
      Registrar::Manager *rm;
     public:
      ManagerImpl(
        DB *_db,
        Mailer::Manager *_mm,
        Contact::Manager *_cm,
        NSSet::Manager *_nm,
        KeySet::Manager *_km,
        Domain::Manager *_dm,
        Document::Manager *_docm,
        Registrar::Manager *_rm
      ): db(_db), mm(_mm), cm(_cm), nm(_nm), dm(_dm), docm(_docm), rm(_rm)
      {}
      std::string getEmailList(const std::stringstream& sql) throw (SQL_ERROR)
      {
        std::string mailList;
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          if (!mailList.empty()) mailList += ",";
          mailList += db->GetFieldValue(i,0);
        }
        db->FreeSelect();
        return mailList;
      }
      std::string getDomainAdminEmailsHistory(TID domain)
      {
        std::stringstream sql;
        sql << "SELECT ch.email "
            << "FROM object_registry dor, domain_history dh, "
            << "object_registry cor, contact_history ch "
            << "WHERE dor.historyid=dh.historyid AND dh.registrant=cor.id "
            << "AND cor.historyid=ch.historyid "
            << "AND dor.id=" << domain << " "
            << "UNION "
            << "SELECT ch.email "
            << "FROM contact_history ch, object_registry cor, "
            << "domain_contact_map_history dcm, object_registry dor "
            << "WHERE ch.historyid=cor.historyid AND cor.id=dcm.contactid "
            << "AND dcm.historyid=dor.historyid AND dor.id=" << domain;
        return getEmailList(sql);
      }
      std::string getDomainAdminEmails(TID domain)
      {
        std::stringstream sql;
        sql << "SELECT c.email "
            << "FROM domain d, contact c "
            << "WHERE d.registrant=c.id AND d.id=" << domain << " "
            << "UNION "
            << "SELECT c.email "
            << "FROM domain_contact_map dcm, contact c "
            << "WHERE dcm.contactid=c.id AND dcm.domainid=" << domain;
        return getEmailList(sql);
      }
      std::string getDomainTechEmailsHistory(TID domain)
      {
        std::stringstream sql;
        sql << "SELECT ch.email "
            << "FROM contact_history ch, object_registry cor, "
            << "nsset_contact_map_history ncm, object_registry nor, "
            << "domain_history dh, object_registry dor "
            << "WHERE ch.historyid=cor.historyid AND cor.id=ncm.contactid "
            << "AND ncm.historyid=nor.historyid AND nor.id=dh.nsset "
            << "AND dh.historyid=dor.historyid AND dor.id=" << domain;
        return getEmailList(sql);
      }
      std::string getDomainTechEmails(TID domain)
      {
        std::stringstream sql;
        sql << "SELECT c.email "
            << "FROM domain d, nsset_contact_map ncm, contact c "
            << "WHERE d.nsset=ncm.nssetid AND ncm.contactid=c.id "
            << "AND d.id=" << domain;
        return getEmailList(sql);
      }
      std::string getNSSetTechEmails(TID nsset)
      {
        std::stringstream sql;
        sql << "SELECT c.email "
            << "FROM nsset_contact_map ncm, contact c "
            << "WHERE ncm.contactid=c.id AND ncm.nssetid=" << nsset;
        return getEmailList(sql);
      }
      std::string getKeySetTechEmails(TID keyset)
      {
          std::stringstream sql;
          sql << "SELECT c.email "
              << "FROM keyset_contact_map kcm, contact c "
              << "WHERE kcm.contactid=c.id AND kcm.keysetid="
              << keyset;
          return getEmailList(sql);
      }
      std::string getContactEmails(TID contact)
      {
        std::stringstream sql;
        sql << "SELECT c.email FROM contact c WHERE c.id=" << contact;
        return getEmailList(sql);
      }
      void fillDomainParamsHistory(
        TID domain, ptime stamp,
        Register::Mailer::Parameters& params
      ) throw (SQL_ERROR)
      {
        std::stringstream sql;
        sql << "SELECT dom.name, cor.name, nor.name, r.handle, "
            << "eh.exdate, dh.exdate "
            << "FROM object_registry dom, object_history doh, "
            << "registrar r, object_registry cor, domain_history dh "
            << "LEFT JOIN object_registry nor ON (nor.id=dh.nsset) "
            << "LEFT JOIN enumval_history eh ON (eh.historyid=dh.historyid) "
            << "WHERE dom.historyid=doh.historyid AND doh.clid=r.id "
            << "AND dom.historyid=dh.historyid AND dh.registrant=cor.id "
            << "AND dom.id=" << domain;
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        params["checkdate"] = to_iso_extended_string(
          date(day_clock::local_day())
        );
        params["domain"] = db->GetFieldValue(0,0);
        params["owner"] = db->GetFieldValue(0,1);
        params["nsset"] = db->GetFieldValue(0,2);
        date val(MAKE_DATE(0,4));
        if (!val.is_special())
          params["valdate"] = to_iso_extended_string(val);
        date ex(MAKE_DATE(0,5));
        params["exdate"] = to_iso_extended_string(ex);
        params["dnsdate"] = to_iso_extended_string(
          ex + date_duration(30)
        );
        params["exregdate"] = to_iso_extended_string(
          ex + date_duration(45)
        );
        params["statechangedate"] = to_iso_extended_string(stamp.date());
        std::string regHandle = db->GetFieldValue(0,3);
        db->FreeSelect();
        // fill information about registrar, query must be closed
        rm->getList()->clearFilter();
        rm->getList()->setHandleFilter(regHandle);
        rm->getList()->reload();
        std::stringstream reg;
        if (rm->getList()->size() != 1)
          // fallback to handle instead of error
          reg << db->GetFieldValue(0,3);
        else {
          reg << rm->getList()->get(0)->getName();
          if (!rm->getList()->get(0)->getURL().empty())
            reg << " (" << rm->getList()->get(0)->getURL() << ")";
        }
        params["registrar"] = reg.str();
      }
      void fillDomainParams(
        TID domain, ptime stamp,
        Register::Mailer::Parameters& params
      ) throw (SQL_ERROR)
      {
        std::auto_ptr<Register::Domain::List> dlist(dm->createList());
        dlist->setIdFilter(domain);
        dlist->reload();
        if (dlist->getCount() != 1) throw SQL_ERROR();
        Register::Domain::Domain *d = dlist->getDomain(0);
        // fill params
        params["checkdate"] = to_iso_extended_string(
          date(day_clock::local_day())
        );
        params["domain"] = d->getFQDN();
        params["owner"] = d->getRegistrantHandle();
        params["nsset"] = d->getNSSetHandle();
        if (!d->getValExDate().is_special())
          params["valdate"] = to_iso_extended_string(d->getValExDate());
        params["exdate"] = to_iso_extended_string(d->getExpirationDate());
        params["dnsdate"] = to_iso_extended_string(
          d->getExpirationDate() + date_duration(30)
        );
        params["exregdate"] = to_iso_extended_string(
          d->getExpirationDate() + date_duration(45)
        );
        params["statechangedate"] = to_iso_extended_string(stamp.date());
        // fill information about registrar
        rm->getList()->clearFilter();
        rm->getList()->setHandleFilter(d->getRegistrarHandle());
        rm->getList()->reload();
        std::stringstream reg;
        if (rm->getList()->size() != 1)
          // fallback to handle instead of error
          reg << db->GetFieldValue(0,3);
        else {
          reg << rm->getList()->get(0)->getName();
          if (!rm->getList()->get(0)->getURL().empty())
            reg << " (" << rm->getList()->get(0)->getURL() << ")";
        }
        params["registrar"] = reg.str();
      }
      void fillSimpleObjectParams(
        TID id,
        Register::Mailer::Parameters& params
      ) throw (SQL_ERROR)
      {
        std::stringstream sql;
        sql << "SELECT c.name, c.type FROM object_registry c WHERE c.id=" << id;
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        params["handle"] = db->GetFieldValue(0,0);
        params["type"] = db->GetFieldValue(0,1);
        params["deldate"] = to_iso_extended_string(
          date(day_clock::local_day())
        );
      }
      void saveNotification(TID state, unsigned notifyType, TID mail)
        throw (SQL_ERROR)
      {
        std::stringstream sql;
        sql << "INSERT INTO notify_statechange (state_id,type,mail_id) "
            << "VALUES ("
            << state << "," << notifyType << ",";
        if (mail) sql << mail;
        else sql << "NULL";
        sql << ")";
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
      }
      struct NotifyRequest {
        TID state_id; ///< id of state change (not id of status)
        unsigned type; ///< notification id
        std::string mtype; ///< template name
        unsigned emails; ///< emails flag (1=normal(admins), 2=techs)
        TID obj_id; ///< id of object
        unsigned obj_type; ///< type of object (domain, contact...)
        ptime stamp; ///< time of state change event
        NotifyRequest(
          TID _state_id, unsigned _type, const std::string& _mtype,
          unsigned _emails, TID _obj_id, unsigned _obj_type, ptime _stamp
        ) :
          state_id(_state_id), type(_type), mtype(_mtype), emails(_emails),
          obj_id(_obj_id), obj_type(_obj_type), stamp(_stamp)
        {}
      };
      void notifyStateChanges(
        const std::string& exceptList,
        unsigned limit,
        std::ostream *debugOutput,
        bool useHistory
      ) throw (SQL_ERROR)
      {
        TRACE("[CALL] Register::Notify::notifyStateChanges()");
        std::stringstream sql;
        sql << "SELECT nt.state_id, nt.type, "
            << "nt.mtype, nt.emails, nt.obj_id, nt.obj_type, nt.valid_from "
            << "FROM "
            << "(SELECT s.id AS state_id, nm.id AS type, "
            << " mt.name AS mtype, nm.emails, "
            << " obr.id AS obj_id, obr.type AS obj_type, s.valid_from "
            << " FROM object_state s, object_registry obr, "
            << " notify_statechange_map nm, mail_type mt "
            << " WHERE s.object_id=obr.id AND obr.type=nm.obj_type "
            << " AND s.state_id=nm.state_id AND s.valid_to ISNULL "
            << " AND mt.id=nm.mail_type_id) AS nt "
            << "LEFT JOIN notify_statechange ns ON ("
            << "nt.state_id=ns.state_id AND nt.type=ns.type) "
            << "WHERE ns.state_id ISNULL ";
        if (!exceptList.empty())
        	sql << "AND nt.type NOT IN (" << exceptList << ") ";
        sql << "ORDER BY nt.state_id ASC ";
        if (limit)
        	sql << "LIMIT " << limit;
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        std::vector<NotifyRequest> nlist;
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++)
          nlist.push_back(NotifyRequest(
            STR_TO_ID(db->GetFieldValue(i,0)),
            atoi(db->GetFieldValue(i,1)),
            db->GetFieldValue(i,2),
            atoi(db->GetFieldValue(i,3)),
            STR_TO_ID(db->GetFieldValue(i,4)),
            atoi(db->GetFieldValue(i,5)),
            MAKE_TIME(i,6) /// Once should handle timestamp conversion
          ));
        db->FreeSelect();
        if (debugOutput) *debugOutput << "<notifications>" << std::endl;
        std::vector<NotifyRequest>::const_iterator i = nlist.begin();
        for (;i!=nlist.end();i++) {
          Register::Mailer::Parameters params;
          // handles are obsolete
          Register::Mailer::Handles handles;
          // these mails has no attachments
          Register::Mailer::Attachments attach;
          std::string emails;
          try {
            switch (i->obj_type) {
             case 1: // contact
              fillSimpleObjectParams(i->obj_id,params);
              emails = getContactEmails(i->obj_id);
              break;
             case 2: // nsset
              fillSimpleObjectParams(i->obj_id,params);
              emails = getNSSetTechEmails(i->obj_id);
              break;
             case 3: // domain
               if (useHistory) {
                 fillDomainParamsHistory(i->obj_id,i->stamp,params);
                 emails =
                   (i->emails == 1 ? getDomainAdminEmailsHistory(i->obj_id) :
                                     getDomainTechEmailsHistory(i->obj_id));
               }
               else {
                 fillDomainParams(i->obj_id,i->stamp,params);
                 emails =
                   (i->emails == 1 ? getDomainAdminEmails(i->obj_id) :
                    getDomainTechEmails(i->obj_id));
               }
             case 4: // keyset
              fillSimpleObjectParams(i->obj_id,params);
              emails = getKeySetTechEmails(i->obj_id);
              break;
            }
            if (debugOutput) {
              *debugOutput << "<notify>"
                           << "<emails>" << emails << "</emails>"
                           << "<template>" << i->mtype << "</template>";
              Register::Mailer::Parameters::const_iterator ci = params.begin();
              for (;ci!=params.end();ci++)
                *debugOutput << "<param><name>" << ci->first << "</name>"
                             << "<value>" << ci->second << "</value></param>";
              *debugOutput << "</notify>" << std::endl;
            } else {
              TID mail = emails.empty() ? 0 : mm->sendEmail(
                "",emails,"",i->mtype,params,handles,attach
              );
              saveNotification(i->state_id,i->type,mail);
            }
          }
          catch (...) {
            LOG(
              ERROR_LOG,
              "Notfication wasn't successful (state=%d, type=%d) ",
              i->state_id, i->type
            );
          }
        }
        if (debugOutput) *debugOutput << "</notifications>" << std::endl;
      }
#define XML_DB_OUT(x,y) "<![CDATA[" << db->GetFieldValue(x,y) << "]]>"
#define WARNING_LETTER_FILE_TYPE 5 // from enum_filetype table
      virtual void generateLetters()
        throw (SQL_ERROR)
      {
        TRACE("[CALL] Register::Notify::generateLetters()");
    	// transaction is needed for 'ON COMMIT DROP' functionality
    	LocalTransaction trans(db);
    	// because every expiration date is
        // generated into separate PDF, there are two SQL queries.
        // first for getting expiration dates and second for real data
        // to fixate set of states between these two queries temporary
        // table is used
        // create temporary table
        const char *create =
          "CREATE TEMPORARY TABLE tmp_notify_letters ("
          " state_id INTEGER PRIMARY KEY "
          ") ON COMMIT DROP ";
        // populate temporary table with states to notify
        if (!db->ExecSQL(create)) throw SQL_ERROR();
        const char *fixateStates =
          "INSERT INTO tmp_notify_letters "
          "SELECT s.id FROM object_state s "
          "LEFT JOIN notify_letters nl ON (s.id=nl.state_id) "
          "WHERE s.state_id=19 AND s.valid_to ISNULL AND nl.state_id ISNULL ";
        if (!db->ExecSQL(fixateStates)) throw SQL_ERROR();
        // select all expiration dates of domain to notify
        const char *selectExDates =
          "SELECT DISTINCT dh.exdate::date "
          "FROM tmp_notify_letters tnl, object_state s, domain_history dh "
          "WHERE tnl.state_id=s.id AND s.ohid_from=dh.historyid ";
        if (!db->ExecSelect(selectExDates)) throw SQL_ERROR();
        std::vector<std::string> exDates;
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++)
          exDates.push_back(db->GetFieldValue(i,0));
        db->FreeSelect();
        // for every expiration date generate PDF
        for (unsigned j=0; j<exDates.size(); j++) {
          std::stringstream filename;
          filename << "letter-" << exDates[j] << ".pdf";
          std::auto_ptr<Document::Generator> gPDF(
            docm->createSavingGenerator(
              Document::GT_WARNING_LETTER,
              filename.str(),WARNING_LETTER_FILE_TYPE,
              "" // default language
            )
          );
          std::ostream& out(gPDF->getInput());
          std::stringstream sql;
          sql << "SELECT dobr.name,r.name || ' (' || r.url || ')',CURRENT_DATE,"
              << "d.exdate::date + INTERVAL '45 days',cor.name,"
              << "CASE WHEN TRIM(COALESCE(c.organization,''))='' THEN c.name "
              << "     ELSE c.organization END, "
              << "TRIM(COALESCE(c.street1,'') || ' ' || "
              << "COALESCE(c.street2,'') || ' ' || "
              << "COALESCE(c.street3,'')), "
              << "c.city, c.postalcode, ec.country "
              << "FROM enum_country ec, contact_history c, "
              << "object_registry cor, registrar r, "
              << "object_registry dobr, object_history doh, domain_history d, "
              << "object_state s, tmp_notify_letters tnl  "
              << "WHERE ec.id=c.country AND c.historyid=cor.historyid "
              << "AND cor.id=d.registrant "
              << "AND d.historyid=s.ohid_from AND dobr.id=d.id "
              << "AND doh.historyid=d.historyid AND tnl.state_id=s.id "
              << "AND d.exdate::date='" << exDates[j] << "' AND doh.clid=r.id "
              << " ORDER BY CASE WHEN c.country='CZ' THEN 0 ELSE 1 END ASC, "
              << "          c.country";
          if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
          out << "<messages>";
          for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++)
            out << "<message>"
                << "<domain>" << XML_DB_OUT(i,0) << "</domain>"
                << "<registrar>" << XML_DB_OUT(i,1) << "</registrar>"
                << "<actual_date>" << XML_DB_OUT(i,2) << "</actual_date>"
                << "<termination_date>" << XML_DB_OUT(i,3)
                << "</termination_date>"
                << "<holder>"
                << "<handle>" << XML_DB_OUT(i,4) << "</handle>"
                << "<name>" << XML_DB_OUT(i,5) << "</name>"
                << "<street>" << XML_DB_OUT(i,6) << "</street>"
                << "<city>" << XML_DB_OUT(i,7) << "</city>"
                << "<zip>" << XML_DB_OUT(i,8) << "</zip>"
                << "<country>" << XML_DB_OUT(i,9) << "</country>"
                << "</holder>"
                << "</message>";
          db->FreeSelect();
          out << "</messages>";
          // return id of generated PDF file
          TID filePDF = gPDF->closeInput();
          sql.str("");
          sql << "INSERT INTO notify_letters "
              << "SELECT tnl.state_id, " << filePDF << " "
              << "FROM tmp_notify_letters tnl, object_state s, "
              << "domain_history dh "
              << "WHERE tnl.state_id=s.id AND s.ohid_from=dh.historyid "
              << "AND dh.exdate::date='" << exDates[j] << "'";
          if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
        }
        trans.commit();
      }
    };
    Manager *Manager::create(
      DB *db,
      Mailer::Manager *mm,
      Contact::Manager *cm,
      NSSet::Manager *nm,
      KeySet::Manager *km,
      Domain::Manager *dm,
      Document::Manager *docm,
      Registrar::Manager *rm
    )
    {
      return new ManagerImpl(db,mm,cm,nm, km, dm,docm,rm);
    }
  }
}
