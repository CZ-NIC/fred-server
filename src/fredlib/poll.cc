/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include "sql.h"
#include "poll.h"
#include "invoicing/invoice.h"
#include "common_impl.h"
#include "old_utils/dbsql.h"
#include "domain.h"
#include "registrar.h"

namespace Fred {
namespace Poll {

using namespace Fred::Registrar;

class MessageImpl : public CommonObjectImpl, virtual public Message {
  TID id;
  TID registrar;
  ptime crTime;
  ptime expTime;
  bool seen;
  unsigned type;

public:
  MessageImpl(unsigned _type) :
    id(0), registrar(0), seen(false), type(_type) {
  }
  MessageImpl(unsigned _type,
              TID _id,
              TID _registrar,
              ptime _crTime,
              ptime _expTime,
              bool _seen) :
    id(_id), registrar(_registrar), crTime(_crTime), expTime(_expTime),
        seen(_seen), type(_type) {
  }
  TID getId() const {
    return id;
  }
  TID getRegistrar() const {
    return registrar;
  }
  ptime getCrTime() const {
    return crTime;
  }
  ptime getExpirationTime() const {
    return expTime;
  }
  bool getSeen() const {
    return seen;
  }
  unsigned getType() const {
    return type;
  }
  void textDump(std::ostream& out) const {
    out << "id:" << id << " registrar:" << registrar << " type:" << type
        << std::endl;
  }
};


class MessageEventImpl : public MessageImpl, virtual public MessageEvent {
  date eventDate;
  std::string objectHandle;

public:
  MessageEventImpl(unsigned _type,
                   TID _id,
                   TID _registrar,
                   ptime _crTime,
                   ptime _expTime,
                   bool _seen) :
    MessageImpl(_type, _id, _registrar, _crTime, _expTime, _seen), eventDate(0) {
  }
  MessageEventImpl(unsigned type,
                   date _eventDate,
                   const std::string& _objectHandle) :
    MessageImpl(type), eventDate(_eventDate), objectHandle(_objectHandle) {
  }
  date getEventDate() const {
    return eventDate;
  }
  const std::string& getObjectHandle() const {
    return objectHandle;
  }
  void setData(date _eventDate, const std::string& _objectHandle) {
    eventDate = _eventDate;
    objectHandle = _objectHandle;
  }
  void textDump(std::ostream& out) const {
    MessageImpl::textDump(out);
    out << "eventDate:" << to_simple_string(eventDate) << " "
        << "objectHandle: " << objectHandle << " ";
  }
};


class MessageEventRegImpl : public MessageEventImpl,
    virtual public MessageEventReg {
  std::string registrar;
public:
  MessageEventRegImpl(unsigned _type,
                      TID _id,
                      TID _registrar,
                      ptime _crTime,
                      ptime _expTime,
                      bool _seen) :
    MessageEventImpl(_type, _id, _registrar, _crTime, _expTime, _seen) {
  }
  MessageEventRegImpl(unsigned type,
                      date eventDate,
                      const std::string& objectHandle,
                      const std::string& _registrar) :
    MessageEventImpl(type, eventDate, objectHandle), registrar(_registrar) {
  }
  const std::string& getRegistrarHandle() const {
    return registrar;
  }
  void setData(date _eventDate,
               const std::string& _objectHandle,
               const std::string& _registrar) {
    MessageEventImpl::setData(_eventDate, _objectHandle);
    registrar = _registrar;
  }
  void textDump(std::ostream& out) const {
    MessageEventImpl::textDump(out);
    out << "registrar:" << registrar << " ";
  }
};


class MessageTechCheckItemImpl : virtual public MessageTechCheckItem {
  std::string testname;
  bool status;
  std::string note;

public:
  MessageTechCheckItemImpl(std::string _testname,
                           bool _status,
                           std::string _note) :
    testname(_testname), status(_status), note(_note) {
  }
  const std::string& getTestname() const {
    return testname;
  }
  bool getStatus() const {
    return status;
  }
  const std::string& getNote() const {
    return note;
  }
};


class MessageTechCheckImpl : public MessageImpl, virtual public MessageTechCheck {
  std::string handle;
  FQDNSList fqdns;
  TechCheckList tests;

public:
  MessageTechCheckImpl(unsigned _type,
                       TID _id,
                       TID _registrar,
                       ptime _crTime,
                       ptime _expTime,
                       bool _seen) :
    MessageImpl(_type, _id, _registrar, _crTime, _expTime, _seen) {
  }
  MessageTechCheckImpl(const std::string& _handle) :
    MessageImpl(MT_TECHCHECK), handle(_handle) {
  }
  ~MessageTechCheckImpl() {
    for (unsigned i=0; i<tests.size(); i++)
      delete (MessageTechCheckImpl *)tests[i];
  }
  void addFQDN(const std::string& fqdn) {
    fqdns.push_back(fqdn);
  }
  void addTest(std::string testname, bool status, std::string note) {
    tests.push_back(new MessageTechCheckItemImpl(testname,status,note));
  }
  const std::string& getHandle() const {
    return handle;
  }
  const FQDNSList& getFQDNS() const {
    return fqdns;
  }
  const TechCheckList& getTests() const {
    return tests;
  }
  void setData(const std::string& _handle) {
    handle = _handle;
  }
  void textDump(std::ostream& out) const {
    MessageImpl::textDump(out);
    out << "nssset:" << handle << " ";
    out << "fqdns: (";
    for (FQDNSList::const_iterator i=fqdns.begin(); i!=fqdns.end(); i++)
      out << (i != fqdns.begin() ? ":" : "") << *i;
    out << ") tests: (";
    for (TechCheckList::const_iterator i=tests.begin(); i!=tests.end(); i++)
      out << (i != tests.begin() ? ":" : "") << (*i)->getTestname() << "," << (*i)->getStatus() << "," << (*i)->getNote();
    out << ")";
  }
};


class MessageLowCreditImpl : public MessageImpl, virtual public MessageLowCredit {
  std::string zone;
  CreditType credit;
  CreditType limit;
public:
  MessageLowCreditImpl(unsigned _type,
                       TID _id,
                       TID _registrar,
                       ptime _crTime,
                       ptime _expTime,
                       bool _seen) :
    MessageImpl(_type, _id, _registrar, _crTime, _expTime, _seen), credit(0),
        limit(0) {
  }
  MessageLowCreditImpl(const std::string& _zone,
                       CreditType _credit,
                       CreditType _limit) :
    MessageImpl(MT_LOW_CREDIT), zone(_zone), credit(_credit), limit(_limit) {
  }
  const std::string& getZone() const {
    return zone;
  }
  CreditType getCredit() const {
    return credit;
  }
  CreditType getLimit() const {
    return limit;
  }
  void setData(const std::string& _zone, CreditType _credit, CreditType _limit) {
    zone = _zone;
    credit = _credit;
    limit = _limit;
  }
  void textDump(std::ostream& out) const {
    MessageImpl::textDump(out);
    out << "zone:" << getZone() << " " << "credit: " << getCredit() << " "
        << "limit: " << getLimit();
  }
};


class MessageRequestFeeInfoImpl : public MessageImpl, virtual public MessageRequestFeeInfo
{
public:
    MessageRequestFeeInfoImpl(unsigned _type,
                       TID _id,
                       TID _registrar,
                       ptime _crTime,
                       ptime _expTime,
                       bool _seen)
        : MessageImpl(_type, _id, _registrar, _crTime, _expTime, _seen),
          period_from_(),
          period_to_(),
          total_free_count_(0),
          used_count_(0),
          price_("0.00")
    {
    }

