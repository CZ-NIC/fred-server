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
#ifndef MAIL_FILTER_HH_7D939F1475434A96B6D35F5698A244A5
#define MAIL_FILTER_HH_7D939F1475434A96B6D35F5698A244A5

#include "src/util/db/query/base_filters.hh"
#include "src/deprecated/model/file_filter.hh"

namespace Database {
namespace Filters {

class Mail : virtual public Compound {
public:
  virtual ~Mail() {
  }

  virtual Table& joinMailTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<int>& addType() = 0;
  virtual Interval<Database::DateTimeInterval>& addCreateTime() = 0;
  virtual Interval<Database::DateTimeInterval>& addModifyTime() = 0;
  virtual Value<int>& addStatus() = 0;
  virtual Value<int>& addAttempt() = 0;
  virtual Value<std::string>& addMessage() = 0;
  virtual File& addAttachment() = 0;
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar, const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

};

class MailImpl : virtual public Mail {
public:
  MailImpl();
  virtual ~MailImpl();

  virtual Table& joinMailTable();
  virtual Value<Database::ID>& addId();
  virtual Value<int>& addType();
  virtual Interval<Database::DateTimeInterval>& addCreateTime();
  virtual Interval<Database::DateTimeInterval>& addModifyTime();
  virtual Value<int>& addStatus();
  virtual Value<int>& addAttempt();
  virtual Value<std::string>& addMessage();
  virtual File& addAttachment();
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Mail);
  }

};



}
}

#endif /*MAIL_FILTER_H_*/
