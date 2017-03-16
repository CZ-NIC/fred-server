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

/**
 *  @file
 *  create transfer keyset poll message
 */

#ifndef CREATE_TRANSFER_KEYSET_POLL_MESSAGE_H_92502F5F5D9B4E89B4654F2078B666DF
#define CREATE_TRANSFER_KEYSET_POLL_MESSAGE_H_92502F5F5D9B4E89B4654F2078B666DF

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "util/printable.h"

namespace Fred {
namespace Poll {

class CreateTransferKeysetPollMessage : public Util::Printable
{
public:
    typedef unsigned long long ObjectHistoryId;

    DECLARE_EXCEPTION_DATA(keyset_not_found, unsigned long long);
    DECLARE_EXCEPTION_DATA(object_history_not_found, unsigned long long);
    struct Exception
    :
        virtual Fred::OperationException,
        ExceptionData_keyset_not_found< Exception >,
        ExceptionData_object_history_not_found< Exception >
    { };

    /**
    * @param _history_id specific history version of a keyset to which the new message shall be related
    */
    CreateTransferKeysetPollMessage(ObjectHistoryId _history_id);

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
