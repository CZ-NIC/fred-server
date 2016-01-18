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
 *  contact info
 */

#include <string>
#include <vector>

#include "info_contact.h"
#include "info_contact_impl.h"

#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{

    InfoContactByHandle::InfoContactByHandle(const std::string& handle)
        : handle_(handle)
        , lock_(false)
    {}

    InfoContactByHandle& InfoContactByHandle::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoContactOutput InfoContactByHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoContactOutput> contact_res;

        try
        {
            InfoContact ic;
            ic.set_inline_view_filter(Database::ParamQuery(InfoContact::GetAlias::handle())(" = UPPER(").param_text(handle_)(")"))
            .set_history_query(false);

            if(lock_)
            {
                ic.set_lock();
            }

            contact_res = ic.exec(ctx,local_timestamp_pg_time_zone_name);

            if (contact_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_contact_handle(handle_));
            }

            if (contact_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return contact_res.at(0);
    }

    std::string InfoContactByHandle::to_string() const
    {
        return Util::format_operation_state("InfoContactByHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle", handle_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoContactById::InfoContactById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    InfoContactById& InfoContactById::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoContactOutput InfoContactById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoContactOutput> contact_res;

        try
        {
            InfoContact ic;
            ic.set_inline_view_filter(Database::ParamQuery(InfoContact::GetAlias::id())(" = ").param_bigint(id_))
            .set_history_query(false);

            if(lock_)
            {
                ic.set_lock();
            }

            contact_res = ic.exec(ctx,local_timestamp_pg_time_zone_name);

            if (contact_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

            if (contact_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return contact_res.at(0);
    }

    std::string InfoContactById::to_string() const
    {
        return Util::format_operation_state("InfoContactById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoContactHistoryByRoid::InfoContactHistoryByRoid(const std::string& roid)
        : roid_(roid)
        , lock_(false)
    {}

    InfoContactHistoryByRoid& InfoContactHistoryByRoid::set_lock()
    {
        lock_ = true;
        return *this;
    }

    std::vector<InfoContactOutput> InfoContactHistoryByRoid::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoContactOutput> contact_history_res;

        try
        {
            InfoContact ic;
            ic.set_inline_view_filter(Database::ParamQuery(InfoContact::GetAlias::roid())(" = ").param_text(roid_))
            .set_history_query(true);

            if(lock_)
            {
                ic.set_lock();
            }

            contact_history_res = ic.exec(ctx,local_timestamp_pg_time_zone_name);

            if (contact_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registry_object_identifier(roid_));
            }
        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return contact_history_res;
    }

    std::string InfoContactHistoryByRoid::to_string() const
    {
        return Util::format_operation_state("InfoContactHistoryByRoid",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoContactHistoryById::InfoContactHistoryById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    InfoContactHistoryById& InfoContactHistoryById::set_lock()
    {
        lock_ = true;
        return *this;
    }

    std::vector<InfoContactOutput> InfoContactHistoryById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoContactOutput> contact_history_res;

        try
        {
            InfoContact ic;
            ic.set_inline_view_filter(Database::ParamQuery(InfoContact::GetAlias::id())(" = ").param_bigint(id_))
            .set_history_query(true);

            if(lock_)
            {
                ic.set_lock();
            }

            contact_history_res = ic.exec(ctx,local_timestamp_pg_time_zone_name);

            if (contact_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return contact_history_res;
    }

    std::string InfoContactHistoryById::to_string() const
    {
        return Util::format_operation_state("InfoContactHistoryById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoContactHistoryByHistoryid::InfoContactHistoryByHistoryid(unsigned long long historyid)
        : historyid_(historyid)
        , lock_(false)
    {}

    InfoContactHistoryByHistoryid& InfoContactHistoryByHistoryid::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoContactOutput InfoContactHistoryByHistoryid::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoContactOutput> contact_history_res;

        try
        {
            InfoContact ic;
            ic.set_inline_view_filter(Database::ParamQuery(InfoContact::GetAlias::historyid())(" = ").param_bigint(historyid_))
            .set_history_query(true);

            if(lock_)
            {
                ic.set_lock();
            }

            contact_history_res = ic.exec(ctx,local_timestamp_pg_time_zone_name);

            if (contact_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_historyid(historyid_));
            }

            if (contact_history_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return contact_history_res.at(0);
    }

    std::string InfoContactHistoryByHistoryid::to_string() const
    {
        return Util::format_operation_state("InfoContactHistoryByHistoryid",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

}//namespace Fred

