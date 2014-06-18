/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @file create_domain_name_blacklist.cc
 *  create domain name blacklist
 */

#include "src/fredlib/domain/create_domain_name_blacklist.h"
#include "src/fredlib/object_state/get_blocking_status_desc_list.h"
#include "src/fredlib/object_state/get_object_state_id_map.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "src/fredlib/object.h"

#include <boost/algorithm/string.hpp>

namespace Fred
{

    CreateDomainNameBlacklist::CreateDomainNameBlacklist(const std::string &_domain,
        const std::string &_reason)
    :   domain_(_domain),
        reason_(_reason)
    {}

    CreateDomainNameBlacklist::CreateDomainNameBlacklist(const std::string &_domain,
        const std::string &_reason,
            const Optional< Time > &_valid_from,
            const Optional< Time > &_valid_to)
    :   domain_(_domain),
        reason_(_reason),
        valid_from_(_valid_from),
        valid_to_(_valid_to)
    {}

    CreateDomainNameBlacklist& CreateDomainNameBlacklist::set_valid_from(const Time &_valid_from)
    {
        valid_from_ = _valid_from;
        return *this;
    }
    
    CreateDomainNameBlacklist& CreateDomainNameBlacklist::set_valid_to(const Time &_valid_to)
    {
        valid_to_ = _valid_to;
        return *this;
    }
    
    namespace
    {

        bool is_blacklisted(Database::StandaloneConnection &_conn, const std::string &_domain,
                            const Optional< CreateDomainNameBlacklist::Time > &_valid_from,
                            const Optional< CreateDomainNameBlacklist::Time > &_valid_to)
        {
            Database::Result blacklisted_result;
            if (_valid_from.isset()) {
                if (_valid_to.isset()) {
                    blacklisted_result = _conn.exec_params(
                        "SELECT 1 " // <from, to)
                        "FROM domain_blacklist "
                        "WHERE LOWER(regexp)=LOWER($1::text) AND "
                              "($2::timestamp<valid_to OR valid_to IS NULL) AND "
                              "valid_from<$3::timestamp",
                        Database::query_param_list(_domain)
                            (_valid_from.get_value())(_valid_to.get_value()));
                }
                else {
                    blacklisted_result = _conn.exec_params(
                        "SELECT 1 " // <from, oo)
                        "FROM domain_blacklist "
                        "WHERE LOWER(regexp)=LOWER($1::text) AND "
                              "($2::timestamp<valid_to OR valid_to IS NULL)",
                        Database::query_param_list(_domain)
                            (_valid_from.get_value()));
                }
            }
            else {
                if (_valid_to.isset()) {
                    blacklisted_result = _conn.exec_params(
                        "SELECT 1 " // <now, to)
                        "FROM domain_blacklist "
                        "WHERE LOWER(regexp)=LOWER($1::text) AND "
                              "(CURRENT_TIMESTAMP<valid_to OR valid_to IS NULL) AND "
                              "valid_from<$2::timestamp",
                        Database::query_param_list(_domain)
                            (_valid_to.get_value()));
                }
                else {
                    blacklisted_result = _conn.exec_params(
                        "SELECT 1 " // <now, oo)
                        "FROM domain_blacklist "
                        "WHERE LOWER(regexp)=LOWER($1::text) AND "
                              "(CURRENT_TIMESTAMP<valid_to OR valid_to IS NULL)",
                        Database::query_param_list(_domain));
                }
            }
            return 0 < blacklisted_result.size();
        }
    }

    void CreateDomainNameBlacklist::exec(OperationContext &_ctx)
    {
        //check time
        if (valid_to_.isset()) {
            if (valid_from_.isset()) { // <from,to)
                if (valid_to_.get_value() < valid_from_.get_value()) {
                    std::string errmsg("valid from-to <");
                    errmsg += boost::posix_time::to_iso_string(valid_from_.get_value()) + ", " +
                              boost::posix_time::to_iso_string(valid_to_.get_value()) + ")";
                    BOOST_THROW_EXCEPTION(Exception().set_out_of_turn(errmsg));
                }
            }
            else { // <now,to)
                Database::Result out_of_turn_result = _ctx.get_conn().exec_params(
                        "SELECT $1::timestamp<CURRENT_TIMESTAMP",
                        Database::query_param_list(valid_to_.get_value()));
                if (bool(out_of_turn_result[0][0])) {
                    std::string errmsg("valid from-to <CURRENT_TIMESTAMP, ");
                    errmsg += boost::posix_time::to_iso_string(valid_to_.get_value()) + ")";
                    BOOST_THROW_EXCEPTION(Exception().set_out_of_turn(errmsg));
                }
            }
        }

        if (is_blacklisted(_ctx.get_conn(), domain_, valid_from_, valid_to_)) {
            BOOST_THROW_EXCEPTION(Exception().set_already_blacklisted_domain(domain_));
        }

        std::ostringstream cmd;
        cmd << "INSERT INTO domain_blacklist "
                   "(regexp,reason,valid_from,valid_to) "
               "VALUES "
                   "($1::text,$2::text";
        Database::query_param_list param(domain_);
        param(reason_);
        if (valid_from_.isset()) {
            param(valid_from_.get_value());
            cmd << ",$" << param.size() << "::timestamp";
        }
        else {
            cmd << ",CURRENT_TIMESTAMP";
        }
        if (valid_to_.isset()) {
            param(valid_to_.get_value());
            cmd << ",$" << param.size() << "::timestamp";
        }
        else {
            cmd << ",NULL";
        }
        cmd << ")";
        _ctx.get_conn().exec_params(cmd.str(), param);
    }//CreateDomainNameBlacklist::exec

}//namespace Fred
