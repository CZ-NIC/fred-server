#include "object_impl.h"
#include "dbsql.h"
#include "sql.h"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::StatusImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Register::StatusImpl::StatusImpl(TID _id, ptime _timeFrom, ptime _timeTo)
  : id(_id), timeFrom(_timeFrom), timeTo(_timeTo)
{
}

Register::StatusImpl::~StatusImpl()
{
}

Register::TID
Register::StatusImpl::getStatusId() const
{
  return id;
}

ptime
Register::StatusImpl::getFrom() const
{
  return timeFrom;
}

ptime 
Register::StatusImpl::getTo() const
{
  return timeTo;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::ObjectImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Register::ObjectImpl::ObjectImpl(
  TID _id, ptime _crDate, ptime _trDate, ptime _upDate,
  TID _registrar, const std::string _registrarHandle,
  TID _createRegistrar, const std::string _createRegistrarHandle,
  TID _updateRegistrar, const std::string _updateRegistrarHandle,
  const std::string& _authPw, const std::string _roid
) :
  CommonObjectImpl(_id), crDate(_crDate), trDate(_trDate), upDate(_upDate), 
  registrar(_registrar), registrarHandle(_registrarHandle),
  createRegistrar(_createRegistrar), 
  createRegistrarHandle(_createRegistrarHandle),
  updateRegistrar(_updateRegistrar), 
  updateRegistrarHandle(_updateRegistrarHandle),
  authPw(_authPw), roid(_roid)
{
}

Register::ObjectImpl::ObjectImpl()
  : CommonObjectImpl(0), 
    crDate(not_a_date_time), trDate(not_a_date_time), 
    upDate(not_a_date_time),
    registrar(0), createRegistrar(0), updateRegistrar(0)
{
}

ptime 
Register::ObjectImpl::getCreateDate() const
{
  return crDate;
}

ptime 
Register::ObjectImpl::getTransferDate() const
{
  return trDate;
}

ptime 
Register::ObjectImpl::getUpdateDate() const
{
  return upDate;
}

Register::TID
Register::ObjectImpl::getRegistrarId() const
{
  return registrar;
}

const std::string&
Register::ObjectImpl::getRegistrarHandle() const
{
  return registrarHandle;
}

Register::TID
Register::ObjectImpl::getUpdateRegistrarId() const
{
  return updateRegistrar;
}

const std::string&
Register::ObjectImpl::getCreateRegistrarHandle() const
{
  return createRegistrarHandle;
}

const std::string&
Register::ObjectImpl::getUpdateRegistrarHandle() const
{
  return updateRegistrarHandle;
}

Register::TID
Register::ObjectImpl::getCreateRegistrarId() const
{
  return createRegistrar;
}

const std::string& 
Register::ObjectImpl::getAuthPw() const
{
  return authPw;
}

void 
Register::ObjectImpl::setAuthPw(const std::string& authPw)
{
  this->authPw = authPw;
}

const std::string& 
Register::ObjectImpl::getROID() const
{
  return roid;
}


unsigned 
Register::ObjectImpl::getStatusCount() const
{
  return slist.size();
}

const Register::Status* 
Register::ObjectImpl::getStatusByIdx(unsigned idx) const
{
  if (idx >= slist.size()) return NULL;
  else return &slist[idx];
}

void
Register::ObjectImpl::insertStatus(TID id, ptime timeFrom, ptime timeTo)
{
  slist.push_back(StatusImpl(id,timeFrom,timeTo));
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::ObjectListImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Register::ObjectListImpl::ObjectListImpl(DB *_db) :
  CommonListImpl(_db),
  registrarFilter(0), 
  createRegistrarFilter(0), 
  updateRegistrarFilter(0),
  crDateIntervalFilter(ptime(neg_infin),ptime(pos_infin)),
  updateIntervalFilter(ptime(neg_infin),ptime(pos_infin)),
  trDateIntervalFilter(ptime(neg_infin),ptime(pos_infin))
{
}

void 
Register::ObjectListImpl::setRegistrarFilter(TID registrarId)
{
  registrarFilter = registrarId;
}

void 
Register::ObjectListImpl::setRegistrarHandleFilter(
  const std::string& registrarHandle
)
{
  registrarHandleFilter = registrarHandle;
}

void 
Register::ObjectListImpl::setCrDateIntervalFilter(time_period period)
{
  crDateIntervalFilter = period;
}

void 
Register::ObjectListImpl::setCreateRegistrarFilter(TID registrarId)
{
  createRegistrarFilter = registrarId;
}

void 
Register::ObjectListImpl::setCreateRegistrarHandleFilter(
  const std::string& registrarHandle
)
{
  createRegistrarHandleFilter = registrarHandle;
}

void 
Register::ObjectListImpl::setUpdateIntervalFilter(time_period period)
{
  updateIntervalFilter = period;
}

void 
Register::ObjectListImpl::setUpdateRegistrarFilter(TID registrarId)
{
  updateRegistrarFilter = registrarId;
}

void 
Register::ObjectListImpl::setUpdateRegistrarHandleFilter(
  const std::string& registrarHandle
)
{
  updateRegistrarHandleFilter = registrarHandle;
}

void 
Register::ObjectListImpl::setTrDateIntervalFilter(time_period period)
{
  trDateIntervalFilter = period;
}

void 
Register::ObjectListImpl::addStateFilter(TID state, bool stateIsOn)
{
  for (unsigned i=0; i<sflist.size(); i++)
    if (sflist[i].stateId == state) {
      sflist[i].stateIsOn = stateIsOn;
      return;
    }
  StatusFilter f;
  f.stateId = state;
  f.stateIsOn = stateIsOn;
  sflist.push_back(f);
}

void 
Register::ObjectListImpl::clearStateFilter(TID state)
{
  for (StatusFilterList::iterator i=sflist.begin(); i!=sflist.end(); i++)
    if (i->stateId == state) {
      sflist.erase(i);
      return;
    }
}

void
Register::ObjectListImpl::clearFilter()
{
  CommonListImpl::clearFilter();
  crDateIntervalFilter = time_period(ptime(neg_infin),ptime(pos_infin));
  updateIntervalFilter = time_period(ptime(neg_infin),ptime(pos_infin));
  trDateIntervalFilter = time_period(ptime(neg_infin),ptime(pos_infin));
  registrarFilter = 0;
  createRegistrarFilter = 0;
  updateRegistrarFilter = 0;
  registrarHandleFilter = "";
  createRegistrarHandleFilter = ""; 
  updateRegistrarHandleFilter = "";
  sflist.clear();
}

void
Register::ObjectListImpl::reload() throw (SQL_ERROR)
{
  std::ostringstream sql;
  sql << "SELECT tmp.id, state_id, valid_from "
      << "FROM " << getTempTableName() << " tmp, object_state os "
      << "WHERE tmp.id=os.object_id AND valid_to ISNULL "
      << "ORDER BY tmp.id ";
  if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
  for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
    ObjectImpl *o = dynamic_cast<ObjectImpl *>(findIDSequence(
      STR_TO_ID(db->GetFieldValue(i,0))
    ));
    if (!o) throw SQL_ERROR(); 
    o->insertStatus(
      STR_TO_ID(db->GetFieldValue(i,1)), MAKE_TIME(i,2), ptime()
    );
  }
  db->FreeSelect();
}
