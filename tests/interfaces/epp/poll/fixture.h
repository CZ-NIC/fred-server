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

#ifndef FIXTURE_H_392BF773C44544998E917DB3040CB769
#define FIXTURE_H_392BF773C44544998E917DB3040CB769

#include "tests/interfaces/epp/util.h"
#include "tests/setup/fixtures_utils.h"
#include "src/fredlib/poll/create_update_object_poll_message.h"

struct HasPollInfoMessage : virtual Test::autorollbacking_context
{
    HasPollInfoMessage()
    {
        Test::domain domain(ctx);
        Fred::Poll::CreateUpdateObjectPollMessage(domain.info_data.historyid).exec(ctx);
    }
};

struct HasTwoPollInfoMessages : virtual Test::autorollbacking_context
{
    HasTwoPollInfoMessages()
    {
        Test::domain domain(ctx);

        Fred::Poll::CreateUpdateObjectPollMessage(domain.info_data.historyid).exec(ctx);

        unsigned long long new_history_id =
            Fred::UpdateDomain(domain.info_data.fqdn,
                               domain.info_data.sponsoring_registrar_handle
                ).set_authinfo("doesntmatter").exec(ctx);
        Fred::Poll::CreateUpdateObjectPollMessage(new_history_id).exec(ctx);
    }
};

#endif
