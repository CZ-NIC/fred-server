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
 *  @object_params.h
 *  header of object client implementation
 */

#ifndef OBJECT_PARAMS_HH_31E89F375BC04621B15997DA1105EE05
#define OBJECT_PARAMS_HH_31E89F375BC04621B15997DA1105EE05

#include "src/util/types/optional.hh"


/**
 * \class ObjectNewStateRequestNameArgs
 * \brief admin client object_new_state_request_name params
 */
struct ObjectNewStateRequestNameArgs
{
    std::string object_name;
    unsigned long object_type;
    std::vector< std::string > object_state_name;
    optional_string valid_from;
    optional_string valid_to;
    bool update_object_state;

    ObjectNewStateRequestNameArgs()
    : object_type(0)
    , update_object_state(false)
    {}//ctor
    ObjectNewStateRequestNameArgs(
        const std::string& _object_name
        , const long _object_type
        , std::vector< std::string > _object_state_name
        , const optional_string& _valid_from
        , const optional_string& _valid_to
        , const bool _update_object_state
        )
    : object_name(_object_name)
    , object_type(_object_type)
    , object_state_name(_object_state_name)
    , valid_from(_valid_from)
    , valid_to(_valid_to)
    , update_object_state(_update_object_state)
    {}//init ctor
};//struct ObjectNewStateRequestNameArgs

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
    optional_string notify_except_types;//OBJECT_NOTIFY_EXCEPT_TYPES_NAME

    ObjectRegularProcedureArgs()
    {}//ctor
    ObjectRegularProcedureArgs(
            const optional_string& _poll_except_types
             , const optional_string& _notify_except_types
            )
    : poll_except_types(_poll_except_types)
    , notify_except_types(_notify_except_types)
    {}//init ctor
};//struct ObjectRegularProcedureArgs

/**
 * \class DeleteObjectsArgs
 * \brief admin client deleteObjects params
 */
struct DeleteObjectsArgs
{
    DeleteObjectsArgs() : object_delete_debug(false) { }
    DeleteObjectsArgs(
            const optional_ulonglong& _object_delete_limit,
            const optional_string& _object_delete_types,
            const optional_ulonglong& _object_delete_parts,
            const optional_ulonglong& _object_delete_spread_during_time,
            const bool _object_delete_debug)
    : object_delete_limit(_object_delete_limit),
      object_delete_types(_object_delete_types),
      object_delete_parts(_object_delete_parts),
      object_delete_spread_during_time(_object_delete_spread_during_time),
      object_delete_debug(_object_delete_debug)
    { }
    optional_ulonglong object_delete_limit;//OBJECT_DELETE_LIMIT_NAME
    optional_string object_delete_types;//OBJECT_DELETE_TYPES_NAME
    optional_ulonglong object_delete_parts;//OBJECT_DELETE_PARTS_NAME
    optional_ulonglong object_delete_spread_during_time;
    bool object_delete_debug;//OBJECT_DEBUG_NAME
};

#endif
