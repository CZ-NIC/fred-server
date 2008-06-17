#ifndef REGISTER_FILTER_H_
#define REGISTER_FILTER_H_

#include "types.h"
#include "db/dbs.h"
#include "model/model_filters.h"

namespace Register {
namespace Filter {

enum FilterType {
  FT_FILTER,
  FT_REGISTRAR,
  FT_OBJ,
  FT_CONTACT,
  FT_NSSET,
  FT_DOMAIN,
  FT_ACTION,
  FT_INVOICE,
  FT_PUBLICREQUEST,
  FT_MAIL,
  FT_FILE
};

class Filter {
public:
  virtual ~Filter() {
  }
  virtual DBase::ID getId() const = 0;
  virtual const std::string& getName() const = 0;
  virtual void setName(const std::string& _name) = 0;
  virtual FilterType getType() const = 0;
  virtual void setType(FilterType _type) = 0;
  virtual DBase::ID getUserId() const = 0;
  virtual void setUserId(DBase::ID _id) = 0;
  virtual DBase::ID getGroupId() const = 0;
  virtual void setGroupId(DBase::ID _id) = 0;
  virtual void save(DBase::Connection *_conn) const = 0;
};

class List {
public:
  virtual ~List() {
  }
  virtual void reload(DBase::Filters::Union &uf) = 0;
  virtual const unsigned size() const = 0;
  virtual void clear() = 0;
  virtual const Filter* get(unsigned _idx) const = 0;
};

class Manager {
public:
  virtual ~Manager() {
  }
  virtual List& getList() = 0;
  virtual void load(DBase::ID _id, DBase::Filters::Union& _uf) const = 0;
  virtual void save(FilterType _type, const std::string& _name, DBase::Filters::Union& _uf) = 0;
  static Manager *create(DBase::Manager* _db_manager);
};

}
}

#endif /*REGISTER_FILTER_H_*/
