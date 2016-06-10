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
 */

#ifndef EPP_NSSET_CREATE_IMPL_H_f47dfe135a5a4dfd9ac14d870146e7c5
#define EPP_NSSET_CREATE_IMPL_H_f47dfe135a5a4dfd9ac14d870146e7c5

#include "src/fredlib/opcontext.h"

#include "src/epp/nsset/nsset_create.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

bool is_unspecified_ip_addr(const boost::asio::ip::address& ipaddr);

struct NssetCreateResult {
    const unsigned long long id;
    const unsigned long long create_history_id;
    // TODO guarantee non-special
    const boost::posix_time::ptime crdate;

    NssetCreateResult(
        unsigned long long _nsset_id,
        unsigned long long _create_history_id,
        const boost::posix_time::ptime& _nsset_crdate
    ) :
        id(_nsset_id),
        create_history_id(_create_history_id),
        crdate(_nsset_crdate)
    { }
};

/**
 * @throws AuthErrorServerClosingConnection
 * @throws ObjectExists
 * @throws AggregatedParamErrors
 */
NssetCreateResult nsset_create_impl(
    Fred::OperationContext& _ctx,
    const NssetCreateInputData& _data,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
);

}

#endif
