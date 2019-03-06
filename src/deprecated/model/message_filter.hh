/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#ifndef MESSAGE_FILTER_HH_986521F658B84CB2BDBF2C69E6440A15
#define MESSAGE_FILTER_HH_986521F658B84CB2BDBF2C69E6440A15

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "src/util/db/query/base_filters.hh"
#include "src/deprecated/model/contact_filter.hh"


namespace Database {
namespace Filters {

class Message : virtual public Compound {
public:
  virtual ~Message() {}
  virtual Table& joinMessageArchiveTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Interval<Database::DateTimeInterval>& addCrDate() = 0;
  virtual Interval<Database::DateTimeInterval>& addModDate() = 0;
  virtual Value<int>& addAttempt() = 0;
  virtual Value<int>& addStatus() = 0;
  virtual Value<int>& addCommType() = 0;
  virtual Value<int>& addMessageType() = 0;
  virtual Value<std::string>& addSmsPhoneNumber() = 0;
  virtual Value<std::string>& addLetterAddrName() = 0;
  virtual Contact& addMessageContact() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

};

class MessageImpl : virtual public Message {
public:
    MessageImpl(bool _set_active = false);
  virtual ~MessageImpl();

  virtual Table& joinMessageArchiveTable();
  virtual Value<Database::ID>& addId();
  virtual Interval<Database::DateTimeInterval>& addCrDate();
  virtual Interval<Database::DateTimeInterval>& addModDate();
  virtual Value<int>& addAttempt();
  virtual Value<int>& addStatus();
  virtual Value<int>& addCommType();
  virtual Value<int>& addMessageType();
  virtual Value<std::string>& addSmsPhoneNumber();
  virtual Value<std::string>& addLetterAddrName();
  virtual Contact& addMessageContact();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
  }


};

}
}

#endif
