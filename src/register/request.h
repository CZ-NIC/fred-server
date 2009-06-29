#ifndef LOG_ENTRY_H_
#define LOG_ENTRY_H_

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
  MT_SERVICE
};

class Request : virtual public Register::CommonObject {
public:
  virtual const Database::DateTime&  getTimeBegin() const = 0;
  virtual const Database::DateTime&  getTimeEnd() const = 0;
  virtual const Database::RequestServiceType& getServiceType() const = 0;
  virtual const std::string& 		getSourceIp() const = 0;

};


class List : virtual public Register::CommonList {

public:
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
  static Manager* create(Database::Manager *_db_manager);
};


};
};

#endif /* LOG_ENTRY_H_ */
