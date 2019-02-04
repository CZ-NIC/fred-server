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

#include "src/deprecated/libfred/common_object.hh"
#include "src/deprecated/libfred/object.hh"
#include "src/deprecated/libfred/filter.hh"
#include "src/deprecated/model/model_filters.hh"
#include "util/log/logger.hh"

namespace LibFred {
namespace Filter {

class FilterImpl : public LibFred::CommonObjectImpl,
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
  virtual void save() const {
    Database::Query insert;
    if (m_data.empty()) {
      LOGGER.error("can't save filter; reason: data empty");
      return;
    }

    Database::InsertQuery insert_filter("filters");
    insert_filter.add("type", m_type);
    insert_filter.add("name", m_name);
    insert_filter.add("userid", m_user_id);
    insert_filter.add("groupid", m_group_id);
    insert_filter.add("data", m_data);

    try {
      //TRACE(boost::format("[IN] LibFred::FilterImpl::save(): going to inserting data SQL = %1%") % insert.str());
      Database::Connection conn = Database::Manager::acquire();
      Database::Result result = conn.exec(insert_filter);
      LOGGER.info(boost::format("filter '%1%' saved successfully")
          % m_name);
    }
    catch (Database::Exception& ex) {
      LOGGER.error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
      LOGGER.error(boost::format("%1%") % ex.what());
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

class ListImpl : public LibFred::CommonListImpl,
                 virtual public List {
public:
  ListImpl() :
    CommonListImpl() {
  }

  virtual ~ListImpl() {
  }
  
  virtual void reload(Database::Filters::Union &uf) {
    TRACE("[CALL] LibFred::Filter::ListImpl::reload()");
    clear();
    uf.clearQueries();

    Database::SelectQuery object_info_query;
    std::unique_ptr<Database::Filters::Iterator> fit(uf.createIterator());
    for (fit->first(); !fit->isDone(); fit->next()) {
      Database::Filters::FilterFilter *ff =
          dynamic_cast<Database::Filters::FilterFilter* >(fit->get());
      if (!ff)
        continue;
      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect("id type name userid groupid", ff->joinFilterTable());
      uf.addQuery(tmp);
    }
    object_info_query.order_by() << "id DESC";
    object_info_query.limit(load_limit_);
    uf.serialize(object_info_query);

    try {
      Database::Connection conn = Database::Manager::acquire();
      Database::Result r_info = conn.exec(object_info_query);
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
  ManagerImpl() : 
	  m_filter_list() {
  }
  virtual List& getList() {
    return m_filter_list;
  }
  virtual void load(Database::ID _id, Database::Filters::Union& _uf) const {
    TRACE(boost::format("[CALL] LibFred::Filter::ManagerImpl::load(%1%)") % _id);
    std::unique_ptr<Database::Filters::FilterFilter> data_filter(new Database::Filters::FilterFilterImpl());
    data_filter->addId().setValue(_id);
    
    Database::SelectQuery data_query;
    data_query.addSelect("data", data_filter->joinFilterTable());
    data_filter->serialize(data_query);
    
    try {
      Database::Connection conn = Database::Manager::acquire();
      
      Database::Result r_data = conn.exec(data_query);
      std::stringstream xml_data((*r_data.begin())[0]);
      
      boost::archive::xml_iarchive load(xml_data);
      load >> BOOST_SERIALIZATION_NVP(_uf);
      
      std::string content;
      Database::Filters::Union::iterator uit = _uf.begin();
      for (; uit != _uf.end(); ++uit) {
        content += "'" + (*uit)->getName() + "' ";
      }
      LOGGER.debug(boost::format("loaded filter content = %1%") % content);
    }
    catch (Database::Exception& ex) {
      LOGGER.error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
      LOGGER.error(boost::format("%1%") % ex.what());
    }
  }
  
  virtual void save(FilterType _type, const std::string& _name, Database::Filters::Union& _uf) {
    TRACE(boost::format("[CALL] LibFred::Filter::ManagerImpl::save(%1%, '%2%')") % _type % _name);
    try {
      std::stringstream xml_data;
      {
        boost::archive::xml_oarchive save(xml_data);
        save << BOOST_SERIALIZATION_NVP(_uf);
      }
      Database::Connection conn = Database::Manager::acquire();

      LibFred::Filter::FilterImpl new_filter(_type, _name, 1, 1, xml_data.str());
      new_filter.save();
    }
    catch (std::exception& ex) {
      LOGGER.error(boost::format("can't save filter (reason: %1%)") % ex.what());
    }
  }
	
private:
	ListImpl		m_filter_list;
};

Manager* Manager::create() {
	TRACE("[CALL] LibFred::Filter::Manager::create()");
	return new ManagerImpl();
}

}
}
