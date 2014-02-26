/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @file
 *  nsset info
 */

#include <string>
#include <vector>
#include <utility>

#include <boost/lexical_cast.hpp>

#include "info_nsset.h"
#include "info_nsset_impl.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "util/util.h"

namespace Fred
{

    InfoNssetByHandle::InfoNssetByHandle(const std::string& handle)
        : handle_(handle)
        , lock_(false)
    {}

    InfoNssetByHandle& InfoNssetByHandle::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoNssetOutput InfoNssetByHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_res;

        try
        {
            InfoNsset in;
            in.set_handle(handle_).set_history_query(false);
            if(lock_) in.set_lock();
            nsset_res = in.exec(ctx,local_timestamp_pg_time_zone_name);

            if (nsset_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_handle(handle_));
            }

            if (nsset_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_res.at(0);
    }//InfoNssetByHandle::exec

    std::string InfoNssetByHandle::to_string() const
    {
        return Util::format_operation_state("InfoNssetByHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle", handle_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoNssetById::InfoNssetById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    InfoNssetById& InfoNssetById::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoNssetOutput InfoNssetById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_res;

        try
        {
            InfoNsset in;
            in.set_id(id_).set_history_query(false);
            if(lock_) in.set_lock();
            nsset_res = in.exec(ctx,local_timestamp_pg_time_zone_name);

            if (nsset_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

            if (nsset_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_res.at(0);
    }//InfoNssetById::exec

    std::string InfoNssetById::to_string() const
    {
        return Util::format_operation_state("InfoNssetById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoNssetHistory::InfoNssetHistory(const std::string& roid
            , const Optional<boost::posix_time::ptime>& history_timestamp)
        : roid_(roid)
        , history_timestamp_(history_timestamp)
        , lock_(false)
    {}

    InfoNssetHistory::InfoNssetHistory(const std::string& roid)
    : roid_(roid)
    , lock_(false)
    {}

    InfoNssetHistory& InfoNssetHistory::set_history_timestamp(boost::posix_time::ptime history_timestamp)//set history timestamp
    {
        history_timestamp_ = history_timestamp;
        return *this;
    }

    InfoNssetHistory& InfoNssetHistory::set_lock()
    {
        lock_ = true;
        return *this;
    }

    std::vector<InfoNssetOutput> InfoNssetHistory::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_res;

        try
        {
            InfoNsset in;
            in.set_roid(roid_).set_history_query(true);
            if(lock_) in.set_lock();
            nsset_res = in.exec(ctx,local_timestamp_pg_time_zone_name);

            if (nsset_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registry_object_identifier(roid_));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_res;
    }//InfoNssetHistory::exec

    std::string InfoNssetHistory::to_string() const
    {
        return Util::format_operation_state("InfoNssetHistory",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid_))
        (std::make_pair("history_timestamp",history_timestamp_.print_quoted()))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }


    HistoryInfoNssetById::HistoryInfoNssetById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    HistoryInfoNssetById& HistoryInfoNssetById::set_lock()
    {
        lock_ = true;
        return *this;
    }

    std::vector<InfoNssetOutput> HistoryInfoNssetById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_history_res;

        try
        {
            InfoNsset in;
            in.set_id(id_).set_history_query(true);
            if(lock_) in.set_lock();
            nsset_history_res = in.exec(ctx,local_timestamp_pg_time_zone_name);

            if (nsset_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_history_res;
    }//HistoryInfoNssetById::exec

    std::string HistoryInfoNssetById::to_string() const
    {
        return Util::format_operation_state("HistoryInfoNssetById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    HistoryInfoNssetByHistoryid::HistoryInfoNssetByHistoryid(unsigned long long historyid)
        : historyid_(historyid)
        , lock_(false)
    {}

    HistoryInfoNssetByHistoryid& HistoryInfoNssetByHistoryid::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoNssetOutput HistoryInfoNssetByHistoryid::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_history_res;

        try
        {
            InfoNsset in;
            in.set_historyid(historyid_).set_history_query(true);
            if(lock_) in.set_lock();
            nsset_history_res = in.exec(ctx,local_timestamp_pg_time_zone_name);

            if (nsset_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_historyid(historyid_));
            }

            if (nsset_history_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_history_res.at(0);
    }//HistoryInfoNssetByHistoryid::exec

    std::string HistoryInfoNssetByHistoryid::to_string() const
    {
        return Util::format_operation_state("HistoryInfoNssetByHistoryid",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

}//namespace Fred

