/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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
 *  domain renew
 */

#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "src/fredlib/domain/renew_domain.h"
#include "src/fredlib/domain/copy_history_impl.h"
#include "src/fredlib/zone/zone.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/registrar/registrar_impl.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "util/printable.h"

namespace Fred
{
    RenewDomain::RenewDomain(const std::string& fqdn,
        const std::string& registrar,
        const boost::gregorian::date& expiration_date)
    : fqdn_(fqdn),
    registrar_(registrar),
    expiration_date_(expiration_date)
    {}

    RenewDomain::RenewDomain(const std::string& fqdn,
        const std::string& registrar,
        const boost::gregorian::date& expiration_date,
        const Optional<boost::gregorian::date>& enum_validation_expiration,
        const Optional<bool>& enum_publish_flag,
        const Optional<unsigned long long>& logd_request_id)
    : fqdn_(fqdn),
    registrar_(registrar),
    expiration_date_(expiration_date),
    enum_validation_expiration_(enum_validation_expiration),
    enum_publish_flag_(enum_publish_flag),
    logd_request_id_(logd_request_id.isset()
        ? Nullable<unsigned long long>(logd_request_id.get_value())
        : Nullable<unsigned long long>())
    {}

    RenewDomain& RenewDomain::set_enum_validation_expiration(const boost::gregorian::date& valexdate)
    {
        enum_validation_expiration_ = valexdate;
        return *this;
    }

    RenewDomain& RenewDomain::set_enum_publish_flag(bool enum_publish_flag)
    {
        enum_publish_flag_ = enum_publish_flag;
        return *this;
    }

    RenewDomain& RenewDomain::set_logd_request_id(unsigned long long logd_request_id)
    {
        logd_request_id_ = logd_request_id;
        return *this;
    }

    unsigned long long RenewDomain::exec(OperationContext& ctx)
    {
        unsigned long long history_id=0;//return
        try
        {
            //check registrar exists
            Registrar::get_registrar_id_by_handle(
                ctx, registrar_, static_cast<Exception*>(0)//set throw
                , &Exception::set_unknown_registrar_handle);

            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Zone::rem_trailing_dot(fqdn_);

        //get domain_id, ENUM flag and lock object_registry row for renew
        unsigned long long domain_id =0;
        bool is_enum_zone = false;
        {
            Database::Result domain_res = ctx.get_conn().exec_params(
                "SELECT oreg.id, z.enum_zone FROM domain d "
                " JOIN zone z ON z.id = d.zone "
                " JOIN object_registry oreg ON d.id = oreg.id "
                " WHERE oreg.type = get_object_type_id('domain'::text) "
                " AND oreg.name = LOWER($1::text) AND oreg.erdate IS NULL "
                " FOR UPDATE OF oreg"
                , Database::query_param_list(no_root_dot_fqdn));

            if (domain_res.size() == 0)
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_domain_fqdn(fqdn_));
            }
            if (domain_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("failed to get domain"));
            }

            domain_id = static_cast<unsigned long long>(domain_res[0][0]);
            is_enum_zone = static_cast<bool>(domain_res[0][1]);
        }

        if (!is_enum_zone)//check ENUM specific parameters
        {
            if(enum_validation_expiration_.isset()) {
                BOOST_THROW_EXCEPTION(InternalError("enum_validation_expiration set for non-ENUM domain"));
            }
            if(enum_publish_flag_.isset()) {
                BOOST_THROW_EXCEPTION(InternalError("enum_publish_flag set for non-ENUM domain"));
            }
        }

        history_id = Fred::InsertHistory(logd_request_id_, domain_id).exec(ctx);

        //object_registry historyid
        Database::Result update_historyid_res = ctx.get_conn().exec_params(
            "UPDATE object_registry SET historyid = $1::bigint "
                " WHERE id = $2::integer RETURNING id"
                , Database::query_param_list(history_id)(domain_id));
        if (update_historyid_res.size() != 1)
        {
            BOOST_THROW_EXCEPTION(Fred::InternalError("historyid update failed"));
        }

        //set new domain exdate
        if(expiration_date_.is_special())
        {
            BOOST_THROW_EXCEPTION(Exception().set_invalid_expiration_date(expiration_date_));
        }

        Database::Result update_domain_res = ctx.get_conn().exec_params(
            "UPDATE domain SET exdate = $1::date WHERE id = $2::integer RETURNING id",
                Database::query_param_list(expiration_date_)(domain_id));
        if (update_domain_res.size() != 1)
        {
            BOOST_THROW_EXCEPTION(InternalError("failed to update domain"));
        }

        //check valexdate if set
        if(enum_validation_expiration_.isset() && enum_validation_expiration_.get_value().is_special())
        {
            BOOST_THROW_EXCEPTION(Exception().set_invalid_enum_validation_expiration_date(enum_validation_expiration_.get_value()));
        }

        //update enumval
        if(enum_validation_expiration_.isset() || enum_publish_flag_.isset())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;
            Util::HeadSeparator set_separator(" SET "," , ");

            sql <<"UPDATE enumval ";

            if(enum_validation_expiration_.isset())
            {
                params.push_back(enum_validation_expiration_.get_value());
                sql << set_separator.get() << " exdate = $"
                    << params.size() << "::date ";
            }

            if(enum_publish_flag_.isset())
            {
                params.push_back(enum_publish_flag_.get_value());
                sql << set_separator.get() << " publish = $"
                    << params.size() << "::boolean ";
            }

            params.push_back(domain_id);
            sql << " WHERE domainid = $" << params.size() << "::integer RETURNING domainid";

            Database::Result update_enumval_res = ctx.get_conn().exec_params(sql.str(), params);
            if (update_enumval_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("failed to update enumval"));
            }
        }

        copy_domain_data_to_domain_history_impl(ctx, domain_id, history_id);

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return history_id;
    }//RenewDomain::exec

    std::string RenewDomain::to_string() const
    {
        return Util::format_operation_state("RenewDomain",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("fqdn",fqdn_))
        (std::make_pair("registrar",registrar_))
        (std::make_pair("expiration_date",boost::gregorian::to_iso_extended_string(expiration_date_)))
        (std::make_pair("enum_validation_expiration",enum_validation_expiration_.print_quoted()))
        (std::make_pair("enum_publish_flag",enum_publish_flag_.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id_.print_quoted()))
        );
    }

}//namespace Fred

