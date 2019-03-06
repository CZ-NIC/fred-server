/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef PAGETABLE_IMPL_HH_41E691C3FA1145959F5C330912F67847
#define PAGETABLE_IMPL_HH_41E691C3FA1145959F5C330912F67847

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <exception>
#include <limits>


#include "src/bin/corba/admin/common.hh"
#include "src/bin/corba/admin/str_corbaout.hh"
#include "src/bin/corba/admin/filter_impl.hh"
#include "src/bin/corba/mailer_manager.hh"

#include "src/deprecated/libfred/registry.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"
#include "src/deprecated/libfred/notify.hh"
#include "src/deprecated/libfred/mail.hh"
#include "src/deprecated/libfred/filter.hh"
#include "src/deprecated/libfred/requests/request_list.hh"

#include "src/deprecated/util/log.hh"
#include "src/deprecated/util/dbsql.hh"

#include "util/log/logger.hh"
#include "util/log/context.hh"

#include "src/deprecated/model/model_filters.hh"

#include "src/bin/corba/connection_releaser.hh"

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
  Registry::TableRow* getRow(CORBA::UShort row); \
  ccReg::TID getRowId(CORBA::UShort row); \
  char* outputCSV();\
  CORBA::Short numRows();\
  CORBA::Short numColumns();\
  void reload_worker();\
  void clear();\
  CORBA::ULongLong resultSize();\
  void loadFilter(ccReg::TID _id);\
  void saveFilter(const char* _name);\
  void sortByColumn(CORBA::Short _column, CORBA::Boolean _dir);\
  CORBA::Boolean numRowsOverLimit();

class ccReg_PageTable_i : virtual public POA_Registry::PageTable {
  unsigned int aPageSize;
  unsigned int aPage;

enum { DEFAULT_QUERY_TIMEOUT = 15000 };

protected:
  Database::Filters::Union uf;
  FilterIteratorImpl it;
  ccReg::FilterType filterType;
  int sorted_by_;
  bool sorted_dir_;
  long query_timeout;
  unsigned long limit_;

  /**
   * context with session object was created - need for futher call on object
   * which are done in separate threads 
   */
  std::string base_context_;

public:
  ccReg_PageTable_i();
  virtual ~ccReg_PageTable_i();
  void setDB();
  CORBA::Short pageSize();
  void pageSize(CORBA::Short _v);
  CORBA::Short page();
  void setPage(CORBA::Short page);
  virtual void setOffset(CORBA::Long _offset);
  virtual void setLimit(CORBA::Long _limit);
  void setTimeout(CORBA::Long _timeout);
  CORBA::Short start();
  CORBA::Short numPages();
  Registry::TableRow* getPageRow(CORBA::Short pageRow);
  CORBA::Short numPageRows();
  ccReg::TID getPageRowId(CORBA::Short row);
  void reloadF();
  void reload();
  virtual void reload_worker() = 0;
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
