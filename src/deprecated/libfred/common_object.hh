#ifndef COMMON_OBJECT_HH_032DFFE6A56A419B96321F6CC7E1868A
#define COMMON_OBJECT_HH_032DFFE6A56A419B96321F6CC7E1868A

#include "libfred/types.hh"
#include "src/deprecated/libfred/exceptions.hh"
#include "src/deprecated/model/model_filters.hh"
#include "libfred/db_settings.hh"

namespace LibFred {

/**
 * Type of searchable objects
 */
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
  FT_STATEMENTITEM
};

/*
 * Object info (for links between objects)
 */
struct OID {
  OID(Database::ID _id) : id(_id) { }
  OID(Database::ID _id, std::string _handle, FilterType _type) : id(_id),
                                                                 handle(_handle),
                                                                 type(_type) { }
  Database::ID   id;
  std::string handle;
  FilterType  type;
};


/**
 * Top of the objects hierarchy
 */
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
  typedef list_type::iterator Iterator;  

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
  /// set offset for result 
  virtual void setOffset(unsigned _offset) = 0;
  /// set limit for result
  virtual void setLimit(unsigned _limit) = 0;
  /// get load lomit
  virtual unsigned getLimit() const = 0;
  /// true if result size has been limited by load_limit_ value
  virtual bool isLimited() const = 0;
  /// set Maximum time for a query
  virtual void setTimeout(unsigned _timeout) = 0;

  /// get detail of loaded objects  
  virtual CommonObject *get(unsigned _idx) const = 0;
  virtual void release(const unsigned long long &_idx) = 0;
  /// get detail of object with given ID
  virtual CommonObject* findId(TID _id) const = 0;
  
  
  /// return count of objects in list
  virtual unsigned getCount() const = 0;
  /// get variable with count of select objects
  virtual unsigned long long getRealCount() = 0;
  /// fill variable with count of select objects
  virtual void makeRealCount() = 0;
  /// get variable with count of select objects
  virtual unsigned long long getRealCount(Database::Filters::Union &_filter) = 0;
  /// make real count for new filters
  virtual void makeRealCount(Database::Filters::Union &_filter) = 0;
  
  /// fill temporary table with selected ids 
  virtual void fillTempTable(bool _limit) const = 0;
  /// get name of temporary table with result of filter
  virtual const char *getTempTableName() const = 0;

  /// set filter for id
  virtual void setIdFilter(TID _id) = 0;
  /// set flag for enabling wildcard expansion in handles 
  virtual void setWildcardExpansion(bool _wcheck) = 0;
  
  /// reload data according to filter
  virtual void reload() = 0;
  
  virtual Iterator begin() = 0;
  virtual Iterator end() = 0;
};

}
; // Fred

#endif
