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
 *  get object state descriptions
 */

#ifndef GET_OBJECT_STATE_DESCRIPTIONS_H_
#define GET_OBJECT_STATE_DESCRIPTIONS_H_

#include <vector>
#include <string>

#include "src/fredlib/opexception.h"

namespace Fred
{
    /**
     * descriptions of object state
     */

    struct ObjectStateDescription
    {
       unsigned long long id;
       std::string handle;
       std::string description;

       ObjectStateDescription(unsigned long long _id,
           const std::string& _handle,
           const std::string& _description)
       : id (_id)
       , handle(_handle)
       , description(_description)
       {}
    };

    /**
     * Gets descriptions of object states.
     * Language of state descriptions is set via constructor.
     * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
     * In case of insuperable failures and inconsistencies, an instance of @ref InternalError or other exception is thrown.
     */
    class GetObjectStateDescriptions
    {
        const std::string description_language_;/**< requested language of object state descriptions like 'EN' or 'CS'*/
        bool external_states;
        std::string object_type_;
    public:
        /**
         * Get descriptions of object states in given language.
         * @param description_language sets required language of descriptions @ref description_language_ attribute
         */
        GetObjectStateDescriptions(const std::string& description_language);
        /**
         * Sets flag to get only the external state descriptions
         */
        GetObjectStateDescriptions& set_external();
        /**
         * Sets object type of states to get descriptions of
         */
        GetObjectStateDescriptions& set_object_type(const std::string& object_type);
        /**
         * Executes getting descriptions of object states.
         * @param ctx contains reference to database and logging interface
         * @return list of ObjectStateDescription
         */
        std::vector<ObjectStateDescription> exec(OperationContext& ctx);
    };
}//namespace Fred

#endif // GET_OBJECT_STATE_DESCRIPTIONS_H_
