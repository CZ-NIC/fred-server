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
#include "src/deprecated/model/object_state_filter.hh"

namespace Database {
namespace Filters {

ObjectStateImpl::ObjectStateImpl() {
}

ObjectStateImpl::~ObjectStateImpl() {
}

Table& ObjectStateImpl::joinObjectStateTable() {
  return joinTable("object_state");
}

Value<Database::ID>& ObjectStateImpl::addStateId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("state_id", joinObjectStateTable()));
  add(tmp);
  tmp->setName("StateId");
  return *tmp;
}

Interval<Database::DateTimeInterval>& ObjectStateImpl::addValidFrom() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("valid_from", joinObjectStateTable()));
  add(tmp);
  tmp->setName("ValidFrom");
  return *tmp;
}

Interval<Database::DateTimeInterval>& ObjectStateImpl::addValidTo() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("valid_to", joinObjectStateTable()));
  add(tmp);
  tmp->setName("ValidTo");
  return *tmp;
}

}
}
