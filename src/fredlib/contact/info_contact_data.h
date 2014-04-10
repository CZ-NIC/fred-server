/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  common contact info data
 */

#ifndef INFO_CONTACT_DATA_H_
#define INFO_CONTACT_DATA_H_

#include "util/db/nullable.h"
#include "util/printable.h"
#include "src/fredlib/contact/place_address.h"

#include <boost/date_time/posix_time/ptime.hpp>

namespace Fred
{
    /**
     * Type of contact address.
     */
    struct ContactAddressType
    {
        enum Value/** Enumeration of possible values. */
        {
            MAILING,/**< where can I send letters */
            BILLING,/**< where can I send bills */
            SHIPPING,/**< where can I send ordered goods */
        };
        /**
         * Default constructor.
         * @warning No default value => default constructor not defined.
         */
        ContactAddressType();
        /**
         * Init constructor.
         * @param _value initializes @ref value
         */
        ContactAddressType(Value _value):value(_value) { }
        /**
         * Copy constructor.
         * @param _src is copied instance
         */
        ContactAddressType(const struct ContactAddressType &_src):value(_src.value) { }
        /**
         * Assign operator.
         * @param _src assigned instance
         * @return self reference
         */
        struct ContactAddressType& operator=(const struct ContactAddressType &_src)
        {
            value = _src.value;
            return *this;
        }
        /**
         * Assign operator.
         * @param _value assigned value
         * @return self reference
         */
        struct ContactAddressType& operator=(Value _value)
        {
            value = _value;
            return *this;
        }
        Value value;/**< one of possible values */
        /**
         * Dumps @a _value into the string.
         * @param _value will be converted to string
         * @return string representation of @a _value
         * @throw std::runtime_error if conversion is impossible
         */
        static std::string to_string(Value _value);
        /**
         * Dumps content of the instance into the string.
         * @return string representation of @ref value
         * @throw std::runtime_error if conversion is impossible
         */
        std::string to_string()const { return to_string(value); }
        /**
         * Converts string @a _value into one of possible integer values.
         * @param _value string representation of address type
         * @return one of possible integer values that conform string @a _value
         * @throw std::runtime_error if conversion is impossible
         */
        static Value from_string(const std::string &_value);
        /**
         * Sets to corresponding value.
         * @param _value string representation of address type
         * @return self reference
         * @throw std::runtime_error if conversion is impossible
         */
        struct ContactAddressType& set_value(const std::string &_value)
        {
            value = from_string(_value);
            return *this;
        }
        /**
         * Comparison operator.
         * @param _value is right hand side of the comparison
         * @return true if equal, false otherwise
         */
        bool operator==(Value _value)const { return this->value == _value; }
        /**
         * Comparison operator.
         * @param _b is right hand side of the comparison
         * @return true if equal, false otherwise
         */
        bool operator==(const struct ContactAddressType &_b)const { return this->value == _b.value; }
    };
    /**
     * Additional postal address of contact.
     */
    struct ContactAddress : Contact::PlaceAddress
    {
        struct ContactAddressType type;/**< type of address (required) */
        Optional< std::string > company_name;/**< company name (optional) */

        /**
         * Dumps content into the string.
         * @return string with description of the instance content
         */
        std::string to_string()const;
        /**
         * Check equality of two instances.
         * @param _b compares @a this instance with @a _b instance
         * @return true if they are the same.
         */
        bool operator==(const struct ContactAddress &_b)const;
    };
    /**
     * Container of additional contact addresses.
     */
    typedef std::vector< struct ContactAddress > ContactAddressList;
    /**
     * Common data of contact.
     * Current or history state of the contact.
     */
    struct InfoContactData : public Util::Printable
    {
        unsigned long long crhistoryid;/**< first historyid of contact history*/
        unsigned long long historyid;/**< last historyid of contact history*/
        Nullable<boost::posix_time::ptime> delete_time; /**< contact delete time in set local zone*/
        std::string handle;/**< contact handle */
        std::string roid;/**< registry object identifier of the contact */
        std::string sponsoring_registrar_handle;/**< registrar administering the contact */
        std::string create_registrar_handle;/**< registrar that created the contact */
        Nullable<std::string> update_registrar_handle;/**< registrar which last time changed the contact */
        boost::posix_time::ptime creation_time;/**< creation time of the contact in set local zone*/
        Nullable<boost::posix_time::ptime> update_time; /**< last update time of the contact in set local zone*/
        Nullable<boost::posix_time::ptime> transfer_time; /**<last transfer time in set local zone*/
        std::string authinfopw;/**< password for transfer */
        Nullable<std::string> name ;/**< name of contact person */
        Nullable<std::string> organization;/**< full trade name of organization */
        Nullable< Contact::PlaceAddress > place;/**< place address of contact */
        Nullable<std::string> telephone;/**<  telephone number */
        Nullable<std::string> fax;/**< fax number */
        Nullable<std::string> email;/**< e-mail address */
        Nullable<std::string> notifyemail;/**< to this e-mail address will be send message in case of any change in domain or nsset affecting contact */
        Nullable<std::string> vat;/**< taxpayer identification number */
        Nullable<std::string> ssntype;/**< type of identification from enumssntype table */
        Nullable<std::string> ssn;/**< unambiguous identification number e.g. social security number, identity card number, date of birth */
        bool disclosename;/**< whether to reveal contact name */
        bool discloseorganization;/**< whether to reveal organization */
        bool discloseaddress;/**< whether to reveal address */
        bool disclosetelephone;/**< whether to reveal phone number */
        bool disclosefax;/**< whether to reveal fax number */
        bool discloseemail;/**< whether to reveal email address */
        bool disclosevat;/**< whether to reveal taxpayer identification number */
        bool discloseident;/**< whether to reveal unambiguous identification number */
        bool disclosenotifyemail;/**< whether to reveal notify email */
        unsigned long long id;/**< id of the contact object*/
        ContactAddressList addresses;/**< additional contact addresses */

    public:
        /**
        * Constructor of the contact data structure.
        */
        InfoContactData();

        /**
         * Postal address of contact.
         */
        struct Address : Contact::PlaceAddress
        {
            Optional< std::string > name;/**< person name (optional) */
            Optional< std::string > organization;/**< organization name (optional) */
            Optional< std::string > company_name;/**< company name (optional) */
            struct Address& operator=(const Contact::PlaceAddress &_src);/**< set PlaceAddress part */
        };

        /**
         * Get permanent address of contact.
         * @return permanent address of contact
         * @throw std::runtime_error if no address exists
         */
        struct Address get_permanent_address()const;
        /**
         * Get address for given purpose.
         * @tparam purpose specifies usage of address
         * @return contact address for given purpose
         * @throw std::runtime_error if no usable address exists
         */
        template < ContactAddressType::Value purpose >
        struct Address get_address()const;
        /**
        * Equality of the contact data structure operator.
        * @param rhs is right hand side of the contact data comparison
        * @return true if equal, false if not
        */
        bool operator==(const InfoContactData& rhs) const;

        /**
        * Inequality of the contact data structure operator.
        * @param rhs is right hand side of the contact data comparison
        * @return true if not equal, false if equal
        */
        bool operator!=(const InfoContactData& rhs) const;

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };

}//namespace Fred

#endif//INFO_CONTACT_DATA_H_
