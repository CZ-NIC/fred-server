#include "epp_action_filter.h"

namespace Database {
namespace Filters {

EppAction* EppAction::create() {
  return new EppActionImpl();
}


EppActionImpl::EppActionImpl() : Compound() {
  setName("EppAction");
  active = true;
}

EppActionImpl::~EppActionImpl() {
}

Table& EppActionImpl::joinActionTable() {
  return joinTable("action");
}

Value<Database::ID>& EppActionImpl::addId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinActionTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

EppSession& EppActionImpl::addSession() {
  EppSessionImpl* tmp = new EppSessionImpl();
  add(tmp);
  tmp->joinOn(new Join(
      Column("clientid", joinActionTable()),
      SQL_OP_EQ,
      Column("id", tmp->joinLoginTable())
  ));
  return *tmp;
}

Registrar& EppActionImpl::addRegistrar() {
  // blocking of addition SessionFilter twice should be done in SessionFilter 
  Registrar &tmp = addSession().addRegistrar();
  tmp.setName("Registrar");
  return tmp;
}

Object& EppActionImpl::addObject() {
  Object* tmp = Object::create();
  addJoin(new Join(
      Column("id", joinActionTable()),
      SQL_OP_EQ,
      Column("action", joinTable("history"))
  ));
  addJoin(new Join(
      Column("id", joinTable("history")),
      SQL_OP_EQ,
      Column("historyid", joinTable("object_history"))
  ));
  tmp->joinOn(new Join(
      Column("id", joinTable("object_history")),
      SQL_OP_EQ,
      Column("id", tmp->joinObjectTable())
  ));
  tmp->setName("Object");
  add(tmp);
  return *tmp;
}

Value<std::string>& EppActionImpl::addRequestHandle() {
  addJoin(new Join(
      Column("id", joinActionTable()),
      SQL_OP_EQ,
      Column("actionid", joinTable("action_elements"))
  ));
  Value<std::string> *tmp = new Value<std::string>(Column("value", joinTable("action_elements")));
  add(tmp);
  tmp->addPreValueString("LOWER(");
  tmp->addPostValueString(")");
  tmp->setName("RequestHandle");
  return *tmp;  
}

Interval<DateTimeInterval>& EppActionImpl::addTime() {
  Interval<DateTimeInterval> *tmp = new Interval<DateTimeInterval>(
      Column("startdate", joinActionTable()));
  add(tmp);
  tmp->setName("Time");
  return *tmp;
}

Value<std::string>& EppActionImpl::addClTRID() {
  Value<std::string> *tmp = new Value<std::string>(Column("clienttrid", joinActionTable()));
  add(tmp);
  tmp->setName("ClTRID");
  return *tmp;
}

Value<std::string>& EppActionImpl::addSvTRID() {
  Value<std::string> *tmp = new Value<std::string>(Column("servertrid", joinActionTable()));
  add(tmp);
  tmp->setName("SvTRID");
  return *tmp;
}

Value<int>& EppActionImpl::addEppCodeResponse() {
  Value<int> *tmp = new Value<int>(Column("response", joinActionTable()));
  add(tmp);
  tmp->setName("EppCodeResponse");
  return *tmp;
}

Value<int>& EppActionImpl::addResponse() {
  Value<int> *tmp = new Value<int>(Column("response", joinActionTable()));
  tmp->addModifier(new ValueModifier<int>(0, 2000, SQL_OP_GE));
  tmp->addModifier(new ValueModifier<int>(1, 2000, SQL_OP_LT));
  add(tmp);
  tmp->setName("Response");
  return *tmp;
}

Value<int>& EppActionImpl::addType() {
  Value<int> *tmp = new Value<int>(Column("action", joinActionTable()));
  add(tmp);
  tmp->setName("Type");
  return *tmp;
}

}
}
