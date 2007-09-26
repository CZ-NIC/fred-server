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
  limitCount(1000),
  wcheck(true),
  db(_db),
  ptrIdx(-1),
  add(false)
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
Register::CommonListImpl::getRealCount() const
{
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
  for (;ptrIdx < olist.size() && olist[ptrIdx]->getId()<id;ptrIdx++);
  if (ptrIdx == olist.size() || olist[ptrIdx]->getId() != id) {
    LOG(ERROR_LOG, "find_sequence: id %ull, ptr %d", id, ptrIdx); 
    resetIDSequence();
    return NULL;
  }
  return olist[ptrIdx];
}
