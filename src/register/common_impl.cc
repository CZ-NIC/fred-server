/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>

#include "common_impl.h"
#include "old_utils/dbsql.h"
#include "old_utils/log.h"
#include "log/logger.h"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::CommonObjectImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace Register {

CommonObjectImpl::CommonObjectImpl() :
  id_(0), modified_(true) {
}

CommonObjectImpl::CommonObjectImpl(TID _id) :
  id_(_id), modified_(true) {
}

TID CommonObjectImpl::getId() const {
  return id_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::CommonListImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

CommonListImpl::CommonListImpl(DB *_db) :
  db(_db), conn_(0), load_limit_(1000), real_size_(0), real_size_initialized_(false),
  ptr_idx_(-1), add(false), wcheck(true), idFilter(0) {
}

CommonListImpl::CommonListImpl(Database::Connection *_conn) :
  conn_(_conn), load_limit_(1000), real_size_(0), real_size_initialized_(false),
  ptr_idx_(-1), add(false), wcheck(true), idFilter(0) {
}

CommonListImpl::~CommonListImpl() {
  clear();
}

void CommonListImpl::clear() {
  TRACE("[CALL] CommonListImpl::clear()");

  for (unsigned i = 0; i < data_.size(); i++)
    delete data_[i];
  data_.clear();

  real_size_ = 0;
  real_size_initialized_ = false;
  add = false;
}

CommonList::size_type CommonListImpl::size() const {
  return data_.size();
}

unsigned long long CommonListImpl::sizeDb() {
  if (!real_size_initialized_)
    makeRealCount();

  return real_size_;
}

void CommonListImpl::setLimit(unsigned _limit) {
  load_limit_ = _limit;
}

unsigned CommonListImpl::getLimit() const {
  return load_limit_;
}

bool CommonListImpl::isLimited() const {
  return load_limit_active_;
}

CommonObject* CommonListImpl::get(unsigned _idx) const {
  return _idx >= getCount() ? NULL : data_[_idx];
}

CommonObject* CommonListImpl::findId(TID _id) const throw (Register::NOT_FOUND) {
  list_type::const_iterator it = std::find_if(data_.begin(),
                                              data_.end(),
                                              CheckId(_id));
  if (it != data_.end()) {
    LOGGER(PACKAGE).debug(boost::format("object list hit! object id=%1% found")
        % _id);
    return *it;
  }
  LOGGER(PACKAGE).debug(boost::format("object list miss! object id=%1% should be loaded from db")
      % _id);
  throw Register::NOT_FOUND();
}

void CommonListImpl::resetIDSequence() {
  ptr_idx_ = -1;
}

CommonObject* CommonListImpl::findIDSequence(TID _id) {
  // must be sorted by ID to make sence
  if (ptr_idx_ < 0)
    ptr_idx_ = 0;
  for (; ptr_idx_ < (int)data_.size() && data_[ptr_idx_]->getId()<_id; ptr_idx_++);
  if (ptr_idx_ == (int)data_.size() || data_[ptr_idx_]->getId() != _id) {
    LOGGER(PACKAGE).debug(boost::format("find id sequence: not found in result set. (id=%1%, ptr_idx=%2%)")
                                        % _id % ptr_idx_);
    resetIDSequence();
    return NULL;
  }
  return data_[ptr_idx_];
}

unsigned CommonListImpl::getCount() const {
  return data_.size();
}

unsigned long long CommonListImpl::getRealCount() {
  if (!real_size_initialized_)
    makeRealCount();

  return real_size_;
}

unsigned long long CommonListImpl::getRealCount(Database::Filters::Union &_filter) {
  TRACE("[CALL] CommonListImpl::getRealCount()");

  if (!real_size_initialized_)
    makeRealCount(_filter);

  return real_size_;
}

void CommonListImpl::makeRealCount(Database::Filters::Union &_filter) {
  TRACE("[CALL] CommonListImpl::makeRealCount()");
  
  if (_filter.empty()) {
    real_size_ = 0;
    real_size_initialized_ = false;
    LOGGER(PACKAGE).warning("can't make real filter data count -- no filter specified...");
    return;
  }

  _filter.clearQueries();

  Database::Filters::Union::iterator it = _filter.begin();
  for (; it != _filter.end(); ++it) {
    Database::SelectQuery *tmp = new Database::SelectQuery();
    tmp->select() << "COUNT(*)";
    _filter.addQuery(tmp);
  }

  Database::SelectQuery count_query;
  _filter.serialize(count_query);
 
  if (conn_) {
    try {
      Database::Result r_count = conn_->exec(count_query);
      real_size_ = (*(r_count.begin()))[0];
      real_size_initialized_ = true;
    }
    catch (Database::Exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
  }
  else {
    if (!db->ExecSelect(count_query.c_str())) {
      LOGGER(PACKAGE).error("filter data count failed - old database library connection used");
    }
    else {
      real_size_ = atoll(db->GetFieldValue(0, 0));
      real_size_initialized_ = true;
    }
  }
}

void CommonListImpl::makeRealCount() throw (SQL_ERROR) {
  std::stringstream sql;
  add = false;
  makeQuery(true, false, sql);
  if (!db->ExecSelect(sql.str().c_str()))
    throw SQL_ERROR();
  if (db->GetSelectRows() != 1)
    throw SQL_ERROR();
  real_size_ = atoll(db->GetFieldValue(0, 0));
  db->FreeSelect();
  real_size_initialized_ = true;
}

void CommonListImpl::fillTempTable(Database::InsertQuery& _query) {
  try {
    Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
    conn_->exec(create_tmp_table);
    conn_->exec(_query);
    
    Database::Query analyze("ANALYZE " + std::string(getTempTableName()));
    conn_->exec(analyze);
  }
  catch (Database::Exception& ex) {
    LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    throw;
  }
  catch (std::exception& ex) {
    LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    throw;
  }
}

void CommonListImpl::fillTempTable(bool _limit) const throw (SQL_ERROR) {
  // this code is same fo every object should be inherited
  std::stringstream sql;
  if (!add) {
    sql << "SELECT create_tmp_table('" << getTempTableName() << "')";
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
    db->FreeSelect();
    sql.str("");
  }
  makeQuery(false, _limit, sql);
  if (!db->ExecSQL(sql.str().c_str()))
    throw SQL_ERROR();
  // TODO: temporary add solution, fix with multiple filter objects
  ((CommonListImpl *)this)->add = true;
  sql.str("");
  sql << "ANALYZE " << getTempTableName();
  if (!db->ExecSQL(sql.str().c_str()))
    throw SQL_ERROR();
}

void CommonListImpl::reload() {
  load_limit_active_ = false;
  if (size() > load_limit_) {
    CommonObject *tmp = data_.back();
    data_.pop_back();
    delete tmp;
    load_limit_active_ = true;
  }
}

void CommonListImpl::setWildcardExpansion(bool _wcheck) {
  wcheck = _wcheck;
}

void CommonListImpl::setIdFilter(TID id) {
  idFilter = id;
}

void CommonListImpl::clearFilter() {
  idFilter = 0;
}

void CommonListImpl::setFilterModified() {
  real_size_initialized_ = false;
}

CommonList::Iterator CommonListImpl::begin() {
  return data_.begin();
}

CommonList::Iterator CommonListImpl::end() {
  return data_.end();
}

}
