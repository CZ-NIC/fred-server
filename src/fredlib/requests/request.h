#ifndef REQUEST_H_
#define REQUEST_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "object.h"



using namespace Database;

namespace Fred {
namespace Logger {

enum Languages { EN, CS  };

struct ObjectReference {
  std::string type;
  Database::ID id;
};

typedef std::vector<ObjectReference> ObjectReferences;

struct RequestProperty {
  std::string name;
  std::string value;
  bool output;
  bool child;

  RequestProperty() : name(), value(), output(), child() {
  }

  RequestProperty(const RequestProperty &p) : name(p.name), value(p.value),
				output(p.output), child(p.child) {

  }

  RequestProperty(const std::string &n, const std::string &v, bool out, bool ch) {
	name = n;
	value = v;
	output = out;
	child = ch;
  }

  const RequestProperty & operator = (RequestProperty &p) {
	name = p.name;
	value = p.value;
	output = p.output;
	child = p.child;

	return *this;
  }
};

typedef std::vector<RequestProperty> RequestProperties;

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

  virtual boost::shared_ptr<RequestProperties> getProperties() = 0;
  virtual boost::shared_ptr<ObjectReferences> getReferences()  = 0;
  virtual const std::pair<int, std::string> getResultCode() const = 0;
  virtual const std::string& getResultCodeName() const = 0;

};


};
};

#endif /* REQUEST_H_ */
