#ifndef _POLL_H_
#define _POLL_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "types.h"
#include "exceptions.h"

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
      MT_TRANSFER_CONTACT,
      MT_TRANSFER_NSSET,
      MT_TRANSFER_DOMAIN,
      MT_DELETE_CONTACT,
      MT_DELETE_NSSET,
      MT_DELETE_DOMAIN,
      MT_TECHCHECK,
      MT_IMP_EXPIRATION,
      MT_EXPIRATION,
      MT_IMP_VALIDATION,
      MT_VALIDATION,
      MT_OUTZONE,
      MT_LOW_CREDIT
    };

    class Message
    {
     protected:
      virtual ~Message() {}
     public:
      virtual TID getId() const = 0;
      virtual TID getRegistrar() const = 0;
      virtual ptime getCrTime() const = 0;
      virtual ptime getExpirationTime() const = 0;
      virtual bool getSeen() const = 0;
      virtual MessageType getType() const = 0;
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
      /// mark message as seen
      virtual void setMessageSeen(TID message) throw 
        (NOT_FOUND) = 0;;
      /// factory method
      static Manager *create(DB *db);      
    };

  }
}

#endif
