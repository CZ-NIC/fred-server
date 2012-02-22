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
#include <boost/assign/list_of.hpp>

using namespace Database;

namespace Fred
{
  namespace Notify
  {
    
    // id of parameter "expiration_dns_protection_period"
    const int EP_OUTZONE = 4;
    // id of parameter "expiration_registration_protection_period"
    const int EP_DELETE = 6;


    class ManagerImpl : virtual public Manager
    {
      DBSharedPtr db;
      Mailer::Manager *mm;
      Contact::Manager *cm;
      NSSet::Manager *nm;
      KeySet::Manager *km;
      Domain::Manager *dm;
      Document::Manager *docm;
      Registrar::Manager *rm;
      Messages::ManagerPtr msgm;

     public:
      ManagerImpl(
        DBSharedPtr _db,
        Mailer::Manager *_mm,
        Contact::Manager *_cm,
        NSSet::Manager *_nm,
        KeySet::Manager *_km,
        Domain::Manager *_dm,
        Document::Manager *_docm,
        Registrar::Manager *_rm,
        Messages::ManagerPtr _msgm
      ): db(_db), mm(_mm), cm(_cm), nm(_nm), km(_km), dm(_dm), docm(_docm), rm(_rm)
          , msgm(_msgm)
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
        Fred::Mailer::Parameters& params
      ) throw (SQL_ERROR)
      {
          try
          {
            std::stringstream sql;
            sql << "SELECT dom.name, cor.name, nor.name, r.handle, "
                << "eh.exdate, dh.exdate, "
                << "dh.exdate::date + "
                << "(SELECT val || ' day' FROM enum_parameters WHERE id = "
                << EP_OUTZONE << ")::interval," // 3
                << "dh.exdate::date + "
                << "(SELECT val || ' day' FROM enum_parameters WHERE id = "
                << EP_DELETE << ")::interval " // 3
                << "FROM object_registry dom, object_history doh, "
                << "registrar r, object_registry cor, domain_history dh "
                << "LEFT JOIN object_registry nor ON (nor.id=dh.nsset) "
                << "LEFT JOIN enumval_history eh ON (eh.historyid=dh.historyid) "
                << "WHERE dom.historyid=doh.historyid AND doh.clid=r.id "
                << "AND dom.historyid=dh.historyid AND dh.registrant=cor.id "
                << "AND dom.id=" << domain;
            if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
            if (db->GetSelectRows() != 1) throw SQL_ERROR();
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
            date dnsdate(MAKE_DATE(0,6));
            params["dnsdate"] = to_iso_extended_string(dnsdate);
            date exregdate(MAKE_DATE(0,7));
            params["exregdate"] = to_iso_extended_string(exregdate);
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
        Fred::Mailer::Parameters& params
      ) throw (SQL_ERROR)
      {
        std::auto_ptr<Fred::Domain::List> dlist(dm->createList());
        dlist->setIdFilter(domain);
        dlist->reload();
        if (dlist->getCount() != 1) throw SQL_ERROR();
        Fred::Domain::Domain *d = dlist->getDomain(0);
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
        params["dnsdate"] = to_iso_extended_string(d->getOutZoneDate());
        params["exregdate"] = to_iso_extended_string(d->getCancelDate());
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
        Fred::Mailer::Parameters& params
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
        TRACE("[CALL] Fred::Notify::notifyStateChanges()");
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
          Fred::Mailer::Parameters params;
          // handles are obsolete
          Fred::Mailer::Handles handles;
          // these mails has no attachments
          Fred::Mailer::Attachments attach;
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
              Fred::Mailer::Parameters::const_iterator ci = params.begin();
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
              // TODO this might as well been a pointer
              std::string exDate;
              Transaction *trans;
              std::vector<TID> state_ids;

public:
              /** holderid is only used for the filename
               * to identify which contact id's are contained in the
               * individual file, call addStateId
               */
            GenMultipleFiles() : gPDF(NULL), trans(NULL) {
            }

            void addStateId(TID id) {
                state_ids.push_back(id);
            }

            std::ostream& getInput() {
                return gPDF->getInput();
            }

            void initFile(const std::string& exD, const TID state_id, Document::Manager *docm, Transaction *tr)
            {
                exDate = exD;
                trans = tr;

                std::stringstream filename;
                filename << "letter-" << exDate << "-" << state_id << ".pdf";

                gPDF =
                  docm->createSavingGenerator(
                    Document::GT_WARNING_LETTER,
                    filename.str(), WARNING_LETTER_FILE_TYPE,
                    "" // default language
                  );
                std::ostream& out(gPDF->getInput());

                out << "<messages>";
                out << "<holder>";
            }

            void endFile(Messages::ManagerPtr msgm
                    , const std::string& contact_handle
                    , const std::string& contact_name
                    , const std::string& contact_org
                    , const std::string& contact_street1
                    , const std::string& contact_street2
                    , const std::string& contact_street3
                    , const std::string& contact_city
                    , const std::string& contact_state
                    , const std::string& contact_code
                    , const std::string& contact_country
                    , unsigned long long contact_object_registry_id
                    , unsigned long long contact_history_historyid
                    ) {
                if(gPDF.get() == NULL || trans == NULL) return;
                try {
                    if (state_ids.empty()) {
                      std::string errmsg("ERROR: no registrant ID specified by caller. Wrong usage of API.");
                      LOGGER(PACKAGE).error(errmsg);
                      throw std::runtime_error(errmsg);
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
                    std::vector<TID>::iterator it = state_ids.begin();
                    TID filePDF = gPDF->closeInput();

                    Fred::Messages::PostalAddress pa;
                    pa.name = contact_name;
                    pa.org = contact_org;
                    pa.street1 = contact_street1;
                    pa.street2 = contact_street2;
                    pa.street3 = contact_street3;
                    pa.city = contact_city;
                    pa.state = contact_state;
                    pa.code = contact_code;
                    pa.country = contact_country;

                    TID letter_id = msgm->save_letter_to_send(contact_handle.c_str()
                            , pa,filePDF
                            ,"domain_expiration"
                            ,contact_object_registry_id
                            , contact_history_historyid
                            , "letter");

                    sql << "INSERT INTO notify_letters (state_id, letter_id) VALUES ("
                        << *it << ", " << letter_id << ")";

                    it++;
                    for (;it != state_ids.end();it++) {
                        sql << ", (" << *it << ", " << letter_id << ")";
                    }

                    conn.exec(sql.str());
                    trans->savepoint();

                    gPDF.reset(NULL);
                    state_ids.clear();

                } catch (std::exception &e) {
                    LOGGER(PACKAGE).error(
                            boost::format("~GenMultipleFiles() : caught exception: %1%") % e.what());
                } catch (...) {
                    LOGGER(PACKAGE).error("~GenMultipleFiles(): Caught unknown exception. ");
                }
            }

      };

#define XML_DB_OUT(x,y) "<![CDATA[" << std::string(res[x][y]) << "]]>"
      virtual void generateLetters(unsigned item_count_limit)
        throw (SQL_ERROR)
      {
        TRACE("[CALL] Fred::Notify::generateLetters()");
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
          sql << "SELECT dobr.name,r.name,CURRENT_DATE," // 0 1 2
              << "d.exdate::date + "
              << "(SELECT val || ' day' FROM enum_parameters WHERE id = "
              << EP_DELETE << ")::interval," // 3
              << "c.name, c.organization, " // 4 5
              << "TRIM(COALESCE(c.street1,'') || ' ' || "
              << "COALESCE(c.street2,'') || ' ' || "
              << "COALESCE(c.street3,'')), " //6
              << "c.city, c.postalcode, ec.country, " // 7 8 9
              << "TRIM( "
               "COALESCE(c.country, '') || ' ' || " 
               "COALESCE(c.organization, '') || ' ' || " 
               "COALESCE(c.name, '') || ' ' || "
               "COALESCE(c.postalcode,'') || ' ' || "
               "COALESCE(c.street1,'') || ' ' || "
               "COALESCE(c.street2,'') || ' ' || "
               "COALESCE(c.street3, '') ) as distinction, " // 10
              << "r.url, tnl.state_id, c.stateorprovince, " // 11 12 13
              << "c.street1, c.street2, c.street3, cor.name, " // 14 15 16 17
              << "cor.id, cor.historyid " //18 19
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

          std::auto_ptr<GenMultipleFiles> gen(new GenMultipleFiles);
          std::string prev_distinction;
          unsigned item_count = 0;

          for (unsigned k=0; k < (unsigned)res.size(); k++) {
                std::string distinction = res[k][10];

                if ((prev_distinction != distinction) || (item_count >= item_count_limit)) {
                    // in this case start creating a new file

                    if (k>0) {
                        gen->endFile(msgm
                                , res[k][17] //const std::string& contact_handle
                                , res[k][4] //const std::string& contact_name
                                , res[k][5] //const std::string& contact_org
                                , res[k][14] //const std::string& contact_street1
                                , res[k][15] //const std::string& contact_street2
                                , res[k][16] //const std::string& contact_street3
                                , res[k][7] //const std::string& contact_city
                                , res[k][13] //const std::string& contact_state
                                , res[k][8] //const std::string& contact_code
                                , res[k][9] //const std::string& contact_country
                                , res[k][18] //unsigned long long contact_object_registry_id
                                , res[k][19] //unsigned long long contact_history_historyid
                        );
                    }
                    gen->initFile(exDates[j], res[k][12], docm, &trans);
                    item_count = 0;

                    gen->getInput()
                    << "<name>" << XML_DB_OUT(k,4) << "</name>"
                    << "<organization>" << XML_DB_OUT(k,5) << "</organization>"

                    << "<street>" << XML_DB_OUT(k,6) << "</street>"
                    << "<city>" << XML_DB_OUT(k,7) << "</city>"
                    << "<stateorprovince>" << XML_DB_OUT(k,13) << "</stateorprovince>"
                    << "<postal_code>" << XML_DB_OUT(k,8) << "</postal_code>"
                    << "<country>" << XML_DB_OUT(k,9) << "</country>"
                    << "<actual_date>" << XML_DB_OUT(k,2) << "</actual_date>"
                    << "<termination_date>" << XML_DB_OUT(k,3) << "</termination_date>";
                } 
                gen->getInput()
                << "<expiring_domain>"
                << "<domain>" << XML_DB_OUT(k,0) << "</domain>"
                << "<registrar>" << XML_DB_OUT(k,1) << "</registrar>"
                << "<registrar_web>" << XML_DB_OUT(k,11) << "</registrar_web>"
                << "</expiring_domain>";

                // has to be called here so it applies to NEWLY CREATED object in if above
                gen->addStateId(res[k][12]);

                prev_distinction = distinction; 
                item_count++;
          }
          if(res.size() > 0) {
              unsigned result_end = res.size() - 1;
              gen->endFile(msgm
                      , res[result_end][17] //const std::string& contact_handle
                      , res[result_end][4] //const std::string& contact_name
                      , res[result_end][5] //const std::string& contact_org
                      , res[result_end][14] //const std::string& contact_street1
                      , res[result_end][15] //const std::string& contact_street2
                      , res[result_end][16] //const std::string& contact_street3
                      , res[result_end][7] //const std::string& contact_city
                      , res[result_end][13] //const std::string& contact_state
                      , res[result_end][8] //const std::string& contact_code
                      , res[result_end][9] //const std::string& contact_country
                      , res[result_end][18] //unsigned long long contact_object_registry_id
                      , res[result_end][19] //unsigned long long contact_history_historyid
                      );
          }

          // return id of generated PDF file
          }
        trans.commit();
      }

    };

    Manager *Manager::create(
      DBSharedPtr db,
      Mailer::Manager *mm,
      Contact::Manager *cm,
      NSSet::Manager *nm,
      KeySet::Manager *km,
      Domain::Manager *dm,
      Document::Manager *docm,
      Registrar::Manager *rm,
      Messages::ManagerPtr msgm
    )
    {
      return new ManagerImpl(db,mm,cm,nm, km, dm,docm,rm, msgm);
    }
  }
}
