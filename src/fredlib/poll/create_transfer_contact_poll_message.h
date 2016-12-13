/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 *  create transfer contact poll message
 */

#ifndef CREATE_TRANSFER_CONTACT_POLL_MESSAGE_H_332D32A4E5D44DB78BAC68C268E7D97D
#define CREATE_TRANSFER_CONTACT_POLL_MESSAGE_H_332D32A4E5D44DB78BAC68C268E7D97D

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "util/printable.h"

namespace Fred {
namespace Poll {

class CreateTransferContactPollMessage : public Util::Printable
{
public:
    typedef unsigned long long ObjectHistoryId;

    DECLARE_EXCEPTION_DATA(contact_not_found, unsigned long long);
    DECLARE_EXCEPTION_DATA(object_history_not_found, unsigned long long);
    struct Exception
    :
        virtual Fred::OperationException,
        ExceptionData_contact_not_found< Exception >,
        ExceptionData_object_history_not_found< Exception >
    { };

    /**
    * @param _history_id specific history version of contact to which the new message shall be related
    */
    CreateTransferContactPollMessage(ObjectHistoryId _history_id);

    /**
    * @return id of newly created message
    * @throws Exception
    */
    unsigned long long exec(Fred::OperationContext &_ctx);

    /**
    * @return string with description of the instance state
    */
    std::string to_string() const;

private:
    ObjectHistoryId history_id_;
};

}
}

#endif
