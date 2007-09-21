#include "sql.h"
#include "poll.h"
#include "common_impl.h"
#include "dbsql.h"

namespace Register
{
  namespace Poll
  {
    class MessageImpl : public CommonObjectImpl, virtual public Message
    {
      TID id;
      TID registrar;
      ptime crTime;
      ptime expTime;
      bool seen;
      unsigned type;
     public:
      MessageImpl(unsigned _type)
        : id(0), registrar(0), seen(false), type(_type)
      {}
      MessageImpl(
        unsigned _type, TID _id, TID _registrar, ptime _crTime,
        ptime _expTime, bool _seen
      ) : type(_type), id(_id), registrar(_registrar), crTime(_crTime),
          expTime(_expTime), seen(_seen)
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
        return expTime;
      }
      bool getSeen() const
      {
        return seen;
      }
      unsigned getType() const
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
        unsigned _type, TID _id, TID _registrar, ptime _crTime,
        ptime _expTime, bool _seen
      ) :
    	MessageImpl(_type,_id,_registrar,_crTime,_expTime, _seen),
    	eventDate(0)
      {}
      MessageEventImpl(
        unsigned type, date _eventDate, 
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
      void setData(date _eventDate, const std::string& _objectHandle)
      {
        eventDate = _eventDate;
        objectHandle = _objectHandle;
      }
      void textDump(std::ostream& out) const 
      {
        MessageImpl::textDump(out);
        out << "eventDate:" << to_simple_string(eventDate) << " "
            << "objectHandle: " << objectHandle << " ";
      }
    };

