#ifndef REQUEST_H_
#define REQUEST_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "object.h"

#include "model/log_filter.h"
#include "db_settings.h"

#include "model_request_data.h"
#include "model_request.h"
#include "model_request_property_name.h"
#include "model_request_property_value.h"
#include "model_session.h"


using namespace Database;

namespace Fred {
namespace Logger {

typedef long int ServiceType;
typedef long int RequestType;

enum MemberType {
  MT_TIME_BEGIN,
  MT_TIME_END,
  MT_SOURCE_IP,
  MT_SERVICE,
  MT_ACTION,
  MT_SESSION_ID,
  MT_USER_NAME,
  MT_MONITORING,
  MT_RAW_REQUEST,
  MT_RAW_RESPONSE
};

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

};

class List : virtual public Fred::CommonList {

public:
  virtual void setPartialLoad(bool partialLoad) = 0;
  virtual Request* get(unsigned _idx) const = 0;
  virtual void reload(Database::Filters::Union& _filter) = 0;
  virtual void sort(MemberType _member, bool _asc) = 0;

  /// from CommonList; propably will be removed in future
  virtual const char* getTempTableName() const = 0;
  virtual void makeQuery(bool, bool, std::stringstream&) const = 0;
  virtual void reload() = 0;
};

enum Languages { EN, CS /*, __max_Languages=0xffffffff */ };

class Manager {
public:

  struct DB_CONNECT_FAILED { };

  virtual ~Manager() {};

  /** Used only in migration  - return a connection used by the connection manager
	it's meant to be used only in single-threaded environment
  */
virtual  Database::ID i_createRequest(const char *sourceIP, ServiceType service, const  char *content, const Fred::Logger::RequestProperties& props, const Fred::Logger::ObjectReferences &refs, RequestType request_type_id, Database::ID session_id) = 0;
virtual  bool i_addRequestProperties(Database::ID id, const Fred::Logger::RequestProperties &props) = 0;
virtual  bool i_closeRequest(Database::ID id, const char *content, const Fred::Logger::RequestProperties &props, const Fred::Logger::ObjectReferences &refs, const long result_code, Database::ID session_id) = 0;
  virtual Database::ID i_createSession(Database::ID id, const char *name) = 0;
virtual  bool i_closeSession(Database::ID id) = 0;
  virtual Database::Result i_getRequestTypesByService(ServiceType service) = 0;
  virtual Database::Result i_getServices() = 0;
  virtual Database::Result i_getResultCodesByService(ServiceType service) = 0;
  virtual Database::Result i_getObjectTypes() = 0;

  virtual List* createList() const = 0;

  static Manager *create();
  static Manager *create(const std::string conn_db, const std::string &monitoring_hosts_file = std::string()) throw(DB_CONNECT_FAILED);

};



};
};

#endif /* REQUEST_H_ */