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

#include "src/fredlib/poll/create_request_fee_message.h"
#include "util/db/param_query_composition.h"
#include "util/db/query_param.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/invoicing/invoice.h"
#include "src/fredlib/requests/request_manager.h"
#include "src/fredlib/domain.h"
#include "src/fredlib/poll/message_type.h"

#include <boost/lexical_cast.hpp>

#include <map>

namespace Fred {
namespace Poll {

namespace {

struct DomainCounts
{
    unsigned long long count_free_base;
    unsigned long long count_free_per_domain;
};

DomainCounts get_domain_counts(Fred::OperationContext& _ctx, unsigned long long _zone_id)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT count_free_base, count_free_per_domain"
              "FROM request_fee_parameter "
              "WHERE zone_id=").param_bigint(_zone_id)
             (" AND valid_from < now() ORDER BY valid_from DESC LIMIT 1");

    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    switch (sql_query_result.size())
    {
        case 0:
        {
            struct NotFound : OperationException
            {
                const char* what() const throw() { return "no record in request_fee_parameter table"; }
            };
            throw NotFound();
        }
        case 1:
        {
            if (sql_query_result[0][0].isnull() || sql_query_result[0][1].isnull())
            {
                struct InvalidValue : OperationException
                {
                    const char* what() const throw() { return "invalid record in request_fee_parameter table"; }
                };
                throw InvalidValue();
            }
            break;
        }
        default:
        {
            struct TooManyRows : InternalError
            {
                TooManyRows() : InternalError(std::string()) { }
                const char* what() const throw() { return "too many rows in request_fee_parameter table"; }
            };
            throw TooManyRows();
        }
    }

    DomainCounts ret;
    ret.count_free_base = static_cast<unsigned long long>(sql_query_result[0][0]);
    ret.count_free_per_domain = static_cast<unsigned long long>(sql_query_result[0][1]);
    return ret;
}

Decimal get_request_unit_price(Fred::OperationContext& _ctx, unsigned long long _zone_id)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT price "
              "FROM price_list "
              "WHERE zone_id=").param_bigint(_zone_id)
             (" AND valid_from < now() AND (valid_to IS NULL OR valid_to > now())"
              " AND operation_id=").param_bigint(static_cast<int>(Fred::Invoicing::INVOICING_GeneralOperation))
             (" ORDER BY valid_from DESC LIMIT 1");

    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    switch (sql_query_result.size())
    {
        case 0:
        {
            struct NotFound : OperationException
            {
                const char* what() const throw() { return "no record in price_list table"; }
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
                TooManyRows() : InternalError(std::string()) { }
                const char* what() const throw() { return "too many rows in price_list table"; }
            };
            throw TooManyRows();
        }
    }

    return static_cast<std::string>(sql_query_result[0][0]);
}

bool is_poll_request_fee_present(
    Fred::OperationContext& _ctx,
    const unsigned long long _registrar_id,
    const boost::gregorian::date& _period_from,
    const boost::gregorian::date& _period_to)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT EXISTS (SELECT id "
              "FROM poll_request_fee prf "
              "JOIN message msg ON msg.id=prf.msgid "
              "WHERE clid=").param_bigint(_registrar_id)
             (" AND period_from = (").param_timestamp(_period_from)(" AT TIME ZONE 'Europe/Prague') AT TIME ZONE 'UTC'"
              " AND period_to   = (").param_timestamp(_period_to)("   AT TIME ZONE 'Europe/Prague') AT TIME ZONE 'UTC')");

    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    switch (sql_query_result.size())
    {
        case 0:
        {
            struct NotFound : OperationException
            {
                const char* what() const throw() { return "no record in poll_request_fee table"; }
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
                TooManyRows() : InternalError(std::string()) { }
                const char* what() const throw() { return "too many rows in poll_request_fee table"; }
            };
            throw TooManyRows();
        }
    }

    return static_cast<bool>(sql_query_result[0][0]);
}

} // namespace Fred::Poll::{anonymous}

