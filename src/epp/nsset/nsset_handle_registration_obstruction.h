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

#ifndef EPP_NSSET_HANDLE_REGISTRATION_OBSTRUCTION_H_8f8b86441d374ace8a52657920472ff8
#define EPP_NSSET_HANDLE_REGISTRATION_OBSTRUCTION_H_8f8b86441d374ace8a52657920472ff8

#include <boost/assign/list_of.hpp>
#include <set>
#include <stdexcept>

#include "src/epp/exception.h"
#include "src/epp/reason.h"

namespace Epp
{

struct NssetHandleRegistrationObstruction {
    enum Enum {
        invalid_handle, protected_handle, registered_handle
    };

    static std::set<NssetHandleRegistrationObstruction::Enum> get_all_values() {
        const std::set<NssetHandleRegistrationObstruction::Enum> all_values = boost::assign::list_of(invalid_handle)(protected_handle)(registered_handle);
        return all_values;
    }
};

/**
 * @throws MissingLocalizedDescription
 */
inline unsigned to_description_db_id(const NssetHandleRegistrationObstruction::Enum state) {

    /**
     * XXX This is wrong - we are "reusing" descriptions of other objects. It is temporary (I've been promised) conscious hack.
     */
    switch(state) {
        case NssetHandleRegistrationObstruction::invalid_handle     : return to_description_db_id(Reason::invalid_handle);
        case NssetHandleRegistrationObstruction::registered_handle  : return to_description_db_id(Reason::existing);
        case NssetHandleRegistrationObstruction::protected_handle   : return to_description_db_id(Reason::protected_period);
    }

    throw MissingLocalizedDescription();
}

/**
 * @throws UnknownLocalizedDescriptionId
 */
template<typename T> inline typename T::Enum from_description_db_id(const unsigned id);

/**
 * @throws ExceptionMissingLocalizedDescription
 */
template<> inline NssetHandleRegistrationObstruction::Enum from_description_db_id<NssetHandleRegistrationObstruction>(const unsigned id) {

    /**
     * XXX This is wrong - we are "reusing" descriptions of other objects. It is temporary (I've been promised) conscious hack.
     */
    switch( from_description_db_id<Reason>(id) ) {
        case Reason::invalid_handle     : return NssetHandleRegistrationObstruction::invalid_handle;
        case Reason::existing           : return NssetHandleRegistrationObstruction::registered_handle;
        case Reason::protected_period   : return NssetHandleRegistrationObstruction::protected_handle;
        default                         : throw UnknownLocalizedDescriptionId();
    }

    throw std::runtime_error("error in from_description_db_id<NssetHandleRegistrationObstruction>()");
}

}

#endif
