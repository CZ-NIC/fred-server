#ifndef REQUEST_H_
#define REQUEST_H_

#include "object.h"
#include "db_settings.h"
#include "model/model_filters.h"
#include "model/log_filter.h"

namespace Register {
namespace Logger {

enum MemberType {
  MT_TIME_BEGIN,
  MT_TIME_END,
  MT_SOURCE_IP,
  MT_SERVICE,
  MT_ACTION,
  MT_SESSION_ID,
  MT_MONITORING,
  MT_RAW_REQUEST,
  MT_RAW_RESPONSE
};

struct RequestProperty {
  std::string name;
  std::string value;
  bool output;
  bool child;

  RequestProperty() : name(), value() {
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

/*
// TODO this is duplicity (src/model/log_filter.h):
typedef long RequestServiceType;
typedef long RequestActionType;
*/


class Request : virtual public Register::CommonObject {
public:
  virtual const boost::posix_time::ptime  getTimeBegin() const = 0;
  virtual const boost::posix_time::ptime  getTimeEnd() const = 0;
  virtual const std::string& 		getSourceIp() const = 0;
  virtual const Database::Filters::RequestServiceType& getServiceType() const = 0;
  virtual const Database::Filters::RequestActionType &getActionType() const = 0;
  virtual const Database::ID& getSessionId() const = 0;
  virtual const bool& getIsMonitoring() const = 0;
 
  virtual const std::string& getRawRequest() const = 0;
  virtual const std::string& getRawResponse() const  = 0;

  virtual 	std::auto_ptr<RequestProperties> getProperties() = 0;

};


class List : virtual public Register::CommonList {

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

class Manager {
public:
  virtual ~Manager() {
  }

  virtual List* createList() const = 0;
  static Manager* create();
};


};
};

#endif /* REQUEST_H_ */
