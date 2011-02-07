/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

// FRED logging
#include "log/logger.h"
#include "log/context.h"

#include "request_impl.h"
#include "request_list.h"
#include "request_manager.h"

// for PartitioningTweak
#include "model/log_filter.h"
#include "db_settings.h"


using namespace Database::Filters;
using namespace Database;

namespace Fred {
namespace Logger {


// TODO:
// * central configuration which types have to be treated like this
// * separate tree browsing & functor(visitor) parts

class CustomPartitioningTweak {
private:
  static void find_values_recurs (Filter *f) {
    // This check can be moved to the higher level
    if(!f->isActive()) return;

    Compound *c = dynamic_cast<Compound*>(f);
    if (c == NULL) {
            if(f->getName() == "TimeBegin") {
                if(time_begin != NULL) {
                    LOGGER(PACKAGE).error("Duplicity TimeBegin found in filters.");
                    return;
                }
                time_begin = dynamic_cast< Interval<Database::DateTimeInterval>* >(f);
                if(time_begin == NULL) {
                    LOGGER(PACKAGE).error("TimeBegin: Inconsistency in filters.");
                    std::cout << "TimeBegin: Inconsistency in filters." << std::endl;
                }

            } else if(f->getName() == "ServiceType") {
                if(service != NULL) {
                    LOGGER(PACKAGE).error("Duplicity ServiceType found in filters.");
                    return;
                }

                service = dynamic_cast<Database::Filters::ServiceType*>(f);
                if(service == NULL) {
                    LOGGER(PACKAGE).error("ServiceType: Inconsistency in filters.");
                }

            } else if(f->getName() == "IsMonitoring") {
                if(is_monitoring != NULL) {
                    LOGGER(PACKAGE).error("Duplicity IsMonitoring found in filters.");
                    return;
                }

                is_monitoring = dynamic_cast< Database::Filters::Value<bool>* >(f);
                if(is_monitoring == NULL) {
                    LOGGER(PACKAGE).error("IsMonitoring: Inconsistency in filters.");
                }
            }
    }  else  {
        std::vector<Filter*>::iterator it = c->begin();

        for(; it != c->end(); it++) {
            find_values_recurs(*it);
        }
    }
  }

  static void add_conds_recurs(Compound *c) {

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

      if(tbl != NULL) {

          if(time_begin != NULL) {
              /*
              Interval<Database::DateTimeInterval> copy_time_begin(time_begin);
              copy_time_begin.setColumn(Column("request_time_begin", *tbl));
              c->add(&copy_time_begin);
              */

              Interval<Database::DateTimeInterval> *copy_time_begin = new Interval<Database::DateTimeInterval>(Column("request_time_begin", *tbl));
              copy_time_begin->setName("RequestTimeBegin");
              copy_time_begin->setValue(time_begin->getValue());
              // TODO  - this should be done via c-tors
              copy_time_begin->setNOT(time_begin->getNOT());
              // copy_time_begin->setConjuction(time_begin->getConjuction());
              // we never run into non-active record, no need to handle this field
              c->add(copy_time_begin);
          }

          if(service != NULL) {
              Database::Filters::ServiceType *copy_service = new Database::Filters::ServiceType(Column("request_service_id", *tbl));
              copy_service->setName("RequestService");
              copy_service->setValue(service->getValue());
              copy_service->setNOT(service->getNOT());
              // copy_service->setConjuction(service->getConjuction());
              // we never run into non-active record, no need to handle this field

              c->add(copy_service);
          }

          if(is_monitoring != NULL) {
              Database::Filters::Value<bool> *copy_monitoring = new Database::Filters::Value<bool>(Column("request_monitoring", *tbl));
              copy_monitoring->setName("RequestIsMonitoring");
              copy_monitoring->setValue(is_monitoring->getValue());
              copy_monitoring->setNOT(is_monitoring->getNOT());
              // copy_monitoring->setConjuction(is_monitoring->getConjuction());
              // we never run into non-active record, no need to handle this field

              c->add(copy_monitoring);
          }
      }
  }

public:

