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

#ifndef INFO_PUBLIC_REQUEST_AUTH_HH_369275A2EB1548C2BE4D42464BFE2CBD
#define INFO_PUBLIC_REQUEST_AUTH_HH_369275A2EB1548C2BE4D42464BFE2CBD

#include "src/libfred/public_request/info_public_request.hh"

bool operator==(const std::string&, const LibFred::PublicRequestAuthInfo&);

namespace LibFred {

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
    PublicRequestAuthInfo(OperationContext &_ctx, const LockedPublicRequest &_locked);

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
    friend bool ::operator==(const std::string&, const LibFred::PublicRequestAuthInfo&);
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
    Result exec(OperationContext &_ctx, const LockedPublicRequest &_locked)const
    {
        return Result(_ctx, _locked);
    }
};

} // namespace LibFred

inline bool operator==(const std::string &_identification, const LibFred::PublicRequestAuthInfo &_data)
{
    return _identification == _data.identification_;
}

inline bool operator==(const LibFred::PublicRequestAuthInfo &_data, const std::string &_identification)
{
    return _identification == _data;
}

inline bool operator==(const LibFred::LockedPublicRequest &_locked, const LibFred::PublicRequestAuthInfo &_data)
{
    return _locked == static_cast< const LibFred::PublicRequestInfo& >(_data);
}

inline bool operator==(const LibFred::PublicRequestAuthInfo &_data, const LibFred::LockedPublicRequest &_locked)
{
    return _locked == _data;
}

#endif
