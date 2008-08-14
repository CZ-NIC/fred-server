#ifndef _AUTH_INFO_H_
#define _AUTH_INFO_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "types.h"
#include "exceptions.h"
#include "mailer.h"
#include "documents.h"

using namespace boost::posix_time;

/// forward declared parameter type 
class DB;

namespace Register
{
  namespace AuthInfoRequest
  {
    /// Type of auth_info request
    enum RequestType 
    {
      RT_EPP, ///< Request was created by registrar through EPP
      RT_AUTO_PIF, ///< Request for automatic answer created through PIF
      RT_EMAIL_PIF, ///< Request waiting for autorization by signed email
      RT_POST_PIF ///< Request waiting for autorization by checked letter
    };
    /// Type for queried object
    enum ObjectType
    {
      OT_DOMAIN, ///< Object of request is domain
      OT_CONTACT, ///< Object of request is contact
      OT_NSSET, ///< Object of request is nsset
      OT_KEYSET ///< Object of request is keyset
    };
    /// Status of request
    enum RequestStatus
    {
      RS_NEW, ///< Request was created and waiting for autorization 
      RS_ANSWERED, ///< Email with answer was sent
      RS_INVALID ///< Time passed without authorization 
    };
    /// Detail od request
    class Detail
    {
     public:
      virtual ~Detail() {}
      virtual TID getId() const = 0;
      virtual TID getObjectId() const = 0;
      virtual const std::string& getObjectHandle() const = 0;
      virtual ObjectType getObjectType() const = 0;
      virtual RequestType getRequestType() const = 0;
      virtual RequestStatus getRequestStatus() const = 0;
      virtual ptime getCreationTime() const = 0;
      virtual ptime getClosingTime() const = 0;
      virtual const std::string& getReason() const = 0;
      virtual const std::string& getEmailToAnswer() const = 0;
      virtual TID getAnswerEmailId() const = 0;
      virtual const std::string& getRegistrarName() const = 0;
      virtual const std::string& getRegistrarInfo() const = 0;
      virtual const std::string& getSvTRID() const = 0;
    }; // Detail
    class List
    {
     public:
      virtual ~List() {}
      virtual unsigned getCount() const = 0;
      virtual Detail *get(unsigned idx) const = 0;
      virtual void setIdFilter(TID id) = 0;
      virtual void setHandleFilter(const std::string& handle) = 0;
      virtual void setEmailFilter(const std::string& email) = 0;
      virtual void setReasonFilter(const std::string& reason) = 0;
      virtual void setSvTRIDFilter(const std::string& svTRID) = 0;
      virtual void setRequestTypeFilter(RequestType type) = 0;
      virtual void setRequestTypeIgnoreFilter(bool ignore) = 0;
      virtual void setRequestStatusFilter(RequestStatus status) = 0;
      virtual void setRequestStatusIgnoreFilter(bool ignore) = 0;
      virtual void setCreationTimeFilter(time_period period) = 0;
      virtual void setCloseTimeFilter(time_period period) = 0;
      virtual void reload() throw (SQL_ERROR) = 0;
      virtual void clearFilter() = 0;
    }; // List
    class Manager
    {
     public:
      /// Exception for unspecified email in ([AUTO|POST]_PIF) request
      struct BAD_EMAIL {};
      /// Exception for bad object id
      struct OBJECT_NOT_FOUND {};
      /// Exception for bad EPP action id in EPP request
      struct ACTION_NOT_FOUND {};
      /// Exception for bad request id
      struct REQUEST_NOT_FOUND {};
      /// Exception in processing closed request
      struct REQUEST_CLOSED {};
      virtual ~Manager() {}
      /// Create request for auth_info
      virtual TID createRequest(
        TID objectId, ///< id of object to take auth_info from
        RequestType requestType, ///< type of request
        TID eppActionId, ///< id of EPP action (EPP)
        const std::string& requestReason, ///< reason from PIF (*_PIF)
        const std::string& emailToAnswer ///< email to answer ([AUTO|POST]_PIF)
      ) throw (BAD_EMAIL, OBJECT_NOT_FOUND, ACTION_NOT_FOUND, SQL_ERROR, Mailer::NOT_SEND) = 0;
      /// Process request by sending email with auth_info or closing as invalid
      virtual void processRequest(TID id, bool invalid) 
        throw (REQUEST_NOT_FOUND, REQUEST_CLOSED, SQL_ERROR, Mailer::NOT_SEND) = 0;
      virtual void getRequestPDF(
        TID id, const std::string& lang, 
        std::ostream& out
      ) throw (REQUEST_NOT_FOUND, SQL_ERROR, Document::Generator::ERROR) = 0;
      /// Create list of requests
      virtual List *getList() = 0;
      /// factory method
      static Manager *create(
        DB *db, 
        Mailer::Manager *mm,
        Document::Manager *docman
      );      
    }; // Manager
  } // AuthInfo
} // Register

#endif
