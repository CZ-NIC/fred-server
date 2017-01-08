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

#ifndef CREATE_CONTACT_H_04AAC9626F1042B0B8B6CFCDB9220277
#define CREATE_CONTACT_H_04AAC9626F1042B0B8B6CFCDB9220277

#include "src/fredlib/opcontext.h"

#include "src/epp/contact/create_contact_localized.h"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>

namespace Epp {
namespace Contact {

struct CreateContactResult
{
    CreateContactResult(unsigned long long _contact_id,
        unsigned long long _create_history_id,
        const boost::posix_time::ptime& _contact_crdate)
    :   id(_contact_id),
        create_history_id(_create_history_id),
        crdate(_contact_crdate)
    { }
    const unsigned long long id;
    const unsigned long long create_history_id;
    // TODO guarantee non-special
    const boost::posix_time::ptime crdate;
};

CreateContactResult create_contact(
        Fred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const CreateContactInputData& _data,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id);

} // namespace Epp::Contact
} // namespace Epp

#endif
