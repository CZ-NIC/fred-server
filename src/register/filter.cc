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

#include "filter.h"
#include "db/dbs.h"
#include "model/model_filters.h"
#include "log/logger.h"

namespace Register {
namespace Filter {

class FilterImpl : virtual public Filter {
public:
  FilterImpl(DBase::ID _id, FilterType _type, const std::string& _name,
      DBase::ID _user_id, DBase::ID _group_id) :
    m_id(_id), m_type(_type), m_name(_name), m_user_id(_user_id),
        m_group_id(_group_id) {
  }
  FilterImpl(FilterType _type, const std::string& _name, DBase::ID _user_id,
      DBase::ID _group_id) :
    m_id(0), m_type(_type), m_name(_name), m_user_id(_user_id),
        m_group_id(_group_id) {
  }
  FilterImpl(FilterType _type, const std::string& _name, DBase::ID _user_id,
      DBase::ID _group_id, const std::string& _data) :
    m_id(0), m_type(_type), m_name(_name), m_user_id(_user_id),
        m_group_id(_group_id), m_data(_data) {
  }
  virtual ~FilterImpl() {
  }
  virtual DBase::ID getId() const {
    return m_id;
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
  virtual DBase::ID getUserId() const {
    return m_user_id;
  }
  virtual void setUserId(DBase::ID _id) {
    m_user_id = _id;
  }
  virtual DBase::ID getGroupId() const {
    return m_group_id;
  }
  virtual void setGroupId(DBase::ID _id) {
    m_group_id = _id;
  }
  virtual void save(DBase::Connection *_conn) const {
    DBase::Query insert;
    if (m_data.empty()) {
      LOGGER("db").error("can't save filter; reason: data empty");
      return;
    }

    insert.buffer() << "INSERT INTO filters VALUES (DEFAULT, " << m_type
        << ", '" << m_name << "', " << m_user_id << ", " << m_group_id << ", '"
        << m_data << "')";

    try {
      //TRACE(boost::format("[IN] Register::FilterImpl::save(): going to inserting data SQL = %1%") % insert.str());
      std::auto_ptr<DBase::Result> result(_conn->exec(insert));
      LOGGER("db").info(boost::format("filter '%1%' saved successfully")
          % m_name);
    }
    catch (DBase::Exception& ex) {
      LOGGER("db").error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
      LOGGER("db").error(boost::format("%1%") % ex.what());
    }
  }

private:
  DBase::ID m_id;
  FilterType m_type;
  std::string m_name;
  DBase::ID m_user_id;
  DBase::ID m_group_id;
  std::string m_data;
};

class ListImpl : virtual public List {
public:
  ListImpl(DBase::Connection* _conn) :
    m_conn(_conn) {
  }
  virtual ~ListImpl() {
    delete m_conn;
  }
  virtual void reload(DBase::Filters::Union &uf) {
    TRACE("[CALL] Register::Filter::ListImpl::reload()");
    clear();
    uf.clearQueries();

    DBase::SelectQuery object_info_query;
    std::auto_ptr<DBase::Filters::Iterator> fit(uf.createIterator());
    for (fit->first(); !fit->isDone(); fit->next()) {
      DBase::Filters::FilterFilter *ff =
          dynamic_cast<DBase::Filters::FilterFilter* >(fit->get());
      if (!ff)
        continue;
      DBase::SelectQuery *tmp = new DBase::SelectQuery();
      tmp->addSelect("id type name userid groupid", ff->joinFilterTable());
      uf.addQuery(tmp);
    }
    object_info_query.limit(5000);
    uf.serialize(object_info_query);

    try {
      std::auto_ptr<DBase::Result> r_info(m_conn->exec(object_info_query));
      std::auto_ptr<DBase::ResultIterator> it(r_info->getIterator());
      for (it->first(); !it->isDone(); it->next()) {
        DBase::ID id = it->getNextValue();
        FilterType type = (FilterType)(int)it->getNextValue();
        std::string name = it->getNextValue();
        DBase::ID userid = it->getNextValue();
        DBase::ID groupid = it->getNextValue();

        m_data.push_back(
            new FilterImpl(
                id,
                type,
                name,
                userid,
                groupid
            ));
      }
    }
    catch (DBase::Exception& ex) {
      LOGGER("db").error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
      LOGGER("db").error(boost::format("%1%") % ex.what());
    }

    // FilterImpl *filter_1_db = new FilterImpl(1, FT_DOMAIN, "Domains which expires next month", 1, 1);
    // m_data.push_back(filter_1_db);
  }
  virtual const unsigned size() const {
    return m_data.size();
  }
  virtual void clear() {
    std::for_each(m_data.begin(), m_data.end(), boost::checked_deleter<FilterImpl>());
		m_data.clear();
	}
	virtual const Filter* get(unsigned _idx) const {
		return m_data.at(_idx);
	}
	
private:
	DBase::Connection			*m_conn;
	std::vector<FilterImpl* > 	 m_data;
};

class ManagerImpl : virtual public Manager {
public:
  ManagerImpl(DBase::Manager* _db_manager) : 
    m_db_manager(_db_manager), 
	m_filter_list(m_db_manager->getConnection()) {
  }
  virtual List& getList() {
    return m_filter_list;
  }
  virtual void load(DBase::ID _id, DBase::Filters::Union& _uf) const {
    TRACE(boost::format("[CALL] Register::Filter::ManagerImpl::load(%1%)") % _id);
    std::auto_ptr<DBase::Filters::FilterFilter> data_filter(new DBase::Filters::FilterFilterImpl());
    data_filter->addId().setValue(_id);
    
    DBase::SelectQuery data_query;
    data_query.addSelect("data", data_filter->joinFilterTable());
    data_filter->serialize(data_query);
    
    try {
      std::auto_ptr<DBase::Connection> conn(m_db_manager->getConnection());
      std::auto_ptr<DBase::Result> r_data(conn->exec(data_query));
      std::auto_ptr<DBase::ResultIterator> it(r_data->getIterator());
      std::stringstream xml_data(it->getNextValue());
      
      boost::archive::xml_iarchive load(xml_data);
      load >> BOOST_SERIALIZATION_NVP(_uf);
      
      std::string content;
      DBase::Filters::Union::iterator uit = _uf.begin();
      for (; uit != _uf.end(); ++uit) {
        content += "'" + (*uit)->getName() + "' ";
      }
      LOGGER("db").debug(boost::format("loaded filter content = %1%") % content);
    }
    catch (DBase::Exception& ex) {
      LOGGER("db").error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
      LOGGER("db").error(boost::format("%1%") % ex.what());
    }
  }
  
  virtual void save(FilterType _type, const std::string& _name, DBase::Filters::Union& _uf) {
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
      LOGGER("db").error(boost::format("can't save filter (reason: %1%)") % ex.what());
    }
  }
	
private:
	DBase::Manager* m_db_manager;
	ListImpl		m_filter_list;
};

Manager* Manager::create(DBase::Manager* _db_manager) {
	TRACE("[CALL] Register::Filter::Manager::create()");
	return new ManagerImpl(_db_manager);
}

}
}
