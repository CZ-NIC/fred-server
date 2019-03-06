/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef DNS_KEY_HH_F64DF3F171154311BBCDDAC817805FF9
#define DNS_KEY_HH_F64DF3F171154311BBCDDAC817805FF9

#include "libfred/opcontext.hh"

#include <algorithm>
#include <ctype.h>
#include <map>
#include <string>

namespace Epp {
namespace Keyset {

/**
 * Container for DNSKEY record data as specified in RFC4034.
 */
class DnsKey
{
public:


    /**
     * Constructor initializing all attributes. Removes whitespaces from @ref _key parameter.
     * @param _flags sets @ref flags_ field
     * @param _protocol sets @ref protocol_ field
     * @param _alg sets @ref alg_ field
     * @param _key sets @ref key_ field in base64 encoding
     */
    DnsKey(
            unsigned short _flags,
            unsigned short _protocol,
            unsigned short _alg,
            const std::string& _key)
        : flags_(_flags),
          protocol_(_protocol),
          alg_(_alg),
          key_(remove_spaces(_key))
    {
    }


    /**
     * Default constructor. Sets integral attributes to zero and key to empty string.
     */
    DnsKey()
        : flags_(0),
          protocol_(0),
          alg_(0),
          key_()
    {
    }


    /**
     * Flags field getter.
     * @return flags field viz @see flags_
     */
    unsigned short get_flags() const
    {
        return flags_;
    }


    bool is_flags_correct() const;


    /**
     * Protocol field getter.
     * @return protocol field @see protocol_
     */
    unsigned short get_protocol() const
    {
        return protocol_;
    }


    bool is_protocol_correct() const;


    /**
     * Algorithm field getter.
     * @return algorithm field @see alg_
     */
    unsigned short get_alg() const
    {
        return alg_;
    }


    class AlgValidator
    {
public:


        explicit AlgValidator(LibFred::OperationContext& _ctx);


        bool is_alg_correct(const DnsKey& _dns_key);


private:
        LibFred::OperationContext& ctx_;

        typedef std::map<unsigned short, bool> AlgNumberToIsCorrect;

        AlgNumberToIsCorrect alg_correctness_;
    };

    /**
     * Key field getter.
     * @return public key field @see key_
     */
    const std::string& get_key() const
    {
        return key_;
    }


    /**
     * Comparison of DNSKEY data operator.
     * @param _b is right hand side instance of the comparison
     */
    bool operator<(const DnsKey& _b) const
    {
        const DnsKey& _a = *this;
        return
            (_a.flags_ <  _b.flags_) ||
            ((_a.flags_ == _b.flags_) && (_a.protocol_ <  _b.protocol_)) ||
            ((_a.flags_ == _b.flags_) && (_a.protocol_ == _b.protocol_) && (_a.alg_ <  _b.alg_)) ||
            ((_a.flags_ == _b.flags_) && (_a.protocol_ == _b.protocol_) && (_a.alg_ == _b.alg_) &&
             (_a.key_ < _b.key_));
    }


    /**
     * Equality of DNSKEY data operator.
     * @param _b is right hand side of DNSKEY data comparison
     * @return true if equal, false in other cases
     */
    bool operator==(const DnsKey& _b) const
    {
        const DnsKey& _a = *this;
        return !((_a < _b) || (_b < _a));
    }


    /**
     * Inequality of DNSKEY data operator.
     * @param _b is right hand side of DNSKEY data comparison
     * @return true if not equal, false in other cases
     */
    bool operator!=(const DnsKey& _b) const
    {
        const DnsKey& _a = *this;
        return !(_a == _b);
    }


    /**
     * Assignment of new data given in parameter, replacing current instance data.
     * @param _src is reference to instance whose content is assigned into this instance
     * @return reference to self
     */
    DnsKey& operator=(const DnsKey& _src)
    {
        flags_ = _src.flags_;
        protocol_ = _src.protocol_;
        alg_ = _src.alg_;
        key_ = _src.key_;
        return *this;
    }


    struct CheckKey
    {
        enum Result
        {
            ok,
            bad_char,
            bad_length

        };

    };

    CheckKey::Result check_key() const;


private:
    static inline std::string remove_spaces(const std::string& _str)
    {
        std::string str = _str;
        str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
        return str;
    }


    unsigned short flags_;   ///< the flags field
    unsigned short protocol_; ///< the protocol field, only valid value is 3
    unsigned short alg_;     ///< the algorithm field identifies the public key's cryptographic algorithm, values can be found in RFC 4034 Apendix A.1.
    std::string key_;        ///< the public key field in base64 encoding
}; // class DnsKey

} // namespace Epp::Keyset
} // namespace Epp

#endif
