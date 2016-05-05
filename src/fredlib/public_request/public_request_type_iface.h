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

#ifndef PUBLIC_REQUEST_TYPE_IFACE_H_E9BC2123C0A6C5F6CF12FF83939D575D//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PUBLIC_REQUEST_TYPE_IFACE_H_E9BC2123C0A6C5F6CF12FF83939D575D

#include "src/fredlib/public_request/public_request_status.h"

#include <set>
#include <boost/shared_ptr.hpp>

namespace Fred {

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
    class IfacePtr
    {
    public:
        IfacePtr(const boost::shared_ptr< PublicRequestTypeIface > &_impl_ptr):impl_ptr_(_impl_ptr) { }
        IfacePtr(const IfacePtr &_src):impl_ptr_(_src.impl_ptr_) { }
        IfacePtr& operator=(const IfacePtr &_src) { impl_ptr_ = _src.impl_ptr_; return *this; }
        IfacePtr& operator=(const boost::shared_ptr< PublicRequestTypeIface > &_impl_ptr) { impl_ptr_ = _impl_ptr; return *this; }
        PublicRequestTypeIface* operator->()const { return impl_ptr_.get(); }
        PublicRequestTypeIface& operator*()const { return *impl_ptr_; }
        bool operator<(const IfacePtr &_b)const
        {
            const IfacePtr &_a = *this;
            return _a->get_public_request_type() < _b->get_public_request_type();
        }
    private:
        boost::shared_ptr< PublicRequestTypeIface > impl_ptr_;
    };
    /**
     * Collection of public request type interfaces.
     */
    typedef std::set< IfacePtr > PublicRequestTypes;
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
};

}//namespace Fred

#endif//PUBLIC_REQUEST_TYPE_IFACE_H_E9BC2123C0A6C5F6CF12FF83939D575D
