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
#include "src/deprecated/model/file_filter.hh"

namespace Database {
namespace Filters {

FileImpl::FileImpl(bool _set_active) :
  Compound() {
  setName("File");
  active = _set_active;
}

FileImpl::~FileImpl() {
}

Table& FileImpl::joinFileTable() {
  return joinTable("files");
}

Value<Database::ID>& FileImpl::addId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinFileTable()));
  add(tmp);
  tmp->setName("Id");
  return *tmp;
}

Value<std::string>& FileImpl::addName() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinFileTable()));
  add(tmp);
  tmp->setName("Name");
  return *tmp;
}

Value<std::string>& FileImpl::addPath() {
  Value<std::string> *tmp = new Value<std::string>(Column("path", joinFileTable()));
  add(tmp);
  tmp->setName("Path");
  return *tmp;
}

Value<std::string>& FileImpl::addMimeType() {
  Value<std::string> *tmp = new Value<std::string>(Column("mimetype", joinFileTable()));
  add(tmp);
  tmp->setName("MimeType");
  return *tmp;
}

Interval<Database::DateTimeInterval>& FileImpl::addCreateTime() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(
                                                  Column("crdate", joinFileTable()));
  add(tmp);
  tmp->setName("CreateTime");
  return *tmp;
}

Value<int>& FileImpl::addSize() {
  Value<int> *tmp = new Value<int>(Column("filesize", joinFileTable()));
  add(tmp);
  tmp->setName("Size");
  return *tmp;
}

Value<int>& FileImpl::addType() {
  Value<int> *tmp = new Value<int>(Column("filetype", joinFileTable()));
  add(tmp);
  tmp->setName("Type");
  return *tmp;
}


}
}
