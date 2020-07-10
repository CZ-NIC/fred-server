/*
 * Copyright (C) 2007-2020  CZ.NIC, z. s. p. o.
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

#include "src/deprecated/libfred/notify.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/util/log.hh"
#include "src/deprecated/libfred/sql.hh"

#include "util/db/result.hh"
#include "util/db/query_param.hh"

#include <sstream>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

using namespace Database;

namespace LibFred {
namespace Notify {

    class ManagerImpl : virtual public Manager
    {
      DBSharedPtr db;
      Mailer::Manager *mm;
      Contact::Manager *cm;
      Nsset::Manager *nm;
      Keyset::Manager *km;
      Domain::Manager *dm;
      Document::Manager *docm;
      Registrar::Manager *rm;
      Messages::ManagerPtr msgm;

     public:
      ManagerImpl(
        DBSharedPtr _db,
        Mailer::Manager *_mm,
        Contact::Manager *_cm,
        Nsset::Manager *_nm,
        Keyset::Manager *_km,
        Domain::Manager *_dm,
        Document::Manager *_docm,
        Registrar::Manager *_rm,
        Messages::ManagerPtr _msgm
      ): db(_db), mm(_mm), cm(_cm), nm(_nm), km(_km), dm(_dm), docm(_docm), rm(_rm)
          , msgm(_msgm)
      {}
      std::string getEmailList(const std::stringstream& sql)
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
      /* ticket #14873 */
      std::string getDomainAdditionalEmails(TID state_id, TID obj_id)
      {
        std::stringstream sql;
        // select unnotified (state_id IS NULL) emails inserted
        // into notify_outzone_unguarded_domain_additional_email
        // between domain expiration date (exdate) and current object state (valid_from)
        sql << "SELECT n.email "
            "FROM notify_outzone_unguarded_domain_additional_email n "
            "JOIN object_state os ON os.object_id = n.domain_id "
            "JOIN domain d ON d.id = n.domain_id "
            "WHERE os.id = " << state_id << " "
            "AND n.domain_id = " << obj_id << " "
            "AND n.state_id IS NULL "
            "AND n.crdate BETWEEN d.exdate AND os.valid_from";
        return getEmailList(sql);
      }
      std::string getNssetTechEmailsHistory(TID nsset)
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
      std::string getKeysetTechEmailsHistory(TID keyset)
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
      std::string getContactEmailsHistory(TID contact)
      {
        std::stringstream sql;
        sql << "SELECT ch.email "
            << "FROM contact_history ch "
            << "JOIN object_registry cobr ON (ch.historyid = cobr.historyid) "
            << "WHERE cobr.id = " << contact;
        return getEmailList(sql);
      }

      std::string getParamZone(TID zone_id)
      {
        std::ostringstream sql;
        sql << "SELECT fqdn FROM zone WHERE id=" << zone_id;
        if (!db->ExecSelect(sql.str().c_str())) {
            throw SQL_ERROR();
        }
        if (db->GetSelectRows() != 1) {
            throw SQL_ERROR();
        }
        const std::string zone_fqdn = db->GetFieldValue(0, 0);
        db->FreeSelect();
        return zone_fqdn;
      }

      void fillDomainParamsAdministratorsHistory(TID domain_id, LibFred::Mailer::Parameters& params)
      {
        std::ostringstream sql;
        sql << "SELECT cor.name "
               "FROM object_registry dor "
               "JOIN domain_contact_map_history dcmh ON dcmh.historyid=dor.historyid AND dcmh.role=1 "
               "JOIN object_registry cor ON cor.id=dcmh.contactid "
               "WHERE dor.id=" << domain_id;
        if (!db->ExecSelect(sql.str().c_str())) {
            throw SQL_ERROR();
        }
        for (int db_row_idx = 0; db_row_idx < db->GetSelectRows(); ++db_row_idx) {
            params["administrators." + boost::lexical_cast<std::string>(db_row_idx)] = db->GetFieldValue(db_row_idx, 0);
        }
        db->FreeSelect();
      }

      void fillDomainParamsAdministrators(TID domain_id, LibFred::Mailer::Parameters& params)
      {
        std::ostringstream sql;
        sql << "SELECT cor.name "
               "FROM domain_contact_map dcm "
               "JOIN object_registry cor ON cor.id=dcm.contactid "
               "WHERE dcm.domainid=" << domain_id << " AND "
                     "dcm.role=1";
        if (!db->ExecSelect(sql.str().c_str())) {
            throw SQL_ERROR();
        }
        for (int db_row_idx = 0; db_row_idx < db->GetSelectRows(); ++db_row_idx) {
            params["administrators." + boost::lexical_cast<std::string>(db_row_idx)] = db->GetFieldValue(db_row_idx, 0);
        }
        db->FreeSelect();
      }

      void fillDomainParamsHistory(
        TID domain_id,
        ptime stamp,
        LibFred::Mailer::Parameters& params
      )
      {
          try
          {
            std::ostringstream sql;
            sql << "SELECT dobr.name,cobr.name,"
                          "(SELECT name FROM object_registry WHERE id=dh.nsset),"
                          "r.handle,"
                          "(SELECT exdate FROM enumval_history WHERE historyid=dobr.historyid),"
                          "dh.exdate,"
                          "dh.exdate+dlp.expiration_dns_protection_period,"
                          "dh.exdate+dlp.expiration_registration_protection_period,"
                          "(SELECT fqdn FROM zone WHERE id=dh.zone) "
                   "FROM object_registry dobr "
                   "JOIN object_history doh ON doh.historyid=dobr.historyid "
                   "JOIN registrar r ON r.id=doh.clid "
                   "JOIN domain_history dh ON dh.historyid=dobr.historyid "
                   "JOIN object_registry cobr ON cobr.id=dh.registrant "
                   "JOIN domain_lifecycle_parameters dlp ON dlp.valid_for_exdate_after=(SELECT MAX(valid_for_exdate_after) FROM domain_lifecycle_parameters WHERE valid_for_exdate_after<=dh.exdate) "
                   "WHERE dobr.id=" << domain_id;
            if (!db->ExecSelect(sql.str().c_str()))
            {
                throw SQL_ERROR();
            }
            if (db->GetSelectRows() != 1)
            {
                throw SQL_ERROR();
            }
            params["checkdate"] = to_iso_extended_string(date(day_clock::local_day()));
            params["domain"] = db->GetFieldValue(0,0);
            params["owner"] = db->GetFieldValue(0,1);
            params["nsset"] = db->GetFieldValue(0,2);
            const std::string regHandle = db->GetFieldValue(0,3);
            const date val(MAKE_DATE(0,4));
            if (!val.is_special())
            {
              params["valdate"] = to_iso_extended_string(val);
            }
            const date ex(MAKE_DATE(0,5));
            params["exdate"] = to_iso_extended_string(ex);
            const date dnsdate(MAKE_DATE(0,6));
            params["dnsdate"] = to_iso_extended_string(dnsdate);
            const date exregdate(MAKE_DATE(0,7));
            params["exregdate"] = to_iso_extended_string(exregdate);
            params["zone"] = db->GetFieldValue(0, 8);
            db->FreeSelect();
            // fill information about registrar, query must be closed
            const date day_before_exregdate = exregdate - boost::gregorian::date_duration(1);
            params["day_before_exregdate"] = to_iso_extended_string(day_before_exregdate);
            params["statechangedate"] = to_iso_extended_string(stamp.date());

            const Registrar::Registrar::AutoPtr regbyhandle(rm->getRegistrarByHandle(regHandle));

            std::ostringstream reg;
            if (regbyhandle.get() == NULL)
            {
              // fallback to handle instead of error
              reg << db->GetFieldValue(0, 3);
            }
            else
            {
              reg << regbyhandle->getName();
              if (!regbyhandle->getURL().empty())
              {
                reg << " (" << regbyhandle->getURL() << ")";
              }
            }
            params["registrar"] = reg.str();
            this->fillDomainParamsAdministratorsHistory(domain_id, params);
          }//try
          catch (...)
          {
              LOGGER.error("fillDomainParamsHistory: an error has occured");
              throw SQL_ERROR();
          }//catch(...)
      }//fillDomainParamsHistory

      void fillSimpleObjectParams(
        TID id,
        LibFred::Mailer::Parameters& params
      )
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
      void saveDomainAdditionalEmailsState(TID state_id, TID obj_id, const std::vector<std::string>& emails) {
        for (std::vector<std::string>::const_iterator email_ptr = emails.begin(); email_ptr != emails.end(); ++email_ptr)
        {
            std::ostringstream sql;
            sql << "UPDATE notify_outzone_unguarded_domain_additional_email "
                   "SET state_id = " << state_id << " "
                   "WHERE domain_id = " << obj_id << " AND email = '" << db->Escape2(*email_ptr) << "' AND state_id IS NULL";
            if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
        }
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
        std::ostream *debugOutput
      )
      {
        TRACE("[CALL] LibFred::Notify::notifyStateChanges()");
        std::ostringstream sql;
        sql << "SELECT s.id AS state_id, nm.id AS type, "
                      "mt.name AS mtype, nm.emails, "
                      "obr.id AS obj_id, obr.type AS obj_type, s.valid_from "
               "FROM object_state s "
               "JOIN object_registry obr ON obr.id=s.object_id "
               "JOIN notify_statechange_map nm ON nm.obj_type=obr.type AND nm.state_id=s.state_id "
               "JOIN mail_type mt ON mt.id=nm.mail_type_id "
               "LEFT JOIN notify_statechange ns ON ns.state_id=s.id AND ns.type=nm.id "
               "WHERE s.valid_to IS NULL AND "
                     "ns.state_id IS NULL";
        if (!exceptList.empty()) {
            sql << " AND nm.id NOT IN (" << exceptList << ")";
        }
        sql << " ORDER BY s.id ASC";
        if (0 < limit) {
            sql << " LIMIT " << limit;
        }
        if (!db->ExecSelect(sql.str().c_str())) {
            throw SQL_ERROR();
        }
        std::vector<NotifyRequest> nlist;
        for (unsigned i = 0; i < (unsigned)db->GetSelectRows(); ++i) {
          nlist.push_back(NotifyRequest(
            STR_TO_ID(db->GetFieldValue(i,0)),
            atoi(db->GetFieldValue(i,1)),
            db->GetFieldValue(i,2),
            atoi(db->GetFieldValue(i,3)),
            STR_TO_ID(db->GetFieldValue(i,4)),
            atoi(db->GetFieldValue(i,5)),
            MAKE_TIME(i,6) /// Once should handle timestamp conversion
          ));
        }
        db->FreeSelect();
        if (debugOutput) {
            *debugOutput << "<notifications>" << std::endl;
        }
        for (std::vector<NotifyRequest>::const_iterator i = nlist.begin(); i != nlist.end(); ++i) {
          LibFred::Mailer::Parameters params;
          // handles are obsolete
          LibFred::Mailer::Handles handles;
          // these mails has no attachments
          LibFred::Mailer::Attachments attach;
          std::string emails;
          try {
            switch (i->obj_type) {
              case 1: // contact
                fillSimpleObjectParams(i->obj_id, params);
                emails = getContactEmailsHistory(i->obj_id);
                break;
              case 2: // nsset
                fillSimpleObjectParams(i->obj_id, params);
                emails = getNssetTechEmailsHistory(i->obj_id);
                break;
              case 3: // domain
                fillDomainParamsHistory(i->obj_id, i->stamp, params);
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
                  case 4:
                    emails = getDomainAdditionalEmails(i->state_id, i->obj_id);
                    break;
                }
                break;
              case 4: // keyset
                fillSimpleObjectParams(i->obj_id, params);
                emails = getKeysetTechEmailsHistory(i->obj_id);
                break;
            }
            if (debugOutput) {
              *debugOutput << "<notify>"
                              "<emails>" << emails << "</emails>"
                              "<template>" << i->mtype << "</template>";
              for (LibFred::Mailer::Parameters::const_iterator ci = params.begin(); ci != params.end(); ++ci) {
                *debugOutput << "<param><name>" << ci->first << "</name>"
                                "<value>" << ci->second << "</value></param>";
              }
              *debugOutput << "</notify>" << std::endl;
            }
            else {
              TID mail = 0;
              const bool some_emails_are_valid = mm->checkEmailList(emails); // remove "emails" without @, remove duplicates, sort them, set separator to " "
              const std::string space_separated_emails = emails; // emails were modified above
              if (some_emails_are_valid) {
                mail = mm->sendEmail("", space_separated_emails, "", i->mtype, params, handles, attach);
              }
              saveNotification(i->state_id, i->type, mail);
              if ((i->obj_type == 3) && (i->emails == 4)) { // 3: domain, 4: additional email
                std::vector<std::string> set_of_emails;
                boost::split(set_of_emails, space_separated_emails, boost::is_any_of(" "), boost::token_compress_on);
                saveDomainAdditionalEmailsState(i->state_id, i->obj_id, set_of_emails);
              }
            }
          }
          catch (...) {
            LOG<Logging::Log::Severity::err>(
              "Notfication wasn't successful (state=%d, type=%d)",
              i->state_id, i->type
            );
          }
        }
        if (debugOutput) {
            *debugOutput << "</notifications>" << std::endl;
        }
      }

      /** Controls output of notification letters into multiple files
       */
