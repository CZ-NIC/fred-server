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

/**
 *  @obj_types.h
 *  header of another object-list implementation made by refactoring of CommonListImplNew
 */

#ifndef OBJ_TYPES_HH_6DDCEE3942994087828436BB4953FB57
#define OBJ_TYPES_HH_6DDCEE3942994087828436BB4953FB57

#include "libfred/db_settings.hh"
#include "src/deprecated/libfred/db_settings.hh"
#include "src/deprecated/model/model_filters.hh"
#include "src/util/types/stringify.hh"

#include <boost/utility.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace LibFred {

struct ObjMemberConversion
{
    enum Type
    {
        MC_NONE,//no member conversion
        MC_DATETIME,//member conversion of datetime
        MC_ULONG // ulong 11 chars
    };
};

template<typename OBJECT_META_INFO>
class ObjType : boost::noncopyable //type for object
{
    typedef std::vector<std::string> ObjData;//type for object data
    //index 0 of data_ member is for "id"
    ObjData data_;
    std::size_t id_;
public:
    typedef typename std::shared_ptr<ObjType<OBJECT_META_INFO>> ObjPtr;
    typedef OBJECT_META_INFO ObjMetaInfo;

    ~ObjType(){}//nv public dtor
    //ctors
    ObjType() : data_(OBJECT_META_INFO::columns), id_(0) {}

    //checking column set
    void set(typename OBJECT_META_INFO::MemberOrderType col
            ,const std::string& value)
    {
        if (data_.size() <= static_cast<std::size_t>(col))
            data_.resize(static_cast<std::size_t>(col + 1));

        if(col == 0)//is id
            id_ = boost::lexical_cast<std::size_t>(value);

        data_[col] = value;
/*
#ifdef HAVE_LOGGER
        if(LOGGER.getLevel() >= Logging::Log::LL_DEBUG)
        {
        	std::string members;
        	for (std::size_t i = 0; i < data_.size(); ++i)
        	{
        	members += 	std::string(" #")
				+ boost::lexical_cast<std::string>(i)
        		+ std::string(" : ") +  data_[i];
        	}

            LOGGER.debug(
                    boost::format("ObjType::set current object state: id %1% members %2%")
                % id_ % members);
        }//id debug
#endif //HAVE_LOGGER
*/
    }

    //checking column get
    const std::string& get(typename OBJECT_META_INFO::MemberOrderType col) const
    {
        return data_.at(static_cast<std::size_t>(col));
    }
    const std::string& get(std::size_t col) const
    {
        return data_.at(col);
    }

    std::size_t get_id() const
    {
        return id_;
    }
    std::size_t get_columns() const
    {
    	return OBJECT_META_INFO::columns;
    }

    //member conversion and set
    void conv_set(typename OBJECT_META_INFO::MemberOrderType col
            ,const std::string& value)
    {
        switch(OBJECT_META_INFO::member_conversion_type(col))
        {
        case ObjMemberConversion::MC_ULONG:
            {
            if(value.empty())
            set(col,value);
        else
            {
                try
                {
                    char new_value[13] ={0};
                    snprintf (new_value, 12 ,"%011u"
                        , static_cast<unsigned>(
                                boost::lexical_cast<unsigned long>(value)));
                    set(col,std::string(new_value));
                }//try
                catch(const std::exception& ex)
                {
                    set(col,value);
                }
            }//else
            }//case block
            break;
        case ObjMemberConversion::MC_DATETIME:
            {
	        if(value.empty())
		    set(col,value);
		else
                    set(col, boost::posix_time::to_iso_string(
                        boost::posix_time::time_from_string(value)));
            }
            break;
        default:
        case ObjMemberConversion::MC_NONE:
            set(col,value);
            break;
        }
    }
    //member get and conversion
     std::string conv_get(
             typename OBJECT_META_INFO::MemberOrderType col) const
    {
        switch(OBJECT_META_INFO::member_conversion_type(col))
        {
        case ObjMemberConversion::MC_ULONG:
        {
            if(get(col).empty())
                return std::string("");
            else
            {
                try
                {
                    return boost::lexical_cast<std::string>(
                        boost::lexical_cast<unsigned long>(get(col)));
                }//try
                catch(const std::exception& ex)
                {
                    return(std::string("got invalid value: ") + get(col));
                }
            }//else
        }//case block
            break;

        case ObjMemberConversion::MC_DATETIME:
        {
            if(get(col).empty())
                return std::string("");
            else
            {
                return stringify(
                        boost::date_time::c_local_adjustor<ptime>::utc_to_local(
                            boost::posix_time::from_iso_string(get(col))));
            }
        }
            break;
        default:
        case ObjMemberConversion::MC_NONE:
            return get(col);
            break;
        }
    }


};