    const ptime& getPeriodFrom() const
    {
        return period_from_;
    }

    const ptime& getPeriodTo() const
    {
        return period_to_;
    }

    const unsigned long long& getTotalFreeCount() const
    {
        return total_free_count_;
    }

    const unsigned long long& getUsedCount() const
    {
        return used_count_;
    }

    const std::string& getPrice() const
    {
        return price_;
    }

    void setData(const ptime &_period_from,
                 const ptime &_period_to,
                 const unsigned long long &_total_free_count,
                 const unsigned long long &_used_count,
                 const std::string &_price)
    {
        period_from_ = _period_from;
        period_to_ = _period_to;
        total_free_count_ = _total_free_count;
        used_count_ = _used_count;
        price_ = _price;
    }

private:
    ptime period_from_;
    ptime period_to_;
    unsigned long long total_free_count_;
    unsigned long long used_count_;
    std::string price_;
};


class ListImpl : public CommonListImpl, virtual public List {
  TID registrarFilter;
  std::string registrarHandleFilter;
  bool nonExpiredFilter;
  bool nonSeenFilter;
  unsigned type;
  std::string order_by_dir;

public:
  ListImpl(DBSharedPtr _db) :
    CommonListImpl(_db), registrarFilter(0), nonExpiredFilter(false),
        nonSeenFilter(false), type(0), order_by_dir("ASC") {
  }

  void setOrderByDesc() {
      order_by_dir = "DESC";
  }
  void setOrderByAsc() {
      order_by_dir = "ASC";
  }

