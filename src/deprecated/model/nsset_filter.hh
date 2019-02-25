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
#ifndef NSSET_FILTER_HH_2FDA0ABDA00C46108887CA0DDC34E522
#define NSSET_FILTER_HH_2FDA0ABDA00C46108887CA0DDC34E522

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "src/deprecated/model/object_filter.hh"
#include "src/deprecated/model/contact_filter.hh"

namespace Database {
namespace Filters {

class NSSet : virtual public Object {
public:
  virtual ~NSSet() {
  }

  virtual Table& joinNSSetTable() = 0;
  virtual Value<ID>& addId() = 0;
  virtual Value<std::string>& addHostFQDN() = 0;
  virtual Value<std::string>& addHostIP() = 0;
  virtual Contact& addTechContact() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
  }
  
  static NSSet* create();
};

class NSSetImpl : public ObjectImpl, virtual public NSSet {
public:
  NSSetImpl();
  virtual ~NSSetImpl();

  virtual ObjectType getType() const {
    return TNSSET;
  }

  virtual Table& joinNSSetTable();
  virtual void _joinPolymorphicTables();

  virtual Value<ID>& addId();
  virtual Value<std::string>& addHandle();
  virtual Value<std::string>& addHostFQDN();
  virtual Value<std::string>& addHostIP();
  virtual Contact& addTechContact();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(NSSet);
  }
};

class NSSetHistoryImpl : public ObjectHistoryImpl, virtual public NSSet {
public:
  NSSetHistoryImpl();
  virtual ~NSSetHistoryImpl();

  virtual ObjectType getType() const {
    return TNSSET;
  }

  virtual Table& joinNSSetTable();
  virtual void _joinPolymorphicTables();

  virtual Value<ID>& addId();
  virtual Value<std::string> &addHandle();
  virtual Value<std::string>& addHostFQDN();
  virtual Value<std::string>& addHostIP();
  virtual Contact& addTechContact();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(NSSet);
  }
};

}
}

#endif /*NSSET_FILTER_H_*/
