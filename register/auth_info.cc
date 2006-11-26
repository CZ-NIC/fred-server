#include "auth_info.h"
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
      std::string registrarName;
      Mailer::Manager *mm;
      DB *db;
     public:
      DetailImpl(
        unsigned long _id, unsigned long _objectId,
        const std::string& _objectHandle, ObjectType _objectType,
        RequestType _requestType, RequestStatus _requestStatus,
        ptime _creationTime, ptime _closingTime,
        const std::string& _reason, const std::string& _emailToAnswer,
        unsigned long _answerEmailId, const std::string& _registrarName,
        Mailer::Manager *_mm, DB *_db
      ) :
        id(_id), objectId(_objectId), objectHandle(_objectHandle),
        objectType(_objectType), requestType(_requestType),
        requestStatus(_requestStatus), creationTime(_creationTime),
        closingTime(_closingTime), reason(_reason), 
        emailToAnswer(_emailToAnswer), answerEmailId(_answerEmailId),
        registrarName(_registrarName), mm(_mm), db(_db)
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
      /// Query object for its addresses where to send answer
      std::string getEmailAddresses()
      {
        return emailToAnswer;
      }
      /// Query object for its authinfo
      std::string getAuthInfo()
      {
        return "AuthInfo";
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
      void process() throw (SQL_ERROR)
      {
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
        mm->sendEmail(
          "auth_info@nic.cz",
          getEmailAddresses(),
          "AuthInfo Request",
          getTemplateName(),params,handles
        );
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
      RequestStatus statusFilter;
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
      virtual void setRequestStatusFilter(RequestStatus status)
      {
        statusFilter = status;
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
        // creationTimeFIlter
        // closeTimeFilter
        // typeFilter
        // statusFilter 
      }      
      virtual void reload() throw (SQL_ERROR)
      {
        clear();
        DetailImpl *d = new DetailImpl(
          0, 0, "CID:JARA", OT_CONTACT, RT_AUTO_PIF, RS_NEW,
          ptime(boost::posix_time::second_clock::local_time()), 
          ptime(not_a_date_time), 
          "reason", 
          "jara@talir.cz", 0, "REG_IPEX",
          mm, db
        );
        requests.push_back(d);
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
        processRequest(0);
        return 0;
      }
      void processRequest(unsigned id)
        throw (REQUEST_NOT_FOUND, REQUEST_CLOSED, SQL_ERROR)
      {
        ListImpl l(mm,db);
        l.setIdFilter(id);
        l.reload();
        if (l.getCount() !=1) throw REQUEST_NOT_FOUND();
        DetailImpl *d = dynamic_cast<DetailImpl *>(l.get(0));
        if (d->getRequestStatus() != RS_NEW) throw REQUEST_CLOSED();
        d->process();
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
