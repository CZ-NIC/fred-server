#ifndef REQUEST_H_
#define REQUEST_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "src/fredlib/object.h"



using namespace Database;

namespace Fred {
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

class Request : virtual public Fred::CommonObject {
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

  virtual boost::shared_ptr<RequestPropertiesDetail> getProperties() = 0;
  virtual boost::shared_ptr<ObjectReferences> getReferences()  = 0;
  virtual const std::pair<int, std::string> getResultCode() const = 0;
  virtual const std::string& getResultCodeName() const = 0;

};


};
};

#endif /* REQUEST_H_ */
