/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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
 *  @poll_params.h
 *  header of poll client implementation
 */

#ifndef POLL_PARAMS_H_
#define POLL_PARAMS_H_

#include "util/types/optional.h"

/**
 * \class PollListAllArgs
 * \brief admin client poll_list_all params
 */
struct PollListAllArgs
{

    optional_ulong poll_type;
    optional_id registrar_id;
    bool poll_nonseen;
    bool poll_nonex;

    PollListAllArgs()
    : poll_nonseen(false)
    , poll_nonex(false)
    {}//ctor
    PollListAllArgs(
            const optional_ulong& _poll_type
            , const optional_id& _registrar_id
            , bool _poll_nonseen
            , bool _poll_nonex
            )
    : poll_type(_poll_type)
    , registrar_id(_registrar_id)
    , poll_nonseen(_poll_nonseen)
    , poll_nonex(_poll_nonex)
    {}//init ctor
};//struct PollListAllArgs

/**
 * \class PollCreateStatechangesArgs
 * \brief admin client poll_create_statechanges params
 */
struct PollCreateStatechangesArgs
{
    optional_string poll_except_types;
    optional_ulong poll_limit;
    bool poll_debug;

    PollCreateStatechangesArgs()
    : poll_debug(false)
    {}//ctor

    PollCreateStatechangesArgs(
            const optional_string& _poll_except_types
            , const optional_ulong _poll_limit
            , bool _poll_debug
            )
    : poll_except_types(_poll_except_types)
    , poll_limit(_poll_limit)
    , poll_debug(_poll_debug)
    {}//init ctor

};

struct PollCreateRequestFeeMessagesArgs
{
    optional_string poll_period_to;

    PollCreateRequestFeeMessagesArgs()
    { }

    PollCreateRequestFeeMessagesArgs(
            const optional_string &_poll_period_to
            )
    : poll_period_to(_poll_period_to)
    { }
};


#endif // BANK_PARAMS_H_
