/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  declaration of LibFred::Messages::Generate class
 */

#ifndef GENERATE_HH_01DB2767767C4522BF6CA4CFBABEEC17
#define GENERATE_HH_01DB2767767C4522BF6CA4CFBABEEC17

#include "src/backend/mojeid/messenger_configuration.hh"
#include "src/deprecated/libfred/documents.hh"
#include "src/deprecated/libfred/messages/messages_impl.hh"
#include "util/optional_value.hh"

#include "libfred/mailer.hh"
#include "libfred/opcontext.hh"
#include "libfred/public_request/public_request_lock_guard.hh"
#include "libfred/public_request/public_request_object_lock_guard.hh"

#include <boost/noncopyable.hpp>

#include <memory>

namespace Fred {
namespace Backend {
namespace MojeId {
namespace Messages {

typedef unsigned long long GeneralId;

struct CommChannel
{
    enum Enum
    {
        sms,
        email,
        letter,

    };

};

class Generate
{
public:
    typedef GeneralId MessageId;
    class message_checker
    {
public:
        virtual void operator()(
                LibFred::OperationContext& _ctx,
                LibFred::ObjectId _object_id) const = 0;


protected:
        virtual ~message_checker()
        {
        }

    };

    struct message_checker_always_success
        : message_checker
    {
        void operator()(
                LibFred::OperationContext&,
                LibFred::ObjectId) const override
        {
        }

        ~message_checker_always_success()
        {
        }

    };

    static void for_new_requests(
            LibFred::OperationContext& _ctx,
            const MojeId::MessengerConfiguration& _messenger_configuration,
            const std::string& _link_hostname_part = "");

    template <CommChannel::Enum COMM_CHANNEL>
    struct Into
    {
        template <typename PUBLIC_REQUEST_TYPE>
        static void for_given_request(
                LibFred::OperationContext& _ctx,
                const MojeId::MessengerConfiguration& _messenger_configuration,
                const LibFred::LockedPublicRequest& _locked_request,
                const LibFred::LockedPublicRequestsOfObject& _locked_contact,
                const std::string& _link_hostname_part = "",
                const Optional<GeneralId>& _contact_history_id = Optional<GeneralId>());
    };
};

} // namespace Fred::Backend::MojeId::Messages
} // namespace Fred::Backend::MojeId
} // namespace Fred::Backend
} // namespace Fred

#endif