    class MessageEventRegImpl : public MessageEventImpl,
                                virtual public MessageEventReg
    {
      std::string registrar;
     public: 
      MessageEventRegImpl(
        unsigned _type, TID _id, TID _registrar, ptime _crTime,
        ptime _expTime, bool _seen
      ) : MessageEventImpl(_type,_id,_registrar,_crTime,_expTime,_seen)
      {}
      MessageEventRegImpl(
        unsigned type, date eventDate, 
        const std::string& objectHandle, const std::string& _registrar
      ) :
        MessageEventImpl(type,eventDate,objectHandle), registrar(_registrar)
      {}
      const std::string& getRegistrarHandle() const
      {
        return registrar;
      }
      void setData(
        date _eventDate, const std::string& _objectHandle, 
        const std::string& _registrar
      )
      {
        MessageEventImpl::setData(_eventDate,_objectHandle);
        registrar = _registrar;
      }
      void textDump(std::ostream& out) const 
      {
        MessageEventImpl::textDump(out);
        out << "registrar:" << registrar << " ";
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
      MessageTechCheckImpl(
        unsigned _type, TID _id, TID _registrar, ptime _crTime,
        ptime _expTime, bool _seen
      ) : MessageImpl(_type,_id,_registrar,_crTime,_expTime, _seen)
      {}
      MessageTechCheckImpl(const std::string& _handle) :
        MessageImpl(MT_TECHCHECK), handle(_handle)
      {}
      ~MessageTechCheckImpl()
      {
        for (unsigned i=0; i<tests.size(); i++)
          delete (MessageTechCheckImpl *)tests[i];
      }
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
      void setData(const std::string& _handle)
      {
        handle = _handle;
      }      
      void textDump(std::ostream& out) const 
      {
        MessageImpl::textDump(out);
        out << "nssset:" << handle << " ";
        out << "fqdns: (";
        for (FQDNSList::const_iterator i=fqdns.begin(); i!=fqdns.end(); i++)
          out << (i != fqdns.begin() ? ":" : "") << *i;
        out << ") tests: (";
        for (TechCheckList::const_iterator i=tests.begin(); i!=tests.end(); 
             i++)
          out << (i != tests.begin() ? ":" : "")
              << (*i)->getTestname() << ","
              << (*i)->getStatus() << ","
              << (*i)->getNote();
        out << ")";
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
        unsigned _type, TID _id, TID _registrar, ptime _crTime,
        ptime _expTime, bool _seen
      ) : 
        MessageImpl(_type,_id,_registrar,_crTime,_expTime,_seen), 
        credit(0), limit(0)
      {}
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
      void setData(
        const std::string& _zone, CreditType _credit, CreditType _limit
      )
      {
    	zone = _zone;
    	credit = _credit; 
    	limit = _limit;
      }      
      void textDump(std::ostream& out) const 
      {
        MessageImpl::textDump(out);
        out << "zone:" << getZone() << " "
            << "credit: " << getCredit() << " "
            << "limit: " << getLimit();
      }
    };

    class ListImpl : public CommonListImpl, virtual public List
    {
      TID registrarFilter;
      std::string registrarHandleFilter;
      bool nonExpiredFilter;
      bool nonSeenFilter;
      unsigned type;
     public:
      ListImpl(DB* _db) : CommonListImpl(_db), 
        registrarFilter(0), nonExpiredFilter(false), nonSeenFilter(false),
        type(0)
      {}
      virtual Message* getMessage(unsigned idx)
      {
        return dynamic_cast<Message *>(get(idx));
      }
      virtual void setTypeFilter(unsigned _type)
      {
        type = _type;
      }
      virtual void setRegistrarFilter(TID id)
      {
        registrarFilter = id;
      }
      virtual void setRegistrarHandleFilter(const std::string& handle)
      {
        registrarHandleFilter = handle;
      }
      virtual void setNonExpiredFilter(bool exp)
      {
        nonExpiredFilter = exp;
      }
      virtual void setNonSeenFilter(bool seen)
      {
        nonSeenFilter = seen;
      }      
      void clearFilter()
      {
        CommonListImpl::clearFilter();
        registrarHandleFilter = "";
        registrarFilter = 0;
        nonExpiredFilter = false;
        nonSeenFilter = false;
      }
      void makeQuery(bool count, bool limit, std::stringstream& sql) const
      {
        std::stringstream from, where;
        sql.str("");
        if (!count) 
          sql << "INSERT INTO " << getTempTableName() << " ";
        sql << "SELECT " << (count ? "COUNT(" : "")
            << "DISTINCT m.id" << (count ? ") " : " ");
        from << "FROM message m ";
        where << "WHERE 1=1 ";
        if (!registrarHandleFilter.empty()) {
          from << ",registrar r ";
          where << "AND m.clid=r.id AND r.handle='" 
                << registrarHandleFilter << "' ";
        }
        if (idFilter)
          where << "AND m.id=" << idFilter << " ";
        if (registrarFilter)
          where << "AND m.clid=" << registrarFilter << " ";
        if (nonExpiredFilter)
          where << "AND m.exdate>CURRENT_TIMESTAMP ";
        if (nonSeenFilter)
          where << "AND NOT(m.seen) ";
        if (type)
          where << "AND m.msgtype=" << type << " ";
        if (!count) where << "ORDER BY m.id ASC ";
        if (limit) where << "LIMIT " << limitCount << " ";
        sql << from.rdbuf();
        sql << where.rdbuf();        
      }
      void reload() throw (SQL_ERROR)
      {
        // to aviod sql loads when not needed
    	bool hasTechCheck = false;
    	bool hasLowCredit= false;
    	bool hasAction= false;
    	bool hasStateChange= false;
        std::ostringstream sql;
        clear();
        fillTempTable(true);
        sql << "SELECT m.msgtype, m.id, m.clid, m.crdate, m.exdate, m.seen "
            << "FROM "
            << getTempTableName() << " tmp, message m "
            << "WHERE tmp.id=m.id "
            << "ORDER BY tmp.id ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          unsigned type = atoi(db->GetFieldValue(i,0));
          MessageImpl *o = NULL;
          switch (type) {
           case MT_TECHCHECK:
            o = new MessageTechCheckImpl(
              type,STR_TO_ID(db->GetFieldValue(i,1)),
              STR_TO_ID(db->GetFieldValue(i,2)),
              MAKE_TIME(i,3), MAKE_TIME(i,4), *db->GetFieldValue(i,5) = 't'
            );
            hasTechCheck = true;
            break;
           case MT_LOW_CREDIT:
            o = new MessageLowCreditImpl(
              type,STR_TO_ID(db->GetFieldValue(i,1)),
              STR_TO_ID(db->GetFieldValue(i,2)),
              MAKE_TIME(i,3), MAKE_TIME(i,4), *db->GetFieldValue(i,5) = 't'
            );
            hasLowCredit = true;
            break;
           case MT_TRANSFER_CONTACT:
           case MT_TRANSFER_NSSET:
           case MT_TRANSFER_DOMAIN:
            o = new MessageEventRegImpl(
              type,STR_TO_ID(db->GetFieldValue(i,1)),
              STR_TO_ID(db->GetFieldValue(i,2)),
              MAKE_TIME(i,3), MAKE_TIME(i,4), *db->GetFieldValue(i,5) = 't'
            );
            hasAction = true;
            break;
           case MT_DELETE_CONTACT:
           case MT_DELETE_NSSET:
           case MT_DELETE_DOMAIN:
           case MT_IMP_EXPIRATION:
           case MT_EXPIRATION:
           case MT_IMP_VALIDATION:
           case MT_VALIDATION:
           case MT_OUTZONE:
            o = new MessageEventImpl(
              type,STR_TO_ID(db->GetFieldValue(i,1)),
              STR_TO_ID(db->GetFieldValue(i,2)),
              MAKE_TIME(i,3), MAKE_TIME(i,4), *db->GetFieldValue(i,5) = 't'
            );
            hasStateChange = true;
            break;
           default:
            o = new MessageImpl(
              type,STR_TO_ID(db->GetFieldValue(i,1)),
              STR_TO_ID(db->GetFieldValue(i,2)),
              MAKE_TIME(i,3), MAKE_TIME(i,4), *db->GetFieldValue(i,5) = 't'
            );
          }
          if (o) olist.push_back(o);
        }
        db->FreeSelect();
        if (hasTechCheck) {
          // load name of nsset 
          sql.str("");
          sql << "SELECT tmp.id, o.name, "
              << "ARRAY_TO_STRING(cn.extra_fqdns,',') "
              << "FROM "
              << getTempTableName() << " tmp, "
              << "poll_techcheck pt, check_nsset cn, "
              << "nsset_history nh, object_registry o "
              << "WHERE tmp.id=pt.msgid AND pt.cnid=cn.id "
              << "AND cn.nsset_hid=nh.historyid AND nh.id=o.id";
          if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
          // assign to nsset
          resetIDSequence();
          for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
            MessageTechCheckImpl *m = 
              dynamic_cast<MessageTechCheckImpl *>(findIDSequence(
                STR_TO_ID(db->GetFieldValue(i,0))
              ));
            if (!m) throw SQL_ERROR();
            m->setData(db->GetFieldValue(i,1));
            std::string fqdns = db->GetFieldValue(i,2);
            while (!fqdns.empty()) {
              std::string::size_type n = fqdns.find(",");
              m->addFQDN(fqdns.substr(0,n));
              fqdns.erase(0,n);
              if (!fqdns.empty()) fqdns.erase(0,1); // remove ','
            }
          }
          db->FreeSelect();
          // test results
          sql.str("");
          sql << "SELECT tmp.id, ct.name, cr.status, cr.note "
              << "FROM "
              << getTempTableName() << " tmp, "
              << "poll_techcheck pt, check_result cr, "
              << "check_test ct "
              << "WHERE tmp.id=pt.msgid AND pt.cnid=cr.checkid "
              << "AND cr.testid=ct.id ";
          // assign to nsset
          if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
          resetIDSequence();
          for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
            MessageTechCheckImpl *m = 
              dynamic_cast<MessageTechCheckImpl *>(findIDSequence(
                STR_TO_ID(db->GetFieldValue(i,0))
              ));
            if (!m) throw SQL_ERROR();
            m->addTest(
              db->GetFieldValue(i,1),
              atoi(db->GetFieldValue(i,2)) != 1, // (ok=0,failed=1,unknown=2)
              db->GetFieldValue(i,3)
            );
          }
          db->FreeSelect();
        } // hasTechCheck
        if (hasLowCredit) {
          sql.str("");
          sql << "SELECT tmp.id, z.fqdn, pl.credit, pl.limit "
              << "FROM "
              << getTempTableName() << " tmp, "
              << "poll_lowcredit pl, zone z "
              << "WHERE tmp.id=pl.msgid AND pl.zone=z.id ";
          if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
          // assign to nsset
          resetIDSequence();
          for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
            MessageLowCreditImpl *m =
              dynamic_cast<MessageLowCreditImpl *>(findIDSequence(
                STR_TO_ID(db->GetFieldValue(i,0))
              ));
            if (!m) throw SQL_ERROR();
            m->setData(
              db->GetFieldValue(i,1),
              atol(db->GetFieldValue(i,2)),
              atol(db->GetFieldValue(i,3))
            );
          }
          db->FreeSelect();
        } // hasLowCredit
        if (hasAction) {
          sql.str("");
          sql << "SELECT tmp.id, oh.trdate::date, obr.name, r.handle "
              << "FROM "
              << getTempTableName() << " tmp, "
              << "poll_eppaction pa, object_history oh, registrar r, "
              << "object_registry obr "
              << "WHERE tmp.id=pa.msgid AND pa.objid=oh.historyid "
              << "AND oh.id=obr.id ";
          if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
          // assign to nsset
          resetIDSequence();
          for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
            MessageEventRegImpl *m =
              dynamic_cast<MessageEventRegImpl *>(findIDSequence(
                STR_TO_ID(db->GetFieldValue(i,0))
              ));
            if (!m) throw SQL_ERROR();
            m->setData(
              MAKE_DATE(i,1),
              db->GetFieldValue(i,2),
              db->GetFieldValue(i,3)
            );
          }
          db->FreeSelect(); 
        } // hasAction
        if (hasStateChange) {
          sql.str("");
          sql << "SELECT tmp.id, oh.trdate::date, obr.name, r.handle "
              << "FROM "
              << getTempTableName() << " tmp, "
              << "poll_statechange ps, object_state s, "
              << "object_history oh, domain_history dh, "
              << "object_registry obr "
              << "LEFT JOIN enumval_history eh (ON eh.historyid=pa.oid)"
              << "WHERE tmp.id=ps.msgid AND ps.stateid=oh.historyid "
              << "AND oh.id=obr.id ";
          if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
          // assign to nsset
          resetIDSequence();
          for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
            MessageEventRegImpl *m =
              dynamic_cast<MessageEventRegImpl *>(findIDSequence(
                STR_TO_ID(db->GetFieldValue(i,0))
              ));
            if (!m) throw SQL_ERROR();
            m->setData(
              MAKE_DATE(i,1),
              db->GetFieldValue(i,2),
              db->GetFieldValue(i,3)
            );
          }
          db->FreeSelect(); 
        } // hasStateChange
      }
      virtual const char *getTempTableName() const
      {
        return "tmp_poll_filter_result";
      }
      /// extract first message from list for passing it outside
      Message *extractFirst()
      {
        if (!getCount()) return NULL;
        Message *m = getMessage(0);
        olist.erase(olist.begin());
        return m;
      }
    };
    
    class ManagerImpl : public Manager
    {
      DB* db;
      void createMessage(TID registrar, unsigned type) throw (SQL_ERROR)
      {
        std::stringstream sql;
        sql << "INSERT INTO message (id, clid, crdate, exdate, seen, msgtype) "
        	<< "VALUES ("
        	<< "nextval('message_id_seq')," 
        	<< registrar << ","
        	<< "CURRENT_TIMESTAMP, CURRENT_TIMESTAMP + INTERVAL '14 days',"
        	<< "'f'," << type << ")";
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
      }
      void prepareListWithNext(ListImpl&l, TID registrar) const
      {
        l.setRegistrarFilter(registrar);
        l.setNonSeenFilter(true);
        l.setNonExpiredFilter(true);
      }
     public:
      ManagerImpl(DB* _db) : db(_db) 
      {
      }
      unsigned long getMessageCount(TID registrar) const
      {
        ListImpl l(db);
        prepareListWithNext(l,registrar);
        l.makeRealCount();        
        return l.getRealCount();
      }
      Message* getNextMessage(TID registrar) // const
      {
        ListImpl l(db);
        prepareListWithNext(l,registrar);
        l.setLimit(1);
        l.reload();
        return l.extractFirst();
      }
      TID getNextMessageId(TID registrar) const
      {
        ListImpl l(db);
        prepareListWithNext(l,registrar);
        l.setLimit(1);
        l.reload();
        return l.getCount() ? l.getMessage(0)->getId() : 0;
      }
      void setMessageSeen(TID message, TID registrar) throw (NOT_FOUND)
      {
        ListImpl l(db);
        prepareListWithNext(l,registrar);
        l.setIdFilter(message);
        l.reload();
        if (l.getCount() != 1) throw NOT_FOUND();
        std::stringstream sql;
        sql << "UPDATE message SET seen='t' WHERE id=" << message;
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();        
      }
      virtual List* createList()
      {
        return new ListImpl(db);
      }
      virtual void createActionMessage(
        TID registrar, unsigned type, TID objectId
      ) throw (SQL_ERROR)
      {
        createMessage(registrar,type);
        std::stringstream sql;
        sql << "INSERT INTO poll_eppaction (msgid,objid) "
            << "SELECT currval('message_id_seq'), historyid "
            << "FROM object_registry WHERE id=" << objectId ; 
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();        
      }
      
    };

    Manager *Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }
    
  };
};
