#include "mail_filter.h"

namespace Database {
namespace Filters {

/* Ticket #1878 
 * for joining mail_handles table - hidden in addHandle() filter
 */
class MailHandles : virtual public Compound {
public:
    virtual ~MailHandles() {
    }

    virtual Table& joinMailHandlesTable() = 0;
    virtual Value<std::string>& addHandle() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar, const unsigned int _version) {
      _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }
};

class MailHandlesImpl : virtual public MailHandles {
public:
    MailHandlesImpl() : Compound() {
        setName("MailHandles");
        active = false;
    }

    virtual ~MailHandlesImpl() { }

    virtual Table& joinMailHandlesTable() {
        return joinTable("mail_handles");
    }

    virtual Value<std::string>& addHandle() {
        Value<std::string> *tmp = new Value<std::string>(Column("associd", joinMailHandlesTable()));
        add(tmp);
        tmp->setName("Handle");
        return *tmp;
    }

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
        const unsigned int _version) {
      _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(MailHandles);
    }
};


MailImpl::MailImpl() : 
  Compound() {
  setName("Mail");
  active = true;
}

MailImpl::~MailImpl() {
}

Table& MailImpl::joinMailTable() {
  return joinTable("mail_archive");
}

Value<Database::ID>& MailImpl::addId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinMailTable()));
  add(tmp);
  tmp->setName("Id");
  return *tmp;
}

Value<int>& MailImpl::addType() {
  Value<int> *tmp = new Value<int>(Column("mailtype", joinMailTable()));
  add(tmp);
  tmp->setName("Type");
  return *tmp;
}

Value<std::string>& MailImpl::addHandle() {
    MailHandles *mh = new MailHandlesImpl();
    mh->joinOn(new Join(Column("id", joinMailTable()), SQL_OP_EQ, Column("mailid", mh->joinMailHandlesTable())));
    Value<std::string> *tmp = new Value<std::string>(Column("associd", mh->joinMailHandlesTable()));
    tmp->setName("Handle");
    mh->add(tmp);
    add(mh);
    return *tmp;
}

Interval<Database::DateTimeInterval>& MailImpl::addCreateTime() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(
                                                  Column("crdate", joinMailTable()));
  add(tmp);
  tmp->setName("CreateTime");
  return *tmp;
}

Interval<Database::DateTimeInterval>& MailImpl::addModifyTime() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(
                                                  Column("moddate", joinMailTable()));
  add(tmp);
  tmp->setName("ModifyTime");
  return *tmp;
}

Value<int>& MailImpl::addStatus() {
  Value<int> *tmp = new Value<int>(Column("status", joinMailTable()));
  add(tmp);
  tmp->setName("Status");
  return *tmp;
}

Value<int>& MailImpl::addAttempt() {
  Value<int> *tmp = new Value<int>(Column("attempt", joinMailTable()));
  add(tmp);
  tmp->setName("Attempt");
  return *tmp;
}

Value<std::string>& MailImpl::addMessage() {
  Value<std::string> *tmp = new Value<std::string>(Column("message", joinMailTable()),
                                                   SQL_OP_LIKE);
  add(tmp);
  tmp->setName("Message");
  return *tmp;
}

File& MailImpl::addAttachment() {
  File *tmp = new FileImpl();
  tmp->addJoin(new Join(
                   Column("id", joinMailTable()),
                   SQL_OP_EQ,
                   Column("mailid", joinTable("mail_attachments"))
                   ));
  tmp->joinOn(new Join(
                       Column("attachid", joinTable("mail_attachments")),
                       SQL_OP_EQ,
                       Column("id", tmp->joinFileTable())
                       ));
  tmp->setName("Attachment");
  add(tmp);
  return *tmp;
}

}
}
