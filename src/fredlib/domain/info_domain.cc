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
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/keyset/check_keyset.h"
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
            id.set_inline_view_filter(Database::ParamQuery(InfoDomain::GetAlias::fqdn())(" = LOWER(").param_text(fqdn_)(")")).set_history_query(false);

            if(lock_)
            {
                id.set_lock();
            }

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
            id.set_inline_view_filter(Database::ParamQuery(InfoDomain::GetAlias::id())(" = ").param_bigint(id_)).set_history_query(false);

            if(lock_)
            {
                id.set_lock();
            }

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

    InfoDomainHistoryByRoid::InfoDomainHistoryByRoid(const std::string& roid)
    : roid_(roid)
    , lock_(false)
    {}

    InfoDomainHistoryByRoid& InfoDomainHistoryByRoid::set_lock()
    {
        lock_ = true;
        return *this;
    }

    std::vector<InfoDomainOutput> InfoDomainHistoryByRoid::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            InfoDomain id;
            id.set_inline_view_filter(Database::ParamQuery(InfoDomain::GetAlias::roid())(" = ").param_text(roid_)).set_history_query(true);

            if(lock_)
            {
                id.set_lock();
            }

            domain_res = id.exec(ctx,local_timestamp_pg_time_zone_name);
        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res;
    }

    std::string InfoDomainHistoryByRoid::to_string() const
    {
        return Util::format_operation_state("InfoDomainHistoryByRoid",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid_))
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
            id.set_inline_view_filter(Database::ParamQuery(InfoDomain::GetAlias::id())(" = ").param_bigint(id_)).set_history_query(true);

            if(lock_)
            {
                id.set_lock();
            }

            domain_history_res = id.exec(ctx,local_timestamp_pg_time_zone_name);
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
            id.set_inline_view_filter(Database::ParamQuery(InfoDomain::GetAlias::historyid())(" = ").param_bigint(historyid_))
            .set_history_query(true);

            if(lock_)
            {
                id.set_lock();
            }

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


    InfoDomainByRegistrantHandle::InfoDomainByRegistrantHandle(const std::string& registrant_handle)
        : registrant_handle_(registrant_handle)
        , lock_(false)
    {}

    InfoDomainByRegistrantHandle& InfoDomainByRegistrantHandle::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoDomainByRegistrantHandle& InfoDomainByRegistrantHandle::set_limit(unsigned long long limit)
    {
        limit_ = Optional<unsigned long long>(limit);
        return *this;
    }

    std::vector<InfoDomainOutput> InfoDomainByRegistrantHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            if(Fred::CheckContact(registrant_handle_).is_invalid_handle())
            {
                BOOST_THROW_EXCEPTION(Exception().set_invalid_registrant_handle(registrant_handle_));
            }

            Database::ParamQuery inline_view_filter_query;

            inline_view_filter_query(
                "SELECT d.id FROM domain d "
                    " JOIN object_registry oreg ON d.registrant = oreg.id"
                        " AND oreg.name = UPPER(").param_text(registrant_handle_)(")"
                        " AND oreg.erdate IS NULL");

            if(limit_.isset())
            {
                inline_view_filter_query (" ORDER BY d.id LIMIT ").param_bigint(limit_.get_value());
            }

            std::pair<std::string,Database::query_param_list> domain_id_by_registrant_handle_param_query
                = inline_view_filter_query.get_query();

            Database::Result domain_id_res = ctx.get_conn().exec_params(
                domain_id_by_registrant_handle_param_query.first,
                domain_id_by_registrant_handle_param_query.second);

            if(domain_id_res.size() == 0)//no domain id found
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registrant_handle(registrant_handle_));
            }

            Database::ParamQuery domain_id_inline_view;
            domain_id_inline_view(InfoDomain::GetAlias::id())(" IN (");
            Util::HeadSeparator in_separator("",",");

            for (unsigned long long i = 0 ; i < domain_id_res.size(); ++i)
            {
                domain_id_inline_view(in_separator.get())
                    .param_bigint(static_cast<unsigned long long>(domain_id_res[i][0]));
            }
            domain_id_inline_view(")");



            InfoDomain id;
            id.set_inline_view_filter(domain_id_inline_view)
                .set_history_query(false);

            if(lock_)
            {
                id.set_lock();
            }

            domain_res = id.exec(ctx,local_timestamp_pg_time_zone_name);
        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res;
    }

    std::string InfoDomainByRegistrantHandle::to_string() const
    {
        return Util::format_operation_state("InfoDomainByRegistrantHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("registrant_handle", registrant_handle_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        (std::make_pair("limit",limit_.print_quoted()))
        );
    }




    InfoDomainByAdminContactHandle::InfoDomainByAdminContactHandle(const std::string& admin_contact_handle)
        : admin_contact_handle_(admin_contact_handle)
        , lock_(false)
    {}

    InfoDomainByAdminContactHandle& InfoDomainByAdminContactHandle::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoDomainByAdminContactHandle& InfoDomainByAdminContactHandle::set_limit(unsigned long long limit)
    {
        limit_ = Optional<unsigned long long>(limit);
        return *this;
    }

    std::vector<InfoDomainOutput> InfoDomainByAdminContactHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            if(Fred::CheckContact(admin_contact_handle_).is_invalid_handle())
            {
                BOOST_THROW_EXCEPTION(Exception().set_invalid_admin_contact_handle(admin_contact_handle_));
            }

            Database::ParamQuery inline_view_filter_query;

            inline_view_filter_query(
                "SELECT dcm.domainid"
                    " FROM object_registry oreg"
                        " JOIN  enum_object_type eot ON oreg.type = eot.id AND eot.name = 'contact'"
                        " JOIN domain_contact_map dcm ON dcm.contactid = oreg.id"
                    " WHERE oreg.name = UPPER(").param_text(admin_contact_handle_)(") AND oreg.erdate IS NULL");

            if(limit_.isset())
            {
                inline_view_filter_query (" ORDER BY dcm.domainid LIMIT ").param_bigint(limit_.get_value());
            }

            std::pair<std::string,Database::query_param_list> domain_id_by_admin_contact_handle_param_query
                = inline_view_filter_query.get_query();

            Database::Result domain_id_res = ctx.get_conn().exec_params(
                domain_id_by_admin_contact_handle_param_query.first,
                domain_id_by_admin_contact_handle_param_query.second);

            if(domain_id_res.size() == 0)//no domain id found
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_admin_contact_handle(admin_contact_handle_));
            }

            Database::ParamQuery domain_id_inline_view;
            domain_id_inline_view(InfoDomain::GetAlias::id())(" IN (");
            Util::HeadSeparator in_separator("",",");

            for (unsigned long long i = 0 ; i < domain_id_res.size(); ++i)
            {
                domain_id_inline_view(in_separator.get())
                    .param_bigint(static_cast<unsigned long long>(domain_id_res[i][0]));
            }
            domain_id_inline_view(")");

            InfoDomain id;
            id.set_inline_view_filter(domain_id_inline_view)
                .set_history_query(false);
            if(lock_)
            {
                id.set_lock();
            }

            domain_res = id.exec(ctx,local_timestamp_pg_time_zone_name);
        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res;
    }

    std::string InfoDomainByAdminContactHandle::to_string() const
    {
        return Util::format_operation_state("InfoDomainByAdminContactHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("admin_contact_handle", admin_contact_handle_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        (std::make_pair("limit",limit_.print_quoted()))
        );
    }


    InfoDomainByNssetHandle::InfoDomainByNssetHandle(const std::string& nsset_handle)
        : nsset_handle_(nsset_handle)
        , lock_(false)
    {}

    InfoDomainByNssetHandle& InfoDomainByNssetHandle::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoDomainByNssetHandle& InfoDomainByNssetHandle::set_limit(unsigned long long limit)
    {
        limit_ = Optional<unsigned long long>(limit);
        return *this;
    }

    std::vector<InfoDomainOutput> InfoDomainByNssetHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            if(Fred::CheckNsset(nsset_handle_).is_invalid_handle())
            {
                BOOST_THROW_EXCEPTION(Exception().set_invalid_nsset_handle(nsset_handle_));
            }

            Database::ParamQuery inline_view_filter_query;

            inline_view_filter_query(
                "SELECT d.id FROM domain d"
                    " JOIN object_registry noreg ON d.nsset = noreg.id AND noreg.name = UPPER(").param_text(nsset_handle_)(")"
                            " AND noreg.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) AND noreg.erdate IS NULL");

            if(limit_.isset())
            {
                inline_view_filter_query (" ORDER BY d.id LIMIT ").param_bigint(limit_.get_value());
            }

            std::pair<std::string,Database::query_param_list> domain_id_by_nsset_handle_param_query
                = inline_view_filter_query.get_query();

            Database::Result domain_id_res = ctx.get_conn().exec_params(
                domain_id_by_nsset_handle_param_query.first,
                domain_id_by_nsset_handle_param_query.second);

            if(domain_id_res.size() == 0)//no domain id found
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_nsset_handle(nsset_handle_));
            }

            Database::ParamQuery domain_id_inline_view;
            domain_id_inline_view(InfoDomain::GetAlias::id())(" IN (");
            Util::HeadSeparator in_separator("",",");

            for (unsigned long long i = 0 ; i < domain_id_res.size(); ++i)
            {
                domain_id_inline_view(in_separator.get())
                    .param_bigint(static_cast<unsigned long long>(domain_id_res[i][0]));
            }
            domain_id_inline_view(")");

            InfoDomain id;
            id.set_inline_view_filter(domain_id_inline_view)
                .set_history_query(false);

            if(lock_)
            {
                id.set_lock();
            }

            domain_res = id.exec(ctx,local_timestamp_pg_time_zone_name);
        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res;
    }

    std::string InfoDomainByNssetHandle::to_string() const
    {
        return Util::format_operation_state("InfoDomainByNssetHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("nsset_handle", nsset_handle_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        (std::make_pair("limit",limit_.print_quoted()))
        );
    }

    InfoDomainByKeysetHandle::InfoDomainByKeysetHandle(const std::string& keyset_handle)
        : keyset_handle_(keyset_handle)
        , lock_(false)
    {}

    InfoDomainByKeysetHandle& InfoDomainByKeysetHandle::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoDomainByKeysetHandle& InfoDomainByKeysetHandle::set_limit(unsigned long long limit)
    {
        limit_ = Optional<unsigned long long>(limit);
        return *this;
    }

    std::vector<InfoDomainOutput> InfoDomainByKeysetHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;
        try
        {
            if(Fred::CheckKeyset(keyset_handle_).is_invalid_handle())
            {
                BOOST_THROW_EXCEPTION(Exception().set_invalid_keyset_handle(keyset_handle_));
            }

            Database::ParamQuery inline_view_filter_query;

            inline_view_filter_query(
                "SELECT d.id FROM domain d"
                    " JOIN object_registry koreg ON d.keyset = koreg.id AND koreg.name = UPPER(").param_text(keyset_handle_)(")"
                        " AND koreg.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) AND koreg.erdate IS NULL");

            if(limit_.isset())
            {
                inline_view_filter_query (" ORDER BY d.id LIMIT ").param_bigint(limit_.get_value());
            }

            std::pair<std::string,Database::query_param_list> domain_id_by_keyset_handle_param_query
                = inline_view_filter_query.get_query();

            Database::Result domain_id_res = ctx.get_conn().exec_params(
                domain_id_by_keyset_handle_param_query.first,
                domain_id_by_keyset_handle_param_query.second);

            if(domain_id_res.size() == 0)//no domain id found
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_keyset_handle(keyset_handle_));
            }

            Database::ParamQuery domain_id_inline_view;
            domain_id_inline_view(InfoDomain::GetAlias::id())(" IN (");
            Util::HeadSeparator in_separator("",",");

            for (unsigned long long i = 0 ; i < domain_id_res.size(); ++i)
            {
                domain_id_inline_view(in_separator.get())
                    .param_bigint(static_cast<unsigned long long>(domain_id_res[i][0]));
            }
            domain_id_inline_view(")");

            InfoDomain id;
            id.set_inline_view_filter(domain_id_inline_view)
            .set_history_query(false);

            if(lock_)
            {
                id.set_lock();
            }

            domain_res = id.exec(ctx,local_timestamp_pg_time_zone_name);
        }
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res;
    }

    std::string InfoDomainByKeysetHandle::to_string() const
    {
        return Util::format_operation_state("InfoDomainByKeysetHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("keyset_handle", keyset_handle_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        (std::make_pair("limit",limit_.print_quoted()))
        );
    }


}//namespace Fred

