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

#ifndef COPY_HISTORY_IMPL_HH_D67B5073A288476C964B05E8618BFE01
#define COPY_HISTORY_IMPL_HH_D67B5073A288476C964B05E8618BFE01

#include "src/libfred/opcontext.hh"


namespace LibFred
{
    /**
     * Copy record from table 'keyset' identified by _keyset_id to table 'keyset_history'. New record will use _historyid.
     * Copy record from table 'dsrecord' identified by _keyset_id to table 'dsrecord_history'. New record will use _historyid.
     * Copy record from table 'dnskey' identified by _keyset_id to table 'dnskey_history'. New record will use _historyid.
     * Copy record from table 'keyset_contact_map' identified by _keyset_id to table 'keyset_contact_map_history'. New record will use _historyid.
     * @param _historyid MUST be existing id in table 'history'
     */
    void copy_keyset_data_to_keyset_history_impl(
        LibFred::OperationContext& _ctx,
        unsigned long long _keyset_id,
        unsigned long long _historyid
    );
}

#endif