unsigned long long CreateRequestFeeInfoMessage::exec(OperationContext& _ctx)
{
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(
        "WITH create_new_message AS ("
            "INSERT INTO message (id, clid, crdate, exdate, msgtype) "
            "VALUES (nextval('message_id_seq'::regclass), $1::bigint, "
            "current_timestamp, current_timestamp + interval '7 days', "
            "(SELECT id FROM messagetype WHERE name=$2::text)) "
            "RETURNING id AS msgid) "
        "INSERT INTO poll_request_fee "
        "(msgid, period_from, period_to, total_free_count, used_count, price) "
        "VALUES ((SELECT msgid FROM create_new_message), $3::timestamp, "
        "$4::timestamp, $5::bigint, $6::bigint, $7::numeric(10,2)) "
        "RETURNING msgid",
        Database::query_param_list
        (registrar_id)
        (Conversion::Enums::to_db_handle(MessageType::request_fee_info))
        (period_from)
        (period_to)
        (total_free_count)
        (request_count)
        (price.get_string()));

    if (sql_query_result.size() == 1)
    {
        return static_cast<unsigned long long>(sql_query_result[0][0]);
    }

    struct UnexpectedNumberOfRows : InternalError
    {
        UnexpectedNumberOfRows() : InternalError(std::string()) { }
        const char* what() const throw() { return "unexpected number of rows"; }
    };
    throw UnexpectedNumberOfRows();
}

void create_request_fee_info_messages(
    Fred::OperationContext& _ctx,
    Logger::LoggerClient& _logger_client, // sigh
    boost::gregorian::date _period_to,
    unsigned long long _zone_id)
{
    if (_period_to.is_special())
    {
        _period_to = boost::gregorian::day_clock::local_day();
    }

    boost::gregorian::date period_from;
    if (_period_to.day() == 1)
    {
        period_from = _period_to - boost::gregorian::months(1);
    }
    else
    {
        period_from = boost::gregorian::date(_period_to.year(), _period_to.month(), 1);
    }

    const DomainCounts domain_counts = get_domain_counts(_ctx, _zone_id);
    const Decimal request_unit_price = get_request_unit_price(_ctx, _zone_id);

    const boost::gregorian::date zone_access_date = _period_to - boost::gregorian::days(1);
    Database::ParamQuery sql_query;
    sql_query("SELECT r.id, r.handle "
              "FROM registrar r "
              "JOIN registrarinvoice ri ON ri.registrarid=r.id "
              "WHERE ri.zone=").param_bigint(_zone_id)
             (" AND ri.fromdate <= ").param_date(zone_access_date)
             (" AND (ri.todate >= ").param_date(zone_access_date)(" OR ri.todate IS NULL)");

    const Database::Result sql_query_result = _ctx.get_conn().exec_params(sql_query);
    if (sql_query_result.size() == 0)
    {
        struct NotFound : OperationException
        {
            const char* what() const throw() { return "no registrar found"; }
        };
        throw NotFound();
    }

    const boost::posix_time::ptime ts_period_to(_period_to);
    const boost::posix_time::ptime ts_period_from(period_from);

    // I wish I could get rid of the auto_ptr; also getRequestCountUsers is not a const overload
    const Fred::Logger::RequestCountInfo request_counts =
        *_logger_client.getRequestCountUsers(ts_period_to, ts_period_from, "EPP");

    for (std::size_t i = 0; i < sql_query_result.size(); ++i)
    {
        const unsigned long long id = static_cast<unsigned long long>(sql_query_result[i][0]);
        const std::string handle = static_cast<std::string>(sql_query_result[i][1]);

        const Fred::Logger::RequestCountInfo::const_iterator it = request_counts.find(handle);
        const unsigned long long request_count = it == request_counts.end() ? 0 : it->second;

        const unsigned long long domain_count = Fred::Domain::getRegistrarDomainCount(id, period_from, _zone_id);

        const unsigned long long total_free_count = std::max(domain_counts.count_free_base,
                                                             domain_counts.count_free_per_domain * domain_count);

        const Decimal price = request_count > total_free_count
            ? Decimal(boost::lexical_cast<std::string>(request_count - total_free_count)) * request_unit_price
            : Decimal("0");

        if (!is_poll_request_fee_present(_ctx, id, period_from, _period_to))
        {
            CreateRequestFeeInfoMessage(id, ts_period_from, ts_period_to, total_free_count, request_count, price).exec(_ctx);
        }
    }
}

} // namespace Fred::Poll
} // namespace Fred
