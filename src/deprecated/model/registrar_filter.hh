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
#ifndef REGISTRAR_FILTER_HH_54BBB0A943DA422CADDBAFB062E29EA6
#define REGISTRAR_FILTER_HH_54BBB0A943DA422CADDBAFB062E29EA6

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "src/util/db/query/base_filters.hh"
#include "src/deprecated/model/zone_filter.hh"

namespace Database {
namespace Filters {

class Registrar : virtual public Compound {
public:
  virtual ~Registrar() {
  }

  virtual Table& joinRegistrarTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<std::string>& addIco() = 0;
  virtual Value<std::string>& addDic() = 0;
  virtual Value<std::string>& addVarSymbol() = 0;
  virtual Value<bool>& addVat() = 0;
  virtual Value<std::string>& addHandle() = 0;
  virtual Value<std::string>& addName() = 0;
  virtual Value<std::string>& addOrganization() = 0;
  virtual Value<std::string>& addStreet() = 0;
  virtual Value<std::string>& addCity() = 0;
  virtual Value<std::string>& addStateOrProvince() = 0;
  virtual Value<std::string>& addPostalCode() = 0;
  virtual Value<std::string>& addCountryCode() = 0;
  virtual Value<std::string>& addTelephone() = 0;
  virtual Value<std::string>& addFax() = 0;
  virtual Value<std::string>& addEmail() = 0;
  virtual Value<std::string>& addUrl() = 0;
  virtual Value<std::string>& addZoneFqdn() = 0;
  virtual Value<Database::ID>& addGroupId() = 0;


  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

};

class RegistrarImpl : virtual public Registrar {
public:
  RegistrarImpl(bool _set_active = false);
  virtual ~RegistrarImpl();

  virtual Table& joinRegistrarTable();
  virtual Value<Database::ID>& addId();
  virtual Value<std::string>& addIco();
  virtual Value<std::string>& addDic();
  virtual Value<std::string>& addVarSymbol();
  virtual Value<bool>& addVat();
  virtual Value<std::string>& addHandle();
  virtual Value<std::string>& addName();
  virtual Value<std::string>& addOrganization();
  virtual Value<std::string>& addStreet();
  virtual Value<std::string>& addCity();
  virtual Value<std::string>& addStateOrProvince();
  virtual Value<std::string>& addPostalCode();
  virtual Value<std::string>& addCountryCode();
  virtual Value<std::string>& addTelephone();
  virtual Value<std::string>& addFax();
  virtual Value<std::string>& addEmail();
  virtual Value<std::string>& addUrl();
  virtual Value<std::string>& addZoneFqdn();
  virtual Value<Database::ID>& addGroupId();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Registrar);
  }


};

}
}

#endif /*REGISTRAR_FILTER_H_*/

