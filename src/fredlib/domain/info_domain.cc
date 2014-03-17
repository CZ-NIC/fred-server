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

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "util/util.h"
#include "util/printable.h"

namespace Fred
{

    InfoDomainByHandle::InfoDomainByHandle(const std::string& fqdn)
        : fqdn_(fqdn)
        , lock_(false)
    {}

    InfoDomainByHandle& InfoDomainByHandle::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoDomainOutput InfoDomainByHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            InfoDomain id;
            id.set_fqdn(fqdn_).set_history_query(false);
            if(lock_) id.set_lock();
            domain_res = id.exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_fqdn(fqdn_));
            }

            if (domain_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res.at(0);
    }

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

    InfoDomainById& InfoDomainById::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoDomainOutput InfoDomainById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            InfoDomain id;
            id.set_id(id_).set_history_query(false);
            if(lock_) id.set_lock();
            domain_res = id.exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

            if (domain_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res.at(0);
    }

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

    InfoDomainHistory& InfoDomainHistory::set_lock()
    {
        lock_ = true;
        return *this;
    }

    std::vector<InfoDomainOutput> InfoDomainHistory::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            InfoDomain id;
            id.set_roid(roid_).set_history_query(true);
            if(lock_) id.set_lock();
            domain_res = id.exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registry_object_identifier(roid_));
            }

        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res;
    }

    std::string InfoDomainHistory::to_string() const
    {
        return Util::format_operation_state("InfoDomainHistory",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid_))
        (std::make_pair("history_timestamp",history_timestamp_.print_quoted()))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoDomainHistoryById::InfoDomainHistoryById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    InfoDomainHistoryById& InfoDomainHistoryById::set_lock()
    {
        lock_ = true;
        return *this;
    }

    std::vector<InfoDomainOutput> InfoDomainHistoryById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_history_res;

        try
        {
            InfoDomain id;
            id.set_id(id_).set_history_query(true);
            if(lock_) id.set_lock();
            domain_history_res = id.exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_history_res;
    }

    std::string InfoDomainHistoryById::to_string() const
    {
        return Util::format_operation_state("InfoDomainHistoryById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoDomainHistoryByHistoryid::InfoDomainHistoryByHistoryid(unsigned long long historyid)
        : historyid_(historyid)
        , lock_(false)
    {}

    InfoDomainHistoryByHistoryid& InfoDomainHistoryByHistoryid::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoDomainOutput InfoDomainHistoryByHistoryid::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_history_res;

        try
        {
            InfoDomain id;
            id.set_historyid(historyid_)
            .set_history_query(true);
            if(lock_) id.set_lock();
            domain_history_res = id.exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_historyid(historyid_));
            }

            if (domain_history_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_history_res.at(0);
    }

    std::string InfoDomainHistoryByHistoryid::to_string() const
    {
        return Util::format_operation_state("InfoDomainHistoryByHistoryid",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }
}//namespace Fred

