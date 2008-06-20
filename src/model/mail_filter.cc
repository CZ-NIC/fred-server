#include "mail_filter.h"

namespace DBase {
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

Value<DBase::ID>& MailImpl::addId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("id", joinMailTable()));
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
  addJoin(new Join(
                   Column("id", joinMailTable()),
                   SQL_OP_EQ,
                   Column("mailid", joinTable("mail_handles"))
                   ));
  Value<std::string> *tmp = new Value<std::string>(Column("associd", joinTable("mail_handles")));
  add(tmp);
  tmp->setName("Handle");
  return *tmp;
}

Interval<DBase::DateTimeInterval>& MailImpl::addCreateTime() {
  Interval<DBase::DateTimeInterval> *tmp = new Interval<DBase::DateTimeInterval>(
                                                  Column("crdate", joinMailTable()));
  add(tmp);
  tmp->setName("CreateTime");
  return *tmp;
}

Interval<DBase::DateTimeInterval>& MailImpl::addModifyTime() {
  Interval<DBase::DateTimeInterval> *tmp = new Interval<DBase::DateTimeInterval>(
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
