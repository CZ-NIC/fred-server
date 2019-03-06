/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#include <utility>

#include "config.h"

// FRED logging
#include "util/log/logger.hh"
#include "util/log/context.hh"

#include "src/deprecated/libfred/requests/request_impl.hh"
#include "src/deprecated/libfred/requests/request_list.hh"
#include "src/deprecated/libfred/requests/request_manager.hh"

// for PartitioningTweak
#include "src/deprecated/model/log_filter.hh"
#include "libfred/db_settings.hh"


using namespace Database::Filters;
using namespace Database;

namespace LibFred {
namespace Logger {


// TODO:
// * central configuration which types have to be treated like this
// * separate tree browsing & functor(visitor) parts

class CustomPartitioningTweak {
public:
    CustomPartitioningTweak()
        : time_begin(NULL), service(NULL), is_monitoring(NULL) {
    }

private:
  void find_values_recurs (Filter *f) {
    // This check can be moved to the higher level
    if(!f->isActive()) return;

    Compound *c = dynamic_cast<Compound*>(f);
    if (c == NULL) {
            if(f->getName() == "TimeBegin") {
                if(time_begin != NULL) {
                    LOGGER.error("Duplicity TimeBegin found in filters.");
                    return;
                }

                time_begin = dynamic_cast< Interval<Database::DateTimeInterval>* >(f);
                if(time_begin == NULL) {
                    LOGGER.error("TimeBegin: Inconsistency in filters.");
                }

            } else if(f->getName() == "ServiceType") {
                if(service != NULL) {
                    LOGGER.error("Duplicity ServiceType found in filters.");
                    return;
                }

                service = dynamic_cast<Database::Filters::ServiceType*>(f);
                if(service == NULL) {
                    LOGGER.error("ServiceType: Inconsistency in filters.");
                }

            } else if(f->getName() == "IsMonitoring") {
                if(is_monitoring != NULL) {
                    LOGGER.error("Duplicity IsMonitoring found in filters.");
                    return;
                }

                is_monitoring = dynamic_cast< Database::Filters::Value<bool>* >(f);
                if(is_monitoring == NULL) {
                    LOGGER.error("IsMonitoring: Inconsistency in filters.");
                }
            }
    }  else  {
        std::vector<Filter*>::iterator it = c->begin();

        for(; it != c->end(); it++) {
            find_values_recurs(*it);
        }
    }
  }

  void add_conds_recurs(Compound *c) {

      std::vector<Filter*>::iterator it = c->begin();
      for(; it != c->end(); it++) {
          Compound *child = dynamic_cast<Compound*> (*it);
          if(child != NULL) {
              add_conds_recurs(child);
          }
      }

      RequestDataImpl *request_data;
      RequestPropertyValueImpl *request_property_value;
      RequestObjectRefImpl *request_object_ref;

      Table *tbl = NULL;

      if((request_data = dynamic_cast<RequestDataImpl*>(c)) != NULL) {
          tbl = &request_data->joinRequestDataTable();
      } else if((request_property_value = dynamic_cast<RequestPropertyValueImpl*>(c)) != NULL) {
          tbl = &request_property_value->joinRequestPropertyValueTable();
      } else if((request_object_ref = dynamic_cast<RequestObjectRefImpl*>(c)) != NULL) {
          tbl = &request_object_ref->joinRequestObjectRefTable();
      }

      if(tbl == NULL) {
          // this table doesn't need additional partitioning condition in where
          return;
      }

      if(time_begin != NULL) {
          Interval<Database::DateTimeInterval> *copy_time_begin
              = new Interval<Database::DateTimeInterval>(*time_begin);
          copy_time_begin->setColumn(Column("request_time_begin", *tbl));
          c->add(copy_time_begin);
      }

      if(service != NULL) {
          Database::Filters::ServiceType *copy_service
              = new Database::Filters::ServiceType(*service);
          copy_service->setColumn(Column("request_service_id", *tbl));
          c->add(copy_service);
      }

      if(is_monitoring != NULL) {
          Database::Filters::Value<bool> *copy_monitoring
              = new Database::Filters::Value<bool>(*is_monitoring);
          copy_monitoring->setColumn(Column("request_monitoring", *tbl));
          c->add(copy_monitoring);
      }

  }

public:

