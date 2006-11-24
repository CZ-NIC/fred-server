#ifndef _AUTH_INFO_H_
#define _AUTH_INFO_H_

#include "exceptions.h"
#include "mailer.h"

/// forward declared parameter type 
class DB;

namespace Register
{
  namespace AuthInfoRequest
  {
    class Detail
    {
    }; // Detail
    class List
    {
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
      /// Type of auth_info request
      enum RequestType 
      {
        RT_EPP, ///< Request was created by registrar through EPP
        RT_AUTO_PIF, ///< Request for automatic answer created through PIF
        RT_EMAIL_PIF, ///< Request waiting for autorization by signed email
        RT_POST_PIF ///< Request waiting for autorization by checked letter
      };
      virtual ~Manager() {}
      /// Create request for auth_info
      virtual unsigned long createRequest(
        unsigned long objectId, ///< id of object to take auth_info from
        RequestType requestType, ///< type of request
        unsigned long eppActionId, ///< id of EPP action (EPP)
        const std::string& requestReason, ///< reason from PIF (*_PIF)
        const std::string& emailToAnswer ///< email to answer ([AUTO|POST]_PIF)
      ) throw (BAD_EMAIL, OBJECT_NOT_FOUND, ACTION_NOT_FOUND, SQL_ERROR) = 0;
      /// Process request by sending email with auth_info
      virtual void processRequest(unsigned id) = 0;
      /// factory method
      static Manager *create(DB *db, Register::Mailer::Manager *mm);      
    }; // Manager
  } // AuthInfo
} // Register

#endif
