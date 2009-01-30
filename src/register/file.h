#ifndef FILE_H_
#define FILE_H_

#include "common_object.h"
#include "object.h"
#include "db_settings.h"
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
  virtual const Database::DateTime& getCreateTime() const = 0;
  virtual const unsigned long getSize() const = 0;  

  virtual void setName(std::string name) = 0;
  virtual void setPath(std::string path) = 0;
  virtual void setMimeType(std::string mimeType) = 0;
  virtual void setType(unsigned int type) = 0;
  virtual void setCreateTime(Database::DateTime create_time) = 0;
  virtual void setCreateTime(std::string create_time) = 0;
  virtual void setSize(unsigned long size) = 0;
  virtual void save() = 0;
};


class List : virtual public Register::CommonList {
public:
  virtual File* get(unsigned _idx) const = 0;
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
  virtual File *createFile() const = 0;
  static Manager *create(Database::Manager *_db_manager);
  static Manager *create(Database::Connection *_conn);
};

}
}

#endif /*FILE_H_*/
