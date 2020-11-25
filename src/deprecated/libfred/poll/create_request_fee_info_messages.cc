/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/libfred/poll/create_request_fee_info_messages.hh"
#include "libfred/poll/create_request_fee_info_message.hh"
#include "util/db/param_query_composition.hh"
#include "libfred/opexception.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"
#include "src/deprecated/libfred/requests/request_manager.hh"
#include "src/deprecated/libfred/registrable_object/domain.hh"

#include <boost/lexical_cast.hpp>
#include <memory>

namespace LibFred {
namespace Poll {

namespace {

struct DomainCounts
{
    unsigned long long count_free_base;
    unsigned long long count_free_per_domain;
};

DomainCounts get_domain_counts(LibFred::OperationContext& ctx, unsigned long long zone_id)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT count_free_base, count_free_per_domain "
              "FROM request_fee_parameter "
              "WHERE zone_id=").param_bigint(zone_id)
             (" AND valid_from <= now() ORDER BY valid_from DESC LIMIT 1");

    const Database::Result sql_query_result = ctx.get_conn().exec_params(sql_query);
    switch (sql_query_result.size())
    {
        case 0:
        {
            struct NotFound : OperationException
            {
                const char* what() const noexcept { return "failed to fetch domain counts from request_fee_parameter table"; }
            };
            throw NotFound();
        }
        case 1:
        {
            if (sql_query_result[0][0].isnull() || sql_query_result[0][1].isnull())
            {
                struct InvalidValue : OperationException
                {
                    const char* what() const noexcept { return "invalid domain counts fetched from request_fee_parameter table"; }
                };
                throw InvalidValue();
            }
            break;
        }
        default:
        {
            struct UnexpectedNumberOfRows : InternalError
            {
                UnexpectedNumberOfRows() : InternalError("unexpected number of rows") { }
            };
            throw UnexpectedNumberOfRows();
        }
    }

    DomainCounts ret;
    ret.count_free_base = static_cast<unsigned long long>(sql_query_result[0][0]);
    ret.count_free_per_domain = static_cast<unsigned long long>(sql_query_result[0][1]);
    return ret;
}

Decimal get_request_unit_price(LibFred::OperationContext& ctx, unsigned long long zone_id)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT price "
              "FROM price_list "
              "WHERE zone_id=").param_bigint(zone_id)
             (" AND valid_from <= now() AND (valid_to IS NULL OR valid_to > now())"
              " AND operation_id=").param_bigint(static_cast<int>(LibFred::Invoicing::INVOICING_GeneralOperation))
             (" ORDER BY valid_from DESC LIMIT 1");

    const Database::Result sql_query_result = ctx.get_conn().exec_params(sql_query);
    switch (sql_query_result.size())
    {
        case 0:
        {
            struct NotFound : OperationException
            {
                const char* what() const noexcept { return "failed to fetch price from price_list table"; }
            };
            throw NotFound();
        }
        case 1:
        {
            break;
        }
        default:
        {
            struct TooManyRows : InternalError
            {
                TooManyRows() : InternalError("too many rows fetched from price_list table") { }
            };
            throw TooManyRows();
        }
    }

    return static_cast<std::string>(sql_query_result[0][0]);
}

bool is_poll_request_fee_present(
        LibFred::OperationContext& ctx,
        const unsigned long long registrar_id,
        const boost::gregorian::date& period_from,
        const boost::gregorian::date& period_to,
        const std::string& time_zone)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT EXISTS (SELECT 0 "
              "FROM poll_request_fee prf "
              "JOIN message msg ON msg.id=prf.msgid "
              "WHERE msg.clid=").param_bigint(registrar_id)
             (" AND prf.period_from = (").param_timestamp(period_from)
             (" AT TIME ZONE ").param_text(time_zone)(") AT TIME ZONE 'UTC'"
              " AND prf.period_to = (").param_timestamp(period_to)
             (" AT TIME ZONE ").param_text(time_zone)(") AT TIME ZONE 'UTC')");

    const Database::Result sql_query_result = ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 1)
    {
        return static_cast<bool>(sql_query_result[0][0]);
    }

    struct UnexpectedNumberOfRows : InternalError
    {
        UnexpectedNumberOfRows() : InternalError("unexpected number of rows") { }
    };
    throw UnexpectedNumberOfRows();
}

