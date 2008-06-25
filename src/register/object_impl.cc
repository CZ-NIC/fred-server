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

#include "object_impl.h"
#include "old_utils/dbsql.h"
#include "sql.h"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::StatusImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Register::StatusImpl::StatusImpl(TID _id, ptime _timeFrom, ptime _timeTo) :
  id(_id), timeFrom(_timeFrom), timeTo(_timeTo) {
}

Register::StatusImpl::~StatusImpl() {
}

Register::TID Register::StatusImpl::getStatusId() const {
  return id;
}

ptime Register::StatusImpl::getFrom() const {
  return timeFrom;
}

ptime Register::StatusImpl::getTo() const {
  return timeTo;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::ObjectImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Register::ObjectImpl::ObjectImpl(TID _id, ptime _crDate, ptime _trDate,
    ptime _upDate, date _erDate, TID _registrar,
    const std::string _registrarHandle, TID _createRegistrar,
    const std::string _createRegistrarHandle, TID _updateRegistrar,
    const std::string _updateRegistrarHandle, const std::string& _authPw,
    const std::string _roid) :
  CommonObjectImpl(_id), crDate(_crDate), trDate(_trDate), upDate(_upDate),
      erDate(_erDate), registrar(_registrar),
      registrarHandle(_registrarHandle), createRegistrar(_createRegistrar),
      createRegistrarHandle(_createRegistrarHandle),
      updateRegistrar(_updateRegistrar),
      updateRegistrarHandle(_updateRegistrarHandle), authPw(_authPw),
      roid(_roid) {
}

Register::ObjectImpl::ObjectImpl() :
  CommonObjectImpl(0), crDate(not_a_date_time), trDate(not_a_date_time),
      upDate(not_a_date_time), registrar(0), createRegistrar(0),
      updateRegistrar(0) {
}

ptime Register::ObjectImpl::getCreateDate() const {
  return crDate;
}

ptime Register::ObjectImpl::getTransferDate() const {
  return trDate;
}

ptime Register::ObjectImpl::getUpdateDate() const {
  return upDate;
}

date Register::ObjectImpl::getDeleteDate() const {
  return erDate;
}

Register::TID Register::ObjectImpl::getRegistrarId() const {
  return registrar;
}

const std::string& Register::ObjectImpl::getRegistrarHandle() const {
  return registrarHandle;
}

Register::TID Register::ObjectImpl::getUpdateRegistrarId() const {
  return updateRegistrar;
}

const std::string& Register::ObjectImpl::getCreateRegistrarHandle() const {
  return createRegistrarHandle;
}

const std::string& Register::ObjectImpl::getUpdateRegistrarHandle() const {
  return updateRegistrarHandle;
}

Register::TID Register::ObjectImpl::getCreateRegistrarId() const {
  return createRegistrar;
}

const std::string& Register::ObjectImpl::getAuthPw() const {
  return authPw;
}

void Register::ObjectImpl::setAuthPw(const std::string& authPw) {
  this->authPw = authPw;
}

const std::string& Register::ObjectImpl::getROID() const {
  return roid;
}

unsigned Register::ObjectImpl::getStatusCount() const {
  return slist.size();
}

const Register::Status* Register::ObjectImpl::getStatusByIdx(unsigned idx) const {
  if (idx >= slist.size())
    return NULL;
  else
    return &slist[idx];
}

void Register::ObjectImpl::insertStatus(TID id, ptime timeFrom, ptime timeTo) {
  slist.push_back(StatusImpl(id, timeFrom, timeTo));
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::ObjectListImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Register::ObjectListImpl::ObjectListImpl(DB *_db) :
  CommonListImpl(_db), registrarFilter(0), createRegistrarFilter(0),
      updateRegistrarFilter(0), crDateIntervalFilter(ptime(neg_infin),
          ptime(pos_infin)), updateIntervalFilter(ptime(neg_infin),
          ptime(pos_infin)), trDateIntervalFilter(ptime(neg_infin),
          ptime(pos_infin)), nonHandleFilterSet(false) {
}

void Register::ObjectListImpl::setRegistrarFilter(TID registrarId) {
  registrarFilter = registrarId;
  nonHandleFilterSet = true;
}

void Register::ObjectListImpl::setRegistrarHandleFilter(
    const std::string& registrarHandle) {
  registrarHandleFilter = registrarHandle;
  nonHandleFilterSet = true;
}

void Register::ObjectListImpl::setCrDateIntervalFilter(time_period period) {
  crDateIntervalFilter = period;
  nonHandleFilterSet = true;
}

void Register::ObjectListImpl::setCreateRegistrarFilter(TID registrarId) {
  createRegistrarFilter = registrarId;
  nonHandleFilterSet = true;
}

void Register::ObjectListImpl::setCreateRegistrarHandleFilter(
    const std::string& registrarHandle) {
  createRegistrarHandleFilter = registrarHandle;
  nonHandleFilterSet = true;
}

void Register::ObjectListImpl::setUpdateIntervalFilter(time_period period) {
  updateIntervalFilter = period;
  nonHandleFilterSet = true;
}

void Register::ObjectListImpl::setUpdateRegistrarFilter(TID registrarId) {
  updateRegistrarFilter = registrarId;
  nonHandleFilterSet = true;
}

void Register::ObjectListImpl::setUpdateRegistrarHandleFilter(
    const std::string& registrarHandle) {
  updateRegistrarHandleFilter = registrarHandle;
  nonHandleFilterSet = true;
}

void Register::ObjectListImpl::setTrDateIntervalFilter(time_period period) {
  trDateIntervalFilter = period;
  nonHandleFilterSet = true;
}

void Register::ObjectListImpl::addStateFilter(TID state, bool stateIsOn) {
  for (unsigned i=0; i<sflist.size(); i++)
    if (sflist[i].stateId == state) {
      sflist[i].stateIsOn = stateIsOn;
      return;
    }
  StatusFilter f;
  f.stateId = state;
  f.stateIsOn = stateIsOn;
  sflist.push_back(f);
  nonHandleFilterSet = true;
}

void Register::ObjectListImpl::clearStateFilter(TID state) {
  for (StatusFilterList::iterator i=sflist.begin(); i!=sflist.end(); i++)
    if (i->stateId == state) {
      sflist.erase(i);
      return;
    }
}

void Register::ObjectListImpl::clearFilter() {
  CommonListImpl::clearFilter();
  crDateIntervalFilter = time_period(ptime(neg_infin), ptime(pos_infin));
  updateIntervalFilter = time_period(ptime(neg_infin), ptime(pos_infin));
  trDateIntervalFilter = time_period(ptime(neg_infin), ptime(pos_infin));
  registrarFilter = 0;
  createRegistrarFilter = 0;
  updateRegistrarFilter = 0;
  registrarHandleFilter = "";
  createRegistrarHandleFilter = "";
  updateRegistrarHandleFilter = "";
  sflist.clear();
  nonHandleFilterSet = false;
}

void Register::ObjectListImpl::reload(const char *handle, int type) 
  throw (SQL_ERROR) {
  std::ostringstream sql;
  sql << "SELECT tmp.id, state_id, valid_from " << "FROM "
      << (!handle ? getTempTableName() : "object_registry ") << " tmp, "
      << " object_state os "
      << "WHERE tmp.id=os.object_id AND valid_to ISNULL ";
  if (handle) {
    sql << "AND tmp.name=" << (type==3 ? "LOWER" : "UPPER")
        << "('" << db->Escape2(handle) << "') "
        << "AND tmp.erdate ISNULL AND tmp.type=" << type << " "; 
  }
  sql << "ORDER BY tmp.id ";
  if (!db->ExecSelect(sql.str().c_str()))
    throw SQL_ERROR();
  resetIDSequence();
  for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
    ObjectImpl *o =
        dynamic_cast<ObjectImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(
            i, 0)) ));
    if (!o)
      throw SQL_ERROR();
    o->insertStatus(STR_TO_ID(db->GetFieldValue(i, 1)), MAKE_TIME(i, 2),
        ptime() );
  }
  db->FreeSelect();
}

