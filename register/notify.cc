#include "notify.h"
#include "dbsql.h"
#include "log.h"
#include <sstream>

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
      Domain::Manager *dm;
     public:
      ManagerImpl(
        DB *_db,
        Mailer::Manager *_mm,
        Contact::Manager *_cm,
        NSSet::Manager *_nm,
        Domain::Manager *_dm
      ): db(_db), mm(_mm), cm(_cm), nm(_nm), dm(_dm)
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
      std::string getContactEmails(TID contact)
      {
        std::stringstream sql;
        sql << "SELECT c.email FROM contact c WHERE c.id=" << contact;
        return getEmailList(sql);        
      }
      void fillDomainParams(
        TID domain, 
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
        params["registrar"] = d->getRegistrarHandle();
        if (!d->getValExDate().is_special())
          params["valdate"] = to_iso_extended_string(d->getValExDate());
        params["exdate"] = to_iso_extended_string(d->getExpirationDate());
        params["dnsdate"] = to_iso_extended_string(
          d->getExpirationDate() + date_duration(30)
        );
        params["exreddate"] = to_iso_extended_string(
          d->getExpirationDate() + date_duration(45)
        );
      }
      void fillContactParams(
        TID id, 
        Register::Mailer::Parameters& params
      ) throw (SQL_ERROR)
      {
        std::auto_ptr<Register::Contact::List> olist(cm->createList());
        olist->setIdFilter(id);
        olist->reload();
        if (olist->getCount() != 1) throw SQL_ERROR();
        Register::Contact::Contact *o = olist->getContact(0);
        // fill params
        params["type"] = "1";
        params["handle"] = o->getHandle();
        params["deldate"] = to_iso_extended_string(
          date(day_clock::local_day())
        );
      }
      void fillNSSetParams(
        TID id, 
        Register::Mailer::Parameters& params
      ) throw (SQL_ERROR)
      {
        std::auto_ptr<Register::NSSet::List> olist(nm->createList());
        olist->setIdFilter(id);
        olist->reload();
        if (olist->getCount() != 1) throw SQL_ERROR();
        Register::NSSet::NSSet *o = olist->getNSSet(0);
        // fill params
        params["type"] = "1";
        params["handle"] = o->getHandle();
        params["deldate"] = to_iso_extended_string(
          date(day_clock::local_day())
        );
      }
      void saveNotification(TID state, unsigned notifyType, TID mail)
        throw (SQL_ERROR)
      {
        std::stringstream sql;
        sql << "INSERT INTO notify_statechange (state_id,type,mail_id) "
            << "VALUES (" << state << "," << notifyType << "," << mail << ")";
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
      }
      struct NotifyRequest { 
        TID state_id;
        unsigned type; ///< notification id
        std::string mtype; ///< template name
        unsigned emails; ///< emails flag (1=normal(admins), 2=techs)
        TID obj_id;
        unsigned obj_type;
        NotifyRequest(
          TID _state_id, unsigned _type, const std::string& _mtype,
          unsigned _emails, TID _obj_id, unsigned _obj_type
        ) :
          state_id(_state_id), type(_type), mtype(_mtype), emails(_emails),
          obj_id(_obj_id), obj_type(_obj_type)
        {}
      };
      void notifyStateChanges(
        const std::string& exceptList,
        unsigned limit,
        std::ostream *debugOutput
      ) throw (SQL_ERROR)
      {
        std::stringstream sql;
        sql << "SELECT nt.state_id, nt.type, "
            << "nt.mtype, nt.emails, nt.obj_id, nt.obj_type FROM "
            << "(SELECT s.id AS state_id, nm.id AS type, "
            << " mt.name AS mtype, nm.emails, "
            << " obr.id AS obj_id, obr.type AS obj_type "
            << " FROM object_state s, object_registry obr, "
            << " notify_statechange_map nm, mail_type mt "
            << " WHERE s.object_id=obr.id AND obr.type=nm.obj_type "
            << " AND s.state_id=nm.state_id AND mt.id=nm.mail_type_id) AS nt "
            << "LEFT JOIN notify_statechange ns ON ("
            << "nt.state_id=ns.state_id AND nt.type=ns.type) "
            << "WHERE ns.state_id ISNULL ";
        if (!exceptList.empty())
        	sql << "AND nt.type NOT IN (" << exceptList << ") ";
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
            atoi(db->GetFieldValue(i,5))
          ));
        db->FreeSelect();
        if (debugOutput) *debugOutput << "<notifications>";
        std::vector<NotifyRequest>::const_iterator i = nlist.begin();
        for (;i!=nlist.end();i++) {
          Register::Mailer::Parameters params;
          // TODO: handles are obsolete, should consider alternative solution
          Register::Mailer::Handles handles;
          Register::Mailer::Attachments attach;
          std::string emails;
          try {
            switch (i->obj_type) {
             case 1: // contact
              fillContactParams(i->obj_id,params);
              emails = getContactEmails(i->obj_id);
              break;
             case 2: // nsset
              fillNSSetParams(i->obj_id,params);
              emails = getNSSetTechEmails(i->obj_id);
              break;
             case 3: // domain
              fillDomainParams(i->obj_id,params);
              emails = (i->emails == 1 ? getDomainAdminEmails(i->obj_id) : 
                                         getDomainTechEmails(i->obj_id));
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
              *debugOutput << "</notify>";
            }
            TID mail = mm->sendEmail(
              "",emails,"",i->mtype,params,handles,attach
            );
            saveNotification(i->state_id,i->type,mail);
          } 
          catch (...) {
            LOG(
              ERROR_LOG,
              "Notfication wasn't successful (state=%d, type=%d) ",
              i->state_id, i->type
            );
          }
          if (debugOutput) *debugOutput << "</notifications>";
        }
      }
    };
    Manager *Manager::create(
      DB *db, 
      Mailer::Manager *mm,
      Contact::Manager *cm,
      NSSet::Manager *nm,
      Domain::Manager *dm
    )
    {
      return new ManagerImpl(db,mm,cm,nm,dm);
    }
  }
}
