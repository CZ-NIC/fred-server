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
#include <algorithm>

#include "src/deprecated/libfred/object_impl.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/libfred/sql.hh"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     LibFred::StatusImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

LibFred::StatusImpl::StatusImpl(const TID& _id,
                                 const TID& _status_id,
                                 const ptime& _timeFrom, 
                                 const ptime& _timeTo, 
                                 const TID& _ohid_from,
                                 const TID& _ohid_to) : id(_id),
                                                        status_id(_status_id),
                                                        timeFrom(_timeFrom), 
                                                        timeTo(_timeTo),
                                                        ohid_from(_ohid_from),
                                                        ohid_to(_ohid_to) {
}

LibFred::StatusImpl::~StatusImpl() {
}

LibFred::TID LibFred::StatusImpl::getId() const {
  return id;
}

LibFred::TID LibFred::StatusImpl::getStatusId() const {
  return status_id;
}

ptime LibFred::StatusImpl::getFrom() const {
  return timeFrom;
}

ptime LibFred::StatusImpl::getTo() const {
  return timeTo;
}

LibFred::TID LibFred::StatusImpl::getHistoryIdFrom() const {
  return ohid_from;
}

LibFred::TID LibFred::StatusImpl::getHistoryIdTo() const {
  return ohid_to;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     LibFred::ObjectImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

LibFred::ObjectImpl::ObjectImpl(TID _id, const Database::ID& _history_id, ptime _crDate, ptime _trDate,
    ptime _upDate, ptime _erDate, TID _registrar,
    const std::string _registrarHandle, TID _createRegistrar,
    const std::string _createRegistrarHandle, TID _updateRegistrar,
    const std::string _updateRegistrarHandle, const std::string& _authPw,
    const std::string _roid) :
  CommonObjectImpl(_id), history_id(_history_id), crDate(_crDate), trDate(_trDate), upDate(_upDate),
      erDate(_erDate), registrar(_registrar),
      registrarHandle(_registrarHandle), createRegistrar(_createRegistrar),
      createRegistrarHandle(_createRegistrarHandle),
      updateRegistrar(_updateRegistrar),
      updateRegistrarHandle(_updateRegistrarHandle), authPw(_authPw),
      roid(_roid) {
}

LibFred::ObjectImpl::ObjectImpl() :
  CommonObjectImpl(0), crDate(not_a_date_time), trDate(not_a_date_time),
      upDate(not_a_date_time), registrar(0), createRegistrar(0),
      updateRegistrar(0) {
}

Database::ID LibFred::ObjectImpl::getHistoryId() const {
  return history_id;
}

Database::ID LibFred::ObjectImpl::getRequestId() const {
  return request_id;
}

Database::DateTime LibFred::ObjectImpl::getRequestStartTime() const {
  return request_start_time;
}

void LibFred::ObjectImpl::setRequestId(const Database::ID& _id, const Database::DateTime& _start_time) {
  request_id = _id;
  request_start_time = _start_time;
}

ptime LibFred::ObjectImpl::getCreateDate() const {
  return crDate;
}

ptime LibFred::ObjectImpl::getTransferDate() const {
  return trDate;
}

ptime LibFred::ObjectImpl::getUpdateDate() const {
  return upDate;
}

ptime LibFred::ObjectImpl::getDeleteDate() const {
  return erDate;
}

LibFred::TID LibFred::ObjectImpl::getRegistrarId() const {
  return registrar;
}

const std::string& LibFred::ObjectImpl::getRegistrarHandle() const {
  return registrarHandle;
}

LibFred::TID LibFred::ObjectImpl::getUpdateRegistrarId() const {
  return updateRegistrar;
}

const std::string& LibFred::ObjectImpl::getCreateRegistrarHandle() const {
  return createRegistrarHandle;
}

const std::string& LibFred::ObjectImpl::getUpdateRegistrarHandle() const {
  return updateRegistrarHandle;
}

LibFred::TID LibFred::ObjectImpl::getCreateRegistrarId() const {
  return createRegistrar;
}

const std::string& LibFred::ObjectImpl::getAuthPw() const {
  return authPw;
}

void LibFred::ObjectImpl::setAuthPw(const std::string& authPw) {
  this->authPw = authPw;
}

const std::string& LibFred::ObjectImpl::getROID() const {
  return roid;
}

unsigned LibFred::ObjectImpl::getStatusCount() const {
  return slist.size();
}

const LibFred::Status* LibFred::ObjectImpl::getStatusByIdx(unsigned idx) const {
  if (idx >= slist.size())
    return NULL;
  else
    return &slist[idx];
}

void LibFred::ObjectImpl::insertStatus(const StatusImpl& _state) {
  slist.push_back(_state);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     LibFred::ObjectListImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

LibFred::ObjectListImpl::ObjectListImpl(DBSharedPtr _db) :
  CommonListImpl(_db), registrarFilter(0), createRegistrarFilter(0),
      updateRegistrarFilter(0), crDateIntervalFilter(ptime(neg_infin),
          ptime(pos_infin)), updateIntervalFilter(ptime(neg_infin),
          ptime(pos_infin)), trDateIntervalFilter(ptime(neg_infin),
          ptime(pos_infin)), nonHandleFilterSet(false), ptr_history_idx_(-1) {
}

void LibFred::ObjectListImpl::setRegistrarFilter(TID registrarId) {
  registrarFilter = registrarId;
  nonHandleFilterSet = true;
}

void LibFred::ObjectListImpl::setRegistrarHandleFilter(
    const std::string& registrarHandle) {
  registrarHandleFilter = registrarHandle;
  nonHandleFilterSet = true;
}

void LibFred::ObjectListImpl::setCrDateIntervalFilter(time_period period) {
  crDateIntervalFilter = period;
  nonHandleFilterSet = true;
}

void LibFred::ObjectListImpl::setCreateRegistrarFilter(TID registrarId) {
  createRegistrarFilter = registrarId;
  nonHandleFilterSet = true;
}

void LibFred::ObjectListImpl::setCreateRegistrarHandleFilter(
    const std::string& registrarHandle) {
  createRegistrarHandleFilter = registrarHandle;
  nonHandleFilterSet = true;
}

void LibFred::ObjectListImpl::setUpdateIntervalFilter(time_period period) {
  updateIntervalFilter = period;
  nonHandleFilterSet = true;
}

void LibFred::ObjectListImpl::setUpdateRegistrarFilter(TID registrarId) {
  updateRegistrarFilter = registrarId;
  nonHandleFilterSet = true;
}

void LibFred::ObjectListImpl::setUpdateRegistrarHandleFilter(
    const std::string& registrarHandle) {
  updateRegistrarHandleFilter = registrarHandle;
  nonHandleFilterSet = true;
}

void LibFred::ObjectListImpl::setTrDateIntervalFilter(time_period period) {
  trDateIntervalFilter = period;
  nonHandleFilterSet = true;
}

void LibFred::ObjectListImpl::addStateFilter(TID state, bool stateIsOn) {
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

void LibFred::ObjectListImpl::clearStateFilter(TID state) {
  for (StatusFilterList::iterator i=sflist.begin(); i!=sflist.end(); i++)
    if (i->stateId == state) {
      sflist.erase(i);
      return;
    }
}

void LibFred::ObjectListImpl::clearFilter() {
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

void LibFred::ObjectListImpl::reload(const char *handle, int type)
  {
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
    o->insertStatus(StatusImpl(0 /* dummy state db id */, STR_TO_ID(db->GetFieldValue(i, 1)), MAKE_TIME(i, 2),
        ptime(), 0, 0 /* dummy history ids */));
  }
  db->FreeSelect();
}

void LibFred::ObjectListImpl::reload(bool _history) {
//  Database::SelectQuery states_query;
//  states_query.select() << "tmp.id, t_1.object_id, t_1.state_id, t_1.valid_from";
//  states_query.from() << getTempTableName() << " tmp "
//                      << "JOIN object_registry t_2 ON (tmp.id = t_2.historyid) "
//                      << "JOIN object_state t_1 ON (t_2.id = t_1.object_id)";
//  states_query.where() << "AND t_1.valid_to IS NULL";
//  states_query.order_by() << "t_1.object_id";

  Database::SelectQuery states_query;
  states_query.select() << "tmp.id, t_1.object_id, t_1.id, t_1.state_id, t_1.valid_from, t_1.valid_to, t_1.ohid_from, t_1.ohid_to";
  states_query.from() << getTempTableName() << " tmp "
                      << "JOIN object_history t_2 ON (tmp.id = t_2.historyid) "
                      << "JOIN object_state t_1 ON (t_2.historyid >= t_1.ohid_from AND (t_2.historyid <= t_1.ohid_to OR t_1.ohid_to IS NULL)) "
                      << "AND t_2.id = t_1.object_id";
  if (!_history) {
    states_query.where() << "AND t_1.ohid_to IS NULL";
  }
  states_query.order_by() << "tmp.id";

  Database::SelectQuery actions_query;
  actions_query.select() << "tmp.id, h.request_id, h.valid_from";
  actions_query.from() << getTempTableName() << " tmp "
                       << "JOIN history h ON (tmp.id = h.id)";
  actions_query.order_by() << "tmp.id";

  try {
    /* load object states */
    resetHistoryIDSequence();
    
    Database::Connection conn = Database::Manager::acquire();
    Database::Result r_states = conn.exec(states_query);
    for (Database::Result::Iterator it = r_states.begin(); it != r_states.end(); ++it) {
      Database::Row::Iterator col = (*it).begin();

      Database::ID       object_historyid = *col;
                                             (++col);//Database::ID       object_id
      Database::ID       state_db_id      = *(++col);
      Database::ID       state_id         = *(++col);
      Database::DateTime valid_from       = *(++col);
      Database::DateTime valid_to         = *(++col);
      Database::ID       ohid_from        = *(++col);
      Database::ID       ohid_to          = *(++col);

      ObjectImpl *object_ptr = dynamic_cast<ObjectImpl *>(findHistoryIDSequence(object_historyid));
      if (object_ptr)
        object_ptr->insertStatus(StatusImpl(state_db_id, state_id, valid_from, valid_to, ohid_from, ohid_to));
    }

    /* load object history actions */
    resetHistoryIDSequence();
    Database::Result r_actions = conn.exec(actions_query);
    for (Database::Result::Iterator it = r_actions.begin(); it != r_actions.end(); ++it) {
      Database::Row::Iterator col = (*it).begin();

      Database::ID       object_historyid = *col;
      Database::ID       request_id        = *(++col);
      Database::DateTime start_date       = *(++col);
      
      ObjectImpl *object_ptr = dynamic_cast<ObjectImpl *>(findHistoryIDSequence(object_historyid));
      if (object_ptr)
        object_ptr->setRequestId(request_id, start_date);
    }
  }
  catch (Database::Exception& ex) {
      std::string message = ex.what();
      if (message.find(Database::Connection::getTimeoutString())
              != std::string::npos) {
          LOGGER.info("Statement timeout in request list.");
          clear();
          throw;
      } else {
          LOGGER.error(boost::format("%1%") % ex.what());
          clear();
      }
  }
  catch (std::exception& ex) {
    LOGGER.error(boost::format("%1%") % ex.what());
  }
}

void LibFred::ObjectListImpl::resetHistoryIDSequence() {
  ptr_history_idx_ = -1;
}


LibFred::Object* LibFred::ObjectListImpl::findHistoryIDSequence(const Database::ID& _history_id) {
  /* data_ container must be sorted by ID to make sence */
  if (ptr_history_idx_ < 0)
    ptr_history_idx_ = 0;

  while (ptr_history_idx_ < (int)data_.size()) {
    LibFred::Object *ptr_object = dynamic_cast<LibFred::Object*>(data_[ptr_history_idx_]);
    if (ptr_object && ptr_object->getHistoryId() < _history_id) {
      ++ptr_history_idx_;
    }
    else {
      break;
    }
  }

  LibFred::Object *ptr_object = dynamic_cast<LibFred::Object*>(data_[ptr_history_idx_]);
  if (ptr_history_idx_ == (int)data_.size() || (ptr_object && (ptr_object->getHistoryId() != _history_id))) {
    LOGGER.debug(boost::format("find history id sequence: not found in result set. (historyid=%1%, ptr_idx=%2%)")
                                        % _history_id % ptr_idx_);
    resetHistoryIDSequence();
    return 0;
  }
  else {
    return ptr_object;
  }
}


void LibFred::ObjectListImpl::deleteDuplicatesId() {
  TRACE("<CALL> LibFred::ObjectListImpl::deleteDuplicatesId()");

  std::stable_sort(data_.begin(), data_.end(), SortByHistoryId());
  LOGGER.debug(boost::format("%1% record(s) in result sorted") % data_.size());
  list_type::iterator it = data_.begin();
  while (data_.begin() != data_.end() && it < data_.end() - 1) {
    if ((*it)->getId() == (*(it + 1))->getId()) {
      delete *it;
      it = data_.erase(it);
    }
    else {
      ++it;
    }
  }
  LOGGER.debug(boost::format("%1% record(s) in result after removing duplicates") % data_.size());
}