#define WARNING_LETTER_FILE_TYPE 5 // from enum_filetype table
      class GenMultipleFiles {
private:
              std::unique_ptr<Document::Generator> gPDF;
              // TODO this might as well been a pointer
              std::string exDate;
              Transaction *trans;
              std::vector<TID> state_ids;

public:
              /** holderid is only used for the filename
               * to identify which contact id's are contained in the
               * individual file, call addStateId
               */
            GenMultipleFiles() : gPDF(nullptr), trans(NULL) {
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
                if ((gPDF.get() == NULL) || (trans == NULL)) {
                    return;
                }
                try {
                    if (state_ids.empty()) {
                      std::string errmsg("ERROR: no registrant ID specified by caller. Wrong usage of API.");
                      LOGGER.error(errmsg);
                      throw std::runtime_error(errmsg);
                    }

                    class ClearOnExit
                    {
                    public:
                        typedef std::vector<TID> Type;
                        ClearOnExit(Type &_obj): obj_(_obj) { }
                        ~ClearOnExit() { obj_.clear(); }
                    private:
                        Type &obj_;
                    } clear_on_exit(state_ids);

                    {
                        std::ostream &out = gPDF->getInput();
                        out << "</holder>";
                        out << "</messages>";
                    }

                    // TODO there's still some room for more DB OPTIMIZATION if
                    // whole operation was made atomic and this query was run
                    // only once with IDs taken from vector
                    // OR ----
                    // most of the inserted data could already be gathered
                    // in the statement which generates the XML
                    // records stored in vector could be used in a simple insert
                    //
                    Connection conn = Database::Manager::acquire();

                    LibFred::Messages::PostalAddress pa;
                    pa.name = contact_name;
                    pa.org = contact_org;
                    pa.street1 = contact_street1;
                    pa.street2 = contact_street2;
                    pa.street3 = contact_street3;
                    pa.city = contact_city;
                    pa.state = contact_state;
                    pa.code = contact_code;
                    pa.country = contact_country;

                    LibFred::Messages::check_postal_address(pa);

                    const TID filePDF = gPDF->closeInput();
                    gPDF.reset(NULL);

                    const TID letter_id = msgm->save_letter_to_send(contact_handle.c_str()
                            , pa, filePDF
                            , "domain_expiration"
                            , contact_object_registry_id
                            , contact_history_historyid
                            , "letter", true);

                    std::vector<TID>::const_iterator it = state_ids.begin();
                    std::ostringstream sql;
                    sql << "INSERT INTO notify_letters (state_id,letter_id) VALUES ("
                        << *it << "," << letter_id << ")";

                    ++it;
                    for (; it != state_ids.end(); ++it) {
                        sql << ",(" << *it << "," << letter_id << ")";
                    }

                    conn.exec(sql.str());
                    trans->savepoint();

                }catch (LibFred::Messages::WrongPostalAddress& e) {
                    LOGGER.warning(
                            boost::format("GenMultipleFiles::endFile(): Caught exception: %1%") % e.what());
                }
                catch (const std::exception &e) {
                    LOGGER.error(
                            boost::format("GenMultipleFiles::endFile(): Caught exception: %1%") % e.what());
                }
                catch (...) {
                    LOGGER.error("GenMultipleFiles::endFile(): Caught unknown exception.");
                }
            }

      };

