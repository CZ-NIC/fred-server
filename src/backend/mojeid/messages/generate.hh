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

#include "libfred/mailer.hh"
#include "libfred/opcontext.hh"
#include "libfred/public_request/public_request_lock_guard.hh"
#include "libfred/public_request/public_request_object_lock_guard.hh"
#include "src/backend/mojeid/messenger_configuration.hh"
#include "src/deprecated/libfred/documents.hh"
#include "src/deprecated/libfred/messages/messages_impl.hh"
#include "util/optional_value.hh"

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

class Multimanager
    : private boost::noncopyable
{
public:
    template <typename MANAGER>
    MANAGER& select();


protected:
    virtual ~Multimanager()
    {
    }

private:
    virtual LibFred::Document::Manager* document() = 0;
    virtual LibFred::Mailer::Manager* mailer() = 0;
    virtual LibFred::Messages::Manager* messages() = 0;

    template <typename MANAGER, bool>
    struct traits;

};

class DefaultMultimanager
    : public Multimanager
{
public:
    DefaultMultimanager()
    {
    }

    ~DefaultMultimanager()
    {
    }

private:
    virtual LibFred::Document::Manager* document();
    virtual LibFred::Mailer::Manager* mailer();
    virtual LibFred::Messages::Manager* messages();


    std::unique_ptr<LibFred::Mailer::Manager> mailer_manager_ptr_;
    std::unique_ptr<LibFred::Document::Manager> document_manager_ptr_;
    LibFred::Messages::ManagerPtr messages_manager_ptr_;
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
                LibFred::ObjectId) const
        {
        }

        ~message_checker_always_success()
        {
        }

    };

    template <CommChannel::Enum COMM_CHANNEL>
    struct Into
    {
        static void for_new_requests(
                LibFred::OperationContext& _ctx,
                Multimanager& _multimanager,
                const MojeId::MessengerConfiguration& _messenger_configuration,
                const message_checker& _check_message_limits = message_checker_always_success(),
                const std::string& _link_hostname_part = "");


        template <typename PUBLIC_REQUEST_TYPE>
        static MessageId for_given_request(
                LibFred::OperationContext& _ctx,
                Multimanager& _multimanager,
                const MojeId::MessengerConfiguration& _messenger_configuration,
                const LibFred::LockedPublicRequest& _locked_request,
                const LibFred::LockedPublicRequestsOfObject& _locked_contact,
                const message_checker& _check_message_limits = message_checker_always_success(),
                const std::string& _link_hostname_part = "",
                const Optional<GeneralId>& _contact_history_id = Optional<GeneralId>());


    };

    template <CommChannel::Enum COMM_CHANNEL>
    static void enable(
            LibFred::OperationContext& _ctx,
            bool flag);


};

} // namespace Fred::Backend::MojeId::Messages
} // namespace Fred::Backend::MojeId
} // namespace Fred::Backend
} // namespace Fred

#endif
