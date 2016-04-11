/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  declaration of Fred::Messages::Generate class
 */

#ifndef GENERATE_H_919490122FEE4648D5B94BDAC5299EAC//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define GENERATE_H_919490122FEE4648D5B94BDAC5299EAC

#include "src/fredlib/opcontext.h"
#include "src/fredlib/public_request/public_request_lock_guard.h"
#include "src/fredlib/public_request/public_request_object_lock_guard.h"
#include "util/optional_value.h"
#include "src/fredlib/documents.h"
#include "src/fredlib/mailer.h"
#include "src/fredlib/messages/messages_impl.h"

#include <memory>

namespace MojeID {
namespace Messages {

typedef unsigned long long GeneralId;

struct CommChannel
{
    enum Value
    {
        SMS,
        EMAIL,
        LETTER,
    };
};

class Multimanager
{
public:
    template < typename MANAGER >
    MANAGER& select();
protected:
    virtual ~Multimanager() { }
private:
    virtual Fred::Document::Manager* document() = 0;
    virtual Fred::Mailer::Manager*   mailer() = 0;
    virtual Fred::Messages::Manager* messages() = 0;
    template < typename MANAGER, bool > struct traits;
};

class DefaultMultimanager:public Multimanager
{
public:
    DefaultMultimanager() { }
    ~DefaultMultimanager() { }
private:
    virtual Fred::Document::Manager* document();
    virtual Fred::Mailer::Manager*   mailer();
    virtual Fred::Messages::Manager* messages();

    std::auto_ptr< Fred::Mailer::Manager > mailer_manager_ptr_;
    std::auto_ptr< Fred::Document::Manager > document_manager_ptr_;
    Fred::Messages::ManagerPtr messages_manager_ptr_;
};

class Generate
{
public:
    typedef GeneralId MessageId;
    class message_checker
    {
    public:
        virtual void operator()(Fred::OperationContext &_ctx,
                                Fred::ObjectId _object_id)const = 0;
    protected:
        virtual ~message_checker() { }
    };
    struct message_checker_always_success:message_checker
    {
        void operator()(Fred::OperationContext&, Fred::ObjectId)const { }
        ~message_checker_always_success() { }
    };

    template < CommChannel::Value COMM_CHANNEL >
    struct Into
    {
        static void for_new_requests(
            Fred::OperationContext &_ctx,
            Multimanager &_multimanager,
            const message_checker &_check_message_limits = message_checker_always_success(),
            const std::string &_link_hostname_part = "");

        template < typename PUBLIC_REQUEST_TYPE >
        static MessageId for_given_request(
            Fred::OperationContext &_ctx,
            Multimanager &_multimanager,
            const Fred::LockedPublicRequest &_locked_request,
            const Fred::LockedPublicRequestsOfObject &_locked_contact,
            const message_checker &_check_message_limits = message_checker_always_success(),
            const std::string &_link_hostname_part = "",
            const Optional< GeneralId > &_contact_history_id = Optional< GeneralId >());
    };

    template < CommChannel::Value COMM_CHANNEL >
    static void enable(Fred::OperationContext &_ctx, bool flag);
};

}//namespace MojeID::Messages
}//namespace MojeID

#endif//GENERATE_H_919490122FEE4648D5B94BDAC5299EAC
