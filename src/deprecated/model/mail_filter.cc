#include "src/deprecated/model/mail_filter.hh"

namespace Database {
namespace Filters {


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
  Value<int> *tmp = new Value<int>(Column("mail_type_id", joinMailTable()));
  add(tmp);
  tmp->setName("Type");
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
  Column column = Column("message_params", joinMailTable());
  column.castTo("text");
  Value<std::string> *tmp = new Value<std::string>(column, SQL_OP_LIKE);
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
