#ifndef _COMMON_OBJECT_H_
#define _COMMON_OBJECT_H_

#include "types.h"
#include "exceptions.h"
#include "model/model_filters.h"
#include "db/dbs.h"

namespace Register {
class CommonObject {
public:
  /// D-tor
  virtual ~CommonObject() {
  }
  /// return id of object
  virtual TID getId() const = 0;
};

class CommonList {
protected:
  typedef std::vector<CommonObject *> list_type;
  typedef list_type::size_type size_type;
  typedef list_type::iterator iterator;  

public:
  /// D-tor
  virtual ~CommonList() {
  }
  /// clear filter
  virtual void clear() = 0;
  
  /// return count of objects in list
  virtual size_type size() const = 0;
  /// return count of objects in database
  virtual unsigned long long sizeDb() = 0;
  /// set limit for result
  virtual void setLimit(unsigned _limit) = 0;

  /// get detail of loaded objects  
  virtual CommonObject *get(unsigned _idx) const = 0;
  /// get detail of object with given ID
  virtual CommonObject* findId(TID _id) const throw (Register::NOT_FOUND) = 0;
  
  
  /// return count of objects in list
  virtual unsigned getCount() const = 0;
  /// get variable with count of select objects
  virtual unsigned long long getRealCount() = 0;
  /// fill variable with count of select objects
  virtual void makeRealCount() throw (SQL_ERROR) = 0;
  /// get variable with count of select objects
  virtual unsigned long long getRealCount(DBase::Filters::Union &_filter) = 0;
  /// make real count for new filters
  virtual void makeRealCount(DBase::Filters::Union &_filter) = 0;
  
  /// fill temporary table with selected ids 
  virtual void fillTempTable(bool _limit) const throw (SQL_ERROR) = 0;
  /// get name of temporary table with result of filter
  virtual const char *getTempTableName() const = 0;

  /// set filter for id
  virtual void setIdFilter(TID _id) = 0;
  /// set flag for enabling wildcard expansion in handles 
  virtual void setWildcardExpansion(bool _wcheck) = 0;
  
  /// reload data according to filter
  virtual void reload() = 0;
  
  virtual iterator begin() = 0;
  virtual iterator end() = 0;
};

}
; // Register

#endif
