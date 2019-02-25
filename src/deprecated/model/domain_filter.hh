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
#ifndef DOMAIN_FILTER_HH_2597303E5C114757ADC791C817A3C219
#define DOMAIN_FILTER_HH_2597303E5C114757ADC791C817A3C219

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "src/deprecated/model/object_filter.hh"
#include "src/deprecated/model/contact_filter.hh"
#include "src/deprecated/model/nsset_filter.hh"
#include "src/deprecated/model/keyset_filter.hh"

namespace Database {
namespace Filters {

class Domain : virtual public Object {
public:
  virtual ~Domain() {
  }

  virtual Table& joinDomainTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<std::string>& addFQDN() = 0;
  virtual Value<Database::ID>& addNSSetId() = 0;
  virtual Value<Database::ID>& addKeySetId() = 0;
  virtual Value<Database::ID>& addZoneId() = 0;
  virtual Value<Database::ID>& addRegistrantId() = 0;
  virtual Interval<Database::DateInterval>& addExpirationDate() = 0;
  virtual Interval<Database::DateInterval>& addOutZoneDate() = 0;
  virtual Interval<Database::DateInterval>& addCancelDate() = 0;
  virtual Contact& addRegistrant() = 0;
  virtual Contact& addAdminContact() = 0;
  virtual Contact& addTempContact() = 0;
  virtual NSSet& addNSSet() = 0;
  virtual KeySet& addKeySet() = 0;
  virtual Value<bool>& addPublish() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
  }
  
  static Domain* create();
};

class DomainImpl : public ObjectImpl, virtual public Domain {
public:
  DomainImpl();
  virtual ~DomainImpl();

  virtual ObjectType getType() const {
    return TDOMAIN;
  }

  Table& joinDomainTable();
  virtual void _joinPolymorphicTables();

  virtual Value<std::string>& addHandle();
  virtual Value<Database::ID>& addId();
  virtual Value<std::string>& addFQDN();
  virtual Value<Database::ID>& addNSSetId();
  virtual Value<Database::ID>& addKeySetId();
  virtual Value<Database::ID>& addZoneId();
  virtual Value<Database::ID>& addRegistrantId();
  virtual Interval<Database::DateInterval>& addExpirationDate();
  virtual Interval<Database::DateInterval>& addOutZoneDate();
  virtual Interval<Database::DateInterval>& addCancelDate();
  virtual Contact& addRegistrant();
  virtual Contact& addAdminContact();
  virtual Contact& addTempContact();
  virtual NSSet& addNSSet();
  virtual KeySet& addKeySet();
  virtual Value<bool>& addPublish();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Domain);
  }

private:
  Contact& _addDCMFilter(unsigned _role);
};

class DomainHistoryImpl : public ObjectHistoryImpl, virtual public Domain {
public:
  DomainHistoryImpl();
  virtual ~DomainHistoryImpl();

  virtual ObjectType getType() const {
    return TDOMAIN;
  }

  Table& joinDomainTable();
  virtual void _joinPolymorphicTables();

  virtual Value<std::string>& addHandle();
  virtual Value<Database::ID>& addId();
  virtual Value<std::string>& addFQDN();
  virtual Value<Database::ID>& addNSSetId();
  virtual Value<Database::ID>& addKeySetId();
  virtual Value<Database::ID>& addZoneId();
  virtual Value<Database::ID>& addRegistrantId();
  virtual Interval<Database::DateInterval>& addExpirationDate();
  virtual Interval<Database::DateInterval>& addOutZoneDate();
  virtual Interval<Database::DateInterval>& addCancelDate();
  virtual Contact& addRegistrant();
  virtual Contact& addAdminContact();
  virtual Contact& addTempContact();
  virtual NSSet& addNSSet();
  virtual KeySet &addKeySet();
  virtual Value<bool>& addPublish();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Domain);
  }

private:
  Contact& _addDCMFilter(unsigned _role);
};

}
}

#endif /*DOMAIN_FILTER_H_*/
