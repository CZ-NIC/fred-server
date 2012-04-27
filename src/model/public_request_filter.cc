#include "public_request_filter.h"

namespace Database {
namespace Filters {

PublicRequest* PublicRequest::create() {
  return new PublicRequestImpl();
}

PublicRequestImpl::PublicRequestImpl() : Compound() {
  setName("Request");
  active = true;
}

PublicRequestImpl::~PublicRequestImpl() {
}

Table& PublicRequestImpl::joinRequestTable() {
  return joinTable("public_request");
}

Value<Database::ID>& PublicRequestImpl::addId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinRequestTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<int>& PublicRequestImpl::addType() {
  Value<int> *tmp = new Value<int>(Column("request_type", joinRequestTable()));
  tmp->setName("Type");
  add(tmp);
  return *tmp;
}

Value<int>& PublicRequestImpl::addStatus() {
  Value<int> *tmp = new Value<int>(Column("status", joinRequestTable()));
  tmp->setName("Status");
  add(tmp);
  return *tmp;
}

Interval<DateTimeInterval>& PublicRequestImpl::addCreateTime() {
  Interval<DateTimeInterval> *tmp = new Interval<DateTimeInterval>(Column("create_time", joinRequestTable()));
  add(tmp);
  tmp->setName("CreateTime");
  return *tmp;
}

Interval<DateTimeInterval>& PublicRequestImpl::addResolveTime() {
  Interval<DateTimeInterval> *tmp = new Interval<DateTimeInterval>(Column("resolve_time", joinRequestTable()));
  add(tmp);
  tmp->setName("ResolveTime");
  return *tmp;
}

Value<std::string>& PublicRequestImpl::addReason() {
  Value<std::string> *tmp = new Value<std::string>(Column("reason", joinRequestTable()));
  add(tmp);
  tmp->setName("Reason");
  return *tmp;
}

Value<std::string>& PublicRequestImpl::addEmailToAnswer() {
  Value<std::string> *tmp = new Value<std::string>(Column("email_to_answer", joinRequestTable()));
  add(tmp);
  tmp->setName("EmailToAnswer");
  return *tmp;
}

Value<Database::ID>& PublicRequestImpl::addAnswerEmailId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("answer_email_id", joinRequestTable()));
  tmp->setName("AnswerEmailId");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& PublicRequestImpl::addRegistrarId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("registrar_id", joinRequestTable()));
  tmp->setName("RegistrarId");
  add(tmp);
  return *tmp;
}

Object& PublicRequestImpl::addObject() {
  Object* tmp = Object::create();
  addJoin(new Join(
                   Column("id", joinRequestTable()),
                   SQL_OP_EQ,
                   Column("request_id", joinTable("public_request_objects_map"))
                   ));
  tmp->joinOn(new Join(
                       Column("object_id", joinTable("public_request_objects_map")),
                       SQL_OP_EQ,
                       Column("id", tmp->joinObjectTable())
                       ));
  tmp->setName("Object");
  add(tmp);
  return *tmp;
}

Registrar& PublicRequestImpl::addRegistrar() { 
  Registrar *tmp = new RegistrarImpl();
  tmp->joinOn(new Join(
                       Column("registrar_id", joinRequestTable()),
                       SQL_OP_EQ,
                       Column("id", tmp->joinRegistrarTable())
                       ));
  tmp->setName("Registrar");
  add(tmp);
  return *tmp;
}

}
}
