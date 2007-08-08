#include "poll.h"

namespace Register
{
  namespace Poll
  {
    class MessageImpl : virtual public Message
    {
      TID id;
      TID registrar;
      ptime crTime;
      ptime expirationTime;
      bool seen;
      MessageType type;
     public:
      MessageImpl(MessageType _type)
        : id(0), registrar(0), seen(false), type(_type)
      {}
      TID getId() const
      {
        return id;
      }
      TID getRegistrar() const
      {
        return registrar;
      }
      ptime getCrTime() const
      {
        return crTime;
      }
      ptime getExpirationTime() const
      {
        return expirationTime;
      }
      bool getSeen() const
      {
        return seen;
      }
      MessageType getType() const
      {
        return type;
      }
      void textDump(std::ostream& out) const 
      {
        out << "id:" << id 
            << " registrar:" << registrar 
            << " type:" << type << std::endl;
      }

    };
    
    class MessageEventImpl : public MessageImpl,
                             virtual public MessageEvent
    {
      date eventDate;
      std::string objectHandle;
     public:
      MessageEventImpl(
        MessageType type, date _eventDate, 
        const std::string& _objectHandle
      ) : 
        MessageImpl(type), eventDate(_eventDate), objectHandle(_objectHandle)
      {}
      date getEventDate() const
      {
        return eventDate;
      }
      const std::string& getObjectHandle() const
      {
        return objectHandle;
      }
    };

    class MessageEventRegImpl : public MessageEventImpl,
                                virtual public MessageEventReg
    {
      std::string registrar;
     public: 
      MessageEventRegImpl(
        MessageType type, date eventDate, 
        const std::string& objectHandle, const std::string& _registrar
      ) :
        MessageEventImpl(type,eventDate,objectHandle), registrar(_registrar)
      {}
      const std::string& getRegistrarHandle() const
      {
        return registrar;
      }
    };

    class MessageTechCheckItemImpl : virtual public MessageTechCheckItem
    {
      std::string testname;
      bool status;
      std::string note;
     public:
      MessageTechCheckItemImpl(
        std::string _testname,
        bool _status,
        std::string _note
        ) : 
        testname(_testname), status(_status), note(_note)
      {}
      const std::string& getTestname() const
      {
        return testname;
      }
      bool getStatus() const
      {
        return status;
      }
      const std::string& getNote() const
      {
        return note;
      }
    };

    class MessageTechCheckImpl : public MessageImpl,
                                 virtual public MessageTechCheck
    {
      std::string handle;
      FQDNSList fqdns;
      TechCheckList tests;
     public:
      MessageTechCheckImpl(const std::string& _handle) :
        MessageImpl(MT_TECHCHECK), handle(_handle)
      {}
      void addFQDN(const std::string& fqdn)
      {
        fqdns.push_back(fqdn);
      }
      void addTest(std::string testname, bool status, std::string note)
      {
        tests.push_back(new MessageTechCheckItemImpl(testname,status,note));
      }
      const std::string& getHandle() const
      {
        return handle;
      }
      const FQDNSList& getFQDNS() const
      {
        return fqdns;
      }
      const TechCheckList& getTests() const
      {
        return tests;
      }
    };

    class MessageLowCreditImpl : public MessageImpl,
                                 virtual public MessageLowCredit
    {
      std::string zone;
      CreditType credit;
      CreditType limit;
     public:
      MessageLowCreditImpl(
        const std::string& _zone, CreditType _credit, CreditType _limit
      ) :
        MessageImpl(MT_LOW_CREDIT), zone(_zone), credit(_credit), 
        limit(_limit)
      {}
      const std::string& getZone() const
      {
        return zone;
      }
      CreditType getCredit() const
      {
        return credit;
      }
      CreditType getLimit() const
      {
        return limit;
      }
    };

    class ManagerImpl : public Manager
    {
      DB* db;
      std::vector<Message *> tmp;
      static unsigned tmp_i;
     public:
      ManagerImpl(DB* _db) : db(_db) 
      {
        // temporary
        tmp.push_back(
          new MessageEventImpl(MT_IMP_EXPIRATION,date(),"test.cz")
        );
        tmp.push_back(
          new MessageEventImpl(MT_EXPIRATION,date(),"test.cz")
        );
        tmp.push_back(
          new MessageEventImpl(MT_IMP_VALIDATION,date(),"test.cz")
        );
        tmp.push_back(
          new MessageEventImpl(MT_VALIDATION,date(),"test.cz")
        );
        tmp.push_back(
          new MessageEventImpl(MT_OUTZONE,date(),"test.cz")
        );
        tmp.push_back(
          new MessageEventRegImpl(MT_TRANSFER_DOMAIN,date(),"test.cz","REG-A")
        );
        tmp.push_back(
          new MessageEventImpl(MT_DELETE_DOMAIN,date(),"test.cz")
        );
        tmp.push_back(
          new MessageEventRegImpl(MT_TRANSFER_CONTACT,date(),"cid:c","REG-A")
        );
        tmp.push_back(
          new MessageEventImpl(MT_DELETE_CONTACT,date(),"cid:c")
        );
        tmp.push_back(
          new MessageEventRegImpl(MT_TRANSFER_NSSET,date(),"nssid:n1","REG-A")
        );
        tmp.push_back(
          new MessageEventImpl(MT_DELETE_NSSET,date(),"nssid:n1")
        );
        MessageTechCheckImpl *m = new MessageTechCheckImpl("NSSID:TEST");
        m->addTest("test1",true,"note1");
        m->addTest("test2",false,"note1");
        m->addFQDN("test1.cz");
        m->addFQDN("test2.cz");
        tmp.push_back(m);
        tmp.push_back(
          new MessageLowCreditImpl("test.cz",100,200)
        );
      }
      unsigned long getMessageCount(TID registrar) const
      {
        return tmp.size() - tmp_i;
      }
      Message* getNextMessage(TID registrar) // const
      {
        if (tmp_i >= tmp.size()) return NULL;
        return tmp[tmp_i++];
      }
      TID getNextMessageId(TID registrar) const
      {
        return tmp_i+1;
      }
      void setMessageSeen(TID message) throw (NOT_FOUND)
      {
        if (!message) throw NOT_FOUND();
        tmp_i = 0;
      }
    };
    
    unsigned ManagerImpl::tmp_i = 0;

    Manager *Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }
    
  };
};
