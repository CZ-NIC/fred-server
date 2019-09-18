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
#ifndef FILE_FILTER_HH_E7A2250C135F4B0A917FA4DAD687ECB6
#define FILE_FILTER_HH_E7A2250C135F4B0A917FA4DAD687ECB6

#include "src/util/db/query/base_filters.hh"

namespace Database {
namespace Filters {

class File : virtual public Compound {
public:
  virtual ~File() {
  }
  
  virtual Table& joinFileTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<std::string>& addName() = 0;
  virtual Value<std::string>& addPath() = 0;
  virtual Value<std::string>& addMimeType() = 0;
  virtual Interval<Database::DateTimeInterval>& addCreateTime() = 0;
  virtual Value<int>& addSize() = 0;
  virtual Value<int>& addType() = 0;
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

};

class FileImpl : virtual public File {
public:
  FileImpl(bool _set_active = false);
  virtual ~FileImpl();
  
  virtual Table& joinFileTable();
  virtual Value<Database::ID>& addId();
  virtual Value<std::string>& addName();
  virtual Value<std::string>& addPath();
  virtual Value<std::string>& addMimeType();
  virtual Interval<Database::DateTimeInterval>& addCreateTime();
  virtual Value<int>& addSize();
  virtual Value<int>& addType();
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(File);
  }

};


}
}


#endif /*FILE_FILTER_H_*/
