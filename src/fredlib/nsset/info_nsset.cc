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
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/contact/check_contact.h"
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
            in.set_inline_view_filter(Database::ParamQuery("info_nsset_handle = UPPER(").param_text(handle_)(")"))
                .set_history_query(false);
            if(lock_)
            {
                in.set_lock();
            }
            nsset_res = in.exec(ctx,local_timestamp_pg_time_zone_name);

            if (nsset_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_handle(handle_));
            }

            if (nsset_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_res.at(0);
    }

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
            in.set_inline_view_filter(Database::ParamQuery("info_nsset_id = ").param_bigint(id_))
                .set_history_query(false);
            if(lock_)
            {
                in.set_lock();
            }
            nsset_res = in.exec(ctx,local_timestamp_pg_time_zone_name);

            if (nsset_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

            if (nsset_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_res.at(0);
    }

    std::string InfoNssetById::to_string() const
    {
        return Util::format_operation_state("InfoNssetById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }


    InfoNssetByDNSFqdn::InfoNssetByDNSFqdn(const std::string& dns_fqdn)
        : dns_fqdn_(dns_fqdn)
        , lock_(false)
    {}

    InfoNssetByDNSFqdn& InfoNssetByDNSFqdn::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoNssetByDNSFqdn& InfoNssetByDNSFqdn::set_limit(unsigned long long limit)
    {
        limit_ = Optional<unsigned long long>(limit);
        return *this;
    }

    std::vector<InfoNssetOutput> InfoNssetByDNSFqdn::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_res;

        try
        {
            if(Fred::CheckDomain(dns_fqdn_).is_invalid_syntax())
            {
                BOOST_THROW_EXCEPTION(Exception().set_invalid_dns_fqdn(dns_fqdn_));
            }

            Database::ParamQuery cte_id_filter_query;

            cte_id_filter_query("SELECT nssetid FROM host WHERE fqdn = ").param_text(dns_fqdn_);

            if(limit_.isset())
            {
                cte_id_filter_query (" ORDER BY nssetid LIMIT ").param_bigint(limit_.get_value());
            }


            std::pair<std::string,Database::query_param_list> check_dns_fqdn_query = cte_id_filter_query.get_query();
            if (ctx.get_conn().exec_params(check_dns_fqdn_query.first, check_dns_fqdn_query.second).size() == 0)
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_dns_fqdn(dns_fqdn_));
            }

            InfoNsset in;
            in.set_cte_id_filter(cte_id_filter_query)
                .set_history_query(false);
            if(lock_)
            {
                in.set_lock();
            }
            nsset_res = in.exec(ctx,local_timestamp_pg_time_zone_name);
        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_res;
    }

    std::string InfoNssetByDNSFqdn::to_string() const
    {
        return Util::format_operation_state("InfoNssetByDNSFqdn",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("dns_fqdn", dns_fqdn_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        (std::make_pair("limit",limit_.print_quoted()))
        );
    }


    InfoNssetByTechContactHandle::InfoNssetByTechContactHandle(const std::string& tc_handle)
        : tech_contact_handle_(tc_handle)
        , lock_(false)
    {}

    InfoNssetByTechContactHandle& InfoNssetByTechContactHandle::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoNssetByTechContactHandle& InfoNssetByTechContactHandle::set_limit(unsigned long long limit)
    {
        limit_ = Optional<unsigned long long>(limit);
        return *this;
    }

    std::vector<InfoNssetOutput> InfoNssetByTechContactHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_res;

        try
        {
            if(Fred::CheckContact(tech_contact_handle_).is_invalid_handle())
            {
                BOOST_THROW_EXCEPTION(Exception().set_invalid_tech_contact_handle(tech_contact_handle_));
            }

            Database::ParamQuery cte_id_filter_query;

            cte_id_filter_query("SELECT ncm.nssetid"
                " FROM object_registry oreg"
                " JOIN  enum_object_type eot ON oreg.type = eot.id AND eot.name = 'contact'"
                " JOIN nsset_contact_map ncm ON ncm.contactid = oreg.id"
                " WHERE oreg.name = UPPER(").param_text(tech_contact_handle_)(") AND oreg.erdate IS NULL");

            if(limit_.isset())
            {
                cte_id_filter_query (" ORDER BY ncm.nssetid LIMIT ").param_bigint(limit_.get_value());
            }

            std::pair<std::string,Database::query_param_list> check_tech_c_query = cte_id_filter_query.get_query();
            if (ctx.get_conn().exec_params(check_tech_c_query.first, check_tech_c_query.second).size() == 0)
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_tech_contact_handle(tech_contact_handle_));
            }

            InfoNsset in;
            in.set_cte_id_filter(cte_id_filter_query)
                .set_history_query(false);
            if(lock_)
            {
                in.set_lock();
            }
            nsset_res = in.exec(ctx,local_timestamp_pg_time_zone_name);
        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_res;
    }


    std::string InfoNssetByTechContactHandle::to_string() const
    {
        return Util::format_operation_state("InfoNssetByTechContactHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("tech_contact_handle", tech_contact_handle_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        (std::make_pair("limit",limit_.print_quoted()))
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
            in.set_inline_view_filter(Database::ParamQuery("info_nsset_roid = ").param_text(roid_))
                .set_history_query(true);
            if(lock_)
            {
                in.set_lock();
            }
            nsset_res = in.exec(ctx,local_timestamp_pg_time_zone_name);
        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_res;
    }

    std::string InfoNssetHistory::to_string() const
    {
        return Util::format_operation_state("InfoNssetHistory",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid_))
        (std::make_pair("history_timestamp",history_timestamp_.print_quoted()))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }


    InfoNssetHistoryById::InfoNssetHistoryById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    InfoNssetHistoryById& InfoNssetHistoryById::set_lock()
    {
        lock_ = true;
        return *this;
    }

    std::vector<InfoNssetOutput> InfoNssetHistoryById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_history_res;

        try
        {
            InfoNsset in;
            in.set_inline_view_filter(Database::ParamQuery("info_nsset_id = ").param_bigint(id_))
                .set_history_query(true);
            if(lock_)
            {
                in.set_lock();
            }
            nsset_history_res = in.exec(ctx,local_timestamp_pg_time_zone_name);
        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_history_res;
    }

    std::string InfoNssetHistoryById::to_string() const
    {
        return Util::format_operation_state("InfoNssetHistoryById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoNssetHistoryByHistoryid::InfoNssetHistoryByHistoryid(unsigned long long historyid)
        : historyid_(historyid)
        , lock_(false)
    {}

    InfoNssetHistoryByHistoryid& InfoNssetHistoryByHistoryid::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoNssetOutput InfoNssetHistoryByHistoryid::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_history_res;

        try
        {
            InfoNsset in;
            in.set_inline_view_filter(Database::ParamQuery("info_nsset_historyid = ").param_bigint(historyid_))
                .set_history_query(true);
            if(lock_)
            {
                in.set_lock();
            }
            nsset_history_res = in.exec(ctx,local_timestamp_pg_time_zone_name);

            if (nsset_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_historyid(historyid_));
            }

            if (nsset_history_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_history_res.at(0);
    }

    std::string InfoNssetHistoryByHistoryid::to_string() const
    {
        return Util::format_operation_state("InfoNssetHistoryByHistoryid",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

}//namespace Fred

