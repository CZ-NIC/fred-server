#ifndef FILE_H_
#define FILE_H_

#include "common_object.h"
#include "object.h"
#include "db/dbs.h"
#include "model/model_filters.h"

namespace Register {
namespace File {

enum MemberType {
  MT_NAME,
  MT_CRDATE,
  MT_TYPE,
  MT_SIZE
};


class File : virtual public Register::CommonObject {
public:
  virtual const std::string& getName() const = 0;
  virtual const std::string& getPath() const = 0;
  virtual const std::string& getMimeType() const = 0;
  virtual const unsigned getType() const = 0;
  virtual const std::string& getTypeDesc() const = 0;
  virtual const DBase::DateTime& getCreateTime() const = 0;
  virtual const unsigned long getSize() const = 0;  
};


class List : virtual public Register::CommonList {
public:
  virtual File* get(unsigned _idx) const = 0;
  virtual void reload(DBase::Filters::Union& _filter) = 0;
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
  static Manager* create(DBase::Manager *_db_manager);
};

}
}

#endif /*FILE_H_*/