void convert_timestamps_to_utc(
        LibFred::OperationContext& ctx,
        boost::posix_time::ptime &utc_from,
        boost::posix_time::ptime &utc_to,
        const boost::posix_time::ptime &local_from,
        const boost::posix_time::ptime &local_to,
        const std::string& time_zone)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT "
        "(").param_timestamp(local_from)(" AT TIME ZONE ").param_text(time_zone)(") AT TIME ZONE 'UTC'"
        "(").param_timestamp(local_to)(" AT TIME ZONE ").param_text(time_zone)(") AT TIME ZONE 'UTC'");
    const Database::Result db_result = ctx.get_conn().exec_params(sql_query);
    utc_from = db_result[0][0];
    utc_to = db_result[0][1];
}

} // namespace LibFred::Poll::{anonymous}

void create_request_fee_info_messages(
        LibFred::OperationContext& ctx,
        Logger::LoggerClient& logger_client,
        unsigned long long zone_id,
        const boost::optional<boost::gregorian::date>& period_to,
        const std::string& time_zone)
{
    boost::gregorian::date local_period_to;
    if (period_to)
    {
        local_period_to = *period_to;
    }
    else
    {
        local_period_to = boost::gregorian::day_clock::local_day();
    }

    boost::gregorian::date local_period_from;
    if (local_period_to.day() == 1)
    {
        local_period_from = local_period_to - boost::gregorian::months(1);
    }
    else
    {
        local_period_from = boost::gregorian::date(local_period_to.year(), local_period_to.month(), 1);
    }

    const DomainCounts domain_counts = get_domain_counts(ctx, zone_id);
    const Decimal request_unit_price = get_request_unit_price(ctx, zone_id);

    const boost::gregorian::date zone_access_date = local_period_to - boost::gregorian::days(1);
    Database::ParamQuery sql_query;
    sql_query("SELECT r.id, r.handle "
              "FROM registrar r "
              "JOIN registrarinvoice ri ON ri.registrarid=r.id "
              "WHERE ri.zone=").param_bigint(zone_id)
             (" AND ri.fromdate <= ").param_date(zone_access_date)
             (" AND (ri.todate >= ").param_date(zone_access_date)(" OR ri.todate IS NULL)");

    const Database::Result sql_query_result = ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        ctx.get_log().info("trying to create request fee info messages but no registrar was found");
        return;
    }

    const boost::posix_time::ptime ts_period_to(local_period_to);
    const boost::posix_time::ptime ts_period_from(local_period_from);

    boost::posix_time::ptime utc_ts_period_from;
    boost::posix_time::ptime utc_ts_period_to;
    convert_timestamps_to_utc(ctx, utc_ts_period_from, utc_ts_period_to, ts_period_from, ts_period_to, time_zone);
    const std::unique_ptr<LibFred::Logger::RequestCountInfo> request_counts =
        logger_client.getRequestCountUsers(utc_ts_period_to, utc_ts_period_from, "EPP");

    for (std::size_t i = 0; i < sql_query_result.size(); ++i)
    {
        const unsigned long long id = static_cast<unsigned long long>(sql_query_result[i][0]);
        const std::string handle = static_cast<std::string>(sql_query_result[i][1]);

        const LibFred::Logger::RequestCountInfo::const_iterator request_count_itr =
            request_counts->find(handle);
        const unsigned long long request_count =
            request_count_itr == request_counts->end() ? 0 : request_count_itr->second;

        const unsigned long long domain_count = LibFred::Domain::getRegistrarDomainCount(id, local_period_from, zone_id);

        const unsigned long long total_free_count = std::max(domain_counts.count_free_base,
                                                             domain_counts.count_free_per_domain * domain_count);

        const Decimal price = request_count > total_free_count
            ? Decimal(boost::lexical_cast<std::string>(request_count - total_free_count)) * request_unit_price
            : Decimal("0");

        if (!is_poll_request_fee_present(ctx, id, local_period_from, local_period_to, time_zone))
        {
            CreateRequestFeeInfoMessage(id, ts_period_from, ts_period_to, total_free_count, request_count, price, time_zone)
                .exec(ctx);
        }
    }
}

} // namespace LibFred::Poll
} // namespace LibFred
