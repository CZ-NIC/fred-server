#include "notify.h"
#include "dbsql.h"
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
            << "FROM domain d, nsset_contact_map ncm contact c "
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
        if (db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
      }
      void notifyStateChanges() throw (SQL_ERROR)
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
            << "nt.state_id=ns.state_id AND nt.type=ns.type)";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          TID state = STR_TO_ID(db->GetFieldValue(i,0));
          // notification id
          unsigned ntype = atoi(db->GetFieldValue(i,1));
          // mail type id
          std::string mtype = db->GetFieldValue(i,2);
          // emails flag (1=normal(admins), 2=techs)
          unsigned emailsType = atoi(db->GetFieldValue(i,3));
          // object id
          TID obj = STR_TO_ID(db->GetFieldValue(i,4));
          // emails flag (1=normal(admins), 2=techs)
          unsigned otype = atoi(db->GetFieldValue(i,5));
          Register::Mailer::Parameters params;
          // TODO: handles are obsolete, should consider alternative solution
          Register::Mailer::Handles handles;
          Register::Mailer::Attachments attach;
          std::string emails;
          try {
            switch (otype) {
             case 1: // contact
              fillContactParams(obj,params);
              emails = getContactEmails(obj);
              break;
             case 2: // nsset
              fillNSSetParams(obj,params);
              emails = getNSSetTechEmails(obj);
              break;
             case 3: // domain
              fillDomainParams(obj,params);
              emails = (emailsType == 1 ? getDomainAdminEmails(obj) : 
                                          getDomainTechEmails(obj));
              break;
            }
            TID mail = mm->sendEmail("",emails,"",mtype,params,handles,attach);
            saveNotification(state,ntype,mail);
          } 
          catch (...) { break; }
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
