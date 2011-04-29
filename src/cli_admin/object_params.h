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
 *  @object_params.h
 *  header of object client implementation
 */

#ifndef OBJECT_PARAMS_H_
#define OBJECT_PARAMS_H_

#include "util/types/optional.h"

/**
 * \class ObjectNewStateRequestArgs
 * \brief admin client object_new_state_request params
 */
struct ObjectNewStateRequestArgs
{
    unsigned long long object_id;//OBJECT_ID_NAME
    unsigned long object_new_state_request;//OBJECT_NEW_STATE_REQUEST_NAME

    ObjectNewStateRequestArgs()
    : object_id(0)
    , object_new_state_request(0)
    {}//ctor
    ObjectNewStateRequestArgs(
            const unsigned long long _object_id
             , const unsigned long _object_new_state_request
            )
    : object_id(_object_id)
    , object_new_state_request(_object_new_state_request)
    {}//init ctor
};//struct ObjectNewStateRequestArgs

/**
 * \class ObjectUpdateStatesArgs
 * \brief admin client object_update_states params
 */
struct ObjectUpdateStatesArgs
{
    optional_id object_id;//OBJECT_ID_NAME

    ObjectUpdateStatesArgs()
    {}//ctor
    ObjectUpdateStatesArgs(
            const optional_id& _object_id
            )
    : object_id(_object_id)
    {}//init ctor
};//struct ObjectUpdateStatesArgs

/**
 * \class ObjectRegularProcedureArgs
 * \brief admin client object_regular_procedure params
 */
struct ObjectRegularProcedureArgs
{
    optional_string poll_except_types;//OBJECT_POLL_EXCEPT_TYPES_NAME
    optional_string object_delete_types;//OBJECT_DELETE_TYPES_NAME
    optional_string notify_except_types;//OBJECT_NOTIFY_EXCEPT_TYPES_NAME

    ObjectRegularProcedureArgs()
    {}//ctor
    ObjectRegularProcedureArgs(
            const optional_string& _poll_except_types
             , const optional_string& _object_delete_types
             , const optional_string& _notify_except_types
            )
    : poll_except_types(_poll_except_types)
    , object_delete_types(_object_delete_types)
    , notify_except_types(_notify_except_types)
    {}//init ctor
};//struct ObjectRegularProcedureArgs

/**
 * \class DeleteObjectsArgs
 * \brief admin client deleteObjects params
 */
struct DeleteObjectsArgs
{
    optional_ulonglong object_delete_limit;//OBJECT_DELETE_LIMIT_NAME
    bool object_delete_debug;//OBJECT_DEBUG_NAME

    DeleteObjectsArgs()
    : object_delete_debug(false)
    {}//ctor
    DeleteObjectsArgs(
             const optional_ulonglong& _object_delete_limit
             , const bool _object_delete_debug
            )
    : object_delete_limit(_object_delete_limit)
    , object_delete_debug(_object_delete_debug)
    {}//init ctor
};//struct DeleteObjectsArgs

#endif // OBJECT_PARAMS_H_