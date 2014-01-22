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
 *  keyset info
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "info_keyset.h"
#include "info_keyset_impl.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "util/util.h"

namespace Fred
{

    InfoKeysetByHandle::InfoKeysetByHandle(const std::string& handle)
        : handle_(handle)
        , lock_(false)
    {}

    InfoKeysetByHandle& InfoKeysetByHandle::set_lock(bool lock)//set lock object_registry row for keyset
    {
        lock_ = lock;
        return *this;
    }

    InfoKeysetOutput InfoKeysetByHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoKeysetOutput> keyset_res;

        try
        {
            keyset_res = InfoKeyset()
                    .set_handle(handle_)
                    .set_lock(lock_)
                    .set_history_query(false)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (keyset_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_handle(handle_));
            }

            if (keyset_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return keyset_res.at(0);
    }//InfoKeysetByHandle::exec

    std::string InfoKeysetByHandle::to_string() const
    {
        return Util::format_operation_state("InfoKeysetByHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle", handle_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoKeysetById::InfoKeysetById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    InfoKeysetById& InfoKeysetById::set_lock(bool lock)//set lock object_registry row for keyset
    {
        lock_ = lock;
        return *this;
    }

    InfoKeysetOutput InfoKeysetById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoKeysetOutput> keyset_res;

        try
        {
            keyset_res = InfoKeyset()
                    .set_id(id_)
                    .set_lock(lock_)
                    .set_history_query(false)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (keyset_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

            if (keyset_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return keyset_res.at(0);
    }//InfoKeysetById::exec

    std::string InfoKeysetById::to_string() const
    {
        return Util::format_operation_state("InfoKeysetById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoKeysetHistory::InfoKeysetHistory(const std::string& roid
            , const Optional<boost::posix_time::ptime>& history_timestamp)
        : roid_(roid)
        , history_timestamp_(history_timestamp)
        , lock_(false)
    {}

    InfoKeysetHistory::InfoKeysetHistory(const std::string& roid)
    : roid_(roid)
    , lock_(false)
    {}

    InfoKeysetHistory& InfoKeysetHistory::set_history_timestamp(boost::posix_time::ptime history_timestamp)//set history timestamp
    {
        history_timestamp_ = history_timestamp;
        return *this;
    }

    InfoKeysetHistory& InfoKeysetHistory::set_lock(bool lock)//set lock object_registry row for keyset
    {
        lock_ = lock;
        return *this;
    }

    std::vector<InfoKeysetOutput> InfoKeysetHistory::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoKeysetOutput> keyset_res;

        try
        {
            keyset_res = InfoKeyset()
                    .set_roid(roid_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (keyset_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registry_object_identifier(roid_));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return keyset_res;
    }//InfoKeysetHistory::exec

    std::string InfoKeysetHistory::to_string() const
    {
        return Util::format_operation_state("InfoKeysetHistory",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid_))
        (std::make_pair("history_timestamp",history_timestamp_.print_quoted()))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }


    HistoryInfoKeysetById::HistoryInfoKeysetById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    HistoryInfoKeysetById& HistoryInfoKeysetById::set_lock(bool lock)//set lock object_registry row for keyset
    {
        lock_ = lock;
        return *this;
    }

    std::vector<InfoKeysetOutput> HistoryInfoKeysetById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoKeysetOutput> keyset_history_res;

        try
        {
            keyset_history_res = InfoKeyset()
                    .set_id(id_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (keyset_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return keyset_history_res;
    }//HistoryInfoKeysetById::exec

    std::string HistoryInfoKeysetById::to_string() const
    {
        return Util::format_operation_state("HistoryInfoKeysetById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    HistoryInfoKeysetByHistoryid::HistoryInfoKeysetByHistoryid(unsigned long long historyid)
        : historyid_(historyid)
        , lock_(false)
    {}

    HistoryInfoKeysetByHistoryid& HistoryInfoKeysetByHistoryid::set_lock(bool lock)//set lock object_registry row for keyset
    {
        lock_ = lock;
        return *this;
    }

    InfoKeysetOutput HistoryInfoKeysetByHistoryid::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoKeysetOutput> keyset_history_res;

        try
        {
            keyset_history_res = InfoKeyset()
                    .set_historyid(historyid_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (keyset_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_historyid(historyid_));
            }

            if (keyset_history_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return keyset_history_res.at(0);
    }//HistoryInfoKeysetByHistoryid::exec

    std::string HistoryInfoKeysetByHistoryid::to_string() const
    {
        return Util::format_operation_state("HistoryInfoKeysetByHistoryid",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

}//namespace Fred

