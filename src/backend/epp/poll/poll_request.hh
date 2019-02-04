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

#ifndef POLL_REQUEST_HH_D457514F0F9D48279E6263633651BD9C
#define POLL_REQUEST_HH_D457514F0F9D48279E6263633651BD9C

#include "src/backend/epp/session_lang.hh"
#include "util/decimal/decimal.hh"
#include "libfred/opcontext.hh"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/variant.hpp>
#include <string>
#include <vector>

namespace Epp {
namespace Poll {

struct TransferEvent
{
    struct TransferContact;
    struct TransferNsset;
    struct TransferDomain;
    struct TransferKeyset;

    template<typename T>
    struct Data
    {
        boost::gregorian::date transfer_date;
        std::string object_handle;
        std::string dst_registrar_handle;
    };

    typedef boost::variant<
        Data<TransferContact>,
        Data<TransferNsset>,
        Data<TransferDomain>,
        Data<TransferKeyset>
        > Message;

    Message message;
};

struct MessageEvent
{
    struct DeleteDomain;
    struct DeleteContact;
    struct Validation;
    struct Outzone;
    struct IdleDeleteContact;
    struct IdleDeleteNsset;
    struct IdleDeleteDomain;
    struct IdleDeleteKeyset;
    struct ImpExpiration;
    struct Expiration;
    struct ImpValidation;

    template<typename T>
    struct Data
    {
        boost::gregorian::date date;
        std::string handle;
    };

    typedef boost::variant<
        Data<DeleteDomain>,
        Data<DeleteContact>,
        Data<Validation>,
        Data<Outzone>,
        Data<IdleDeleteContact>,
        Data<IdleDeleteNsset>,
        Data<IdleDeleteDomain>,
        Data<IdleDeleteKeyset>,
        Data<ImpExpiration>,
        Data<Expiration>,
        Data<ImpValidation>
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
    Test(const std::string& _testname, const std::string& _note, int _status);
    bool is_test_successful()const;
    int get_status()const;
    std::string testname;
    std::string note;
private:
    int status;
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
    struct UpdateContact;
    struct UpdateDomain;
    struct UpdateNsset;
    struct UpdateKeyset;

    template<typename T>
    struct Data
    {
        unsigned long long transaction_id;
        unsigned long long poll_id;
    };

    typedef boost::variant<
        Data<UpdateContact>,
        Data<UpdateDomain>,
        Data<UpdateNsset>,
        Data<UpdateKeyset>
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
    LibFred::OperationContext& _ctx,
    unsigned long long _registrar_id);

} // namespace Epp::Poll
} // namespace Epp

#endif
