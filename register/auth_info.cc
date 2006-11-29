#include "auth_info.h"
#include "object_impl.h"
#include "dbsql.h"
//#include </usr/include/boost/date_time/time_clock.hpp>
#include </usr/include/boost/date_time/local_time/local_time.hpp>
#include <sstream>

#define PIF_PAGE "enum.nic.cz";

//using namespace boost::date_time;
using namespace boost::local_time;

namespace Register
{
  namespace AuthInfoRequest
  {
    class DetailImpl : virtual public Detail
    {
      unsigned long id;
      unsigned long objectId;
      std::string objectHandle;
      ObjectType objectType;
      RequestType requestType;
      RequestStatus requestStatus;
      ptime creationTime;
      ptime closingTime;
      std::string reason;
      std::string emailToAnswer;
      unsigned long answerEmailId;
      unsigned long actionId;
      std::string registrarName;
      std::string svTRID;
      Mailer::Manager *mm;
      DB *db;
     public:
      DetailImpl(
        unsigned long _id, unsigned long _objectId,
        const std::string& _objectHandle, ObjectType _objectType,
        RequestType _requestType, RequestStatus _requestStatus,
        ptime _creationTime, ptime _closingTime,
        const std::string& _reason, const std::string& _emailToAnswer,
        unsigned long _answerEmailId, unsigned long _actionId,
        const std::string& _registrarName, const std::string& _svTRID,
        Mailer::Manager *_mm, DB *_db
      ) :
        id(_id), objectId(_objectId), objectHandle(_objectHandle),
        objectType(_objectType), requestType(_requestType),
        requestStatus(_requestStatus), creationTime(_creationTime),
        closingTime(_closingTime), reason(_reason), 
        emailToAnswer(_emailToAnswer), answerEmailId(_answerEmailId),
        actionId(_actionId), registrarName(_registrarName), svTRID(_svTRID),
        mm(_mm), db(_db)
      {}
      virtual unsigned long getId() const
      {
        return id;
      }
      virtual unsigned long getObjectId() const
      {
        return objectId;
      }
      virtual const std::string& getObjectHandle() const
      {
        return objectHandle;
      }
      virtual ObjectType getObjectType() const
      {
        return objectType;
      }
      virtual RequestType getRequestType() const
      {
        return requestType;
      }
      virtual RequestStatus getRequestStatus() const
      {
        return requestStatus;
      }
      virtual ptime getCreationTime() const
      {
        return creationTime;
      }
      virtual ptime getClosingTime() const
      {
        return closingTime;
      }
      virtual const std::string& getReason() const
      {
        return reason;
      }
      virtual const std::string& getEmailToAnswer() const
      {
        return emailToAnswer;
      }
      virtual unsigned long getAnswerEmailId() const
      {
        return answerEmailId;
      }
      virtual unsigned long getActionId() const
      {
        return actionId;
      }
      virtual const std::string& getRegistrarName() const
      {
        return registrarName;
      }
      virtual const std::string& getSvTRID() const
      {
        return svTRID;
      }      
      /// Query object for its addresses where to send answer
      std::string getEmailAddresses()
      {
        //TODO: return specific email for RT_EPP & RT_AUTO_PIF
        return emailToAnswer;
      }
      /// Query object for its authinfo
      std::string getAuthInfo() throw (SQL_ERROR, Manager::OBJECT_NOT_FOUND) 
      {
        std::stringstream sql;
        sql << "SELECT AuthInfoPw FROM object WHERE id=" << objectId;
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        if (db->GetSelectRows() != 1) throw Manager::OBJECT_NOT_FOUND();
        return db->GetFieldValue(0,0);
      }
      /// Select template name according to request type
      std::string getTemplateName()
      {
        switch (requestType) {
          case RT_EPP: return "sendauthinfo_epp";
          case RT_AUTO_PIF:
          case RT_EMAIL_PIF:
          case RT_POST_PIF: return "sendauthinfo_pif";
        }
        return "";
      }
      /// Process request by sending email with answer 
      void process(bool invalid) throw (SQL_ERROR)
      {
      	if (invalid) requestStatus = RS_INVALID;
        {
          // TODO: repair datetime formatting
          local_time_facet *ltf = new local_time_facet("%d/%m/%Y");
          ltf->format("%d/%m/%Y");
          std::ostringstream buf;
          buf.imbue(std::locale(std::locale::classic(), ltf));
          Mailer::Parameters params;
          params["registrar"] = registrarName;
          params["wwwpage"] = PIF_PAGE;
          buf << creationTime;
          params["reqdate"] = buf.str();
          buf.str("");
          buf << id;
          params["reqid"] = buf.str();
          params["handle"] = objectHandle;
          params["authinfo"] = getAuthInfo();
          Mailer::Handles handles;
          handles.push_back(objectHandle);
          answerEmailId = mm->sendEmail(
            "auth_info@nic.cz",
            getEmailAddresses(),
            "AuthInfo Request",
            getTemplateName(),params,handles
          );
          requestStatus = RS_ANSWERED;
        }
        closingTime = ptime(boost::posix_time::second_clock::local_time());
        save();
      }
#define RT_SQL(x) ((x) == RT_EPP ? 1 : \
                  ((x) == RT_AUTO_PIF ? 2 :\
                  ((x) == RT_EMAIL_PIF ? 3 : 4)))
#define RS_SQL(x) ((x) == RS_NEW ? 1 : \
                  ((x) == RS_ANSWERED ? 2 : 3))
      void save() throw (SQL_ERROR)
      {
        std::stringstream sql;
        if (!id) {
          id = db->GetSequenceID("auth_info_requests");
          sql << "INSERT INTO auth_info_requests "
              << "(id,object_id,request_type,epp_action_id,reason,"
              << "email_to_answer) "
              << "VALUES ("
              << id << ","
              << objectId << ","
              << RT_SQL(requestType) << ",";
          if (actionId)
            sql << actionId << ",";
          else
            sql << "NULL,";
          sql << "'" << reason << "',"
              << "'" << emailToAnswer << "')";
        } else {
          // TODO: proper update
          sql << "UPDATE auth_info_requests SET "
              << "status=" << RS_SQL(requestStatus) << ","
              << "resolve_time=now() "
              << "WHERE id=" << id;
        }
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();          
      }
    }; // DetailImpl
    class ListImpl : virtual public List
    {
      std::vector<DetailImpl *> requests;
      unsigned long idFilter;
      std::string handleFilter;
      std::string emailFilter;
      std::string reasonFilter;
      std::string svTRIDFilter;
      RequestType typeFilter;
      bool typeIgnoreFilter;
      RequestStatus statusFilter;
      bool statusIgnoreFilter;
      time_period creationTimeFilter;
      time_period closeTimeFilter;     
      Mailer::Manager *mm;
      DB *db;
      void clear()
      {
        for (unsigned i=0; i<requests.size(); i++)
          delete requests[i];
        requests.clear();
      }
     public:
      ListImpl(Mailer::Manager *_mm, DB *_db) : 
        idFilter(0),
        typeIgnoreFilter(true), statusIgnoreFilter(true),
        creationTimeFilter(ptime(neg_infin),ptime(pos_infin)),
        closeTimeFilter(ptime(neg_infin),ptime(pos_infin)),         
        mm(_mm), db(_db)
      {}
      virtual unsigned long getCount() const
      {
        return requests.size();
      }
      virtual Detail *get(unsigned long idx) const
      {
        if (idx >= requests.size()) return NULL;
        return requests[idx];
      }
      virtual void setIdFilter(unsigned long _id)
      {
        idFilter = _id;
      }
      virtual void setHandleFilter(const std::string& _handle)
      {
        handleFilter = _handle;
      }        
      virtual void setEmailFilter(const std::string& _email)
      {
        emailFilter = _email;
      }
      virtual void setRequestTypeFilter(RequestType type)
      {
        typeFilter = type;
      }
      virtual void setRequestTypeIgnoreFilter(bool ignore)
      {
      	typeIgnoreFilter = ignore;
      }      
      virtual void setRequestStatusFilter(RequestStatus status)
      {
        statusFilter = status;
      }
      virtual void setRequestStatusIgnoreFilter(bool ignore)
      {
      	statusIgnoreFilter = ignore;
      }
      virtual void setCreationTimeFilter(time_period period)
      {
        creationTimeFilter = period;
      }
      virtual void setCloseTimeFilter(time_period period)
      {
        closeTimeFilter = period;
      }
      virtual void setReasonFilter(const std::string& reason)
      {
        reasonFilter = reason;
      }
      virtual void setSvTRIDFilter(const std::string& svTRID)
      {
        svTRIDFilter = svTRID;
      }
      virtual void clearFilter()
      {
        idFilter = 0;
        handleFilter = "";
        emailFilter = "";
        reasonFilter = "";
        svTRIDFilter = "";
        creationTimeFilter = time_period(ptime(neg_infin),ptime(pos_infin));
        closeTimeFilter = time_period(ptime(neg_infin),ptime(pos_infin));
        typeIgnoreFilter = true;
        statusIgnoreFilter = true;
      }
#define SQL_RT(x) ((x) == 1 ? RT_EPP : \
                  ((x) == 2 ? RT_AUTO_PIF : \
                  ((x) == 3 ? RT_EMAIL_PIF : RT_POST_PIF)))       
