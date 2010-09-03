#include <algorithm>
#include <boost/tokenizer.hpp>
#include <boost/utility.hpp>

#include "union_filter.h"
#include "log/logger.h"

namespace Database {
namespace Filters {

    // TODO:
    // * central configuration which types have to be treated like this
    // * separate tree browsing & functor(visitor) parts
class CustomPartitioningTweak {
private:
  CustomPartitioningTweak() {};

public:
  static void find_values_recurs (Filter *f) {
    // This check can be moved to the higher level
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
  
            service = dynamic_cast<ServiceType*>(f);
            if(service == NULL) {
                LOGGER(PACKAGE).error("ServiceType: Inconsistency in filters.");
            }
  
        } else if(f->getName() == "IsMonitoring") {
            if(is_monitoring != NULL) {
                LOGGER(PACKAGE).error("Duplicity IsMonitoring found in filters.");
                return;
            }
  
            is_monitoring = dynamic_cast< Value<bool>* >(f);
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
              Interval<Database::DateTimeInterval> *copy_time_begin = new Interval<Database::DateTimeInterval>(Column("request_time_begin", *tbl));
              copy_time_begin->setName("RequestTimeBegin");
              copy_time_begin->setValue(time_begin->getValue());
              c->add(copy_time_begin);
          }

          if(service != NULL) {
              ServiceType *copy_service = new ServiceType(Column("request_service_id", *tbl));
              copy_service->setName("RequestService");
              copy_service->setValue(service->getValue());
              c->add(copy_service);
          } 

          if(is_monitoring != NULL) {
              Value<bool> *copy_monitoring = new Value<bool>(Column("request_monitoring", *tbl));
              copy_monitoring->setName("RequestIsMonitoring");
              copy_monitoring->setValue(is_monitoring->getValue());
              c->add(copy_monitoring);
          }
      }
  }

  static void process_filters(std::vector<Filter*> & flist) {
        std::vector<Filter*>::iterator it_r = flist.begin();
                
        time_begin = NULL;
        service  = NULL;
        is_monitoring = NULL; 

        for (; it_r != flist.end(); it_r++) {
            RequestImpl *ri = dynamic_cast<RequestImpl*> (*it_r);
            if(ri != NULL) {
              std::cout << "Found RequestImpl, running recurs" << std::endl;
              find_values_recurs (ri);
        
              add_conds_recurs(ri);
            }
        }
  }

  
private:
  Interval<Database::DateTimeInterval> static * time_begin;
  ServiceType static * service;
  Value<bool> static * is_monitoring; 
  
};

Interval<Database::DateTimeInterval> * CustomPartitioningTweak::time_begin = NULL;
ServiceType * CustomPartitioningTweak::service = NULL;
Value<bool> * CustomPartitioningTweak::is_monitoring = NULL;

void Union::serialize(Database::SelectQuery& _q) {
  TRACE("[CALL] Union::serialize()");
  if (_q.initialized()) {
    return;
  }

  CustomPartitioningTweak::process_filters(filter_list);  

  if (filter_list.size() > query_list.size()) {
      boost::format msg = boost::format("assert(filter_list.size() > query_list.size());  %1% <! %2%")
        % filter_list.size() % query_list.size();
    LOGGER(PACKAGE).error(msg);
    throw std::runtime_error(msg.str());
  }

  bool at_least_one = false;
  std::vector<Filter*>::iterator it_f = filter_list.begin();
  std::vector<SelectQuery*>::iterator it_q = query_list.begin();
  for (; it_f != filter_list.end(); ++it_f, ++it_q) {
    if ((*it_f)->isActive()) {
      (*it_f)->serialize(*(*it_q), settings_ptr_);
      (*it_q)->make();
      _q.buffer() << (it_f != filter_list.begin() ? " UNION " : "" );
      _q.buffer() << (*it_q)->buffer().str();
      at_least_one = true;
    }
  }
  _q.finalize();
  TRACE(boost::format("[IN] Union::serialize(): total %1% filters serialized")
      % filter_list.size());
  TRACE(boost::format("[IN] Union::serialize(): generated SQL = %1%")
      % _q.buffer().str());
}

void Union::clearQueries() {
  std::for_each(query_list.begin(), query_list.end(), boost::checked_deleter<SelectQuery>());
  query_list.clear();
}

void Union::clearFilters() {
	std::for_each(filter_list.begin(), filter_list.end(), boost::checked_deleter<Filter>());
	filter_list.clear();
}

void Union::clear() {
	TRACE("[CALL] Union::clear()");
	clearFilters();
	clearQueries();
}

}
}
