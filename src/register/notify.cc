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
#include "util/hp/hpmail.h"

#include "faked_args.h"
#include "handle_args.h"
#include "config_handler.h"
#include "handle_general_args.h"
#include "hp/handle_hpmail_args.h"


#include <sstream>
#include <boost/assign/list_of.hpp>

using namespace Database;

namespace Register
{
  namespace Notify
  {
    
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
      /* ticket #3797 */
      std::string getDomainGenericEmails(const std::string &_fqdn)
      {
          std::string emails = "";

          if (!_fqdn.empty()) {
              emails += "info@" + _fqdn;
              emails += ",kontakt@" + _fqdn;
              emails += ",postmaster@" + _fqdn;
              emails += "," + _fqdn.substr(0, _fqdn.find(".")) + "@" + _fqdn;
          }

          return emails;
      }
      std::string getNSSetTechEmailsHistory(TID nsset)
      {
        std::stringstream sql;
        sql << "SELECT ch.email "
            << "FROM contact_history ch "
            << "JOIN object_registry cobr ON (cobr.historyid = ch.historyid) "
            << "JOIN nsset_contact_map_history ncmap ON (ncmap.contactid = ch.id) "
            << "JOIN nsset_history nh ON (ncmap.historyid = nh.historyid) "
            << "JOIN object_registry nobr ON (nobr.historyid = nh.historyid) "
            << "WHERE nobr.id = " << nsset;
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
      std::string getKeySetTechEmailsHistory(TID keyset)
      {
        std::stringstream sql;
        sql << "SELECT ch.email "
            << "FROM contact_history ch "
            << "JOIN object_registry cobr ON (cobr.historyid = ch.historyid) "
            << "JOIN keyset_contact_map_history kcmap ON (kcmap.contactid = ch.id) "
            << "JOIN keyset_history kh ON (kcmap.historyid = kh.historyid) "
            << "JOIN object_registry kobr ON (kobr.historyid = kh.historyid) "
            << "WHERE kobr.id = " << keyset;
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
      std::string getContactEmailsHistory(TID contact)
      {
        std::stringstream sql;
        sql << "SELECT ch.email "
            << "FROM contact_history ch "
            << "JOIN object_registry cobr ON (ch.historyid = cobr.historyid) "
            << "WHERE cobr.id = " << contact;
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
          try
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

            Registrar::Registrar::AutoPtr regbyhandle ( rm->getRegistrarByHandle(regHandle));

            std::stringstream reg;
            if (regbyhandle.get() == 0)
              // fallback to handle instead of error
              reg << db->GetFieldValue(0,3);
            else {
              reg << regbyhandle->getName();
              if (!regbyhandle->getURL().empty())
                reg << " (" << regbyhandle->getURL() << ")";
            }
            params["registrar"] = reg.str();
          }//try
          catch(...)
          {
              LOGGER(PACKAGE).error("fillDomainParamsHistory: an error has occured");
              throw SQL_ERROR();
          }//catch(...)
      }//fillDomainParamsHistory

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
        Registrar::Registrar::AutoPtr regbyhandle ( rm->getRegistrarByHandle(d->getRegistrarHandle()));

        std::stringstream reg;
        if (regbyhandle.get() == 0)
          // fallback to handle instead of error
          reg << db->GetFieldValue(0,3);
        else {
          reg << regbyhandle->getName();
          if (!regbyhandle->getURL().empty())
            reg << " (" << regbyhandle->getURL() << ")";
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
              if (useHistory) {
                emails = getContactEmailsHistory(i->obj_id);
              }
              else {
                emails = getContactEmails(i->obj_id);
              }
              break;
             case 2: // nsset
              fillSimpleObjectParams(i->obj_id,params);
              if (useHistory) {
                emails = getNSSetTechEmailsHistory(i->obj_id);
              }
              else {
                emails = getNSSetTechEmails(i->obj_id);
              }
              break;
             case 3: // domain
               if (useHistory) {
                 fillDomainParamsHistory(i->obj_id,i->stamp,params);
                 switch (i->emails) {
                   case 1:
                     emails = getDomainAdminEmailsHistory(i->obj_id);
                     break;
                   case 2:
                     emails = getDomainTechEmailsHistory(i->obj_id);
                     break;
                   case 3:
                     emails = getDomainGenericEmails(params["domain"]);
                     break;
                 }

               }
               else {
                 fillDomainParams(i->obj_id,i->stamp,params);
                 switch (i->emails) {
                   case 1:
                     emails = getDomainAdminEmails(i->obj_id);
                     break;
                   case 2:
                     emails = getDomainTechEmails(i->obj_id);
                     break;
                   case 3:
                     emails = getDomainGenericEmails(params["domain"]);
                     break;
                 }
               }
               break;
             case 4: // keyset
              fillSimpleObjectParams(i->obj_id,params);
              if (useHistory) {
                emails = getKeySetTechEmailsHistory(i->obj_id);
              }
              else {
                emails = getKeySetTechEmails(i->obj_id);
              }
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
              TID mail = 0;
              if (mm->checkEmailList(emails)) {
                mail = mm->sendEmail("", emails, "", i->mtype, params, handles, attach);
              }
              saveNotification(i->state_id, i->type, mail);
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

      /** Controls output of notification letters into multiple files
       */
#define WARNING_LETTER_FILE_TYPE 5 // from enum_filetype table
      class GenMultipleFiles {
          private:
              std::auto_ptr<Document::Generator> gPDF;
              const std::string &exDate;
              Transaction &trans;
              std::vector<TID> holder_ids;

          public:
              /** holderid is only used for the filename
               * to identify which contact id's are contained in the 
               * individual file, call addHolderId 
               */
              GenMultipleFiles(const std::string &exDate_, const TID holder_id, 
                    Document::Manager *docm, Transaction &tr) 
                : exDate(exDate_), trans(tr)  {
  
              std::stringstream filename;
              filename << "letter-" << exDate << "-" << holder_id << ".pdf";
                  
              gPDF.reset(
                docm->createSavingGenerator(
                  Document::GT_WARNING_LETTER,
                  filename.str(), WARNING_LETTER_FILE_TYPE,
                  "" // default language
                )
              );
              std::ostream& out(gPDF->getInput());

              out << "<messages>";
              out << "<holder>";

            }
            
            ~GenMultipleFiles() {
                try {
                if (holder_ids.empty()) {
                  std::string errmsg("ERROR: no registrant ID specified by caller. Wrong usage of API.");
                  LOGGER(PACKAGE).error(errmsg);
                  throw Exception(errmsg);
                }

                std::ostream& out(gPDF->getInput());
                std::stringstream sql;

                out << "</holder>";
                out << "</messages>";

                // TODO there's still some room for more DB OPTIMIZATION if
                // whole operation was made atomic and this query was run 
                // only once with IDs taken from vector
                // OR ----
                // most of the inserted data could already be gathered 
                // in the statement which generates the XML
                // records stored in vector could be used in a simple insert
                //
                Connection conn = Database::Manager::acquire();
                std::vector<TID>::iterator it = holder_ids.begin();
                TID filePDF = gPDF->closeInput();
                    // status 1 represents 'ready generated - ready for sending
                    sql << "INSERT INTO letter_archive (status, file_id) VALUES (1, "
                      << filePDF << ")";
                    conn.exec(sql.str());
                    sql.str("");

                    Result res = conn.exec("select currval('letter_archive_id_seq'::regclass)");
                    TID letter_id = res[0][0];

                    sql << "INSERT INTO notify_letters (state_id, contact_history_id, letter_id) "
                    "SELECT tnl.state_id, cor.historyid, " << letter_id << " "
                    "FROM tmp_notify_letters tnl "
                    "JOIN object_state s ON tnl.state_id=s.id "
                    "JOIN domain_history dh ON s.ohid_from=dh.historyid AND dh.exdate::date='" << exDate << "' "
                    "JOIN object_registry cor ON cor.id = dh.registrant "
                    "WHERE dh.registrant in ("
                      << *it;

                  it++;

                  for (;it != holder_ids.end();it++) {
                      sql << ", " << *it;
                  }
                  sql << ") ";

                conn.exec(sql.str());
                trans.savepoint();

                if(holder_ids.size() > 1) {
                  LOGGER(PACKAGE).debug(boost::format("File %1% contains several registrants") % filePDF);
                }

                } catch (std::exception &e) {
                    LOGGER(PACKAGE).error(
                            boost::format("~GenMultipleFiles() : caught exception: %1%") % e.what());
                } catch (...) {
                    LOGGER(PACKAGE).error("~GenMultipleFiles(): Caught unknown exception. ");
                }
            }
            
            void addHolderId(TID id) {
                holder_ids.push_back(id);
            }

            std::ostream& getInput() {
                return gPDF->getInput();
            }
      };

#define XML_DB_OUT(x,y) "<![CDATA[" << std::string(res[x][y]) << "]]>"
      virtual void generateLetters()
        throw (SQL_ERROR)
      {
        TRACE("[CALL] Register::Notify::generateLetters()");
    	// transaction is needed for 'ON COMMIT DROP' functionality
        
        Connection conn = Database::Manager::acquire();
        Database::Transaction trans(conn);
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
        conn.exec(create);

        const char *fixateStates =
          "INSERT INTO tmp_notify_letters "
          "SELECT s.id FROM object_state s "
          "LEFT JOIN notify_letters nl ON (s.id=nl.state_id) "
          " JOIN domain_history d      ON d.historyid = s.ohid_from "
          " JOIN zone z                ON z.id = d.zone "
          "WHERE s.state_id=19 AND s.valid_to ISNULL AND nl.state_id ISNULL "
          " AND z.warning_letter=true ";
        conn.exec(fixateStates);
        // select all expiration dates of domain to notify
        const char *selectExDates =
          "SELECT DISTINCT dh.exdate::date "
          "FROM tmp_notify_letters tnl, object_state s, domain_history dh "
          "WHERE tnl.state_id=s.id AND s.ohid_from=dh.historyid ";
        Result res = conn.exec(selectExDates);

        std::vector<std::string> exDates;
        for (unsigned i=0; i < (unsigned)res.size(); i++) {
          exDates.push_back(res[i][0]);
        }

        // for every expiration date generate PDF
        for (unsigned j=0; j<exDates.size(); j++) {

            /*
SELECT s.id from object_state s left join notify_letters nl ON (s.id=nl.state_id) where s.state_id=19 and s.valid_to isnull and nl.state_id isnull
                -- general query for testing purposes
            SELECT dobr.name, d.exdate, cor.name, c.name, c.organization, d.registrant
                   FROM
                   object_state s 
                   LEFT JOIN notify_letters nl  ON nl.state_id=s.id
                   JOIN domain_history d             ON d.historyid = s.ohid_from
                   JOIN object_registry dobr         ON dobr.id = d.id

                   JOIN object_registry cor          ON cor.id = d.registrant
                   JOIN contact_history c            ON c.historyid = cor.historyid
                   WHERE s.state_id=19 AND s.valid_to ISNULL AND nl.state_id ISNULL

                              */

          std::stringstream sql;
          sql << "SELECT dobr.name,r.name,CURRENT_DATE,"
              << "d.exdate::date + INTERVAL '45 days',"
              << "c.name, c.organization, "
              << "TRIM(COALESCE(c.street1,'') || ' ' || "
              << "COALESCE(c.street2,'') || ' ' || "
              << "COALESCE(c.street3,'')), "
              << "c.city, c.postalcode, ec.country, "
              << "TRIM( "
               "COALESCE(c.country, '') || ' ' || " 
               "COALESCE(c.organization, '') || ' ' || " 
               "COALESCE(c.name, '') || ' ' || "
               "COALESCE(c.postalcode,'') || ' ' || "
               "COALESCE(c.street1,'') || ' ' || "
               "COALESCE(c.street2,'') || ' ' || "
               "COALESCE(c.street3, '') ) as distinction, "
              << "r.url, d.registrant, c.stateorprovince "

              << "FROM tmp_notify_letters tnl "
                   "JOIN object_state s               ON tnl.state_id = s.id "
                   "JOIN domain_history d             ON d.historyid = s.ohid_from "
                   "JOIN object_history doh           ON doh.historyid = d.historyid "
                   "JOIN object_registry dobr         ON dobr.id = d.id "
                   "JOIN object_registry cor          ON cor.id = d.registrant "
                   "JOIN contact_history c            ON c.historyid = cor.historyid "
                   "JOIN enum_country ec              ON ec.id = c.country "
                   "JOIN registrar r                  ON r.id = doh.clid "

              << "WHERE "
              << " d.exdate::date='" << exDates[j] << "' "
              << "ORDER BY CASE WHEN c.country='CZ' THEN 0 ELSE 1 END ASC,"
              << " distinction, dobr.name";
          res = conn.exec(sql.str());

          std::auto_ptr<GenMultipleFiles> gen;
          std::string prev_distinction;
          for (unsigned i=0; i < (unsigned)res.size(); i++) {
                std::string distinction = res[i][10];

                if (prev_distinction != distinction) {
                    // in this case start creating a new file
                    gen.reset ( new GenMultipleFiles(exDates[j], res[i][12], docm, trans));
                    /*
                    if(!prev_distinction.empty()) {
                        out << "</holder>";
                    }
                    */

                    gen->getInput()
                    << "<name>" << XML_DB_OUT(i,4) << "</name>"
                    << "<organization>" << XML_DB_OUT(i,5) << "</organization>"

                    << "<street>" << XML_DB_OUT(i,6) << "</street>"
                    << "<city>" << XML_DB_OUT(i,7) << "</city>"
                    << "<stateorprovince>" << XML_DB_OUT(i,13) << "</stateorprovince>"
                    << "<postal_code>" << XML_DB_OUT(i,8) << "</postal_code>"
                    << "<country>" << XML_DB_OUT(i,9) << "</country>"
                    << "<actual_date>" << XML_DB_OUT(i,2) << "</actual_date>"
                    << "<termination_date>" << XML_DB_OUT(i,3) << "</termination_date>";
                } 
                gen->getInput()
                << "<expiring_domain>"
                << "<domain>" << XML_DB_OUT(i,0) << "</domain>"
                << "<registrar>" << XML_DB_OUT(i,1) << "</registrar>"
                << "<registrar_web>" << XML_DB_OUT(i,11) << "</registrar_web>"
                << "</expiring_domain>";

                // has to be called here so it applies to NEWLY CREATED object in if above
                gen->addHolderId(res[i][12]);

                prev_distinction = distinction; 
          }

          // return id of generated PDF file
          }
        trans.commit();
      }

        HPCfgMap 
        readHPConfig(const std::string &conf_file)
        {
            FakedArgs fa;

            boost::shared_ptr<HandleHPMailArgs> hhp(new HandleHPMailArgs);

            HandlerPtrVector handlers =
            boost::assign::list_of
            (HandleArgsPtr(new HandleGeneralArgs(conf_file) ))
            (HandleArgsPtr(hhp));

            try
            {
                // CfgArgs always needs some argv vector, argc cannot be 0
                int argc = 1;
                char *argv[argc];
                argv[0] = new char [1];
                // zero-length string into argv[0]
                argv[0][0] = '\0';

                fa = CfgArgs::instance<HandleHelpArg>(handlers)->handle(argc, argv);
                HPCfgMap set_cfg = boost::assign::map_list_of
                // basic login parametres
                ("hp_login_name",hhp->login)
                ("hp_login_password",hhp->password)
                ("hp_login_batch_id",hhp->hp_login_batch_id)
                ("hp_login_note",hhp->note)
                // from here it's a copy of map initialization from client-hp.cc
                ("mb_proc_tmp_dir",hhp->mb_proc_tmp_dir)
                ("postservice_cert_dir",hhp->postservice_cert_dir)
                ("postservice_cert_file", hhp->postservice_cert_file)
                ("hp_login_interface_url",hhp->hp_login_interface_url)
		("hp_upload_interface_url",hhp->hp_upload_interface_url)
		("hp_ack_interface_url",hhp->hp_ack_interface_url)
		("hp_cancel_interface_url",hhp->hp_cancel_interface_url)
		("hp_upload_archiver_filename",hhp->hp_upload_archiver_filename)
		("hp_upload_archiver_additional_options"
				,hhp->hp_upload_archiver_additional_options)
		("hp_upload_curlopt_timeout",hhp->hp_upload_curlopt_timeout )
		("hp_upload_curlopt_connect_timeout"
				,hhp->hp_upload_curlopt_connect_timeout)
		("hp_upload_curl_verbose",hhp->hp_upload_curl_verbose )
		("hp_upload_retry",hhp->hp_upload_retry )
		;

                return set_cfg;
            }
            catch(const ReturnFromMain&)
            {
                return HPCfgMap();
                // return 0;
            }
        }


      /** This method sends letters from table letter_archive
       * it sets current processed row to status=6 (under processing)
       * and cancels execution at the beginning if any row in this table
       * is already being processed.
       *
       */
      virtual void sendLetters(std::auto_ptr<Register::File::Transferer> fileman, const std::string &conf_file)
      {
         TRACE("[CALL] Register::Notify::sendLetters()");
    	// transaction is needed for 'ON COMMIT DROP' functionality
        
         HPCfgMap hpmail_config = readHPConfig(conf_file);

         Connection conn = Database::Manager::acquire();
   
         /* now bail out if other process (presumably another instance of this
          * method) is already doing something with this table. No support 
          * for multithreaded access - we would have to remember which 
          * records exactly are we processing
          */ 
         Result res = conn.exec("SELECT EXISTS "
                 "(SELECT * FROM letter_archive WHERE status=6)");

         if ((bool)res[0][0]) {
                LOGGER(PACKAGE).notice("The files are already being processed. No action");
                return;
         }

         Database::Transaction trans(conn);

         // acquire lock which conflicts with itself but not basic locks used
         // by select and stuff..
         conn.exec("LOCK TABLE letter_archive IN SHARE UPDATE EXCLUSIVE MODE");
                  
         res = conn.exec("SELECT file_id, attempt FROM letter_archive WHERE status=1 or status=4");
         
         if(res.size() == 0) {
             LOGGER(PACKAGE).debug("Register::Notify::sendLetters(): No files ready for processing"); 
             return;
         }
         TID id = res[0][0];
         int new_status = 6;
         // we have to set application lock(status 6) somewhere while the table is locked in db
         conn.exec(boost::format("UPDATE letter_archive SET status=6 WHERE file_id=%1%") % id) ;
         
         // unlock the table - from now we need to hold the app lock until the end of processing
         trans.commit();

         for (unsigned int i=0;i<res.size(); i++) {

             Database::Transaction t(conn);
             // start processing the next record
             id = res[i][0];             
             int attempt = res[i][1];

             // get the file from filemanager client and create the batch
             MailFile one;
             fileman->download(id, one);

             LOGGER(PACKAGE).debug(boost::format ("sendLetters File ID: %1% ") % res[i][0]);

             MailBatch batch;
             batch.push_back(one);

             // data's ready, we can send it
             new_status=5;
             try {
                 HPMail::init_session(hpmail_config);
                 HPMail::get()->upload(batch);
             } catch(std::exception& ex) {
                 std::cout << "Error: " << ex.what() << " on file ID " << id << std::endl;
                 new_status = 4; // set error status in database
             } catch(...) {
                 std::cout << "Unknown Error" << " on file ID " << id << std::endl;
                 new_status = 4; // set error status in database
             }

             // set status for next item first
             if(i+1 < res.size()) {
                 TID next_id = res[i+1][0];
                conn.exec(boost::format("UPDATE letter_archive SET status=6 where file_id=%1%")
                % next_id);
             }

             conn.exec(boost::format("UPDATE letter_archive SET status=%1%, moddate=now(), attempt=%2% "
                    "where file_id=%3%") % new_status % (attempt+1) % id);
            
             t.commit();
         }                
      }

      virtual void sendFile(const std::string &filename, const std::string &conf_file) {

          TRACE("[CALL] Register::Notify::sendFile()");


          HPCfgMap hpmail_config = readHPConfig(conf_file);

          LOGGER(PACKAGE).debug(boost::format("File to send %1% ") % filename);

          std::ifstream infile(filename.c_str());

          if(infile.fail()) {
              std::cerr << "Failed to open file" << filename << std::endl;
              throw;
              return;
          }

          infile.seekg(0, std::ios::end);
          std::streampos infile_size = infile.tellg();
          infile.seekg(0, std::ios::beg);
           
          MailFile f(infile_size);
          infile.read(&f[0], infile_size);

          MailBatch batch;
          batch.push_back(f);

          try {
             HPMail::init_session(hpmail_config);
             HPMail::get()->upload(batch);

          } catch(std::exception& ex) {
             std::cerr << "Error: " << ex.what() << std::endl;
             throw;
          } catch(...) {
             std::cerr << "Unknown Error" << std::endl;
             throw;
          }

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
