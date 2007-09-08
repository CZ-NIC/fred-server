#ifndef _COMMON_OBJECT_H_
#define _COMMON_OBJECT_H_

#include "types.h"
#include "exceptions.h"

namespace Register
{
  class CommonObject 
  {
   public:
	virtual ~CommonObject() {}
	/// return id of object
	virtual TID getId() const = 0;
  }; // CommonObject

  class CommonList
  {
   public:
    virtual ~CommonList() {}
    /// return count of objects in list
    virtual unsigned getCount() const = 0;
    /// get detail of loaded objects
    virtual CommonObject *get(unsigned idx) const = 0;
    /// set filter for id
    virtual void setIdFilter(TID id) = 0;
    /// clear filter
    virtual void clear() = 0;
    /// set limit for result
    virtual void setLimit(unsigned count) = 0;
    /// fill temporary table with selected ids 
    virtual void fillTempTable(bool limit) const throw (SQL_ERROR) = 0;
    /// fill variable with count of select objects
    virtual void makeRealCount() throw (SQL_ERROR) = 0;    
    /// get name of temporary table with result of filter
    virtual const char *getTempTableName() const = 0;    
    /// get variable with count of select objects
    virtual unsigned long long getRealCount() const = 0;
    /// set flag for enabling wildcard expansion in handles 
    virtual void setWildcardExpansion(bool _wcheck) = 0;
  };

}; // Register

#endif
