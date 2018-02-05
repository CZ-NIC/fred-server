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

#include "src/libfred/poll/get_request_fee_message.hh"
#include "src/libfred/opexception.hh"
#include "src/libfred/poll/message_type.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/util/db/param_query_composition.hh"

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/version.hpp>

#include <string>

namespace LibFred {
namespace Poll {

namespace {

struct period_to_with_tz_t
{
    const boost::posix_time::ptime& period_to;
    const std::string& tz;
};

#ifdef BOOST_VERSION
#if (BOOST_VERSION < 105600)
// The (in)equality comparison with boost::none does not require that T be EqualityComparable (fixed in boost-1.56.0)
inline bool operator==(const boost::optional<period_to_with_tz_t>& optional, boost::none_t)
{
    return static_cast<bool>(optional);
}
#endif
#endif

RequestFeeInfoEvent get_request_fee_info_message_impl(
        LibFred::OperationContext& ctx,
        unsigned long long registrar_id,
        const boost::optional<period_to_with_tz_t>& period_to_with_tz)
{
    Database::ParamQuery sql_query;
    sql_query("SELECT prf.period_from, prf.period_to, prf.total_free_count, prf.used_count, prf.price "
              "FROM poll_request_fee prf "
              "JOIN message m ON m.id=prf.msgid "
              "JOIN messagetype mt ON mt.id=m.msgtype "
              "WHERE m.clid=").param_bigint(registrar_id)
             (" AND mt.name=").param_text(Conversion::Enums::to_db_handle(MessageType::request_fee_info));
    if (period_to_with_tz == boost::none)
    {
        sql_query(" ORDER BY m.id DESC LIMIT 1");
    }
    else
    {
        sql_query(" AND period_to = (");
        sql_query.param_timestamp(period_to_with_tz->period_to)(" AT TIME ZONE ")
            .param_text(period_to_with_tz->tz)(" AT TIME ZONE 'UTC')");
    }


    const Database::Result sql_query_result = ctx.get_conn().exec_params(sql_query);
    switch (sql_query_result.size())
    {
        case 0:
        {
            class NotFound : public OperationException
            {
            public:
                explicit NotFound(const std::string& _message) : message_(_message) {}
                const char* what() const noexcept { return message_.c_str(); }
            private:
                const std::string message_;
            };
            const std::string repr_registrar_id = boost::lexical_cast<std::string>(registrar_id);
            std::string registrar_handle;
            try
            {
                registrar_handle = LibFred::InfoRegistrarById(registrar_id).exec(ctx).info_registrar_data.handle;
            }
            catch (...)
            {
                registrar_handle = "<unknown handle>";
            }
            throw NotFound("Poll request fee message for registrar "
                           + registrar_handle
                           + " (" + repr_registrar_id + ") "
                           + (period_to_with_tz == boost::none
                              ? std::string("(the last message)")
                              : "with period_to " + to_simple_string(period_to_with_tz->period_to))
                           + " not found");
        }
        case 1:
            break;
        default:
        {
            struct TooManyRows : InternalError
            {
                TooManyRows() : InternalError("too many rows") { }
            };
            throw TooManyRows();
        }
    }

    RequestFeeInfoEvent ret;
    ret.from = boost::posix_time::time_from_string(static_cast<std::string>(sql_query_result[0][0]));
    ret.to = boost::posix_time::time_from_string(static_cast<std::string>(sql_query_result[0][1]));
    ret.free_count = static_cast<unsigned long long>(sql_query_result[0][2]);
    ret.used_count = static_cast<unsigned long long>(sql_query_result[0][3]);
    ret.price = static_cast<std::string>(sql_query_result[0][4]);
    return ret;
}

} // namespace LibFred::Poll::{anonymous}

RequestFeeInfoEvent get_request_fee_info_message(
        LibFred::OperationContext& ctx,
        unsigned long long registrar_id,
        const boost::posix_time::ptime& period_to,
        const std::string& time_zone)
{
    const period_to_with_tz_t period_to_with_tz = { period_to, time_zone };
    return get_request_fee_info_message_impl(ctx, registrar_id, period_to_with_tz);
}

RequestFeeInfoEvent get_last_request_fee_info_message(
        LibFred::OperationContext& ctx,
        unsigned long long registrar_id)
{
    return get_request_fee_info_message_impl(ctx, registrar_id, boost::none);
}

} // namespace LibFred::Poll
} // namespace LibFred
