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

#include "common_impl.h"
#include "dbsql.h"
#include "log.h"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::CommonObjectImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Register::CommonObjectImpl::CommonObjectImpl(
  TID _id
) : id(_id), modified(true)
{
}

Register::CommonObjectImpl::CommonObjectImpl()
  : id(0), modified(true)
{
}

Register::TID
Register::CommonObjectImpl::getId() const
{
  return id;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::CommonListImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Register::CommonListImpl::CommonListImpl(DB *_db) :
  idFilter(0),
  realCount(0),
  limitCount(5000),
  wcheck(true),
  db(_db),
  ptrIdx(-1),
  add(false),
  realCountMade(false)
{
}

Register::CommonListImpl::~CommonListImpl()
{
  clear();
}

void 
Register::CommonListImpl::setIdFilter(TID id)
{
  idFilter = id;
}

unsigned 
Register::CommonListImpl::getCount() const
{
  return olist.size();
}

Register::CommonObject*
Register::CommonListImpl::get(unsigned idx) const
{
  return idx >= getCount() ? NULL : olist[idx];
}      

void
Register::CommonListImpl::clear()
{
  for (unsigned i=0; i<olist.size(); i++) delete olist[i];
  olist.clear();
  add = false;
}

void
Register::CommonListImpl::clearFilter()
{
  idFilter = 0;
}

unsigned long long 
Register::CommonListImpl::getRealCount()
{
  if (!realCountMade)
    makeRealCount();
  return realCount;
}

void 
Register::CommonListImpl::fillTempTable(bool limit) const throw (SQL_ERROR)
{
  // this code is same fo every object should be inherited
  std::stringstream sql;
  if (!add) {
    sql << "SELECT create_tmp_table('"  << getTempTableName() << "')";
    if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
    db->FreeSelect();
    sql.str("");
  }
  makeQuery(false,limit,sql);
  if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
  // TODO: temporary add solution, fix with multiple filter objects
  ((CommonListImpl *)this)->add = true;
  sql.str("");
  sql << "ANALYZE " << getTempTableName();
  if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
}

void 
Register::CommonListImpl::makeRealCount() throw (SQL_ERROR)
{
  std::stringstream sql;
  makeQuery(true,false,sql);
  if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
  if (db->GetSelectRows() != 1) throw SQL_ERROR();
  realCount = atoll(db->GetFieldValue(0,0));
  db->FreeSelect();
  realCountMade = true;
}

void 
Register::CommonListImpl::setLimit(unsigned count)
{
  limitCount = count;
}

void 
Register::CommonListImpl::setWildcardExpansion(bool _wcheck)
{
  wcheck = _wcheck;
}

void 
Register::CommonListImpl::resetIDSequence()
{
  ptrIdx = -1;
}

Register::CommonObject*
Register::CommonListImpl::findIDSequence(TID id)
{
  // must be sorted by ID to make sence
  if (ptrIdx < 0) ptrIdx = 0;
  for (;ptrIdx < (int)olist.size() && olist[ptrIdx]->getId()<id;ptrIdx++);
  if (ptrIdx == (int)olist.size() || olist[ptrIdx]->getId() != id) {
    LOG(ERROR_LOG, "find_sequence: id %ull, ptr %d", id, ptrIdx); 
    resetIDSequence();
    return NULL;
  }
  return olist[ptrIdx];
}

void 
Register::CommonListImpl::setFilterModified()
{
  realCountMade = false;
}
