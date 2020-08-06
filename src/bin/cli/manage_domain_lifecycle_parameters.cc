/*
 * Copyright (C) 2020  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "src/bin/cli/manage_domain_lifecycle_parameters.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"

#include "src/util/cfg/config_handler_decl.hh"

#include "util/log/context.hh"

#include "libfred/opcontext.hh"

#include <time.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace {

using Time = std::chrono::system_clock::time_point;
using Day = std::chrono::duration<int, std::ratio<24 * 60 * 60>>;

static_assert(std::is_same<Time, DomainLifecycleParams::Time>::value, "Time declarations differ");
static_assert(std::is_same<Day, DomainLifecycleParams::Day>::value, "Day declarations differ");

std::string to_psql_timestamp(const Time& time)
{
    const auto t = std::chrono::system_clock::to_time_t(time);
    struct ::tm tm;
    if (::gmtime_r(&t, &tm) == nullptr)
    {
        throw std::runtime_error{"unable convert to time"};
    }
    const auto us = std::chrono::time_point_cast<std::chrono::microseconds>(time).time_since_epoch().count() % 1000000;
    std::ostringstream out;
    out << (1900 + tm.tm_year) << "-"
        << std::setw(2) << std::setfill('0') << (tm.tm_mon + 1) << "-"
        << std::setw(2) << std::setfill('0') << tm.tm_mday << " "
        << std::setw(2) << std::setfill('0') << tm.tm_hour << ":"
        << std::setw(2) << std::setfill('0') << tm.tm_min << ":"
        << std::setw(2) << std::setfill('0') << tm.tm_sec << "."
        << std::setw(6) << std::setfill('0') << us;
    return out.str();
}

std::string to_psql_interval(const Day& day)
{
    return std::to_string(day.count()) + "DAYS";
}

void exec_list_all(const LibFred::OperationContext& ctx)
{
    const auto dbres = ctx.get_conn().exec(
            "SELECT valid_for_exdate_after::DATE,"
                   "expiration_notify_period,"
                   "outzone_unguarded_email_warning_period,"
                   "expiration_dns_protection_period,"
                   "expiration_letter_warning_period,"
                   "expiration_registration_protection_period,"
                   "validation_notify1_period,"
                   "validation_notify2_period "
            "FROM domain_lifecycle_parameters "
            "ORDER BY valid_for_exdate_after");
    std::cout << " "
                 "valid_for_exdate_after | "
                 "expiration_notify_period | "
                 "outzone_unguarded_email_warning_period | "
                 "expiration_dns_protection_period | "
                 "expiration_letter_warning_period | "
                 "expiration_registration_protection_period | "
                 "validation_notify1_period | "
                 "validation_notify2_period" << std::endl
              << "------------------------+-"
                 "-------------------------+-"
                 "---------------------------------------+-"
                 "---------------------------------+-"
                 "---------------------------------+-"
                 "------------------------------------------+-"
                 "--------------------------+-"
                 "--------------------------" << std::endl;
    for (unsigned idx = 0; idx < dbres.size(); ++idx)
    {
        std::cout << std::setw(23) << std::right << static_cast<std::string>(dbres[idx][0]) << " |" <<
                     std::setw(25) << std::right << static_cast<std::string>(dbres[idx][1]) << " |" <<
                     std::setw(39) << std::right << static_cast<std::string>(dbres[idx][2]) << " |" <<
                     std::setw(33) << std::right << static_cast<std::string>(dbres[idx][3]) << " |" <<
                     std::setw(33) << std::right << static_cast<std::string>(dbres[idx][4]) << " |" <<
                     std::setw(42) << std::right << static_cast<std::string>(dbres[idx][5]) << " |" <<
                     std::setw(26) << std::right << static_cast<std::string>(dbres[idx][6]) << " |" <<
                     std::setw(26) << std::right << static_cast<std::string>(dbres[idx][7]) << std::endl;
    }
}

void exec_append_top(const LibFred::OperationContext& ctx, const DomainLifecycleParams::Command::AppendTop& options)
{
    options.check();
    const auto valid_for_exdate_after = to_psql_timestamp(options.valid_for_exdate_after());
    const auto expiration_notify_period = to_psql_interval(options.expiration_notify_period());
    const auto outzone_unguarded_email_warning_period = to_psql_interval(options.outzone_unguarded_email_warning_period());
    const auto expiration_dns_protection_period = to_psql_interval(options.expiration_dns_protection_period());
    const auto expiration_letter_warning_period = to_psql_interval(options.expiration_letter_warning_period());
    const auto expiration_registration_protection_period = to_psql_interval(options.expiration_registration_protection_period());
    const auto validation_notify1_period = to_psql_interval(options.validation_notify1_period());
    const auto validation_notify2_period = to_psql_interval(options.validation_notify2_period());
    const auto dbres = ctx.get_conn().exec_params(
            "INSERT INTO domain_lifecycle_parameters ("
                "valid_for_exdate_after,"
                "expiration_notify_period,"
                "outzone_unguarded_email_warning_period,"
                "expiration_dns_protection_period,"
                "expiration_letter_warning_period,"
                "expiration_registration_protection_period,"
                "validation_notify1_period,"
                "validation_notify2_period) "
            "SELECT $1::TIMESTAMP,"
                   "$2::INTERVAL,"
                   "$3::INTERVAL,"
                   "$4::INTERVAL,"
                   "$5::INTERVAL,"
                   "$6::INTERVAL,"
                   "$7::INTERVAL,"
                   "$8::INTERVAL "
            "FROM domain_lifecycle_parameters "
                 "HAVING COALESCE(MAX(valid_for_exdate_after)<$1::TIMESTAMP,True) AND "
                 "NOW()<=$1::TIMESTAMP "
            "RETURNING id,valid_for_exdate_after", Database::QueryParams{{
                    valid_for_exdate_after,
                    expiration_notify_period,
                    outzone_unguarded_email_warning_period,
                    expiration_dns_protection_period,
                    expiration_letter_warning_period,
                    expiration_registration_protection_period,
                    validation_notify1_period,
                    validation_notify2_period}});
    if (dbres.size() != 1)
    {
        throw std::runtime_error{"nothing to append, conditions not satisfied"};
    }
}

void exec_delete_top(const LibFred::OperationContext& ctx)
{
    const auto dbres = ctx.get_conn().exec(
            "DELETE FROM domain_lifecycle_parameters "
            "WHERE valid_for_exdate_after=(SELECT MAX(valid_for_exdate_after) "
                                          "FROM domain_lifecycle_parameters "
                                               "HAVING NOW()<MAX(valid_for_exdate_after)) "
            "RETURNING valid_for_exdate_after::DATE");
    if (dbres.size() != 1)
    {
        throw std::runtime_error{"nothing to delete, conditions not satisfied"};
    }
}

}//namespace {anonymous}

void manage_domain_lifecycle_parameters()
{
    try
    {
        struct NoOperation : std::runtime_error
        {
            NoOperation() : std::runtime_error{"no operation on domain_lifecycle_parameters selected"} {}
        };

        const auto command_option = CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientDomainLifecycleParametersArgsGrp>()->get_params();
        if (!command_option.has_command())
        {
            throw NoOperation{};
        }
        const Logging::Context log_ctx{__func__};
        LibFred::OperationContextCreator ctx;
        class Executor : public boost::static_visitor<>
        {
        public:
            explicit Executor(const LibFred::OperationContext& ctx)
                : ctx_{ctx}
            { }
            void operator()(const DomainLifecycleParams::Command::ListAll&) const
            {
                exec_list_all(ctx_);
            }
            void operator()(const DomainLifecycleParams::Command::AppendTop& options) const
            {
                exec_append_top(ctx_, options);
            }
            void operator()(const DomainLifecycleParams::Command::DeleteTop&) const
            {
                exec_delete_top(ctx_);
            }
            void operator()(const boost::blank&) const
            {
                throw NoOperation{};
            }
        private:
            const LibFred::OperationContext& ctx_;
        };
        boost::apply_visitor(Executor{ctx}, command_option.command);
        ctx.commit_transaction();
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error{std::string{"exception: "} + e.what()};
    }
    catch (...)
    {
        throw std::runtime_error{"exception: unknown error"};
    }
}