  virtual Message* getMessage(unsigned idx) {
    return dynamic_cast<Message *>(get(idx));
  }
  virtual void setTypeFilter(unsigned _type) {
    type = _type;
  }
  virtual void setRegistrarFilter(TID id) {
    registrarFilter = id;
  }
  virtual void setRegistrarHandleFilter(const std::string& handle) {
    registrarHandleFilter = handle;
  }
  virtual void setNonExpiredFilter(bool exp) {
    nonExpiredFilter = exp;
  }
  virtual void setNonSeenFilter(bool seen) {
    nonSeenFilter = seen;
  }
  void clearFilter() {
    CommonListImpl::clearFilter();
    registrarHandleFilter = "";
    registrarFilter = 0;
    nonExpiredFilter = false;
    nonSeenFilter = false;
  }
  void makeQuery(bool count, bool limit, std::stringstream& sql) const {
    std::stringstream from, where;
    sql.str("");
    if (!count)
      sql << "INSERT INTO " << getTempTableName() << " ";
    sql << "SELECT " << (count ? "COUNT(" : "") << "DISTINCT m.id"
        << (count ? ") " : " ");
    from << "FROM message m ";
    where << "WHERE 1=1 ";
    if (!registrarHandleFilter.empty()) {
      from << ",registrar r ";
      where << "AND m.clid=r.id AND r.handle='" << registrarHandleFilter
          << "' ";
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
    if (!count)
      where << "ORDER BY m.id " << order_by_dir << " ";
    if (limit)
      where << "LIMIT " << load_limit_ << " ";
    sql << from.rdbuf();
    sql << where.rdbuf();
  }
  void reload() {
    // to aviod sql loads when not needed
    bool hasTechCheck = false;
    bool hasLowCredit= false;
    bool hasAction= false;
    bool hasStateChange= false;
    bool hasRequestFeeInfo = false;
    std::ostringstream sql;
    clear();
    fillTempTable(true);
    sql << "SELECT m.msgtype, m.id, m.clid, m.crdate, m.exdate, m.seen "
        << "FROM " << getTempTableName() << " tmp, message m "
        << "WHERE tmp.id=m.id " << "ORDER BY tmp.id ";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
      unsigned type = atoi(db->GetFieldValue(i, 0));
      MessageImpl *o = NULL;
      switch (type) {
        case MT_TECHCHECK:
          o = new MessageTechCheckImpl(
              type,STR_TO_ID(db->GetFieldValue(i,1)),
              STR_TO_ID(db->GetFieldValue(i,2)),
              MAKE_TIME(i,3), MAKE_TIME(i,4), *db->GetFieldValue(i,5) == 't'
          );
          hasTechCheck = true;
          break;
        case MT_LOW_CREDIT:
          o = new MessageLowCreditImpl(
              type,STR_TO_ID(db->GetFieldValue(i,1)),
              STR_TO_ID(db->GetFieldValue(i,2)),
              MAKE_TIME(i,3), MAKE_TIME(i,4), *db->GetFieldValue(i,5) == 't'
          );
          hasLowCredit = true;
          break;
        case MT_TRANSFER_CONTACT:
        case MT_TRANSFER_NSSET:
        case MT_TRANSFER_DOMAIN:
        case MT_TRANSFER_KEYSET:
          o = new MessageEventRegImpl(
              type,STR_TO_ID(db->GetFieldValue(i,1)),
              STR_TO_ID(db->GetFieldValue(i,2)),
              MAKE_TIME(i,3), MAKE_TIME(i,4), *db->GetFieldValue(i,5) == 't'
          );
          hasAction = true;
          break;
        case MT_DELETE_CONTACT:
        case MT_DELETE_NSSET:
        case MT_DELETE_DOMAIN:
        case MT_DELETE_KEYSET:
        case MT_IMP_EXPIRATION:
        case MT_EXPIRATION:
        case MT_IMP_VALIDATION:
        case MT_VALIDATION:
        case MT_OUTZONE:
          o = new MessageEventImpl(
              type,STR_TO_ID(db->GetFieldValue(i,1)),
              STR_TO_ID(db->GetFieldValue(i,2)),
              MAKE_TIME(i,3), MAKE_TIME(i,4), *db->GetFieldValue(i,5) == 't'
          );
          hasStateChange = true;
          break;
        case MT_REQUEST_FEE_INFO:
          o = new MessageRequestFeeInfoImpl(
              type,STR_TO_ID(db->GetFieldValue(i,1)),
              STR_TO_ID(db->GetFieldValue(i,2)),
              MAKE_TIME(i,3), MAKE_TIME(i,4), *db->GetFieldValue(i,5) == 't'
          );
          hasRequestFeeInfo = true;
          break;
        default:
          o = new MessageImpl(
              type,STR_TO_ID(db->GetFieldValue(i,1)),
              STR_TO_ID(db->GetFieldValue(i,2)),
              MAKE_TIME(i,3), MAKE_TIME(i,4), *db->GetFieldValue(i,5) == 't'
          );
      }
      if (o)
        data_.push_back(o);
    }
    db->FreeSelect();
    if (hasTechCheck) {
      // load name of nsset
      sql.str("");
      sql << "SELECT tmp.id, o.name, "
          << "ARRAY_TO_STRING(cn.extra_fqdns,',') " << "FROM "
          << getTempTableName() << " tmp, "
          << "poll_techcheck pt, check_nsset cn, "
          << "nsset_history nh, object_registry o "
          << "WHERE tmp.id=pt.msgid AND pt.cnid=cn.id "
          << "AND cn.nsset_hid=nh.historyid AND nh.id=o.id "
          << "ORDER BY tmp.id ";
      if (!db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
      // assign to nsset
      resetIDSequence();
      for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
        MessageTechCheckImpl
            *m =
                dynamic_cast<MessageTechCheckImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(i,
                                                                                                0)) ));
        if (!m)
          throw SQL_ERROR();
        m->setData(db->GetFieldValue(i, 1));
        std::string fqdns = db->GetFieldValue(i, 2);
        while (!fqdns.empty()) {
          std::string::size_type n = fqdns.find(",");
          m->addFQDN(fqdns.substr(0, n));
          fqdns.erase(0, n);
          if (!fqdns.empty())
            fqdns.erase(0, 1); // remove ','
        }
      }
      db->FreeSelect();
      // test results
      sql.str("");
      sql << "SELECT tmp.id, ct.name, cr.status, cr.note " << "FROM "
          << getTempTableName() << " tmp, "
          << "poll_techcheck pt, check_result cr, " << "check_test ct "
          << "WHERE tmp.id=pt.msgid AND pt.cnid=cr.checkid "
          << "AND cr.testid=ct.id " << "ORDER BY tmp.id ";
      // assign to nsset
      if (!db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
      resetIDSequence();
      for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
        MessageTechCheckImpl
            *m =
                dynamic_cast<MessageTechCheckImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(i,
                                                                                                0)) ));
        if (!m)
          throw SQL_ERROR();
        m->addTest(db->GetFieldValue(i, 1), atoi(db->GetFieldValue(i, 2)) != 1, // (ok=0,failed=1,unknown=2)
                   db->GetFieldValue(i, 3) );
      }
      db->FreeSelect();
    } // hasTechCheck
    if (hasLowCredit) {
      sql.str("");
      sql << "SELECT tmp.id, z.fqdn, pl.credit, pl.credlimit " << "FROM "
          << getTempTableName() << " tmp, " << "poll_credit pl, zone z "
          << "WHERE tmp.id=pl.msgid AND pl.zone=z.id " << "ORDER BY tmp.id ";
      if (!db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
      // assign to nsset
      resetIDSequence();
      for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
        MessageLowCreditImpl
            *m =
                dynamic_cast<MessageLowCreditImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(i,
                                                                                                0)) ));
        if (!m)
          throw SQL_ERROR();
        m->setData(db->GetFieldValue(i, 1),
                   atol(db->GetFieldValue(i, 2)),
                   atol(db->GetFieldValue(i, 3)) );
      }
      db->FreeSelect();
    } // hasLowCredit
    if (hasAction) {
      sql.str("");
      sql << "SELECT tmp.id, oh.trdate::date, obr.name, r.handle " << "FROM "
          << getTempTableName() << " tmp, "
          << "poll_eppaction pa, object_history oh, registrar r, "
          << "object_registry obr "
          << "WHERE tmp.id=pa.msgid AND pa.objid=oh.historyid "
          << "AND oh.id=obr.id AND oh.clid=r.id " << "ORDER BY tmp.id ";
      if (!db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
      // assign to nsset
      resetIDSequence();
      for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
        MessageEventRegImpl
            *m =
                dynamic_cast<MessageEventRegImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(i,
                                                                                               0)) ));
        if (!m)
          throw SQL_ERROR();
        m->setData(MAKE_DATE(i, 1),
                   db->GetFieldValue(i, 2),
                   db->GetFieldValue(i, 3) );
      }
      db->FreeSelect();
    } // hasAction
    if (hasStateChange) {
      sql.str("");
      sql << "SELECT tmp.id, "
          << "COALESCE(dh.exdate::date,s.valid_from::date), "
          << "eh.exdate::date, obr.name "
          << "FROM " << getTempTableName() << " tmp "
          << "JOIN poll_statechange ps ON (tmp.id=ps.msgid) "
          << "JOIN object_state s ON (ps.stateid=s.id) "
          << "JOIN object_registry obr ON (s.object_id=obr.id) "
          << "LEFT JOIN domain_history dh ON (s.ohid_from=dh.historyid) "
          << "LEFT JOIN enumval_history eh ON (eh.historyid=dh.historyid) "
          << "ORDER BY tmp.id ";
      if (!db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
      // assign to nsset
      resetIDSequence();
      for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
        MessageEventImpl
            *m =
                dynamic_cast<MessageEventImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(i,
                                                                                            0)) ));
        if (!m)
          throw SQL_ERROR();
        date d;
        switch (m->getType()) {
          case MT_IMP_VALIDATION:
          case MT_VALIDATION:
            d = MAKE_DATE(i, 2);
            break;
          case MT_IMP_EXPIRATION:
          case MT_EXPIRATION:
          case MT_OUTZONE:
          case MT_DELETE_CONTACT:
          case MT_DELETE_NSSET:
          case MT_DELETE_DOMAIN:
          case MT_DELETE_KEYSET:
            d = MAKE_DATE(i, 1);
            break;
          default:
            // strange data in tables
            throw SQL_ERROR();
        }
        m->setData(d, db->GetFieldValue(i, 3) );
      }
      db->FreeSelect();
    } // hasStateChange
    if (hasRequestFeeInfo)
    {
      sql.str("");
      sql << "SELECT tmp.id, "
          << "prf.period_from, "
          << "prf.period_to, "
          << "prf.total_free_count, prf.used_count, "
          << "prf.price "
          << "FROM " << getTempTableName() << " tmp "
          << "JOIN poll_request_fee prf ON prf.msgid = tmp.id "
          << "ORDER BY tmp.id ";
      if (!db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
      resetIDSequence();
      for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
        MessageRequestFeeInfoImpl *m = dynamic_cast<MessageRequestFeeInfoImpl *>(
                findIDSequence(STR_TO_ID(db->GetFieldValue(i, 0))));
        if (!m) {
          throw SQL_ERROR();
        }
        m->setData(time_from_string(db->GetFieldValue(i, 1)),
                   time_from_string(db->GetFieldValue(i, 2)),
                   boost::lexical_cast<unsigned long long>(db->GetFieldValue(i, 3)),
                   boost::lexical_cast<unsigned long long>(db->GetFieldValue(i, 4)),
                   db->GetFieldValue(i, 5));
      }
    } // hasRequestFeeInfo
  }
  virtual const char *getTempTableName() const {
    return "tmp_poll_filter_result";
  }
  /// extract first message from list for passing it outside
  Message *extractFirst() {
    if (!getCount())
      return NULL;
    Message *m = getMessage(0);
    data_.erase(data_.begin());
    return m;
  }
};


