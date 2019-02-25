/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/deprecated/model/contact_filter.hh"

namespace Database {
namespace Filters {

Contact* Contact::create() {
  return new ContactHistoryImpl();
}

/*
 * CONTACT IMPLEMENTATION
 */
ContactImpl::ContactImpl() :
  ObjectImpl() {
  setName("Contact");
  addType().setValue(getType());
}

ContactImpl::~ContactImpl() {
}

Value<std::string>& ContactImpl::addHandle() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinObjectRegistryTable()));
  add(tmp);
  tmp->addPreValueString("UPPER(");
  tmp->addPostValueString(")");
  tmp->setName("Handle");
  return *tmp;
}

Value<Database::ID>& ContactImpl::addId() {
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

Value<std::string>& ContactImpl::addPhoneNumber() {
  Value<std::string> *tmp = new Value<std::string>(Column("telephone", joinContactTable()));
  tmp->setName("PhoneNumber");
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
  addType().setValue(getType());
}

ContactHistoryImpl::~ContactHistoryImpl() {
}

Value<std::string>& ContactHistoryImpl::addHandle() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinObjectRegistryTable()));
  add(tmp);
  tmp->addPreValueString("UPPER(");
  tmp->addPostValueString(")");
  tmp->setName("Handle");
  return *tmp;
}

Value<Database::ID>& ContactHistoryImpl::addId() {
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
  Value<std::string> *tmp = new Value<std::string>(Column("notifyemail", joinContactTable()));
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

Value<std::string>& ContactHistoryImpl::addPhoneNumber() {
  Value<std::string> *tmp = new Value<std::string>(Column("telephone", joinContactTable()));
  tmp->setName("PhoneNumber");
  add(tmp);
  return *tmp;
}

Table& ContactHistoryImpl::joinContactTable() {
  return joinTable("contact_history");
}

void ContactHistoryImpl::_joinPolymorphicTables() {
  Table *c = findTable("contact_history");
  if (c) {
    joins.push_back(new Join(
        Column("historyid", joinTable("object_history")),
        SQL_OP_EQ,
        Column("historyid", *c)
    ));
  }
  ObjectHistoryImpl::_joinPolymorphicTables();
}


}
}