void Register::ObjectListImpl::reload2(DBase::Connection* _conn) {
  DBase::SelectQuery states_query;
  states_query.select() << "tmp.id, t_1.object_id, t_1.state_id, t_1.valid_from";
  states_query.from() << getTempTableName() << " tmp "
                      << "JOIN object_registry t_2 ON (tmp.id = t_2.historyid) "
                      << "JOIN object_state t_1 ON (t_2.id = t_1.object_id)";
  states_query.where() << "AND t_1.valid_to IS NULL";
  states_query.order_by() << "t_1.object_id";

  resetIDSequence();
  try {
    std::auto_ptr<DBase::Result> r_states(_conn->exec(states_query));
    std::auto_ptr<DBase::ResultIterator> rit(r_states->getIterator());
    for (rit->first(); !rit->isDone(); rit->next()) {
      DBase::ID object_historyid = rit->getNextValue();
      DBase::ID object_id = rit->getNextValue();
      DBase::ID state_id = rit->getNextValue();
      DBase::DateTime valid_from = rit->getNextValue();
      
      ObjectImpl *object_ptr = dynamic_cast<ObjectImpl *>(findIDSequence(object_id));
      if (object_ptr)
        object_ptr->insertStatus(state_id, valid_from, ptime());
    }
  }
  catch (DBase::Exception& ex) {
    LOGGER("db").error(boost::format("%1%") % ex.what());
  }
  catch (std::exception& ex) {
    LOGGER("db").error(boost::format("%1%") % ex.what());
  }

}
