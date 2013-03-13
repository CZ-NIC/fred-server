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
}; //class DnsKey


}//namespace Fred

#endif//KEYSET_DNS_KEY_H_
