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
#ifndef PUBLIC_REQUEST_FILTER_HH_16BD74C725D346D28E6C0166401D97DC
#define PUBLIC_REQUEST_FILTER_HH_16BD74C725D346D28E6C0166401D97DC

#include "src/util/db/query/base_filters.hh"
#include "src/deprecated/model/object_filter.hh"

namespace Database {
namespace Filters {

class PublicRequest : virtual public Compound {
public:
  virtual ~PublicRequest() {
  }

  virtual Table& joinRequestTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
//  virtual Value<LibFred::Request::Type>& addType() = 0;
//  virtual Value<LibFred::Request::Status>& addStatus() = 0;
  virtual Value<int>& addType() = 0;
  virtual Value<int>& addStatus() = 0;
  virtual Interval<DateTimeInterval>& addCreateTime() = 0;
  virtual Interval<DateTimeInterval>& addResolveTime() = 0;
  virtual Value<std::string>& addReason() = 0;
  virtual Value<std::string>& addEmailToAnswer() = 0;
  virtual Value<Database::ID>& addAnswerEmailId() = 0;
  virtual Value<Database::ID>& addRegistrarId() = 0;
  virtual Object& addObject() = 0;

  virtual Registrar& addRegistrar() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

  static PublicRequest* create();
};


class PublicRequestImpl : virtual public PublicRequest {
public:
  PublicRequestImpl();
  virtual ~PublicRequestImpl();

  virtual Table& joinRequestTable();
  virtual Value<Database::ID>& addId();
  virtual Value<int>& addType();
  virtual Value<int>& addStatus();
  virtual Interval<DateTimeInterval>& addCreateTime();
  virtual Interval<DateTimeInterval>& addResolveTime();
  virtual Value<std::string>& addReason();
  virtual Value<std::string>& addEmailToAnswer();
  virtual Value<Database::ID>& addAnswerEmailId();
  virtual Value<Database::ID>& addRegistrarId();
  virtual Object& addObject();
  virtual Registrar& addRegistrar();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(PublicRequest);
  }

};

}
}

#endif /*PUBLIC_REQUEST_FILTER_H_*/