  static void process_filters(Filter *f) {
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
          return *service ;
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
  Interval<Database::DateTimeInterval> static * time_begin;
  Database::Filters::ServiceType static * service;
  Database::Filters::Value<bool> static * is_monitoring;

};

Interval<Database::DateTimeInterval> * CustomPartitioningTweak::time_begin = NULL;
Database::Filters::ServiceType * CustomPartitioningTweak::service = NULL;
Database::Filters::Value<bool> * CustomPartitioningTweak::is_monitoring = NULL;





class RequestImpl;

COMPARE_CLASS_IMPL(RequestImpl, TimeBegin)
COMPARE_CLASS_IMPL(RequestImpl, TimeEnd)
COMPARE_CLASS_IMPL(RequestImpl, SourceIp)
COMPARE_CLASS_IMPL(RequestImpl, ServiceType)
COMPARE_CLASS_IMPL(RequestImpl, ActionType)
COMPARE_CLASS_IMPL(RequestImpl, SessionId)
COMPARE_CLASS_IMPL(RequestImpl, UserName)
COMPARE_CLASS_IMPL(RequestImpl, IsMonitoring)
COMPARE_CLASS_IMPL(RequestImpl, RawRequest)
COMPARE_CLASS_IMPL(RequestImpl, RawResponse)


class ListImpl : public Fred::CommonListImpl,
                 virtual public List {
private:
  Manager *manager_;
  bool partialLoad;

public:
  ListImpl(Manager *_manager) : CommonListImpl(), manager_(_manager), partialLoad(false) {
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
    TRACE("[CALL] Fred::Logger::ListImpl::reload()");
    clear();
    _filter.clearQueries();

    CustomPartitioningTweak pt;

    // iterate through all the filters
    bool at_least_one = false;
    Database::SelectQuery id_query;
    Compound::iterator fit = _filter.begin();
    for (; fit != _filter.end(); ++fit) {
      Database::Filters::Request *mf = dynamic_cast<Database::Filters::Request* >(*fit);
      if (!mf)
        continue;

      pt.process_filters(mf);

      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect(new Database::Column("id", mf->joinRequestTable(), "DISTINCT"));
      _filter.addQuery(tmp);
      at_least_one = true;
    }
    if (!at_least_one) {
      LOGGER(PACKAGE).error("wrong filter passed for reload!");
      return;
    }

    // make an id query according to the filters
    id_query.order_by() << "id DESC";
    id_query.offset(load_offset_);
    id_query.limit(load_limit_);
    _filter.serialize(id_query);

    Database::InsertQuery tmp_table_query = Database::InsertQuery(getTempTableName(), id_query);
    LOGGER(PACKAGE).debug(boost::format("temporary table '%1%' generated sql = %2%") % getTempTableName() % tmp_table_query.str());

    // make the actual query for data
    Database::SelectQuery query;


    // add conditions for JOIN with partitioned table
    SelectQuery sql_time_begin, sql_service_type, sql_is_monitoring;
    // table which will be used in embedded SQL statement (t_1 has to match)
    Table trequest("request", "t_1");

    Interval<Database::DateTimeInterval> time_begin = pt.getTimeBegin();
    Database::Filters::ServiceType service_type = pt.getServiceType();
    Database::Filters::Value<bool> is_monitoring = pt.getIsMonitoring();

    if(time_begin.isActive()) {
        time_begin.setColumn(Column("time_begin", trequest));
        time_begin.serialize(sql_time_begin, NULL);
        sql_time_begin.make();
        LOGGER(PACKAGE).info(boost::format("DEBUG: Generated filer for time_begin: %1%") % sql_time_begin.where().str());
    } else {
        LOGGER(PACKAGE).info("DEBUG:  Filter for time_begin not active (isActive()=false)");
    }

    if(service_type.isActive()) {
        service_type.setColumn(Column("service_id", trequest));
        service_type.serialize(sql_service_type, (const Settings*)NULL);

        sql_service_type.make();
        LOGGER(PACKAGE).info(boost::format("DEBUG: Generated filer for service_type: %1%") % sql_service_type.where().str());
    } else {
        LOGGER(PACKAGE).info("DEBUG:  Filter for service_type not active (isActive()=false)");
    }

    if(is_monitoring.isActive()) {
        is_monitoring.setColumn(Column("is_monitoring", trequest));
        is_monitoring.serialize(sql_is_monitoring, NULL);
        sql_is_monitoring.make();
        LOGGER(PACKAGE).info(boost::format("DEBUG: Generated filer for is_monitoring: %1%") % sql_is_monitoring.where().str());
    } else {
        LOGGER(PACKAGE).info("DEBUG: Filter for is_monitoring not active (isActive()=false)");
    }
    // additional partitioning conditions complete

    if(partialLoad) {
        query.select() << "tmp.id, t_1.time_begin, t_1.time_end, t_3.name, "
                       << "t_1.source_ip, t_2.name, t_1.session_id, "
                       << "t_1.user_name, t_1.is_monitoring, "
                       << "t_4.result_code, t_4.name";
        query.from() << getTempTableName() << " tmp join request t_1 on tmp.id = t_1.id "
                     << "join request_type t_2 on t_2.id = t_1.request_type_id "
                     << "join service t_3 on t_3.id = t_1.service_id "
                     << "left join result_code t_4 on t_4.id = t_1.result_code_id";

        // add conditions to improve join with partitioned table
        if(time_begin.isActive()) {
            query.where() << sql_time_begin.where().str();
        }
        if(service_type.isActive()) {
            query.where() << sql_service_type.where().str();
        }
        if(is_monitoring.isActive()) {
            query.where() << sql_is_monitoring.where().str();
        }

        query.order_by() << "t_1.time_begin desc";
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
            // add conditions to improve join with partitioned table
            if (time_begin.isActive()) {
                query.where() << sql_time_begin.where().str();
            }
            if (service_type.isActive()) {
                query.where() << sql_service_type.where().str();
            }
            if (is_monitoring.isActive()) {
                query.where() << sql_is_monitoring.where().str();
            }

            query.order_by() << "t_1.time_begin desc";
        }

    // TODO this should be part of connection
    Connection conn = Database::Manager::acquire();
    conn.exec("set constraint_exclusion=ON");

    try {

        // run all the queries
        Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
        conn.exec(create_tmp_table);
        conn.exec(tmp_table_query);

        Result res = conn.exec(query);
        for (Result::Iterator it = res.begin(); it != res.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            ID id                 = *col;
            DateTime time_begin   = *(++col);
            DateTime time_end     = *(++col);
            std::string serv_type = *(++col);
            int serv_id = 0;
            if(!partialLoad) {
                serv_id           = *(++col);
            }
            std::string source_ip = *(++col);
            std::string request_type_id = *(++col);
            ID session_id         = *(++col);
            std::string user_name = *(++col);

            ID user_id = 0;
            if(!partialLoad) {
                user_id           = *(++col);
            }
            bool is_monitoring    = *(++col);
            int rc_code           = *(++col);
            std::string rc_name   = *(++col);

            // fields dependent on partialLoad
            std::string request;
            std::string response;

            std::auto_ptr<RequestProperties> props;
            std::auto_ptr<ObjectReferences> refs;

            if(!partialLoad) {
                request  = (std::string)*(++col);
                response = (std::string)*(++col);
                props    = getPropsForId(id, time_begin, serv_id, is_monitoring);
                refs     = getObjectRefsForId(id, time_begin, serv_id, is_monitoring);
            }

            data_.push_back(new RequestImpl(
                    id,
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
                    props,
                    refs,
                    rc_code,
                    rc_name));
        }

        if(data_.empty()) {
            return;
        }
    }
    catch (Database::Exception& ex) {
        std::string message = ex.what();
        if(message.find(conn.getTimeoutString()) != std::string::npos) {
            LOGGER(PACKAGE).info("Statement timeout in request list.");
            clear();
            throw;
        } else {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            clear();
        }
    }
    catch (std::exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
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
          default:
              break;
      }

  }

  virtual std::auto_ptr<RequestProperties> getPropsForId(ID id, DateTime time_begin, int sid, bool mon) {
    std::auto_ptr<RequestProperties> ret(new RequestProperties());
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

        ret->push_back(RequestProperty(name, value, output, is_child));

    }
    return ret;
  }

  virtual std::auto_ptr<ObjectReferences> getObjectRefsForId(ID id, DateTime time_begin, int sid, bool mon) {
        std::auto_ptr<ObjectReferences> ret(new ObjectReferences());

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
      LOGGER(PACKAGE).error("Unimplemented method called: Logger::ListImpl::makeQuery()");
  }

  virtual void reload() {
     TRACE("[CALL] Fred::Logger::ListImpl::reload()");
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

                std::auto_ptr<RequestProperties> props;
                                std::auto_ptr<ObjectReferences> refs;

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
                            props,
                                                        refs));
            }

            if(data_.empty()) {
                return;
            }
        }
        catch (Database::Exception& ex) {
            std::string message = ex.what();
            if (message.find(conn.getTimeoutString())
                    != std::string::npos) {
                LOGGER(PACKAGE).info("Statement timeout in request list.");
                clear();
                throw;
            } else {
                LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
                clear();
            }
        }
        catch (std::exception& ex) {
          LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
          clear();
        }
  }

};

// TODO where to place this?
List *ManagerImpl::createList() const {
    return new ListImpl((Manager *)this);
}

}; // namespace Logger
};
