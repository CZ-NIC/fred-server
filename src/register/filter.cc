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

#include <boost/utility.hpp>
#include <algorithm>
#include <vector>
#include <map>
#include <sstream>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include "common_object.h"
#include "object.h"
#include "filter.h"
#include "db/manager.h"
#include "model/model_filters.h"
#include "log/logger.h"

namespace Register {
namespace Filter {

class FilterImpl : public Register::CommonObjectImpl,
                   virtual public Filter {
public:
  FilterImpl(Database::ID _id, FilterType _type, const std::string& _name,
      Database::ID _user_id, Database::ID _group_id) :
    CommonObjectImpl(_id), m_type(_type), m_name(_name), m_user_id(_user_id),
        m_group_id(_group_id) {
  }
  FilterImpl(FilterType _type, const std::string& _name, Database::ID _user_id,
      Database::ID _group_id) :
    m_id(0), m_type(_type), m_name(_name), m_user_id(_user_id),
        m_group_id(_group_id) {
  }
  FilterImpl(FilterType _type, const std::string& _name, Database::ID _user_id,
      Database::ID _group_id, const std::string& _data) :
    m_id(0), m_type(_type), m_name(_name), m_user_id(_user_id),
        m_group_id(_group_id), m_data(_data) {
  }
  virtual ~FilterImpl() {
  }
  virtual const std::string& getName() const {
    return m_name;
  }
  virtual void setName(const std::string& _name) {
    m_name = _name;
  }
  virtual FilterType getType() const {
    return m_type;
  }
  virtual void setType(FilterType _type) {
    m_type = _type;
  }
  virtual Database::ID getUserId() const {
    return m_user_id;
  }
  virtual void setUserId(Database::ID _id) {
    m_user_id = _id;
  }
  virtual Database::ID getGroupId() const {
    return m_group_id;
  }
  virtual void setGroupId(Database::ID _id) {
    m_group_id = _id;
  }
  virtual void save(Database::Connection *_conn) const {
    Database::Query insert;
    if (m_data.empty()) {
      LOGGER(PACKAGE).error("can't save filter; reason: data empty");
      return;
    }

    insert.buffer() << "INSERT INTO filters VALUES (DEFAULT, " << m_type
        << ", '" << m_name << "', " << m_user_id << ", " << m_group_id << ", '"
        << m_data << "')";

    try {
      //TRACE(boost::format("[IN] Register::FilterImpl::save(): going to inserting data SQL = %1%") % insert.str());
      Database::Result result = _conn->exec(insert);
      LOGGER(PACKAGE).info(boost::format("filter '%1%' saved successfully")
          % m_name);
    }
    catch (Database::Exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
  }

private:
  Database::ID m_id;
  FilterType m_type;
  std::string m_name;
  Database::ID m_user_id;
  Database::ID m_group_id;
  std::string m_data;
};

class ListImpl : public Register::CommonListImpl,
                 virtual public List {
public:
  ListImpl(Database::Connection* _conn) :
    CommonListImpl(_conn) {
  }

  virtual ~ListImpl() {
    boost::checked_delete<Database::Connection>(conn_);
  }
  
  virtual void reload(Database::Filters::Union &uf) {
    TRACE("[CALL] Register::Filter::ListImpl::reload()");
    clear();
    uf.clearQueries();

    Database::SelectQuery object_info_query;
    std::auto_ptr<Database::Filters::Iterator> fit(uf.createIterator());
    for (fit->first(); !fit->isDone(); fit->next()) {
      Database::Filters::FilterFilter *ff =
          dynamic_cast<Database::Filters::FilterFilter* >(fit->get());
      if (!ff)
        continue;
      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect("id type name userid groupid", ff->joinFilterTable());
      uf.addQuery(tmp);
    }
    object_info_query.limit(load_limit_);
    uf.serialize(object_info_query);

    try {
      Database::Result r_info = conn_->exec(object_info_query);
      for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID id      = *col;
        FilterType   type    = (FilterType)(int)*(++col);
        std::string  name    = *(++col);
        Database::ID userid  = *(++col);
        Database::ID groupid = *(++col);

        data_.push_back(
            new FilterImpl(
                id,
                type,
                name,
                userid,
                groupid
            ));
      }
      /* checks if row number result load limit is active and set flag */ 
      CommonListImpl::reload();
    }
    catch (Database::Exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
  }

  virtual Filter* get(unsigned _idx) const {
    try {
      Filter *filter = dynamic_cast<Filter* >(data_.at(_idx));
      if (filter) 
        return filter;
      else
        throw std::exception();
    }
    catch (...) {
      throw std::exception();
    } 
  }
  
  virtual void makeQuery(bool, bool, std::stringstream&) const {
    /* dummy implementation */
  }
  
  virtual const char* getTempTableName() const {
    return ""; /* dummy implementation */ 
  }

  virtual void reload() {
    /* dummy implementation */
  }

};

class ManagerImpl : virtual public Manager {
public:
  ManagerImpl(Database::Manager* _db_manager) : 
    m_db_manager(_db_manager), 
	  m_filter_list(m_db_manager->getConnection()) {
  }
  virtual List& getList() {
    return m_filter_list;
  }
  virtual void load(Database::ID _id, Database::Filters::Union& _uf) const {
    TRACE(boost::format("[CALL] Register::Filter::ManagerImpl::load(%1%)") % _id);
    std::auto_ptr<Database::Filters::FilterFilter> data_filter(new Database::Filters::FilterFilterImpl());
    data_filter->addId().setValue(_id);
    
    Database::SelectQuery data_query;
    data_query.addSelect("data", data_filter->joinFilterTable());
    data_filter->serialize(data_query);
    
    try {
      std::auto_ptr<Database::Connection> conn(m_db_manager->getConnection());
      
      Database::Result r_data = conn->exec(data_query);
      std::stringstream xml_data((*r_data.begin())[0]);
      
      boost::archive::xml_iarchive load(xml_data);
      load >> BOOST_SERIALIZATION_NVP(_uf);
      
      std::string content;
      Database::Filters::Union::iterator uit = _uf.begin();
      for (; uit != _uf.end(); ++uit) {
        content += "'" + (*uit)->getName() + "' ";
      }
      LOGGER(PACKAGE).debug(boost::format("loaded filter content = %1%") % content);
    }
    catch (Database::Exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
  }
  
  virtual void save(FilterType _type, const std::string& _name, Database::Filters::Union& _uf) {
    TRACE(boost::format("[CALL] Register::Filter::ManagerImpl::save(%1%, '%2%')") % _type % _name);
    try {
      std::stringstream xml_data;
      {
        boost::archive::xml_oarchive save(xml_data);
        save << BOOST_SERIALIZATION_NVP(_uf);
      }
      Register::Filter::FilterImpl new_filter(_type, _name, 1, 1, xml_data.str());
      new_filter.save(m_db_manager->getConnection());
    }
    catch (std::exception& ex) {
      LOGGER(PACKAGE).error(boost::format("can't save filter (reason: %1%)") % ex.what());
    }
  }
	
private:
	Database::Manager* m_db_manager;
	ListImpl		m_filter_list;
};

Manager* Manager::create(Database::Manager* _db_manager) {
	TRACE("[CALL] Register::Filter::Manager::create()");
	return new ManagerImpl(_db_manager);
}

}
}
