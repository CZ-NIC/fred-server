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

#ifndef GET_OBJECT_STATES_BY_HISTORY_ID_HH_BD592E4538FA457F808161FCB173E937
#define GET_OBJECT_STATES_BY_HISTORY_ID_HH_BD592E4538FA457F808161FCB173E937

#include "src/libfred/object_state/get_object_states.hh"
#include "src/libfred/opcontext.hh"

namespace LibFred {

class GetObjectStatesByHistoryId
{
public:
    GetObjectStatesByHistoryId(unsigned long long _history_id);
    typedef std::vector<ObjectStateData> ObjectState;
    struct Result
    {
        ObjectState object_state_at_begin;
        ObjectState object_state_at_end;
    };
    Result exec(OperationContext& _ctx);
private:
    const unsigned long long history_id_;
};

} // namespace LibFred

#endif
