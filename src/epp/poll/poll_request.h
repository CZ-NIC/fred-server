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

#ifndef POLL_REQUEST_H_EE8A599182D141B7987A82D5DBD174D1
#define POLL_REQUEST_H_EE8A599182D141B7987A82D5DBD174D1

#include "src/epp/impl/session_lang.h"
#include "util/decimal/decimal.h"
#include "src/fredlib/opcontext.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/variant.hpp>
#include <string>
#include <vector>

namespace Epp {
namespace Poll {

struct TransferEvent
{
    struct transfer_contact;
    struct transfer_nsset;
    struct transfer_domain;
    struct transfer_keyset;

    template<typename T>
    struct Data
    {
        boost::gregorian::date transfer_date;
        std::string object_handle;
        std::string dst_registrar_handle;
    };

    typedef boost::variant<
        Data<transfer_contact>,
        Data<transfer_nsset>,
        Data<transfer_domain>,
        Data<transfer_keyset>
        > Message;

    Message message;
};

struct MessageEvent
{
    struct delete_domain;
    struct delete_contact;
    struct validation;
    struct outzone;
    struct idle_delete_contact;
    struct idle_delete_nsset;
    struct idle_delete_domain;
    struct idle_delete_keyset;
    struct imp_expiration;
    struct expiration;
    struct imp_validation;

    template<typename T>
    struct Data
    {
        boost::gregorian::date date;
        std::string handle;
    };

    typedef boost::variant<
        Data<delete_domain>,
        Data<delete_contact>,
        Data<validation>,
        Data<outzone>,
        Data<idle_delete_contact>,
        Data<idle_delete_nsset>,
        Data<idle_delete_domain>,
        Data<idle_delete_keyset>,
        Data<imp_expiration>,
        Data<expiration>,
        Data<imp_validation>
        > Message;

    Message message;
};

struct LowCreditEvent
{
    std::string zone;
    Decimal credit;
    Decimal limit;
};

struct Test
{
    std::string testname;
    int status;
    std::string note;
};

struct TechCheckEvent
{
    std::string handle;
    std::vector<std::string> fqdns;
    std::vector<Test> tests;
};

struct RequestFeeInfoEvent
{
    boost::posix_time::ptime from;
    boost::posix_time::ptime to;
    unsigned long long free_count;
    unsigned long long used_count;
    Decimal price;
};

struct UpdateInfoEvent
{
    struct update_domain;
    struct update_nsset;
    struct update_keyset;

    template<typename T>
    struct Data
    {
        unsigned long long transaction_id;
        unsigned long long poll_id;
    };

    typedef boost::variant<
        Data<update_domain>,
        Data<update_nsset>,
        Data<update_keyset>
        > Message;

    Message message;
};


struct PollRequestOutputData
{
    typedef boost::variant<
        TransferEvent,
        MessageEvent,
        LowCreditEvent,
        TechCheckEvent,
        RequestFeeInfoEvent,
        UpdateInfoEvent
        > Message;

    unsigned long long message_id;
    boost::posix_time::ptime creation_time;
    unsigned long long number_of_unseen_messages;
    Message message;
};

PollRequestOutputData poll_request(
    Fred::OperationContext& _ctx,
    unsigned long long _registrar_id);

} // namespace Epp::Poll
} // namespace Epp

#endif
