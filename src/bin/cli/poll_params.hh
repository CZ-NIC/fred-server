/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @poll_params.h
 *  header of poll client implementation
 */

#ifndef POLL_PARAMS_HH_83FFE3CAB49344E6A3005AE265696847
#define POLL_PARAMS_HH_83FFE3CAB49344E6A3005AE265696847

#include "src/util/types/optional.hh"

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


#endif
