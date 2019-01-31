#ifndef COMMON_IMPL_HH_5E0AB69DC1D24D3682950E8DEB599D77
#define COMMON_IMPL_HH_5E0AB69DC1D24D3682950E8DEB599D77

#include <sstream>
#include <vector>

#include "src/deprecated/libfred/common_object.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/util/db/query/query.hh"

namespace LibFred {

/// Implementation of common registry object properties
class CommonObjectImpl : virtual public CommonObject {
public:
  CommonObjectImpl();
  CommonObjectImpl(TID _id);
  TID getId() const;
  void setId(TID id);

protected:
  TID id_;
  bool modified_;
};


#define COMPARE_CLASS_IMPL(_object_type, _by_what)                          \
class Compare##_by_what {                                                   \
public:                                                                     \
  Compare##_by_what(bool _asc) : asc_(_asc) { }                             \
  bool operator()(CommonObject *_left, CommonObject *_right) const {        \
    _object_type *l_casted = dynamic_cast<_object_type *>(_left);           \
    _object_type *r_casted = dynamic_cast<_object_type *>(_right);          \
    if (l_casted == 0 || r_casted == 0) {                                   \
      /* this should never happen */                                        \
      throw std::bad_cast();                                                \
    }                                                                       \
                                                                            \
    return (asc_ ? l_casted->get##_by_what() < r_casted->get##_by_what()    \
                 : l_casted->get##_by_what() > r_casted->get##_by_what());  \
    }                                                                       \
private:                                                                    \
  bool asc_;                                                                \
};                                                                   


struct CheckId {
  CheckId(LibFred::TID _id) :
    find_id_(_id) {
  }
  bool operator()(LibFred::CommonObject* _object) {
    return _object->getId() == find_id_;
  }
  LibFred::TID find_id_;
};


/// Implementation of common object list properties
class CommonListImpl : virtual public CommonList {
protected:
  DBSharedPtr db;

  list_type data_;
  unsigned load_offset_;
  unsigned load_limit_;
  unsigned long long real_size_;
  bool real_size_initialized_;
  bool load_limit_active_;

  int ptr_idx_;
  bool add;
  bool wcheck;
  TID idFilter;

public:
  CommonListImpl(DBSharedPtr _db);
  CommonListImpl();
  ~CommonListImpl();
  virtual void clear();
  
  size_type size() const;
  unsigned long long sizeDb();
  void setOffset(unsigned _offset);
  virtual void setLimit(unsigned _limit);
  virtual unsigned getLimit() const;
  virtual bool isLimited() const;
  void setTimeout(unsigned _timeout);
  
  CommonObject* get(unsigned _idx) const;
  void release(const unsigned long long &_idx);
  CommonObject* findId(TID _id) const;
  
  void resetIDSequence();
  CommonObject* findIDSequence(TID _id);

  virtual void fillTempTable(Database::InsertQuery& _query);
  virtual void fillTempTable(bool _limit) const;
  
  unsigned getCount() const;
  virtual unsigned long long getRealCount();
  virtual void makeRealCount();
  virtual unsigned long long getRealCount(Database::Filters::Union &_filter);
  virtual void makeRealCount(Database::Filters::Union &_filter);

  virtual void reload();

  
  virtual void setWildcardExpansion(bool _wcheck);
  virtual void setIdFilter(TID id);
  virtual void clearFilter();
  virtual void setFilterModified();

  virtual void
      makeQuery(bool count, bool limit, std::stringstream& sql) const = 0;
  
  virtual Iterator begin();
  virtual Iterator end();
};

}

#endif
