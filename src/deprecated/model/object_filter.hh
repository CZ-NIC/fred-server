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
#ifndef OBJECT_FILTER_HH_19C85B79F8BC4A3892AE8652015A8D69
#define OBJECT_FILTER_HH_19C85B79F8BC4A3892AE8652015A8D69

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "src/deprecated/model/object_registry_filter.hh"
#include "src/deprecated/model/registrar_filter.hh"
#include "src/deprecated/model/object_state_filter.hh"

namespace Database {
namespace Filters {

class Object : virtual public ObjectRegistry {
public:
  virtual ~Object() {
  }

  virtual Table& joinObjectTable() = 0;
  virtual Interval<Database::DateTimeInterval>& addTransferTime() = 0;
  virtual Interval<Database::DateTimeInterval>& addUpdateTime() = 0;
  virtual Value<std::string>& addAuthInfo() = 0;
  virtual Value<Database::ID>& addRegistrarId() = 0;
  virtual Value<Database::ID>& addUpdateRegistrarId() = 0;
  virtual Registrar& addRegistrar() = 0;
  virtual Registrar& addUpdateRegistrar() = 0;
  virtual ObjectState& addObjectState() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ObjectRegistry);
  }
  
  static Object* create();
};

class ObjectImpl : public ObjectRegistryImpl, virtual public Object {
public:
  ObjectImpl();
  virtual ~ObjectImpl();

  virtual Table& joinObjectTable();
  virtual void _joinPolymorphicTables();
  
  virtual Interval<Database::DateTimeInterval>& addTransferTime();
  virtual Interval<Database::DateTimeInterval>& addUpdateTime();
  virtual Value<std::string>& addAuthInfo();
  virtual Value<Database::ID>& addRegistrarId();
  virtual Value<Database::ID>& addUpdateRegistrarId();
  virtual Registrar& addRegistrar();
  virtual Registrar& addUpdateRegistrar();
  virtual ObjectState& addObjectState();
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
  }
};

class ObjectHistoryImpl : public ObjectRegistryImpl, virtual public Object {
public:
  ObjectHistoryImpl();
  virtual ~ObjectHistoryImpl();

  virtual Table& joinObjectTable();
  virtual void _joinPolymorphicTables();
  
  virtual Value<Database::ID>& addHistoryId();
  virtual Interval<Database::DateTimeInterval>& addTransferTime();
  virtual Interval<Database::DateTimeInterval> & addUpdateTime();
  virtual Value<std::string>& addAuthInfo();
  virtual Value<Database::ID>& addRegistrarId();
  virtual Value<Database::ID>& addUpdateRegistrarId();
  virtual Registrar& addRegistrar();
  virtual Registrar& addUpdateRegistrar();
  virtual ObjectState& addObjectState();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
  }
};

}
}

#endif /*OBJECT_FILTER_H_*/
