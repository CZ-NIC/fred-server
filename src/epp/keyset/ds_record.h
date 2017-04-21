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
 *  ds record
 */

#ifndef DS_RECORD_H_BD42642FAB5A4153B45827E3BDEB2805
#define DS_RECORD_H_BD42642FAB5A4153B45827E3BDEB2805

#include <string>

namespace Epp {
namespace Keyset {

/**
 * Container for DS record data.
 */
class DsRecord
{
public:
    DsRecord(long _key_tag,
             long _alg,
             long _digest_type,
             const std::string &_digest,
             long _max_sig_life)
    :   key_tag_(),
        alg_(),
        digest_type_(),
        digest_(),
        max_sig_life_()
    { }

    DsRecord()
    :   key_tag_(0),
        alg_(0),
        digest_type_(0),
        digest_(),
        max_sig_life_(0)
    { }

    long get_key_tag()const
    {
        return key_tag_;
    }

    long get_alg()const
    {
        return alg_;
    }

    long get_digest_type()const
    {
        return digest_type_;
    }

    const std::string& get_digest()const
    {
        return digest_;
    }

    long get_max_sig_life()const
    {
        return max_sig_life_;
    }

    bool operator<(const DsRecord &_b)const
    {
        const DsRecord &_a = *this;
        return
             (_a.key_tag_ <  _b.key_tag_) ||
            ((_a.key_tag_ == _b.key_tag_) && (_a.alg_ <  _b.alg_)) ||
            ((_a.key_tag_ == _b.key_tag_) && (_a.alg_ == _b.alg_) && (_a.digest_type_ <  _b.digest_type_)) ||
            ((_a.key_tag_ == _b.key_tag_) && (_a.alg_ == _b.alg_) && (_a.digest_type_ == _b.digest_type_) && (_a.digest_ <  _b.digest_)) ||
            ((_a.key_tag_ == _b.key_tag_) && (_a.alg_ == _b.alg_) && (_a.digest_type_ == _b.digest_type_) && (_a.digest_ == _b.digest_) && (_a.max_sig_life_ < _b.max_sig_life_));
    }

    /**
    * Equality of DsRecord data operator.
    * @param _b is right hand side of DsRecord data comparison
    * @return true if equal, false in other cases
    */
    bool operator==(const DsRecord &_b)const
    {
        const DsRecord &_a = *this;
        return !((_a < _b) || (_b < _a));
    }

    /**
    * Inequality of DsRecord data operator.
    * @param _b is right hand side of DsRecord data comparison
    * @return true if not equal, false in other cases
    */
    bool operator!=(const DsRecord &_b) const
    {
        const DsRecord &_a = *this;
        return !(_a == _b);
    }

    /**
     * Assignment of new data given in parameter, replacing current instance data.
     * @param _src is reference to instance whose content is assigned into this instance
     * @return reference to self
     */
    DsRecord& operator=(const DsRecord &_src)
    {
        key_tag_ = _src.key_tag_;
        alg_ = _src.alg_;
        digest_type_ = _src.digest_type_;
        digest_ = _src.digest_;
        max_sig_life_ = _src.max_sig_life_;
        return *this;
    }
private:
    long key_tag_;
    long alg_;
    long digest_type_;
    std::string digest_;
    long max_sig_life_;
}; //class DsRecord


} // namespace Epp::Keyset
} // namespace Epp

#endif