// Local transction needed for proper on commit handling of TEMP table
struct LocalTransaction {
  DBSharedPtr db;
  bool closed;
  LocalTransaction(DBSharedPtr _db) :
    db(_db), closed(false) {
    (void)db->BeginTransaction();
  }
  ~LocalTransaction() {
    if (!closed)
      db->RollbackTransaction();
  }
  void commit() {
    db->CommitTransaction();
    closed = true;
  }
};


class ManagerImpl : public Manager {
  DBSharedPtr db;
  void createMessage(TID registrar, unsigned type) {
    std::stringstream sql;
    sql << "INSERT INTO message (id, clid, crdate, exdate, seen, msgtype) "
        << "VALUES (" << "nextval('message_id_seq')," << registrar << ","
        << "CURRENT_TIMESTAMP, CURRENT_TIMESTAMP + INTERVAL '7 days',"
        << "'f'," << type << ")";
    if (!db->ExecSQL(sql.str().c_str()))
      throw SQL_ERROR();
  }
  void prepareListWithNext(ListImpl&l, TID registrar) const {
    l.setRegistrarFilter(registrar);
    l.setNonSeenFilter(true);
    l.setNonExpiredFilter(true);
  }

public:
  ManagerImpl(DBSharedPtr _db) :
    db(_db) {
  }
  unsigned long getMessageCount(std::string registrar) const
  {
      return getMessageCount(db->GetRegistrarID(registrar.c_str()));
  }
  unsigned long getMessageCount(TID registrar) const {
    ListImpl l(db);
    prepareListWithNext(l, registrar);
    l.makeRealCount();
    return l.getRealCount();
  }
  Message *getNextMessage(std::string registrar)
  {
      return getNextMessage(db->GetRegistrarID(registrar.c_str()));
  }
  Message* getNextMessage(TID registrar) // const
  {
    ListImpl l(db);
    prepareListWithNext(l, registrar);
    l.setLimit(1);
    l.reload();
    return l.extractFirst();
  }
  TID getNextMessageId(std::string registrar) const
  {
      return getNextMessageId(db->GetRegistrarID(registrar.c_str()));
  }
  TID getNextMessageId(TID registrar) const {
    ListImpl l(db);
    prepareListWithNext(l, registrar);
    l.setLimit(1);
    l.reload();
    return l.getCount() ? l.getMessage(0)->getId() : 0;
  }
  void setMessageSeen(TID message, std::string registrar)
  {
      setMessageSeen(message, db->GetRegistrarID(registrar.c_str()));
  }
  void setMessageSeen(TID message, TID registrar) {
    ListImpl l(db);
    prepareListWithNext(l, registrar);
    l.setIdFilter(message);
    l.reload();
    if (l.getCount() != 1)
      throw NOT_FOUND();
    std::stringstream sql;
    sql << "UPDATE message SET seen='t' WHERE id=" << message;
    if (!db->ExecSQL(sql.str().c_str()))
      throw SQL_ERROR();
  }
  virtual List* createList() {
    return new ListImpl(db);
  }
  virtual void createActionMessage(TID registrar, unsigned type, TID objectId)
  {
    createMessage(registrar, type);
    std::stringstream sql;
    sql << "INSERT INTO poll_eppaction (msgid,objid) "
        << "SELECT currval('message_id_seq'), historyid "
        << "FROM object_registry WHERE id=" << objectId;
    if (!db->ExecSQL(sql.str().c_str()))
      throw SQL_ERROR();
  }
  virtual void createStateMessages(const std::string& exceptList,
                                   int limit,
                                   std::ostream* debug) {
    TRACE("[CALL] Fred::Poll::createStateMessages()");
    // transaction is needed for 'ON COMMIT DROP' functionality
    LocalTransaction trans(db);
    // for each new state appearance of state type (expirationWarning,
    // expiration, validationWarning1, outzoneUnguarded and
    // deleteCandidate for all object type that has not associated
    // poll message create one new poll message
    std::string caseSQL =
    // MT_IMP_EXPIRATION
        " CASE WHEN os.state_id=8 THEN 9 "
        // MT_EXPIRATION
              "      WHEN os.state_id=9 THEN 10 "
              // MT_IMP_VALIDATION
              "      WHEN os.state_id=11 THEN 11 "
              // MT_VALIDATION
              "      WHEN os.state_id=13 THEN 12 "
              // MT_OUTZONE
              "      WHEN os.state_id=20 THEN 13 "
              // MT_DELETE_CONTACT
              "      WHEN os.state_id=17 AND ob.type=1 THEN 6 "
              // MT_DELETE_NSSET
              "      WHEN os.state_id=17 AND ob.type=2 THEN 7 "
              // MT_DELETE_DOMAIN
              "      WHEN os.state_id=17 AND ob.type=3 THEN 8 "
              // MT_DELETE_DOMAIN
              "      WHEN os.state_id=17 AND ob.type=4 THEN 15 END ";
    std::stringstream insertSelect;
    insertSelect << "SELECT "
      " oh.clid AS reg, " << caseSQL << " AS msgtype, "
      " os.id AS stateid, ob.name AS name "
      "FROM object_registry ob, object_history oh, object_state os "
      "LEFT JOIN poll_statechange ps ON (os.id=ps.stateid) "
      "WHERE os.state_id in (8,9,11,13,20,17) AND os.valid_to ISNULL "
      "AND oh.historyid=os.ohid_from AND ob.id=os.object_id "
      "AND ps.stateid ISNULL ";
    if (!exceptList.empty())
      insertSelect << "AND " << caseSQL << " NOT IN (" << exceptList << ") ";
    insertSelect << "ORDER BY os.id ";
    if (limit)
      insertSelect << "LIMIT " << limit;
    if (debug) {
      if (!db->ExecSelect(insertSelect.str().c_str()))
        throw SQL_ERROR();
      *debug << "<messages>\n";
      for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
        *debug << "<message " << "reg_id='" << db->GetFieldValue(i, 0)
            << "' msg_type='" << db->GetFieldValue(i, 1) << "' state_id='"
            << db->GetFieldValue(i, 2) << "' name='" << db->GetFieldValue(i, 3)
            << "'/>\n";
      }
      *debug << "</messages>\n";
      db->FreeSelect();
      return; // rollback (never mind in debug mode)
    }
    // create temporary table because poll message need to be inserted
    // into two tables joined by message id
    const char *create = "CREATE TEMPORARY TABLE tmp_poll_state_insert ("
      " id INTEGER PRIMARY KEY, reg INTEGER, "
      " msgtype INTEGER, stateid INTEGER "
      ") ON COMMIT DROP ";
    if (!db->ExecSQL(create))
      throw SQL_ERROR();
    std::stringstream insertTemp;
    insertTemp << "INSERT INTO tmp_poll_state_insert "
      "SELECT "
      " nextval('message_id_seq'), t.reg, t.msgtype, t.stateid "
      " FROM (" << insertSelect.str() << ") t ";
    if (!db->ExecSQL(insertTemp.str().c_str()))
      throw SQL_ERROR();
    // insert into table message appropriate part from temp table
    const char *insertMessage = "INSERT INTO message "
      "SELECT id,reg,CURRENT_TIMESTAMP,"
      "CURRENT_TIMESTAMP + INTERVAL '7days','f',msgtype "
      "FROM tmp_poll_state_insert ORDER BY stateid ";
    if (!db->ExecSQL(insertMessage))
      throw SQL_ERROR();
    // insert into table poll_statechange appropriate part from temp table
    const char *insertPollStateChange = "INSERT INTO poll_statechange "
      "SELECT id, stateid FROM tmp_poll_state_insert ORDER BY stateid ";
    if (!db->ExecSQL(insertPollStateChange))
      throw SQL_ERROR();
    trans.commit();
  }
  virtual void createLowCreditMessages() {
    // transaction is needed for 'ON COMMIT DROP' functionality
    LocalTransaction trans(db);
    // create temporary table because poll message need to be inserted
    // into two tables joined by message id
    const char *create = "CREATE TEMPORARY TABLE tmp_poll_credit_insert ("
      " id INTEGER PRIMARY KEY, zoneid INTEGER, reg INTEGER, "
      " credit INTEGER, credlimit INTEGER "
      ") ON COMMIT DROP ";
    if (!db->ExecSQL(create))
      throw SQL_ERROR();
    // for each reagistrar and zone count credit from advance invoices.
    // if credit is lower than limit and last poll message for this
    // registrar and zone is older than last advance invoice,
    // insert new poll message
    const char *insertTemp = "INSERT INTO tmp_poll_credit_insert "
        "SELECT nextval('message_id_seq') , rc.zone_id, rc.registrar_id, rc.credit, l.credlimit "
         "FROM registrar_credit rc "
             "JOIN poll_credit_zone_limit l ON rc.zone_id = l.zone "
             "LEFT JOIN (SELECT m.clid, pc.zone, MAX(m.crdate) AS crdate "
                         "FROM message m, poll_credit pc "
                         "WHERE m.id=pc.msgid GROUP BY m.clid, pc.zone) AS mt "
                 "ON (mt.clid=rc.registrar_id AND mt.zone=rc.zone_id) "
             "LEFT JOIN (SELECT i.registrar_id, i.zone_id, MAX(i.crdate) AS crdate "
                         "FROM invoice i JOIN invoice_prefix ip ON i.invoice_prefix_id = ip.id AND ip.typ=0 "
                         "GROUP BY i.registrar_id, i.zone_id) AS iii "
                 "ON (iii.registrar_id=rc.registrar_id AND iii.zone_id=rc.zone_id) "
         "WHERE rc.credit < l.credlimit AND (mt.crdate IS NULL OR mt.crdate < iii.crdate ) ";

    if (!db->ExecSQL(insertTemp))
      throw SQL_ERROR();
    // insert into table message appropriate part from temp table
    const char *insertMessage = "INSERT INTO message "
      "SELECT id,reg,CURRENT_TIMESTAMP,"
      "CURRENT_TIMESTAMP + INTERVAL '7days','f',1 "
      "FROM tmp_poll_credit_insert ";
    if (!db->ExecSQL(insertMessage))
      throw SQL_ERROR();
    // insert into table poll_credit appropriate part from temp table
    const char *insertPollCredit = "INSERT INTO poll_credit "
      "SElECT id, zoneid, credlimit, credit FROM tmp_poll_credit_insert ";
    if (!db->ExecSQL(insertPollCredit))
      throw SQL_ERROR();
    trans.commit();
  }

  bool is_poll_request_fee_present(
            const Database::ID &reg_id,
            const boost::gregorian::date &period_from,
            const boost::gregorian::date &period_to)
    {

        Database::Connection conn = Database::Manager::acquire();

        Database::Result res = conn.exec_params(" SELECT id FROM poll_request_fee prf "
                "JOIN message msg ON msg.id = prf.msgid "
                "WHERE clid = $1::integer "
                " AND period_from = ($2::timestamp AT TIME ZONE 'Europe/Prague') AT TIME ZONE 'UTC' "
                " AND period_to = ($3::timestamp AT TIME ZONE 'Europe/Prague') AT TIME ZONE 'UTC' ",
                Database::query_param_list
                (reg_id)
                (period_from)
                (period_to)
        );

        if(res.size() > 0) {
            return true;
        } else {
            return false;
        }

    }

  void save_poll_request_fee(
          const Database::ID &reg_id,
          const boost::gregorian::date &period_from,
          const boost::gregorian::date &period_to,
          const unsigned long long &total_free_count,
          const unsigned long long &request_count,
          const Money &price)
  {
      Database::Connection conn = Database::Manager::acquire();
      Database::Transaction tx(conn);

      Database::Result res_msg_id =
      conn.exec("SELECT nextval('message_id_seq'::regclass)");

      if(res_msg_id.size() == 0 || res_msg_id[0][0].isnull() ) {
          throw std::runtime_error("Couldn't get next element from ID sequence for message table");
      }

      Database::ID poll_msg_id = res_msg_id[0][0];

      conn.exec_params("INSERT INTO message (id, clid, crdate, exdate, msgtype)"
              " VALUES ($1::integer, $2::integer,"
              " current_timestamp, current_timestamp + interval '7 days',"
              " (SELECT id FROM messagetype WHERE name='request_fee_info'))",
              Database::query_param_list(poll_msg_id)(reg_id));

      conn.exec_params("INSERT INTO poll_request_fee"
              " (msgid, period_from, period_to, total_free_count, used_count, price)"
              " VALUES ($1::integer, ($2::timestamp AT TIME ZONE 'Europe/Prague') AT TIME ZONE 'UTC',"
              " ($3::timestamp AT TIME ZONE 'Europe/Prague') AT TIME ZONE 'UTC', $4::bigint,"
              " $5::bigint, $6::numeric(10,2))",
              Database::query_param_list
                  (poll_msg_id)
                  (period_from)
                  (period_to)
                  (total_free_count)
                  (request_count)
                  (price.get_string()));

      tx.commit();
  }

  virtual void createRequestFeeMessages(Logger::LoggerClient *logger_client, boost::gregorian::date period_to)
  {
      if(period_to.is_special()) {
          period_to = boost::gregorian::day_clock::local_day();
      }

      boost::gregorian::date p_from;
      if (period_to.day() == 1) {
          p_from = period_to - boost::gregorian::months(1);
      }
      else {
          p_from = boost::gregorian::date(period_to.year(), period_to.month(), 1);
      }

      LOGGER(PACKAGE).debug(boost::format("creating request fee messages"
                  " for interval <%1%; %2%)") % p_from % period_to);

      std::auto_ptr<Fred::Registrar::Manager> regman(
               Fred::Registrar::Manager::create(DBDisconnectPtr(NULL)));
      std::auto_ptr<RequestFeeDataMap> request_fee
          = regman->getRequestFeeDataMap(
                  logger_client,
                  boost::posix_time::ptime(p_from),
                  boost::posix_time::ptime(period_to),
                  period_to - days(1));

      for (RequestFeeDataMap::iterator it = request_fee->begin();
              it != request_fee->end();
              it++) {
          RequestFeeData rfd = it->second;

          // duplicity check
         if (is_poll_request_fee_present(rfd.reg_id, p_from, period_to)) {
             LOGGER(PACKAGE).info(boost::format(
                "Poll request fee message for parametres registrar"
                " %1%, from: %2%, to %3% already created, skipping")
                   % rfd.reg_id
                   % p_from
                   % period_to);

             continue;
         }

         LOGGER(PACKAGE).info(boost::format(
                 "Saving poll request fee message, registrar"
                 " %1%, requests: %2%, total free: %3%, price: %4%")
                 % rfd.reg_handle
                 % rfd.request_count
                 % rfd.request_total_free
                 % rfd.price);

         save_poll_request_fee(rfd.reg_id, p_from, period_to, rfd.request_total_free, rfd.request_count, rfd.price);

      }
  }


  virtual std::auto_ptr<MessageRequestFeeInfo> getLastRequestFeeInfoMessage(const std::string &_registrar) const
  {
      TRACE("[CALL] Poll::Manager::getLastRequestFeeInfoMessage");

      ListImpl l(db);

      unsigned int rid = db->GetRegistrarID(_registrar.c_str());
      if (!rid) {
          LOGGER(PACKAGE).error("Registrar ID not found");
          throw NOT_FOUND();
      }
      LOGGER(PACKAGE).debug(boost::format("registar %1% => id=%2%") % _registrar % rid);

      l.setRegistrarFilter(rid);
      l.setTypeFilter(MT_REQUEST_FEE_INFO);
      l.setOrderByDesc();
      l.setLimit(1);
      l.reload();

      Message *m = l.extractFirst();
      if (!m) {
          LOGGER(PACKAGE).error("Registrar ID not found");
          throw NOT_FOUND();
      }
      MessageRequestFeeInfo* rfi = dynamic_cast<MessageRequestFeeInfo*>(m);
      if (!rfi) {
          LOGGER(PACKAGE).error("Dynamic cast to MessageRequestFeeInfo failed");
          throw NOT_FOUND();
      }
      return std::auto_ptr<MessageRequestFeeInfo> (rfi);
  }

  virtual std::auto_ptr<MessageRequestFeeInfo> getRequestFeeInfoMessage(const Database::ID &registrar_id, const ptime &period_to) const
  {
     Database::Connection conn = Database::Manager::acquire();

     Result r =
     conn.exec_params("SELECT    m.msgtype, m.id, m.clid, m.crdate, m.exdate, m.seen, "
                             "prf.period_from, prf.period_to, prf.total_free_count, prf.used_count, prf.price "
                         "FROM poll_request_fee prf "
                           "JOIN message m ON prf.msgid = m.id "
                         "WHERE m.clid = $1::bigint "
                          "AND  period_to =  ($2::timestamp AT TIME ZONE 'Europe/Prague' AT TIME ZONE 'UTC') "
                          "AND m.msgtype = $3::integer",
                          Database::query_param_list(registrar_id)
                                                     (period_to)
                                                     (MT_REQUEST_FEE_INFO));

     if(r.size() < 1) {
         boost::format msg ("Poll request fee message for registrar %1% with period_to %2% not found");
         msg % registrar_id % period_to;
         throw std::runtime_error(msg.str());
     }
     if(r.size() > 1) {
         boost::format msg ("Duplicit request fee poll messages: Registrar %1% with period_to %2%. Database inconsistent.");
         msg % registrar_id % period_to;
         throw std::runtime_error(msg.str());
     }

     std::auto_ptr<MessageRequestFeeInfoImpl> rfi (new MessageRequestFeeInfoImpl(
             r[0][0], r[0][1], r[0][2], r[0][3], r[0][4], r[0][5])
     );

     rfi->setData(r[0][6], r[0][7], r[0][8], r[0][9], r[0][10]);

     return std::auto_ptr<Fred::Poll::MessageRequestFeeInfo>(rfi);
  }

};

Manager *Manager::create(DBSharedPtr db) {
  return new ManagerImpl(db);
}

}
}

