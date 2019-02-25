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
#include "src/deprecated/model/filter_filter.hh"

namespace Database {
namespace Filters {

FilterFilterImpl::FilterFilterImpl() {
  setName("Filter");
}

FilterFilterImpl::~FilterFilterImpl() {
}

Table& FilterFilterImpl::joinFilterTable() {
  return joinTable("filters");
}

Value<Database::ID>& FilterFilterImpl::addId() {
  Value<Database::ID> *tmp = new Value<ID>(Column("id", joinFilterTable()));
  add(tmp);
  tmp->setName("Id");
  return *tmp;
}

Value<Database::ID>& FilterFilterImpl::addUserId() {
  Value<Database::ID> *tmp = new Value<ID>(Column("userid", joinFilterTable()));
  add(tmp);
  tmp->setName("UserId");
  return *tmp;
}

Value<Database::ID>& FilterFilterImpl::addGroupId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("groupid", joinFilterTable()));
  add(tmp);
  tmp->setName("GroupId");
  return *tmp;
}

Value<int>& FilterFilterImpl::addType() {
  Value<int> *tmp = new Value<int>(Column("type", joinFilterTable()));
  add(tmp);
  tmp->setName("Type");
  return *tmp;
}

}
}