  /// this is expected to be called only once on each instance
  void process_filters(Filter *f) {
        time_begin = NULL;
        service  = NULL;
        is_monitoring = NULL;

        Database::Filters::RequestImpl *ri = dynamic_cast<Database::Filters::RequestImpl*> (f);
        if(ri != NULL) {
            find_values_recurs (ri);

            if(time_begin     != NULL ||
              service         != NULL ||
              is_monitoring   != NULL) {
              add_conds_recurs(ri);
            }
        }
  }

  /// return value or empty filter - isActive()==false
  Interval<Database::DateTimeInterval> getTimeBegin() {
      if(time_begin != NULL) {
          return *time_begin;
      } else {
          // empty filter with isActive() == false
          return Interval<Database::DateTimeInterval>();
      }
  }
  Database::Filters::ServiceType getServiceType() {
      if(service != NULL) {
          return *service;
      } else {
          return Database::Filters::ServiceType();
      }
  }
  Database::Filters::Value<bool> getIsMonitoring() {
      if(is_monitoring != NULL) {
          return *is_monitoring;
      } else {
          return Database::Filters::Value<bool>();
      }
  }

private:
  const Interval<Database::DateTimeInterval> * time_begin;
  const Database::Filters::ServiceType * service;
  const Database::Filters::Value<bool> * is_monitoring;

};


bool IdFilterActive(const Database::Filters::Compound* f)
{

  const Database::Filters::RequestImpl *ri = dynamic_cast<const Database::Filters::RequestImpl*> (f);

  if(ri == NULL) {
      return false;
  }

  std::vector<Filter*>::const_iterator it = ri->begin();
  for(; it != ri->end(); it++) {
    if((*it)->getName() == "Id") {
        return true;
    }
  }
  return false;
}

Database::Result oneRecordQuery(Database::Connection &conn, Database::SelectQuery &id_query, bool partialLoad)
{
    Result res = conn.exec(id_query);
    Database::SelectQuery query;

    std::string q_id;
    std::string q_time_begin;
    std::string q_service_id;
    std::string q_monitoring;

    // we didn't use DISTINCT, let's not make
    // any assumptions about number of records in return
    if (res.size() > 0) {
        q_id         =  (std::string)res[0][0];
        q_time_begin =  (std::string)res[0][1];
        q_service_id =  (std::string)res[0][2];
        q_monitoring =  (std::string)res[0][3];
    } else {
        // no records found , return some empty result
        return res;
    }

    // these are mandatory
    if(q_id.empty() || q_time_begin.empty()) {
        return res;
    }

    if (partialLoad) {
        query.select()
                << "t_1.id, t_1.time_begin, t_1.time_end, t_3.name, "
                << "t_1.source_ip, t_2.name, t_1.session_id, "
                << "t_1.user_name, t_1.is_monitoring, "
                << "t_4.result_code, t_4.name";
        query.from() << "request t_1 "
                << "join request_type t_2 on t_2.id = t_1.request_type_id "
                << "join service t_3 on t_3.id = t_1.service_id "
                << "left join result_code t_4 on t_4.id = t_1.result_code_id";

        query.where() << "and t_1.id=" << q_id
            << " and t_1.time_begin='" << q_time_begin << "'";

        if(!q_service_id.empty()) {
            query.where() << " and t_1.service_id=" << q_service_id;
        }
        if(!q_monitoring.empty()) {
            query.where() << " and t_1.is_monitoring='" << q_monitoring << "'";
        }

        query.order_by() << "t_1.time_begin desc";
    } else {
        // hardcore optimizations have to be done on this statement
        query.select()
            << "t_1.id, t_1.time_begin, t_1.time_end, t_3.name, t_1.service_id, "
            << "t_1.source_ip, t_2.name, t_1.session_id, "
            << "t_1.user_name, t_1.user_id, t_1.is_monitoring, "
            << "t_4.result_code, t_4.name, "
            << "(select content from request_data where "
            << "request_id=" << q_id
            << " and request_time_begin='" << q_time_begin << "'";

        if(!q_service_id.empty()) {
            query.select() << " and request_service_id=" << q_service_id;
        }
        if(!q_monitoring.empty()) {
            query.select() << " and request_monitoring='" << q_monitoring << "'";
        }
        query.select()
            << " and is_response=false limit 1) as request, "
            << "(select content from request_data where "
            << "request_id=" << q_id
            << " and request_time_begin='" << q_time_begin << "'"
            << " and request_service_id=" << q_service_id
            << " and request_monitoring='" << q_monitoring << "'"
            << " and is_response=true  limit 1) as response ";
        query.from() << "request t_1 "
            << "join request_type t_2 on t_2.id=t_1.request_type_id "
            << "join service t_3 on t_3.id=t_1.service_id "
            << "left join result_code t_4 on t_4.id=t_1.result_code_id";
        // add conditions to improve join with partitioned table

        query.where() << " and t_1.id=" << q_id
                    << " and t_1.time_begin='" << q_time_begin << "'";

        if (!q_service_id.empty()) {
            query.where() << " and t_1.service_id="
                    << q_service_id;
        }
        if (!q_monitoring.empty()) {
            query.where() << " and t_1.is_monitoring='" << q_monitoring << "'";
        }

        query.order_by() << "t_1.time_begin desc";
    }
    return conn.exec(query);
}

template <typename T>
std::string generateCondition(T & filter,
        const std::string &name, Table &table)
{
    std::string where_cond;

    if (filter.isActive()) {
        filter.setColumn(Column(name, table));
        SelectQuery sql;
        filter.serialize(sql, NULL);
        sql.make();
        where_cond = sql.where().str();
        LOGGER.info(boost::format(
                    "DEBUG: Generated filer for %1%: %2%")
                    % name % where_cond);
    } else {
        LOGGER.info(
                boost::format("DEBUG:  Filter for %1% not active (isActive()=false)")
                    % name);
    }
    return where_cond;
}


class RequestImpl;

COMPARE_CLASS_IMPL(RequestImpl, TimeBegin)
COMPARE_CLASS_IMPL(RequestImpl, TimeEnd)
COMPARE_CLASS_IMPL(RequestImpl, SourceIp)
COMPARE_CLASS_IMPL(RequestImpl, ServiceType)
COMPARE_CLASS_IMPL(RequestImpl, ActionType)
COMPARE_CLASS_IMPL(RequestImpl, SessionId)
COMPARE_CLASS_IMPL(RequestImpl, UserName)
COMPARE_CLASS_IMPL(RequestImpl, IsMonitoring)
COMPARE_CLASS_IMPL(RequestImpl, ResultCodeName)
COMPARE_CLASS_IMPL(RequestImpl, RawRequest)
COMPARE_CLASS_IMPL(RequestImpl, RawResponse)


class ListImpl : public LibFred::CommonListImpl,
                 virtual public List {
private:
  bool partialLoad;

public:
  ListImpl() : CommonListImpl(), partialLoad(false) {
  }

  virtual Request* get(unsigned _idx) const {
    try {
      Request *request = dynamic_cast<Request*>(data_.at(_idx));
      if (request)
        return request;
      else
        throw std::exception();
    }
    catch (...) {
      throw std::exception();
    }
  }

  virtual void setPartialLoad(bool _partialLoad) {
    partialLoad = _partialLoad;
  }

  virtual void reload(Database::Filters::Union& _filter) {
    TRACE("[CALL] LibFred::Logger::ListImpl::reload()");
    clear();
    _filter.clearQueries();

    CustomPartitioningTweak pt;

    bool one_record_optimization = false;
    // iterate through all the filters
    bool at_least_one = false;
    Compound::iterator fit = _filter.begin();
    for (; fit != _filter.end(); ++fit) {
      Database::Filters::Request *mf = dynamic_cast<Database::Filters::Request* >(*fit);
      if (!mf)
        continue;

      pt.process_filters(mf);

      Database::SelectQuery *tmp = new Database::SelectQuery();

      if(IdFilterActive(mf)) {
          one_record_optimization = true;
          tmp->addSelect(new Database::Column("id", mf->joinRequestTable()));
          tmp->addSelect(new Database::Column("time_begin", mf->joinRequestTable()));
          tmp->addSelect(new Database::Column("service_id", mf->joinRequestTable()));
          tmp->addSelect(new Database::Column("is_monitoring", mf->joinRequestTable()));
      } else {
          one_record_optimization = false;
          tmp->addSelect(new Database::Column("id",
                mf->joinRequestTable(), "DISTINCT"));
      }
      _filter.addQuery(tmp);

      at_least_one = true;
    }
    if (!at_least_one) {
      LOGGER.error("wrong filter passed for reload!");
      return;
    }

    Database::SelectQuery id_query;
    // the actual query for data
    Database::SelectQuery query;

    Connection conn = Database::Manager::acquire();

    try {
        // TODO this should be part of connection
        conn.exec("set constraint_exclusion=ON");
        Result res;

        if (one_record_optimization) {
            _filter.serialize(id_query);

            res = oneRecordQuery(conn, id_query, partialLoad);

        } else {
            // make an id query according to the filters
            id_query.order_by() << "id DESC";
            id_query.offset(load_offset_);
            id_query.limit(load_limit_);
            _filter.serialize(id_query);

            Database::InsertQuery tmp_table_query = Database::InsertQuery(
                    getTempTableName(), id_query);
            LOGGER.debug(boost::format(
                    "temporary table '%1%' generated sql = %2%")
                    % getTempTableName() % tmp_table_query.str());

            // table which will be used in embedded SQL statement (t_1 has to match)
            Table trequest("request", "t_1");

            // create some additional partitioning conditions
            Interval<Database::DateTimeInterval> time_begin =
                    pt.getTimeBegin();
            Database::Filters::ServiceType service_type =
                    pt.getServiceType();
            Database::Filters::Value<bool> is_monitoring =
                    pt.getIsMonitoring();

            std::string where_time_begin
                = generateCondition<Interval<Database::DateTimeInterval> >(time_begin, "time_begin", trequest);
            std::string where_service_type
                = generateCondition<Database::Filters::ServiceType>(service_type, "service_id", trequest);
            std::string where_is_monitoring
                = generateCondition<Database::Filters::Value<bool> >(is_monitoring, "is_monitoring", trequest);

            if (partialLoad) {
                query.select()
                        << "tmp.id, t_1.time_begin, t_1.time_end, t_3.name, "
                        << "t_1.source_ip, t_2.name, t_1.session_id, "
                        << "t_1.user_name, t_1.is_monitoring, "
                        << "t_4.result_code, t_4.name";
                query.from() << getTempTableName()
                        << " tmp join request t_1 on tmp.id = t_1.id "
                        << "join request_type t_2 on t_2.id = t_1.request_type_id "
                        << "join service t_3 on t_3.id = t_1.service_id "
                        << "left join result_code t_4 on t_4.id = t_1.result_code_id";

            } else {
                // hardcore optimizations have to be done on this statement
                query.select()
                        << "tmp.id, t_1.time_begin, t_1.time_end, t_3.name, t_1.service_id, "
                        << "t_1.source_ip, t_2.name, t_1.session_id, "
                        << "t_1.user_name, t_1.user_id, t_1.is_monitoring, "
                        << "t_4.result_code, t_4.name, "
                        << "(select content from request_data where request_time_begin=t_1.time_begin and request_id=tmp.id and is_response=false limit 1) as request, "
                        << "(select content from request_data where request_time_begin=t_1.time_begin and request_id=tmp.id and is_response=true  limit 1) as response ";
                query.from() << getTempTableName()
                        << " tmp join request t_1 on tmp.id=t_1.id "
                        << "join request_type t_2 on t_2.id=t_1.request_type_id "
                        << "join service t_3 on t_3.id=t_1.service_id "
                        << "left join result_code t_4 on t_4.id = t_1.result_code_id";

            }
            // add conditions to improve join with partitioned table
            if (time_begin.isActive()) {
                query.where() << where_time_begin;
            }
            if (service_type.isActive()) {
                query.where() << where_service_type;
            }
            if (is_monitoring.isActive()) {
                query.where() << where_is_monitoring;
            }

            query.order_by() << "t_1.time_begin desc";

            // run all the queries
            Database::Query create_tmp_table("SELECT create_tmp_table('"
                    + std::string(getTempTableName()) + "')");
            conn.exec(create_tmp_table);
            conn.exec(tmp_table_query);

            res = conn.exec(query);
        }

        for (Result::Iterator it = res.begin(); it != res.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            ID id = *col;
            DateTime time_begin = *(++col);
            DateTime time_end = *(++col);
            std::string serv_type = *(++col);
            int serv_id = 0;
            if (!partialLoad) {
                serv_id = *(++col);
            }
            std::string source_ip = *(++col);
            std::string request_type_id = *(++col);
            ID session_id = *(++col);
            std::string user_name = *(++col);

            ID user_id = 0;
            if (!partialLoad) {
                user_id = *(++col);
            }
            bool is_monitoring = *(++col);
            int rc_code = *(++col);
            std::string rc_name = *(++col);

            // fields dependent on partialLoad
            std::string request;
            std::string response;

            std::unique_ptr<RequestPropertiesDetail> props;
            std::unique_ptr<ObjectReferences> refs;

            if (!partialLoad) {
                request = (std::string) *(++col);
                response = (std::string) *(++col);
                props = getPropsForId(id, time_begin, serv_id,
                        is_monitoring);
                refs = getObjectRefsForId(id, time_begin, serv_id,
                        is_monitoring);
            }

            data_.push_back(new RequestImpl(id, time_begin, time_end,
                    serv_type, source_ip, request_type_id, session_id,
                    user_name, user_id, is_monitoring, request, response,
                    std::move(props), std::move(refs), rc_code, rc_name));
        }

        if (data_.empty()) {
            return;
        }
        /* checks if row number result load limit is active and set flag */
        CommonListImpl::reload();
    } catch (Database::Exception& ex) {
        std::string message = ex.what();
        if (message.find(conn.getTimeoutString()) != std::string::npos) {
            LOGGER.info("Statement timeout in request list.");
            clear();
            throw;
        } else {
            LOGGER.error(boost::format("%1%") % ex.what());
            clear();
        }
    } catch (std::exception& ex) {
        LOGGER.error(boost::format("%1%") % ex.what());
        clear();
    }
  }

  virtual void sort(MemberType _member, bool _asc) {

      switch(_member) {
          case  MT_TIME_BEGIN:
              stable_sort(data_.begin(), data_.end(), CompareTimeBegin(_asc));
              break;
          case MT_TIME_END:
              stable_sort(data_.begin(), data_.end(), CompareTimeEnd(_asc));
              break;
          case MT_SOURCE_IP:
              stable_sort(data_.begin(), data_.end(), CompareSourceIp(_asc));
              break;
          case MT_SERVICE:
              stable_sort(data_.begin(), data_.end(), CompareServiceType(_asc));
              break;
          case MT_ACTION:
              stable_sort(data_.begin(), data_.end(), CompareActionType(_asc));
              break;
          case MT_SESSION_ID:
              stable_sort(data_.begin(), data_.end(), CompareSessionId(_asc));
              break;
          case MT_USER_NAME:
              stable_sort(data_.begin(), data_.end(), CompareUserName(_asc));
              break;
          case MT_MONITORING:
              stable_sort(data_.begin(), data_.end(), CompareIsMonitoring(_asc));
              break;
          case MT_RESULT_CODE:
              stable_sort(data_.begin(), data_.end(), CompareResultCodeName(_asc));
              break;
          default:
              break;
      }

  }

  virtual std::unique_ptr<RequestPropertiesDetail> getPropsForId(ID id, DateTime time_begin, int sid, bool mon) {
    std::unique_ptr<RequestPropertiesDetail> ret(new RequestPropertiesDetail());
    Database::SelectQuery query;

    query.select() << "t_2.name, t_1.value, t_1.output, (t_1.parent_id is not null)";
    query.from()   << "request_property_value t_1 join request_property_name t_2 on t_1.property_name_id=t_2.id";
    query.where()  << (boost::format("and t_1.request_id = '%1%' and t_1.request_time_begin = '%2%' and t_1.request_service_id = %3% and t_1.request_monitoring = %4%") % id % time_begin % sid % (mon ? "true" : "false")).str();
        query.order_by() << "t_1.id";

        Database::Connection conn = Database::Manager::acquire();
    Result res = conn.exec(query);

        for(Result::Iterator it = res.begin(); it != res.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        std::string         name   = *col;
        std::string         value  = *(++col);
        bool            output = *(++col);
        bool            is_child = *(++col);

        ret->push_back(RequestPropertyDetail(name, value, output, is_child));

    }
    return ret;
  }

  virtual std::unique_ptr<ObjectReferences> getObjectRefsForId(ID id, DateTime time_begin, int sid, bool mon) {
        std::unique_ptr<ObjectReferences> ret(new ObjectReferences());

        Connection conn = Database::Manager::acquire();

        Result res = conn.exec((boost::format
                ("SELECT name, object_id FROM request_object_ref oref "
                "JOIN request_object_type ot ON ot.id = oref.object_type_id "
                "WHERE oref.request_id = %1% and oref.request_time_begin = '%2%' and oref.request_service_id = %3% and oref.request_monitoring = %4%") % id % time_begin % sid % (mon ? "true" : "false")).str());

        for(unsigned i=0;i<res.size();i++) {
                ObjectReference oref;
                oref.type = (std::string)res[i][0];
                oref.id = res[i][1];
                ret->push_back(oref);
        }

        return ret;
  }

  /// from CommonList; propably will be removed in future
  virtual const char* getTempTableName() const {
    return "tmp_request_filter_result";
  }

  virtual void makeQuery(bool, bool, std::stringstream&) const {
      LOGGER.error("Unimplemented method called: Logger::ListImpl::makeQuery()");
  }

  virtual void reload() {
     TRACE("[CALL] LibFred::Logger::ListImpl::reload()");
        clear();

    // here we go
        Database::SelectQuery query;

     if(partialLoad) {
            query.select() << "t_1.time_begin, t_1.time_end, t_3.name, t_1.source_ip, t_1.request_type_id, t_1.session_id, t_1.is_monitoring";
            query.from() << " tmp JOIN request t_1"
                                 << " join service t_3 on t_3.id=t_1.service_id ";
            query.order_by() << "t_1.time_begin desc";
        } else {
               query.select() << "t_1.time_begin, t_1.time_end, t_3.name, t_1.service_id, t_1.source_ip, t_1.user_name, t_1.request_type_id, t_1.session_id, t_1.is_monitoring, "
                                    " (select content from request_data where request_time_begin=t_1.time_begin and request_id=t_1.id and is_response=false limit 1) as request, "
                                    " (select content from request_data where request_time_begin=t_1.time_begin and request_id=t_1.id and is_response=true  limit 1) as response ";
                    query.from() << getTempTableName() << " tmp JOIN request t_1 "
                                         << "join service t_3 on t_3.id=t_1.service_id ";
                    query.order_by() << "t_1.time_begin desc";

        }

        Connection conn = Database::Manager::acquire();
        // TODO this should be part of connection
        conn.exec("set constraint_exclusion=ON");

        try {
            Result res = conn.exec(query);

            for(Result::Iterator it = res.begin(); it != res.end(); ++it) {
                Database::Row::Iterator col = (*it).begin();

                ID      id      = *col;
                DateTime    time_begin      = *(++col);
                DateTime    time_end    = *(++col);
                std::string             serv_type  = *(++col);
                                int serv_id;
                                if(!partialLoad) {
                                                        serv_id         = *(++col);
                                }
                std::string         source_ip   = *(++col);
                                std::string             user_name       = *(++col);

                                ID              user_id;
                                if(!partialLoad) {
                                                        user_id = *(++col);
                                }
                std::string         request_type_id = *(++col);
                ID              session_id  = *(++col);
                bool            is_monitoring   = *(++col);
                std::string     request;
                std::string     response;

                std::unique_ptr<RequestPropertiesDetail> props;
                                std::unique_ptr<ObjectReferences> refs;

                if(!partialLoad) {
                    request     = (std::string)*(++col);
                    response    = (std::string)*(++col);
                    props       = getPropsForId(id, time_begin, serv_id, is_monitoring);
                                        refs            = getObjectRefsForId(id, time_begin, serv_id, is_monitoring);
                }

                data_.push_back(new RequestImpl(id,
                            time_begin,
                            time_end,
                            serv_type,
                            source_ip,
                            request_type_id,
                            session_id,
                                                        user_name,
                                                        user_id,
                            is_monitoring,
                            request,
                            response,
                            std::move(props),
                                                        std::move(refs)));
            }

            if(data_.empty()) {
                return;
            }
            /* checks if row number result load limit is active and set flag */
            CommonListImpl::reload();
        }
        catch (Database::Exception& ex) {
            std::string message = ex.what();
            if (message.find(conn.getTimeoutString())
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
          clear();
        }
  }

};

// TODO where to place this?
List *ManagerImpl::createList() const {
    return new ListImpl();
}

}; // namespace Logger
};
