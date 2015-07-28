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
 *  declaration of InfoPublicRequest operation
 */

#ifndef INFO_PUBLIC_REQUEST_H_6189924FC8F697665AC188B830E29EB7//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define INFO_PUBLIC_REQUEST_H_6189924FC8F697665AC188B830E29EB7

#include "util/db/nullable.h"
#include "src/fredlib/object_state/typedefs.h"
#include "src/fredlib/public_request/public_request_lock_guard.h"
#include "src/fredlib/public_request/public_request_status.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Fred {

class PublicRequestInfo;
class PublicRequestAuthInfo;

}

bool operator==(const Fred::PublicRequestLockGuard&, const Fred::PublicRequestInfo&);

namespace Fred {

/**
 * Information about public request.
 */
class PublicRequestInfo
{
public:
    typedef boost::posix_time::ptime Time;        ///< class for time representation
    typedef ObjectId                 EmailId;     ///< email database identification
    typedef ObjectId                 RequestId;   ///< some request identification
    typedef ::uint64_t               LogRequestId;///< logging request identification
    /**
     * Obtain information from database.
     * @param _ctx    operation context
     * @param _locked locked public request
     */
    PublicRequestInfo(OperationContext &_ctx, const PublicRequestLockGuard &_locked);

    /**
     * Copy constructor.
     * @param _src source object which copy is created
     */
    PublicRequestInfo(const PublicRequestInfo &_src);

    ~PublicRequestInfo() { }

    /**
     * Assignment operator.
     * @param _src source object which is assigned
     */
    PublicRequestInfo& operator=(const PublicRequestInfo &_src);

    const std::string&              get_type()const               { return type_; }
    const Time&                     get_create_time()const        { return create_time_; }
    PublicRequest::Status::Value    get_status()const             { return status_; }
    const Nullable< Time >&         get_resolve_time()const       { return resolve_time_; }
    const Nullable< std::string >&  get_reason()const             { return reason_; }
    const Nullable< std::string >&  get_email_to_answer()const    { return email_to_answer_; }
    const Nullable< EmailId >&      get_answer_email_id()const    { return answer_email_id_; }
    const Nullable< RegistrarId >&  get_registrar_id()const       { return registrar_id_; }
    const Nullable< LogRequestId >& get_create_request_id()const  { return create_request_id_; }
    const Nullable< LogRequestId >& get_resolve_request_id()const { return resolve_request_id_; }
    const Nullable< ObjectId >&     get_object_id()const          { return object_id_; }
private:
    PublicRequestId              id_;
    std::string                  type_;
    Time                         create_time_;
    PublicRequest::Status::Value status_;
    Nullable< Time >             resolve_time_;
    Nullable< std::string >      reason_;
    Nullable< std::string >      email_to_answer_;
    Nullable< EmailId >          answer_email_id_;
    Nullable< RegistrarId >      registrar_id_;
    Nullable< LogRequestId >     create_request_id_;
    Nullable< LogRequestId >     resolve_request_id_;
    Nullable< ObjectId >         object_id_;
    friend class PublicRequestAuthInfo;
    friend bool ::operator==(const Fred::PublicRequestLockGuard&, const Fred::PublicRequestInfo&);
};

class InfoPublicRequest
{
public:
    InfoPublicRequest() { }
    typedef PublicRequestInfo Result;
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

inline bool operator==(const Fred::PublicRequestLockGuard &_locked, const Fred::PublicRequestInfo &_data)
{
    return _locked.get_public_request_id() == _data.id_;
}

inline bool operator==(const Fred::PublicRequestInfo &_data, const Fred::PublicRequestLockGuard &_locked)
{
    return _locked == _data;
}

#endif//INFO_PUBLIC_REQUEST_H_6189924FC8F697665AC188B830E29EB7