#define SQL_RS(x) ((x) == 1 ? RS_NEW : \
                  ((x) == 2 ? RS_ANSWERED : RS_INVALID))       
#define SQL_OT(x) ((x) == 1 ? OT_DOMAIN : \
                  ((x) == 2 ? OT_CONTACT : OT_NSSET))       
#define MAKE_TIME(ROW,COL)  \
 (ptime(db->IsNotNull(ROW,COL) ? \
 time_from_string(db->GetFieldValue(ROW,COL)) : not_a_date_time))
      virtual void reload() throw (SQL_ERROR)
      {
        clear();
        std::stringstream sql;
        sql << "SELECT air.id,o.id,o.name,o.type,air.request_type,air.status,"
            << "air.create_time,air.resolve_time,air.reason,"
            << "air.email_to_answer,air.answer_email_id,"
            << "a.id,r.handle,a.servertrid "
            << "FROM object o, auth_info_requests air "
            << "LEFT JOIN action a ON (air.epp_action_id=a.id) "
            << "LEFT JOIN login l ON (a.clientid=l.id) "
            << "LEFT JOIN registrar r ON (l.registrarid=r.id) "
            << "WHERE o.id=air.object_id ";
        SQL_ID_FILTER(sql,"air.id",idFilter);
        SQL_HANDLE_FILTER(sql,"o.name",handleFilter);
        SQL_HANDLE_FILTER(sql,"air.email_to_answer",emailFilter);
        SQL_HANDLE_FILTER(sql,"air.reason",reasonFilter);
        SQL_HANDLE_FILTER(sql,"a.servertrid",svTRIDFilter);        
        SQL_DATE_FILTER(sql,"air.create_time",creationTimeFilter);
        SQL_DATE_FILTER(sql,"air.close_time",closeTimeFilter);
        if (!typeIgnoreFilter)
          sql << "AND air.request_type=" << RT_SQL(typeFilter) << " ";
        if (!statusIgnoreFilter)
          sql << "AND air.status=" << RS_SQL(statusFilter) << " ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          DetailImpl *d = new DetailImpl(
            atoi(db->GetFieldValue(i,0)), // id
            atoi(db->GetFieldValue(i,1)), // objectId
            db->GetFieldValue(i,2), // handle
            SQL_OT(atoi(db->GetFieldValue(i,3))), // object type
            SQL_RT(atoi(db->GetFieldValue(i,4))), // request type
            SQL_RS(atoi(db->GetFieldValue(i,5))), // status
            MAKE_TIME(i,6), // create time
            MAKE_TIME(i,7), // resolve time
            db->GetFieldValue(i,8), // reason 
            db->GetFieldValue(i,9), // email
            atoi(db->GetFieldValue(i,10)), // answered email id
            atoi(db->GetFieldValue(i,11)), // action id
            db->GetFieldValue(i,12), // registrar
            db->GetFieldValue(i,13), // svtrid
            mm, db
          );
          requests.push_back(d);
        }
      }
    };
    class ManagerImpl : virtual public Manager
    {
      DB *db;
      Mailer::Manager *mm;
      ListImpl l;
     public:
      ManagerImpl(DB *_db, Mailer::Manager *_mm) : 
        db(_db), mm(_mm), l(_mm,_db)
      {}
      unsigned long createRequest(
        unsigned long objectId,
        RequestType requestType,
        unsigned long eppActionId,
        const std::string& requestReason,
        const std::string& emailToAnswer
      ) throw (BAD_EMAIL, OBJECT_NOT_FOUND, ACTION_NOT_FOUND, SQL_ERROR)
      {
      	// TODO - must be solved in specific modules (object & action)
      	// maybe it should be solved in save() but this these are not mutable
      	// membmers
      	std::stringstream sql;
      	sql << "SELECT id FROM object WHERE id=" << objectId;
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
      	if (db->GetSelectRows() != 1) throw OBJECT_NOT_FOUND();
      	sql.str("");
      	if (eppActionId) {
      	  sql << "SELECT id FROM action WHERE id=" << eppActionId;
          if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
          if (db->GetSelectRows() != 1) throw ACTION_NOT_FOUND();
        }
        // TODO - There should be specific constructor for creation
        DetailImpl d(
          0,
          // objectId details are ignored
          objectId, "", OT_DOMAIN,
          requestType, RS_NEW,
          ptime(boost::posix_time::second_clock::local_time()), 
          ptime(not_a_date_time),
          requestReason, emailToAnswer, 0,
          // actionId strings are ignored 
          eppActionId, "", "",         
          mm,db
        );
        d.save();
        // imidiate processing of automatic requests
        if (requestType == RT_EPP || requestType == RT_AUTO_PIF) {
          // must be processed with another load to fill registrar 
          // and object details
          try { processRequest(d.getId(),false); }
          catch (REQUEST_NOT_FOUND) {
            // strange!! log error
          }
          catch (REQUEST_CLOSED) {
            // strange!! somebody faster then me?
          }
        }
        return d.getId();
      }
      void processRequest(unsigned id, bool invalid)
        throw (REQUEST_NOT_FOUND, REQUEST_CLOSED, SQL_ERROR)
      {
        ListImpl l(mm,db);
        l.setIdFilter(id);
        l.reload();
        if (l.getCount() !=1) throw REQUEST_NOT_FOUND();
        DetailImpl *d = dynamic_cast<DetailImpl *>(l.get(0));
        if (d->getRequestStatus() != RS_NEW) throw REQUEST_CLOSED();
        d->process(invalid);
      }
      List *getList()
      {
        return &l;
      }
    }; // ManagerImpl
    Manager *Manager::create(DB *db, Mailer::Manager *mm)
    {
      return new ManagerImpl(db,mm);
    }
  }; // AuthInfoRequest
}; // Register
