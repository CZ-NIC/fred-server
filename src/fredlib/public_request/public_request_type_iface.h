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

#include <string>
#include <memory>

namespace Fred {

/**
 * Contains template class Into of which template class BasedOn adds `iface()` methods family.
 * @section add_iface_method_sample Sample of usage
 * @code
 * class SomeParticularPublicRequest:public AddIfaceMethod::Into
 *     < SomeParticularPublicRequest >::BasedOn< PublicRequestTypeIface >
 * {
 * public:
 *     virtual ~SomeParticularPublicRequest() { }
 * private:
 *     SomeParticularPublicRequest() { }
 *     std::string get_public_request_type()const;
 *     friend class BasedOn;
 * };
 * @endcode
 */
struct AddIfaceMethod
{
    /**
     * Contains template class BasedOn which adds `iface()` methods family to the DERIVED class.
     * @tparam DERIVED class derived from IFACE and extended with `iface()` methods family
     */
    template < class DERIVED >
    struct Into
    {
        /**
         * Extends DERIVED class derived from IFACE class with `iface()` methods family.
         * @tparam IFACE base class from which is derived DERIVED class
         */
        template < class IFACE >
        class BasedOn:private IFACE
        {
        public:
            typedef DERIVED Instanceable;///< DERIVED class is able to be instance
            typedef IFACE   Iface;       ///< IFACE is pure virtual interface class which can't be instantiated
            /**
             */
            static const Iface& iface()
            {
                static const Instanceable instance;
                return static_cast< const Iface& >(instance);
            }
            typedef std::auto_ptr< Iface > IfacePtr;
            template < typename T >
            static IfacePtr iface(const T &_arg) { return IfacePtr(static_cast< Iface* >(new Instanceable(_arg))); }
            template < typename T >
            static IfacePtr iface(T *_arg_ptr) { return IfacePtr(static_cast< Iface* >(new Instanceable(_arg_ptr))); }
        protected:
            ~BasedOn() { }
        };
    };
};

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
};

}//namespace Fred

#endif//PUBLIC_REQUEST_TYPE_IFACE_H_E9BC2123C0A6C5F6CF12FF83939D575D
