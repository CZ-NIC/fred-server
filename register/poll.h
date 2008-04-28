#ifndef _POLL_H_
#define _POLL_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "types.h"
#include "exceptions.h"
#include "common_object.h"
#include <ostream>

using namespace boost::posix_time;
using namespace boost::gregorian;

/// forward declared parameter type 
class DB;

namespace Register
{
  namespace Poll
  {
    enum MessageType
    {
      MT_LOW_CREDIT = 1,
      MT_TECHCHECK = 2,
      MT_TRANSFER_CONTACT = 3,
      MT_TRANSFER_NSSET = 4,
      MT_TRANSFER_DOMAIN = 5,
      MT_DELETE_CONTACT = 6,
      MT_DELETE_NSSET = 7,
      MT_DELETE_DOMAIN = 8,
      MT_IMP_EXPIRATION = 9,
      MT_EXPIRATION = 10,
      MT_IMP_VALIDATION = 11,
      MT_VALIDATION = 12,
      MT_OUTZONE = 13
    };

    class Message
    {
     public:
      // destructor should be protected but cannot because getNextMessage() 
      virtual ~Message() {}
      virtual TID getId() const = 0;
      virtual TID getRegistrar() const = 0;
      virtual ptime getCrTime() const = 0;
      virtual ptime getExpirationTime() const = 0;
      virtual bool getSeen() const = 0;
      virtual unsigned getType() const = 0;
      virtual void textDump(std::ostream& out) const = 0;
    };
    
    class MessageEvent : virtual public Message
    {
     public:
      virtual ~MessageEvent() {}
      virtual date getEventDate() const = 0;
      virtual const std::string& getObjectHandle() const = 0;
    };

    class MessageEventReg : virtual public MessageEvent
    {
     public: 
      virtual ~MessageEventReg() {}
      virtual const std::string& getRegistrarHandle() const = 0;
    };

    class MessageTechCheckItem
    {
     protected:
      virtual ~MessageTechCheckItem() {}
     public:
      /// name (identifier) of technical test
      virtual const std::string& getTestname() const = 0;
      /// result (T = OK or Unknown, F = Failure)
      virtual bool getStatus() const = 0;
      /// optional text note (not used yet)
      virtual const std::string& getNote() const = 0;
    };

    class MessageTechCheck : virtual public Message
    {
     public:
      virtual ~MessageTechCheck() {}
      /// type for list of fqdns
      typedef std::vector<std::string> FQDNSList;
      /// type for list of tech-check results
      typedef std::vector<MessageTechCheckItem *> TechCheckList;
      /// handle of tested nsset
      virtual const std::string& getHandle() const = 0;
      /// list of tested domains
      virtual const FQDNSList& getFQDNS() const = 0;
      ///< list of executed tests
      virtual const TechCheckList& getTests() const = 0; 
    };

    class MessageLowCredit : virtual public Message
    {
     public:
      virtual ~MessageLowCredit() {}
      typedef unsigned long CreditType;
      virtual const std::string& getZone() const = 0;
      virtual CreditType getCredit() const = 0;
      virtual CreditType getLimit() const = 0;
    };

    class List : virtual public CommonList
    {
     public:
      /// return message by index
      virtual Message* getMessage(unsigned idx) = 0;
      /// set filter for type
      virtual void setTypeFilter(unsigned type) = 0;
      /// set filter for registrar
      virtual void setRegistrarFilter(TID id) = 0;
      /// set filter for registrar by handle
      virtual void setRegistrarHandleFilter(const std::string& handle) = 0;
      /// set filter for registrar by handle
      virtual void setNonExpiredFilter(bool exp) = 0;
      /// set filter for registrar by handle
      virtual void setNonSeenFilter(bool seen) = 0;
    };
    
    class Manager
    {
     public:
      virtual ~Manager() {}
      /// return count of messages in queue for registrar
      virtual unsigned long getMessageCount(TID registrar) const = 0;
      /// return next unseen and unexpired message for registrar
      virtual Message* getNextMessage(TID registrar) = 0; // const = 0;
      /// return id of next unseen and unexpired message for registrar
      virtual TID getNextMessageId(TID registrar) const = 0;;
      /// mark message as seen, check ownership to registrar and if unseen
      virtual void setMessageSeen(TID message, TID registrar) 
        throw (NOT_FOUND) = 0;
      /// create action poll message (used in EPP transfer commands)
      virtual void createActionMessage(
        TID registrar, unsigned type, TID objectId
      ) throw (SQL_ERROR) = 0;
      /// create messages about state change (called regulary after)
      virtual void createStateMessages(
        const std::string& exceptList, int limit, std::ostream* debug
      ) throw (SQL_ERROR) = 0;
      /// create messages about low credit (called after credit change)
      virtual void createLowCreditMessages() throw (SQL_ERROR) = 0;
      // create list of messages
      virtual List* createList() = 0;
      /// factory method
      static Manager *create(DB *db);      
    };

  }
}

#endif
