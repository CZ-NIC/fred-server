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


#ifndef OBJ_TYPES_H_
#define OBJ_TYPES_H_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>

#include "register/db_settings.h"
#include "model/model_filters.h"

namespace Register
{

typedef std::vector<std::string> ObjData;//type for object data

template<typename MEMBER_TYPE>
class ObjType //type for object
{
    //index 0 of data_ member is for "id"
    ObjData data_;
    std::size_t id_;
public:
    ~ObjType(){}//nv public dtor

    ObjType()
    : data_(0)
    , id_(0)
    {}

    //ctor
    ObjType(std::size_t cols)
    : data_(cols)
    , id_(0)
    {}
    //copy
    ObjType(const ObjType& param )
    : data_(param.data_)
    , id_(param.id_)
    {}
    //operators
    ObjType& operator=(const ObjType& param)
    {
        if (this != &param)
        {
            data_=param.data_;
            id_=param.id_;
        }
        return *this;
    }//operator=

    bool operator< (const ObjType& param) const
    {
        return
            this->id_ < param.id_;
    }//operator<

    bool operator== (const ObjType& param) const
    {
        return
            this->id_ == param.id_;
    }//operator==

    //checking column set
    void set(MEMBER_TYPE col,const std::string& value)
    {
        if (data_.size() <= static_cast<std::size_t>(col))
            data_.resize(static_cast<std::size_t>(col + 1));

        if(col == 0)//is id
            id_ = boost::lexical_cast<std::size_t>(value);

        data_[col] = value;
    }

    //checking column get
    const std::string& get(MEMBER_TYPE col) const
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
};

template<typename MEMBER_TYPE, typename OBJ_PTR>
class CompareObj //functor for sorting
{
    bool asc_;
    std::size_t col_;
public:
    CompareObj(bool _asc, MEMBER_TYPE _col)
    : asc_(_asc)
    , col_(static_cast<std::size_t>(_col))
    { }

  bool operator()(OBJ_PTR _left, OBJ_PTR _right) const
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

typedef std::map<std::size_t, std::size_t> ObjIdx;//object list search by index type

template<typename MEMBER_TYPE, typename OBJ_PTR>
class ObjList //type for object list
{
    typedef std::vector<OBJ_PTR> ListType;
    ListType ml_;
    ObjIdx by_id_;
    bool loadLimitActive_;
    std::size_t limit_;
    bool realSizeInitialized_;
    std::size_t realSize_;


public:
    ObjList()
    : loadLimitActive_(false)
    , limit_(0)
    , realSizeInitialized_(false)
    , realSize_(0)
    {}

    bool isLimited() const
    {
        return loadLimitActive_;
    }

    std::size_t getLimit() const
    {
        return limit_;
    }

    void setLimit(std::size_t limit) const
    {
        limit_ = limit;
    }

    OBJ_PTR get(std::size_t row) const
    {
        return ml_.at(row);
    }

    std::size_t size() const
    {
        return ml_.size();
    }

    void sort(MEMBER_TYPE _member, bool _asc)
    {
        std::stable_sort(ml_.begin(), ml_.end(), CompareObj<MEMBER_TYPE,OBJ_PTR>(_asc, _member));
    }

    void reload(Database::Filters::Union &uf)
    {
        reload_impl(uf,ml_, limit_, loadLimitActive_);

        //fill index by id
        for(std::size_t i = 0; i < ml_.size(); ++i)
        {
            by_id_[boost::lexical_cast<std::size_t>(ml_.at(i)->get(0))//id
                  ] =i;//list idx
        }
    }

    void clear()
    {
        ml_.clear();
    }

    void makeRealCount(Database::Filters::Union &filter)
    {

        if (filter.empty()) {
            realSize_ = 0;
            realSizeInitialized_ = false;
            LOGGER(PACKAGE).warning(
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
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        }
        catch (std::exception& ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        }
    }

    unsigned long long getRealCount(Database::Filters::Union &filter)
    {
        if (!realSizeInitialized_) {
            makeRealCount(filter);
        }
        return realSize_;
    }

    OBJ_PTR findId(unsigned long long id) const
    {
        ObjIdx::const_iterator it;
         it = by_id_.find(id);
         if(it != by_id_.end())
             return ml_.at(it->second);

        LOGGER(PACKAGE).debug(boost::format("object list miss! object id=%1% not found")
        % id);
        throw std::runtime_error("id not found");
    }

};//ObjList

}//namespace Register
#endif //OBJ_TYPES_H_

