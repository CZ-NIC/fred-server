/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  get object states
 */

#ifndef GET_OBJECT_STATES_H_
#define GET_OBJECT_STATES_H_

#include <vector>
#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "src/libfred/opcontext.hh"
#include "src/util/db/nullable.hh"

namespace LibFred
{
    /**
     * Object state data structure
     */
    struct ObjectStateData
    {
        unsigned long long state_id;/**< object state database id */
        std::string state_name;/**< object state name */
        boost::posix_time::ptime valid_from_time;/**< object state valid from time in UTC*/
        Nullable<boost::posix_time::ptime> valid_to_time; /**< object state valid to time in UTC or open if null*/
        Nullable<unsigned long long> valid_from_history_id;/**< history id of object in the moment of entering state (may be null)*/
        Nullable<unsigned long long> valid_to_history_id; /**< history id of object in the moment of leaving state or null*/
        bool is_external;/**< if object state is externally visible */
        bool is_manual;/**< if object state is set automatically or manually */
        long importance;/**< frontend state importance */
    };

    /**
     * Gets current state of given object.
     * Object database id is set via constructor.
     * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
     * In case of insuperable failures and inconsistencies, exception is thrown.
     */
    class GetObjectStates
    {
        const unsigned long long object_id_;/**< database id of the object */
    public:
        /**
         * Get states of the object with given database id sorted by importance.
         * @param object_id sets database id of the object @ref object_id_ attribute
         */
        GetObjectStates(unsigned long long object_id);
        /**
         * Executes getting states of given object.
         * @param ctx contains reference to database and logging interface
         * @return list of object state data sorted by importance
         */
        std::vector<ObjectStateData> exec(OperationContext& ctx);
    };

} // namespace LibFred

#endif // GET_OBJECT_STATES_H_
