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

#include "src/deprecated/libfred/common_impl.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/util/log.hh"
#include "util/log/logger.hh"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     LibFred::CommonObjectImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace LibFred {

CommonObjectImpl::CommonObjectImpl() :
  id_(0), modified_(true) {
}

CommonObjectImpl::CommonObjectImpl(TID _id) :
  id_(_id), modified_(true) {
}

TID CommonObjectImpl::getId() const {
  return id_;
}
void CommonObjectImpl::setId(TID id) {
  id_ = id;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     LibFred::CommonListImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

CommonListImpl::CommonListImpl(DBSharedPtr _db) :
  db(_db), load_offset_(0), load_limit_(1000), real_size_(0), real_size_initialized_(false)
    , load_limit_active_(), ptr_idx_(-1), add(false), wcheck(true), idFilter(0)
{}

CommonListImpl::CommonListImpl() :
  db(), load_offset_(0), load_limit_(1000), real_size_(0), real_size_initialized_(false),
  load_limit_active_(), ptr_idx_(-1), add(false), wcheck(true), idFilter(0)
{}

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

void CommonListImpl::setOffset(unsigned _offset) {
  load_offset_ = _offset;
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

void CommonListImpl::setTimeout(unsigned _timeout) {
  Database::Connection conn = Database::Manager::acquire();

  conn.setQueryTimeout(_timeout);
}

CommonObject* CommonListImpl::get(unsigned _idx) const {
  return _idx >= getCount() ? NULL : data_[_idx];
}

void CommonListImpl::release(const unsigned long long &_idx)
{
    data_.erase(data_.begin() + _idx);
}

CommonObject* CommonListImpl::findId(TID _id) const {
  list_type::const_iterator it = std::find_if(data_.begin(),
                                              data_.end(),
                                              CheckId(_id));
  if (it != data_.end()) {
    LOGGER.debug(boost::format("object list hit! object id=%1% found")
        % _id);
    return *it;
  }
  LOGGER.debug(boost::format("object list miss! object id=%1% should be loaded from db")
      % _id);
  throw LibFred::NOT_FOUND();
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
    LOGGER.debug(boost::format("find id sequence: not found in result set. (id=%1%, ptr_idx=%2%)")
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
    LOGGER.warning("can't make real filter data count -- no filter specified...");
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

  Database::Connection conn = Database::Manager::acquire();
 
    try {
      Database::Result r_count = conn.exec(count_query);
      real_size_ = (*(r_count.begin()))[0];
      real_size_initialized_ = true;
    }
    catch (Database::Exception& ex) {
      LOGGER.error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
      LOGGER.error(boost::format("%1%") % ex.what());
    }
}

void CommonListImpl::makeRealCount() {
  std::stringstream sql;
  add = false;
  makeQuery(true, false, sql);
  DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
  if (!db->ExecSelect(sql.str().c_str()))
    throw SQL_ERROR();
  if (db->GetSelectRows() != 1)
    throw SQL_ERROR();
  real_size_ = atoll(db->GetFieldValue(0, 0));
  real_size_initialized_ = true;
}

void CommonListImpl::fillTempTable(Database::InsertQuery& _query) {
  try {
    Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
    Database::Connection conn = Database::Manager::acquire();
    conn.exec(create_tmp_table);
    conn.exec(_query);
    
    Database::Query analyze("ANALYZE " + std::string(getTempTableName()));
    conn.exec(analyze);
  }
  catch (Database::Exception& ex) {
    LOGGER.error(boost::format("%1%") % ex.what());
    throw;
  }
  catch (std::exception& ex) {
    LOGGER.error(boost::format("%1%") % ex.what());
    throw;
  }
}

void CommonListImpl::fillTempTable(bool _limit) const {
  // this code is same fo every object should be inherited
  std::stringstream sql;
  if (!add) {
    sql << "SELECT create_tmp_table('" << getTempTableName() << "')";
    DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
    if (!db->ExecSelect(sql.str().c_str()))
      throw SQL_ERROR();
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
