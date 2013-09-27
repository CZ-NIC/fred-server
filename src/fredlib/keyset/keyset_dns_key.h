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
 *  keyset dns key
 */

#ifndef KEYSET_DNS_KEY_H_
#define KEYSET_DNS_KEY_H_

#include <string>
#include <sstream>
#include <vector>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"

namespace Fred
{

/**
 * Container for DNSKEY record data as specified in RFC4034.
 */
class DnsKey  : public Util::Printable
{
    unsigned short flags_;/**< the flags field */
    unsigned short protocol_;/**< the protocol field, only valid value is 3*/
    unsigned short alg_;/**< the algorithm field identifies the public key's cryptographic algorithm, values can be found in RFC 4034 Apendix A.1. */
    std::string key_;/**< the public key field in base64 encoding */
public:
    /**
     * Empty destructor.
     */
    virtual ~DnsKey(){}

    /**
     * Constructor initializing all attributes. Removes whitespaces from @ref _key parameter.
     * @param _flags sets @ref flags_ field
     * @param _protocol sets @ref protocol_ field
     * @param _alg sets @ref alg_ field
     * @param _key sets @ref key_ field in base64 encoding
     */
    DnsKey(unsigned short _flags
            , unsigned short _protocol
            , unsigned short _alg
            , const std::string& _key)
    : flags_(_flags)
    , protocol_(_protocol)
    , alg_(_alg)
    , key_(_key)
    {
        //erase spaces in key
        key_.erase(std::remove_if(key_.begin(), key_.end(), isspace), key_.end());
    }

    /**
     * Default constructor. Sets integral attributes to zero and key to empty string.
     */
    DnsKey()
    : flags_(0)
    , protocol_(0)
    , alg_(0)
    //, key_("")
    {}

    /**
     * Flags field getter.
     * @return flags field viz @ref flags_
     */
    unsigned short get_flags()
    {
        return flags_;
    }

    /**
     * Protocol field getter.
     * @return protocol field viz @ref protocol_
     */
    unsigned short get_protocol()
    {
        return protocol_;
    }
    /**
     * Algorithm field getter.
     * @return algorithm field viz @ref alg_
     */
    unsigned short get_alg()
    {
        return alg_;
    }

    /**
     * Key field getter.
     * @return public key field viz @ref key_
     */
    std::string get_key()
    {
        return key_;
    }
    /**
    * Equality of DNSKEY data operator.
    * @param rhs is right hand side of DNSKEY data comparison
    * @return true if equal, false if not
    */
    bool operator==(const DnsKey& rhs) const
    {
        return (flags_ == rhs.flags_)
            && (protocol_ == rhs.protocol_)
            && (alg_ == rhs.alg_)
            && (key_.compare( rhs.key_) == 0)
            ;
    }

    /**
    * Inequality of DNSKEY data operator.
    * @param rhs is right hand side of DNSKEY data comparison
    * @return true if not equal, false if equal
    */
    bool operator!=(const DnsKey& rhs) const
    {
        return !this->operator ==(rhs);
    }
    /**
     * Exchanges the data of the instance by the data of instance given in parameter.
     * @param dk is reference to instance whose content is swapped with this instance
     */
    void swap(DnsKey& dk) //throw()
    {
        std::swap(this->flags_, dk.flags_);
        std::swap(this->protocol_, dk.protocol_);
        std::swap(this->alg_, dk.alg_);
        std::swap(this->key_, dk.key_);
    }

    /**
     * Assignment of new data given in parameter, replacing current instance data. Using copy and @swap.
     * @param dk is reference to instance whose content is assigned into this instance
     * @return reference to self
     */
    DnsKey& operator=(const DnsKey& dk)
    {
        if (this != &dk)
            DnsKey(dk).swap (*this); //copy and swap
        return *this;
    }

    /**
     * Comparison of instances converted to std::string
     * @param rhs is right hand side instance of the comparison
     */
    bool operator<(const DnsKey& rhs) const
    {
        return to_string() < rhs.to_string();
    }

    /**
    * Dumps state of the instance into the string
    * @return string with description of the instance state
    */
    std::string to_string() const
    {
        return Util::format_data_structure("DnsKey",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("flags",boost::lexical_cast<std::string>(flags_)))
        (std::make_pair("protocol",boost::lexical_cast<std::string>(protocol_)))
        (std::make_pair("alg",boost::lexical_cast<std::string>(alg_)))
        (std::make_pair("key",key_))
        );
    }
}; //class DnsKey


}//namespace Fred

#endif//KEYSET_DNS_KEY_H_
