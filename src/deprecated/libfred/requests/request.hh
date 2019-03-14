/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#ifndef REQUEST_HH_C46456A865AB435E8281F74957AE025D
#define REQUEST_HH_C46456A865AB435E8281F74957AE025D

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "src/deprecated/libfred/object.hh"



using namespace Database;

namespace LibFred {
namespace Logger {

enum Languages { EN, CS  };

struct ObjectReference {
  std::string type;
  Database::ID id;

  ObjectReference() : type(), id()
  {
  }

  ObjectReference(const std::string &_type, const unsigned long long _id)
      : type(_type), id(_id)
  {
  }
};

typedef std::vector<ObjectReference> ObjectReferences;

struct RequestProperty {
  std::string name;
  std::string value;
  bool child;

  RequestProperty() : name(), value(), child() {
  }

  RequestProperty(const RequestProperty &p) : name(p.name), value(p.value), child(p.child) {

  }

  RequestProperty(const std::string &n, const std::string &v, bool ch) {
	name = n;
	value = v;
	child = ch;
  }

  const RequestProperty & operator = (RequestProperty &p) {
	name = p.name;
	value = p.value;
	child = p.child;

	return *this;
  }
};

typedef std::vector<RequestProperty> RequestProperties;

struct RequestPropertyDetail {
  std::string name;
  std::string value;
  bool output;
  bool child;

  RequestPropertyDetail() : name(), value(), output(), child() {
  }

  RequestPropertyDetail(const RequestPropertyDetail &p) : name(p.name), value(p.value), output(p.output), child(p.child) {
  }

  RequestPropertyDetail(const std::string &_name, const std::string &_value, bool _output, bool _child) {
    name = _name;
    value = _value;
    output = _output;
    child = _child;
  }

  const RequestPropertyDetail & operator = (RequestPropertyDetail &p) {
    name = p.name;
    value = p.value;
    output = p.output;
    child = p.child;

    return *this;
  }
};

typedef std::vector<RequestPropertyDetail> RequestPropertiesDetail;

class Request : virtual public LibFred::CommonObject {
public:
  virtual const boost::posix_time::ptime  getTimeBegin() const = 0;
  virtual const boost::posix_time::ptime  getTimeEnd() const = 0;
  virtual const std::string&		getSourceIp() const = 0;
  virtual const std::string&    getUserName() const = 0;
  virtual const Database::ID&    getUserId() const = 0;
  virtual const std::string& getServiceType() const = 0;
  virtual const std::string& getActionType() const = 0;
  virtual const Database::ID& getSessionId() const = 0;
  virtual const bool& getIsMonitoring() const = 0;

  virtual const std::string& getRawRequest() const = 0;
  virtual const std::string& getRawResponse() const  = 0;

  virtual std::shared_ptr<RequestPropertiesDetail> getProperties() = 0;
  virtual std::shared_ptr<ObjectReferences> getReferences()  = 0;
  virtual const std::pair<int, std::string> getResultCode() const = 0;
  virtual const std::string& getResultCodeName() const = 0;

};


};
};

#endif /* REQUEST_H_ */
