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
 *  declaration of InfoPublicRequestAuth class
 */

#ifndef INFO_PUBLIC_REQUEST_AUTH_H_FCCB5C08F0D3771F7E4BE2ABCC46F5FB//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define INFO_PUBLIC_REQUEST_AUTH_H_FCCB5C08F0D3771F7E4BE2ABCC46F5FB

#include "src/fredlib/public_request/info_public_request.h"

bool operator==(const std::string&, const Fred::PublicRequestAuthInfo&);

namespace Fred {

/**
 * Information about public request with authentication.
 */
class PublicRequestAuthInfo:public PublicRequestInfo
{
public:
    /**
     * Obtain information from database.
     * @param _ctx    operation context
     * @param _locked locked public request
     */
    PublicRequestAuthInfo(OperationContext &_ctx, const PublicRequestLockGuard &_locked);

    /**
     * Copy constructor.
     * @param _src source object which copy is created
     */
    PublicRequestAuthInfo(const PublicRequestAuthInfo &_src);

    ~PublicRequestAuthInfo() { }

    /**
     * Assignment operator.
     * @param _src source object which is assigned
     */
    PublicRequestAuthInfo& operator=(const PublicRequestAuthInfo &_src);

    typedef bool Success;
    Success check_password(const std::string &_pwd)const { return password_ == _pwd; }
private:
    std::string identification_;
    std::string password_;
    friend bool ::operator==(const std::string&, const Fred::PublicRequestAuthInfo&);
};

class InfoPublicRequestAuth
{
public:
    InfoPublicRequestAuth() { }
    typedef PublicRequestAuthInfo Result;
    /**
     * Obtain information from database.
     * @param _ctx    operation context
     * @param _locked locked public request
     */
    Result exec(OperationContext &_ctx, const PublicRequestLockGuard &_locked)const
    {
        return Result(_ctx, _locked);
    }
};

}//namespace Fred

inline bool operator==(const std::string &_identification, const Fred::PublicRequestAuthInfo &_data)
{
    return _identification == _data.identification_;
}

inline bool operator==(const Fred::PublicRequestAuthInfo &_data, const std::string &_identification)
{
    return _identification == _data;
}

inline bool operator==(const Fred::PublicRequestLockGuard &_locked, const Fred::PublicRequestAuthInfo &_data)
{
    return _locked == static_cast< const Fred::PublicRequestInfo& >(_data);
}

inline bool operator==(const Fred::PublicRequestAuthInfo &_data, const Fred::PublicRequestLockGuard &_locked)
{
    return _locked == _data;
}

#endif//INFO_PUBLIC_REQUEST_AUTH_H_FCCB5C08F0D3771F7E4BE2ABCC46F5FB
