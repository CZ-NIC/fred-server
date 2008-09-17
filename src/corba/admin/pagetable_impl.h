#ifndef PAGETABLE_IMPL_H_
#define PAGETABLE_IMPL_H_

#include <memory>
#include <string>
#include <map>
#include <vector>

#include "common.h"
#include "filter_impl.h"
#include "corba/mailer_manager.h"

#include "register/register.h"
#include "register/invoice.h"
#include "register/notify.h"
#include "register/mail.h"
#include "register/filter.h"

#include "old_utils/log.h"
#include "old_utils/dbsql.h"
#include "old_utils/conf.h"

#include "log/logger.h"
#include "log/context.h"

#include "db/manager.h"
#include "model/model_filters.h"

using namespace Database;

#define DECL_ATTRIBUTE(name,type,settype,gettype) \
 private: \
  type name##Filter; \
 public: \
  void name(settype _v); \
  gettype name()

#define DECL_ATTRIBUTE_TYPE(name,type) \
  DECL_ATTRIBUTE(name,type,type,type)

#define DECL_ATTRIBUTE_STR(name) \
  DECL_ATTRIBUTE(name,std::string,const char *,char *)

#define DECL_ATTRIBUTE_ID(name) \
  DECL_ATTRIBUTE(name,ccReg::TID,ccReg::TID,ccReg::TID)

#define DECL_ATTRIBUTE_DATE(name) \
  DECL_ATTRIBUTE(name,ccReg::DateInterval,\
                 const ccReg::DateInterval&,ccReg::DateInterval)

#define DECL_ATTRIBUTE_DATETIME(name) \
  DECL_ATTRIBUTE(name,ccReg::DateTimeInterval,\
                 const ccReg::DateTimeInterval&,ccReg::DateTimeInterval)

#define DECL_PAGETABLE_I \
  Registry::Table::ColumnHeaders* getColumnHeaders(); \
  Registry::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);\
  ccReg::TID getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);\
  char* outputCSV();\
  CORBA::Short numRows();\
  CORBA::Short numColumns();\
  void reload();\
  void clear();\
  CORBA::ULongLong resultSize();\
  void loadFilter(ccReg::TID _id);\
  void saveFilter(const char* _name);\
  void sortByColumn(CORBA::Short _column, CORBA::Boolean _dir);\
  CORBA::Boolean numRowsOverLimit();

class ccReg_PageTable_i : virtual public POA_Registry::PageTable {
  unsigned int aPageSize;
  unsigned int aPage;

protected:
  Database::Filters::Union uf;
  FilterIteratorImpl it;
  Database::Manager* dbm;
  ccReg::FilterType filterType;
  int sorted_by_;
  bool sorted_dir_;

  /**
   * context with session object was created - need for futher call on object
   * which are done in separate threads 
   */
  std::string base_context_;

public:
  ccReg_PageTable_i();
  virtual ~ccReg_PageTable_i();
  void setDB(Database::Manager* dbm);
  CORBA::Short pageSize();
  void pageSize(CORBA::Short _v);
  CORBA::Short page();
  void setPage(CORBA::Short page) throw (ccReg::PageTable::INVALID_PAGE);
  CORBA::Short start();
  CORBA::Short numPages();
  Registry::TableRow* getPageRow(CORBA::Short pageRow) throw (ccReg::Table::INVALID_ROW);
  CORBA::Short numPageRows();
  ccReg::TID getPageRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  void reloadF();
  ccReg::Filters::Compound_ptr add();
  ccReg::FilterType filter() {
    return filterType;
  }
  ccReg::Filters::Iterator_ptr getIterator();
  void clear();
  void loadFilter(ccReg::TID _id);
  void saveFilter(const char* _name);
  void sortByColumn(CORBA::Short _column, CORBA::Boolean _dir);
  void getSortedBy(CORBA::Short &_column, CORBA::Boolean &_dir);
  CORBA::Boolean numRowsOverLimit();
};


#endif /*PAGETABLE_IMPL_H_*/
