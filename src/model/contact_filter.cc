#include "contact_filter.h"

namespace DBase {
namespace Filters {

/*
 * CONTACT IMPLEMENTATION
 */
ContactImpl::ContactImpl() :
  ObjectImpl() {
  setName("Contact");
  setType(getType());
}

ContactImpl::~ContactImpl() {
}

Value<DBase::ID>& ContactImpl::addId() {
  Value<ID> *tmp = new Value<ID>(Column("id", joinContactTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactImpl::addName() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinContactTable()));
  tmp->setName("Name");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactImpl::addOrganization() {
  Value<std::string> *tmp = new Value<std::string>(Column("organization", joinContactTable()));
  tmp->setName("Organization");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactImpl::addCity() {
  Value<std::string> *tmp = new Value<std::string>(Column("city", joinContactTable()));
  tmp->setName("City");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactImpl::addEmail() {
  Value<std::string> *tmp = new Value<std::string>(Column("email", joinContactTable()));
  tmp->setName("Email");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactImpl::addNotifyEmail() {
  Value<std::string> *tmp = new Value<std::string>(Column("notifyemail", joinContactTable()));
  tmp->setName("NotifyEmail");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactImpl::addVat() {
  Value<std::string> *tmp = new Value<std::string>(Column("vat", joinContactTable()));
  tmp->setName("Vat");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactImpl::addSsn() {
  Value<std::string> *tmp = new Value<std::string>(Column("ssn", joinContactTable()));
  tmp->setName("Ssn");
  add(tmp);
  return *tmp;
}

Table& ContactImpl::joinContactTable() {
  return joinTable("contact");
}

void ContactImpl::_joinPolymorphicTables() {
  ObjectImpl::_joinPolymorphicTables();
  Table *c = findTable("contact");
  if (c) {
    joins.push_back(new Join(
        Column("id", joinTable("object_registry")),
        SQL_OP_EQ,
        Column("id", *c)
    ));
  }
}

/*
 * CONTACT HISTORY IMPLEMENTATION
 */
ContactHistoryImpl::ContactHistoryImpl() :
  ObjectHistoryImpl() {
  setName("ContactHistory");
  setType(getType());
}

ContactHistoryImpl::~ContactHistoryImpl() {
}

Value<DBase::ID>& ContactHistoryImpl::addId() {
  Value<ID> *tmp = new Value<ID>(Column("id", joinContactTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactHistoryImpl::addName() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinContactTable()));
  tmp->setName("Name");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactHistoryImpl::addOrganization() {
  Value<std::string> *tmp = new Value<std::string>(Column("organization", joinContactTable()));
  tmp->setName("Organization");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactHistoryImpl::addCity() {
  Value<std::string> *tmp = new Value<std::string>(Column("city", joinContactTable()));
  tmp->setName("City");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactHistoryImpl::addEmail() {
  Value<std::string> *tmp = new Value<std::string>(Column("email", joinContactTable()));
  tmp->setName("Email");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactHistoryImpl::addNotifyEmail() {
  Value<std::string> *tmp = new Value<std::string>(Column("email", joinContactTable()));
  tmp->setName("NotifyEmail");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactHistoryImpl::addVat() {
  Value<std::string> *tmp = new Value<std::string>(Column("vat", joinContactTable()));
  tmp->setName("Vat");
  add(tmp);
  return *tmp;
}

Value<std::string>& ContactHistoryImpl::addSsn() {
  Value<std::string> *tmp = new Value<std::string>(Column("ssn", joinContactTable()));
  tmp->setName("Ssn");
  add(tmp);
  return *tmp;
}

Table& ContactHistoryImpl::joinContactTable() {
  return joinTable("contact_history");
}

void ContactHistoryImpl::_joinPolymorphicTables() {
  ObjectHistoryImpl::_joinPolymorphicTables();
  Table *c = findTable("contact_history");
  if (c) {
    joins.push_back(new Join(
        Column("historyid", joinTable("object_registry")),
        SQL_OP_EQ,
        Column("historyid", *c)
    ));
  }
}


}
}
