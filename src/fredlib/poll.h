#ifndef _POLL_H_
#define _POLL_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "types.h"
#include "types/money.h"
#include "exceptions.h"
#include "common_object.h"
#include <ostream>
#include "old_utils/dbsql.h"
#include "logger_client.h"

using namespace boost::posix_time;
using namespace boost::gregorian;

namespace Fred
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
      MT_OUTZONE = 13,
      MT_TRANSFER_KEYSET = 14,
      MT_DELETE_KEYSET = 15,
      MT_REQUEST_FEE_INFO = 16
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

    class MessageRequestFeeInfo : virtual public Message
    {
     public:
         virtual ~MessageRequestFeeInfo() {}
         virtual const ptime& getPeriodFrom() const = 0;
         virtual const ptime& getPeriodTo() const = 0;
         virtual const unsigned long long& getTotalFreeCount() const = 0;
         virtual const unsigned long long& getUsedCount() const = 0;
         virtual const std::string& getPrice() const = 0;
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
      virtual unsigned long getMessageCount(std::string registrar) const = 0;
      virtual unsigned long getMessageCount(TID registrar) const = 0;
      /// return next unseen and unexpired message for registrar
      virtual Message* getNextMessage(std::string registrar) = 0; // const = 0;
      virtual Message* getNextMessage(TID registrar) = 0; // const = 0;
      /// return id of next unseen and unexpired message for registrar
      virtual TID getNextMessageId(std::string registrar) const = 0;;
      virtual TID getNextMessageId(TID registrar) const = 0;;
      /// mark message as seen, check ownership to registrar and if unseen
      virtual void setMessageSeen(TID message, std::string registrar) = 0;
      virtual void setMessageSeen(TID message, TID registrar) = 0;
      /// create action poll message (used in EPP transfer commands)
      virtual void createActionMessage(
        TID registrar, unsigned type, TID objectId) = 0;
      /// create messages about state change (called regulary after)
      virtual void createStateMessages(
        const std::string& exceptList, int limit, std::ostream* debug) = 0;
      /// create messages about low credit (called after credit change)
      virtual void createLowCreditMessages() = 0;
      /// create messages about request count and request fee
      virtual void createRequestFeeMessages(Logger::LoggerClient *logger_client, boost::gregorian::date period_to) = 0;
      /// get last request fee message data
      virtual std::auto_ptr<MessageRequestFeeInfo> getLastRequestFeeInfoMessage(const std::string &_registrar) const = 0;
      /// get request fee message data with specified PeriodTo
      virtual std::auto_ptr<MessageRequestFeeInfo> getRequestFeeInfoMessage(const Database::ID &registrar_id, const ptime &period_to) const = 0;
      // create list of messages
      virtual List* createList() = 0;
      /// create poll message about request fees - meant mostly for use in tests
      virtual void save_poll_request_fee(
              const Database::ID &reg_id,
              const boost::gregorian::date &period_from,
              const boost::gregorian::date &period_to,
              const unsigned long long &total_free_count,
              const unsigned long long &request_count,
              const Money &price) = 0;
      /// factory method
      static Manager *create(DBSharedPtr db);
    };

  }
}

#endif
