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
 *  @file keyset_dns_key.h
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

namespace Fred
{

class DnsKey
{
    unsigned short flags_;
    unsigned short protocol_;
    unsigned short alg_;
    std::string key_;//base64 encoding
public:
    virtual ~DnsKey(){}

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

    DnsKey()
    : flags_(0)
    , protocol_(0)
    , alg_(0)
    //, key_("")
    {}

    unsigned short get_flags()
    {
        return flags_;
    }

    unsigned short get_protocol()
    {
        return protocol_;
    }

    unsigned short get_alg()
    {
        return alg_;
    }

    std::string get_key()
    {
        return key_;
    }

    bool operator==(const DnsKey& rhs) const
    {
        return (flags_ == rhs.flags_)
            && (protocol_ == rhs.protocol_)
            && (alg_ == rhs.alg_)
            && (key_.compare( rhs.key_) == 0)
            ;
    }

    bool operator!=(const DnsKey& rhs) const
    {
        return !this->operator ==(rhs);
    }

    void swap(DnsKey& dk) //throw()
    {
        std::swap(this->flags_, dk.flags_);
        std::swap(this->protocol_, dk.protocol_);
        std::swap(this->alg_, dk.alg_);
        std::swap(this->key_, dk.key_);
    }

    DnsKey& operator=(const DnsKey& dk)
    {
        if (this != &dk)
            DnsKey(dk).swap (*this); //copy and swap
        return *this;
    }

    operator std::string() const
    {
        std::stringstream ret;

        ret << "flags:" << flags_ << " protocol:" << protocol_ << " alg:" << alg_ << " key:" << key_ ;

        return ret.str();
    }

    bool operator<(const DnsKey& rhs) const
    {
        return static_cast<std::string>(*this) < static_cast<std::string>(rhs);
    }

}; //class DnsKey


}//namespace Fred

#endif//KEYSET_DNS_KEY_H_
