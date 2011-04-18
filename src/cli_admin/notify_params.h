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
 *  @notify_params.h
 *  header of notify client implementation
 */

#ifndef NOTIFY_PARAMS_H_
#define NOTIFY_PARAMS_H_

#include "util/types/optional.h"

/**
 * \class NotifyStateChangesArgs
 * \brief admin client notify_state_changes params
 */
struct NotifyStateChangesArgs
{
    optional_string notify_except_types;
    optional_ulonglong notify_limit;
    bool notify_debug;
    bool notify_use_history_tables;

    NotifyStateChangesArgs()
    : notify_debug(false)
    , notify_use_history_tables(false)
    {}//ctor
    NotifyStateChangesArgs(
            const optional_string& _notify_except_types
            , const optional_ulonglong _notify_limit
            , bool _notify_debug
            , bool _notify_use_history_tables
            )
    : notify_except_types(_notify_except_types)
    , notify_limit(_notify_limit)
    , notify_debug(_notify_debug)
    , notify_use_history_tables(_notify_use_history_tables)
    {}//init ctor
};//struct NotifyStateChangesArgs

/**
 * \class SmsSendArgs
 * \brief admin client notify_sms_send params
 */
struct SmsSendArgs
{
    optional_string sms_command;
    optional_string command;

    SmsSendArgs()
    {}//ctor
    SmsSendArgs(
            const optional_string& _sms_command
            , const optional_string& _command
            )
    : sms_command(_sms_command)
    , command(_command)
    {}//init ctor
};//struct SmsSendArgs

#endif // NOTIFY_PARAMS_H_
