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
 */

#ifndef EPP_CONTACT_CREATE_IMPL_H_548537357434
#define EPP_CONTACT_CREATE_IMPL_H_548537357434

#include "src/epp/contact/ident_type.h"
#include "src/fredlib/opcontext.h"

#include "src/epp/contact/contact_create.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

struct ContactCreateResult {
    const unsigned long long id;
    const unsigned long long create_history_id;
    // TODO guarantee non-special
    const boost::posix_time::ptime crdate;

    ContactCreateResult(
        unsigned long long _contact_id,
        unsigned long long _create_history_id,
        const boost::posix_time::ptime& _contact_crdate
    ) :
        id(_contact_id),
        create_history_id(_create_history_id),
        crdate(_contact_crdate)
    { }
};

/**
 * @throws AuthErrorServerClosingConnection
 * @throws ObjectExists
 * @throws AggregatedParamErrors
 */
ContactCreateResult contact_create_impl(
    Fred::OperationContext& _ctx,
    const ContactCreateInputData& _data,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
);

}

#endif
