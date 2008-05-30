#include "epp_action_filter.h"

namespace DBase {
namespace Filters {

EppActionImpl::EppActionImpl() :
  Compound() {
  setName("EppAction");
  active = true;
}

EppActionImpl::~EppActionImpl() {
}

Table& EppActionImpl::joinActionTable() {
  return joinTable("action");
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
  Object* tmp = new ObjectHistoryImpl();
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

Value<int>& EppActionImpl::addResponse() {
  Value<int> *tmp = new Value<int>(Column("response", joinActionTable()));
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
