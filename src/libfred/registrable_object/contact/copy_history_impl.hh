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

#ifndef COPY_HISTORY_IMPL_HH_25A19CC50B82473AAD0A727ED0B278E2
#define COPY_HISTORY_IMPL_HH_25A19CC50B82473AAD0A727ED0B278E2

#include "src/libfred/opcontext.hh"


namespace LibFred
{
    /**
     * Copies record from table 'contact' identified by _contact_id to table 'contact_history'. New record will use _historyid.
     * Copies record from table 'contact_address' identified by _contact_id to table 'contact_address_history'. New record will use _historyid.
     * @param _historyid MUST be existing id in table 'history'
     */
    void copy_contact_data_to_contact_history_impl(
        LibFred::OperationContext& _ctx,
        unsigned long long _contact_id,
        unsigned long long _historyid
    );
}

#endif