#define XML_DB_OUT(x,y) "<![CDATA[" << std::string(res[x][y]) << "]]>"
      virtual void generateLetters(unsigned item_count_limit)
      {
        TRACE("[CALL] LibFred::Notify::generateLetters()");
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

         // select valid domain states 'deleteWarning' to notify that:
         // - was not notified so far with the notification letter
         // - belongs to domains whose zone have set warning_letter flag to send
         // - belongs to domains whose owner have set warning_letter flag to send or unspecified
        const char *fixateStates =
          "INSERT INTO tmp_notify_letters "
          "SELECT s.id FROM object_state s "
          "LEFT JOIN notify_letters nl ON (s.id=nl.state_id) "
          " JOIN domain_history d "   "ON d.historyid = s.ohid_from "
          " JOIN zone z "             "ON z.id = d.zone "
          " JOIN object_registry cor ON cor.id=d.registrant "
          " JOIN contact_history c ON c.historyid=cor.historyid "
          "JOIN domain_lifecycle_parameters dlp ON dlp.valid_for_exdate_after=(SELECT MAX(valid_for_exdate_after) FROM domain_lifecycle_parameters WHERE valid_for_exdate_after<=d.exdate) "
          "WHERE s.state_id = (SELECT id FROM enum_object_states WHERE name = 'deleteWarning') "
          " AND s.valid_to ISNULL AND nl.state_id ISNULL "
          " AND (now() - (dlp.expiration_registration_protection_period - dlp.expiration_letter_warning_period)/2) < s.valid_from "
          " AND z.warning_letter=true "
          " AND (c.warning_letter IS NULL OR c.warning_letter=true)";

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

          std::ostringstream sql;
          sql <<
"WITH expirated_domain AS ("
    "SELECT dobr.name AS domain_name,r.name AS registrar_handle,"
           "d.exdate+dlp.expiration_registration_protection_period AS termination_date,"
           "ca.id IS NOT NULL AS has_mailing_address,"
           "c.name AS contact_name,c.organization,"
           "CASE WHEN ca.id IS NULL THEN c.street1 ELSE ca.street1 END AS street1,"
           "CASE WHEN ca.id IS NULL THEN c.street2 ELSE ca.street2 END AS street2,"
           "CASE WHEN ca.id IS NULL THEN c.street3 ELSE ca.street3 END AS street3,"
           "CASE WHEN ca.id IS NULL THEN c.city ELSE ca.city END AS city,"
           "CASE WHEN ca.id IS NULL THEN c.stateorprovince ELSE ca.stateorprovince END "
               "AS stateorprovince,"
           "CASE WHEN ca.id IS NULL THEN c.postalcode ELSE ca.postalcode END AS postalcode,"
           "CASE WHEN ca.id IS NULL THEN c.country ELSE ca.country END AS country,"
           "r.url AS registrar_web,tnl.state_id,cor.name AS contact_handle,"
           "cor.id AS contact_object_registry_id,cor.historyid AS contact_history_historyid "
    "FROM tmp_notify_letters tnl "
    "JOIN object_state s ON tnl.state_id=s.id "
    "JOIN domain_history d ON d.historyid=s.ohid_from "
    "JOIN object_history doh ON doh.historyid=d.historyid "
    "JOIN object_registry dobr ON dobr.id=d.id "
    "JOIN object_registry cor ON cor.id=d.registrant "
    "JOIN contact_history c ON c.historyid=cor.historyid "
    "LEFT JOIN contact_address_history ca ON ca.historyid=cor.historyid AND ca.type='MAILING' "
    "JOIN registrar r ON r.id=doh.clid "
    "JOIN domain_lifecycle_parameters dlp ON dlp.valid_for_exdate_after=(SELECT MAX(valid_for_exdate_after) FROM domain_lifecycle_parameters WHERE valid_for_exdate_after<=d.exdate) "
    "WHERE d.exdate::date='" << exDates[j] << "') "
"SELECT ed.domain_name,ed.registrar_handle,CURRENT_DATE,ed.termination_date," // 0 1 2 3
       "ed.contact_name,ed.organization," // 4 5
       "TRIM(COALESCE(ed.street1,'')||' '||"
            "COALESCE(ed.street2,'')||' '||"
            "COALESCE(ed.street3,'')) AS street," // 6
       "ed.city,ed.postalcode,ec.country," // 7 8 9
       "TRIM(COALESCE(ed.country,'')||' '||"
            "COALESCE(ed.organization,'')||' '||"
            "COALESCE(ed.contact_name,'')||' '||"
            "COALESCE(ed.postalcode,'')||' '||"
            "COALESCE(ed.street1,'')||' '||"
            "COALESCE(ed.street2,'')||' '||"
            "COALESCE(ed.street3,'')) AS distinction," // 10
       "ed.registrar_web,ed.state_id,ed.stateorprovince," // 11 12 13
       "ed.street1,ed.street2,ed.street3," // 14 15 16
       "ed.contact_handle,ed.contact_object_registry_id,ed.contact_history_historyid " // 17 18 19
"FROM expirated_domain ed "
"JOIN enum_country ec ON ec.id=ed.country "
"ORDER BY ed.country='CZ' DESC,distinction,ed.domain_name";
          res = conn.exec(sql.str());

          std::unique_ptr<GenMultipleFiles> gen(new GenMultipleFiles);
          std::string prev_distinction;
          unsigned item_count = 0;

          for (unsigned k=0; k < (unsigned)res.size(); k++) {
                std::string distinction = res[k][10];

                if ((prev_distinction != distinction) || (item_count >= item_count_limit)) {
                    // in this case start creating a new file

                    if (k>0) {
                        gen->endFile(msgm
                                , res[k-1][17] //const std::string& contact_handle
                                , res[k-1][4] //const std::string& contact_name
                                , res[k-1][5] //const std::string& contact_org
                                , res[k-1][14] //const std::string& contact_street1
                                , res[k-1][15] //const std::string& contact_street2
                                , res[k-1][16] //const std::string& contact_street3
                                , res[k-1][7] //const std::string& contact_city
                                , res[k-1][13] //const std::string& contact_state
                                , res[k-1][8] //const std::string& contact_code
                                , res[k-1][9] //const std::string& contact_country
                                , res[k-1][18] //unsigned long long contact_object_registry_id
                                , res[k-1][19] //unsigned long long contact_history_historyid
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
      Nsset::Manager *nm,
      Keyset::Manager *km,
      Domain::Manager *dm,
      Document::Manager *docm,
      Registrar::Manager *rm,
      Messages::ManagerPtr msgm
    )
    {
      return new ManagerImpl(db,mm,cm,nm, km, dm,docm,rm, msgm);
    }

}//namespace LibFred::Notify
}//namespace LibFred