template<typename OBJECT_META_INFO>
class CompareObj //functor for sorting
{
    bool asc_;
    std::size_t col_;
public:
    typedef typename ObjType<OBJECT_META_INFO>::ObjPtr  ObjPtr;

    CompareObj(bool _asc, typename OBJECT_META_INFO::MemberOrderType _col)
    : asc_(_asc)
    , col_(static_cast<std::size_t>(_col))
    { }

  bool operator()(ObjPtr _left, ObjPtr _right) const
  {
      if(col_ == 0) //id
          return (asc_
                  ? (_left->get_id() < _right->get_id())
                  : (_left->get_id() > _right->get_id())
                 );
      else  //others
          return (asc_
                   ? (_left->get(col_) < _right->get(col_))
                   : (_left->get(col_) > _right->get(col_))
                 );
  }//operator()
};//class CompareObj


template<typename OBJECT_META_INFO
, typename RELOAD_FUNCTOR
>
class ObjList //type for object list
{
	typedef typename ObjType<OBJECT_META_INFO>::ObjPtr  ObjPtr;
    typedef std::vector<ObjPtr> ListType;
    typedef std::map<std::size_t, ObjPtr> ObjIdx;//object list search by index type

    ListType list_;
    ObjIdx by_id_;
    bool loadLimitActive_;
    std::size_t limit_;
    bool realSizeInitialized_;
    std::size_t realSize_;
    std::unique_ptr<RELOAD_FUNCTOR> reload_impl_;
public:
    ObjList(std::unique_ptr<RELOAD_FUNCTOR> reload_impl)
    : loadLimitActive_(false)
    , limit_(1000)
    , realSizeInitialized_(false)
    , realSize_(0)
    , reload_impl_(std::move(reload_impl))
    {}

    bool isLimited() const
    {
        return loadLimitActive_;
    }

    std::size_t getLimit() const
    {
        return limit_;
    }

    void setLimit(std::size_t limit)
    {
        limit_ = limit;
    }

    ObjPtr get(std::size_t row) const
    {
        return list_.at(row);
    }

    std::size_t size() const
    {
        return list_.size();
    }

    void sort(typename OBJECT_META_INFO::MemberOrderType _member, bool _asc)
    {
        std::stable_sort(list_.begin(), list_.end()
        		, CompareObj<OBJECT_META_INFO>(_asc, _member));
    }

    void reload(Database::Filters::Union &uf)
    {	//RELOAD_FUNCTOR call
        (*reload_impl_)(uf,list_, limit_, loadLimitActive_);

        //fill index by id
        for(std::size_t i = 0; i < list_.size(); ++i)
        {
            by_id_[list_.at(i)->get_id()//id
                  ] =list_.at(i);//list obj ptr
        }
    }

    void clear()
    {
        list_.clear();
    }

    void makeRealCount(Database::Filters::Union &filter)
    {

        if (filter.empty()) {
            realSize_ = 0;
            realSizeInitialized_ = false;
            LOGGER.warning(
                    "can't make real filter data count -- no filter specified...");
            return;
        }

        filter.clearQueries();

        Database::Filters::Union::iterator it = filter.begin();
        for (; it != filter.end(); ++it) {
            Database::SelectQuery *tmp = new Database::SelectQuery();
            tmp->select() << "COUNT(*)";
            filter.addQuery(tmp);
        }

        Database::SelectQuery count_query;
        filter.serialize(count_query);

        Database::Connection conn = Database::Manager::acquire();

        try {
            Database::Result r_count = conn.exec(count_query);
            realSize_ = (*(r_count.begin()))[0];
            realSizeInitialized_ = true;
        }
        catch (Database::Exception& ex) {
            LOGGER.error(boost::format("%1%") % ex.what());
        }
        catch (std::exception& ex) {
            LOGGER.error(boost::format("%1%") % ex.what());
        }
    }

    unsigned long long getRealCount(Database::Filters::Union &filter)
    {
        if (!realSizeInitialized_) {
            makeRealCount(filter);
        }
        return realSize_;
    }

    ObjPtr findId(unsigned long long id) const
    {
        typename ObjIdx::const_iterator it;
         it = by_id_.find(id);
         if(it != by_id_.end())
             return it->second;

    	LOGGER.debug(boost::format("object list miss! object id=%1% not found")
        % id);
        throw std::runtime_error("id not found");
    }

    void setTimeout(unsigned _timeout)
    {
      Database::Connection conn = Database::Manager::acquire();

      conn.setQueryTimeout(_timeout);
    }
};

}//namespace LibFred

#endif//OBJ_TYPES_HH_6DDCEE3942994087828436BB4953FB57
