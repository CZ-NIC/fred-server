#ifndef COMMON_IMPL_H_
#define COMMON_IMPL_H_

#include <sstream>
#include <vector>
#include "common_object.h"

class DB;

namespace Register
{
  /// Implementation of common register object properties
  class CommonObjectImpl : virtual public CommonObject
  {
   protected:
    TID id;
    bool modified;
   public:
	CommonObjectImpl();
    CommonObjectImpl(TID _id);
    TID getId() const;
  }; // CommonObjectImpl
  
  /// Implementation of common object list properties
  class CommonListImpl : virtual public CommonList
  {
   public:
    typedef std::vector<CommonObject *> ListType;
   protected:
    TID idFilter;
    unsigned long long realCount;
    unsigned limitCount;
    bool wcheck; ///< do wildcard expansion in filter handles
    DB *db;
    ListType olist;
    int ptrIdx;
    bool add;
    bool realCountMade; ///< was realCount already initialized?
   public: 
    ~CommonListImpl();
    CommonListImpl(DB *db);
    virtual void setIdFilter(TID id);
    unsigned getCount() const;
    CommonObject* get(unsigned idx) const;
    virtual void makeQuery(
      bool count, bool limit, std::stringstream& sql
    ) const = 0;
    virtual void clear();
    virtual void clearFilter();
    virtual void setLimit(unsigned count);
    virtual unsigned long long getRealCount();
    virtual void fillTempTable(bool limit) const throw (SQL_ERROR);
    virtual void makeRealCount() throw (SQL_ERROR);    
    virtual void setWildcardExpansion(bool _wcheck);
    void resetIDSequence();
    CommonObject* findIDSequence(TID id);    
    
  }; // CommonListImpl
  
}; // Register

#endif
