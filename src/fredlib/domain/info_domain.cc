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
 *  domain history info
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "info_domain.h"
#include "info_domain_impl.h"
#include "src/fredlib/object/object.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "util/printable.h"

namespace Fred
{

    InfoDomainByHandle::InfoDomainByHandle(const std::string& fqdn)
        : fqdn_(fqdn)
        , lock_(false)
    {}

    InfoDomainByHandle& InfoDomainByHandle::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    InfoDomainOutput InfoDomainByHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            domain_res = InfoDomain()
                    .set_fqdn(fqdn_)
                    .set_lock(lock_)
                    .set_history_query(false)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_fqdn(fqdn_));
            }

            if (domain_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res.at(0);
    }//InfoDomainByHandle::exec

    std::string InfoDomainByHandle::to_string() const
    {
        return Util::format_operation_state("InfoDomainByHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle", fqdn_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoDomainById::InfoDomainById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    InfoDomainById& InfoDomainById::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    InfoDomainOutput InfoDomainById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            domain_res = InfoDomain()
                    .set_id(id_)
                    .set_lock(lock_)
                    .set_history_query(false)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

            if (domain_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res.at(0);
    }//InfoDomainById::exec

    std::string InfoDomainById::to_string() const
    {
        return Util::format_operation_state("InfoDomainById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoDomainHistory::InfoDomainHistory(const std::string& roid
            , const Optional<boost::posix_time::ptime>& history_timestamp)
        : roid_(roid)
        , history_timestamp_(history_timestamp)
        , lock_(false)
    {}

    InfoDomainHistory::InfoDomainHistory(const std::string& roid)
    : roid_(roid)
    , lock_(false)
    {}

    InfoDomainHistory& InfoDomainHistory::set_history_timestamp(boost::posix_time::ptime history_timestamp)//set history timestamp
    {
        history_timestamp_ = history_timestamp;
        return *this;
    }

    InfoDomainHistory& InfoDomainHistory::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    std::vector<InfoDomainOutput> InfoDomainHistory::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            domain_res = InfoDomain()
                    .set_roid(roid_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registry_object_identifier(roid_));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res;
    }//InfoDomainHistory::exec

    std::string InfoDomainHistory::to_string() const
    {
        return Util::format_operation_state("InfoDomainHistory",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid_))
        (std::make_pair("history_timestamp",history_timestamp_.print_quoted()))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    HistoryInfoDomainById::HistoryInfoDomainById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    HistoryInfoDomainById& HistoryInfoDomainById::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    std::vector<InfoDomainOutput> HistoryInfoDomainById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_history_res;

        try
        {
            domain_history_res = InfoDomain()
                    .set_id(id_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_history_res;
    }//HistoryInfoDomainById::exec

    std::string HistoryInfoDomainById::to_string() const
    {
        return Util::format_operation_state("HistoryInfoDomainById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    HistoryInfoDomainByHistoryid::HistoryInfoDomainByHistoryid(unsigned long long historyid)
        : historyid_(historyid)
        , lock_(false)
    {}

    HistoryInfoDomainByHistoryid& HistoryInfoDomainByHistoryid::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    InfoDomainOutput HistoryInfoDomainByHistoryid::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_history_res;

        try
        {
            domain_history_res = InfoDomain()
                    .set_historyid(historyid_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_historyid(historyid_));
            }

            if (domain_history_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_history_res.at(0);
    }//HistoryInfoDomainByHistoryid::exec

    std::string HistoryInfoDomainByHistoryid::to_string() const
    {
        return Util::format_operation_state("HistoryInfoDomainByHistoryid",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }
}//namespace Fred

