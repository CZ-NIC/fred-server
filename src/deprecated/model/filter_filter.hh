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
#ifndef FILTER_FILTER_HH_1EFB25A32ABF49B1AF70B2A917F7E91B
#define FILTER_FILTER_HH_1EFB25A32ABF49B1AF70B2A917F7E91B

#include "src/util/db/query/base_filters.hh"

namespace Database {
namespace Filters {

class FilterFilter : virtual public Compound {
public:
  virtual ~FilterFilter() {
  }

  virtual Table& joinFilterTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<Database::ID>& addUserId() = 0;
  virtual Value<Database::ID>& addGroupId() = 0;
  virtual Value<int>& addType() = 0;
};

class FilterFilterImpl : virtual public FilterFilter {
public:
  FilterFilterImpl();
  virtual ~FilterFilterImpl();

  virtual Table& joinFilterTable();
  virtual Value<Database::ID>& addId();
  virtual Value<Database::ID>& addUserId();
  virtual Value<Database::ID>& addGroupId();
  virtual Value<int>& addType();
};

}
}

#endif
