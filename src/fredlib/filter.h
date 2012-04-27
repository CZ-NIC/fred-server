#ifndef REGISTER_FILTER_H_
#define REGISTER_FILTER_H_

#include "common_impl.h"
#include "types.h"
#include "db_settings.h"
#include "model/model_filters.h"

namespace Fred {
namespace Filter {

enum FilterType {
  FT_FILTER,
  FT_REGISTRAR,
  FT_OBJ,
  FT_CONTACT,
  FT_NSSET,
  FT_KEYSET,
  FT_DOMAIN,
  FT_INVOICE,
  FT_PUBLICREQUEST,
  FT_MAIL,
  FT_FILE,
  FT_LOGGER,
  FT_SESSION,
  FT_STATEMENTITEM,
  FT_ZONE,
  FT_STATEMENTHEAD,
  FT_MESSAGE
};

class Filter : virtual public Fred::CommonObject {
public:
  virtual ~Filter() {
  }
  virtual const std::string& getName() const = 0;
  virtual void setName(const std::string& _name) = 0;
  virtual FilterType getType() const = 0;
  virtual void setType(FilterType _type) = 0;
  virtual Database::ID getUserId() const = 0;
  virtual void setUserId(Database::ID _id) = 0;
  virtual Database::ID getGroupId() const = 0;
  virtual void setGroupId(Database::ID _id) = 0;
  virtual void save() const = 0;
};

class List : virtual public Fred::CommonList {
public:
  virtual ~List() {
  }
  virtual Filter* get(unsigned _idx) const = 0;
  virtual void reload(Database::Filters::Union &uf) = 0;
  
  /// from CommonList; propably will be removed in future
  virtual const char* getTempTableName() const = 0;
  virtual void makeQuery(bool, bool, std::stringstream&) const = 0;
  virtual void reload() = 0;
};

class Manager {
public:
  virtual ~Manager() {
  }
  virtual List& getList() = 0;
  virtual void load(Database::ID _id, Database::Filters::Union& _uf) const = 0;
  virtual void save(FilterType _type, const std::string& _name, Database::Filters::Union& _uf) = 0;
  static Manager *create();
};

}
}

#endif /*REGISTER_FILTER_H_*/
