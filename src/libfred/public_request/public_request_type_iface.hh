/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  declaration of PublicRequestTypeIface class
 */

#ifndef PUBLIC_REQUEST_TYPE_IFACE_HH_801D66A174C44EE6B82B5D9811CDDD2C
#define PUBLIC_REQUEST_TYPE_IFACE_HH_801D66A174C44EE6B82B5D9811CDDD2C

#include "src/libfred/public_request/public_request_status.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/libfred/public_request/public_request_status.hh"

#include <set>
#include <memory>

namespace LibFred {

/**
 * Common class for particular public request type.
 */
class PublicRequestTypeIface
{
public:
    /**
     * Convert public request type into string representation used in database.
     * @return string representation of this public request type
     */
    virtual std::string get_public_request_type()const = 0;
    /**
     * Instance pointer is publicly deletable.
     */
    virtual ~PublicRequestTypeIface() { }
    typedef std::shared_ptr< PublicRequestTypeIface > IfacePtr;
    struct IfaceCompare
    {
        bool operator()(const IfacePtr &_a, const IfacePtr &_b)const
        {
            return _a->get_public_request_type() < _b->get_public_request_type();
        }
    };
    /**
     * Collection of public request type interfaces.
     */
    typedef std::set< IfacePtr, IfaceCompare > PublicRequestTypes;
    /**
     * Get collection of public request types which have to be cancelled before creation of this.
     * @return collection of public request types to cancel before creation of this
     */
    virtual PublicRequestTypes get_public_request_types_to_cancel_on_create()const = 0;
    /**
     * Get collection of public request types which have to be cancelled after changing status of this.
     * @return collection of public request types to cancel after changing status of this
     */
    virtual PublicRequestTypes get_public_request_types_to_cancel_on_update(
        PublicRequest::Status::Enum _old_status, PublicRequest::Status::Enum _new_status)const = 0;

    virtual LibFred::PublicRequest::OnStatusAction::Enum get_on_status_action(LibFred::PublicRequest::Status::Enum)const
    {
        return LibFred::PublicRequest::OnStatusAction::processed;
    }
};

} // namespace LibFred

#endif
