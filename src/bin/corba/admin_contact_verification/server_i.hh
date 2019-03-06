/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  header of admin contact verification wrapper over corba
 */
#ifndef SERVER_I_HH_A8647A097EF94EDE84D4B3AD5B608FDD
#define SERVER_I_HH_A8647A097EF94EDE84D4B3AD5B608FDD

// generated from idl
#include "src/bin/corba/AdminContactVerification.hh"

#include <string>

namespace CorbaConversion {
namespace AdminContactVerification {

class Server_i
    : public POA_Registry::AdminContactVerification::Server
{
private:
    const std::string server_name_;


    // do not copy
    Server_i(const Server_i&); // no definition

    Server_i& operator=(const Server_i&); // no definition


public:


    Server_i()
        : server_name_("fred-adifd")
    {
    }


    virtual ~Server_i()
    {
    }

    // Methods corresponding to IDL attributes and operations
    virtual Registry::AdminContactVerification::ContactCheckDetail* getContactCheckDetail(
            const char* check_handle);

    virtual Registry::AdminContactVerification::ContactCheckList* getChecksOfContact(
            ::CORBA::ULongLong contact_id,
            Registry::NullableString* testsuite,
            ::CORBA::ULong max_item_count);

    virtual Registry::AdminContactVerification::ContactCheckList* getActiveChecks(
            Registry::NullableString* testsuite);

    virtual void updateContactCheckTests(
            const char* check_handle,
            const Registry::AdminContactVerification::TestUpdateSeq& changes,
            ::CORBA::ULongLong logd_request_id);

    virtual void resolveContactCheckStatus(
            const char* check_handle,
            const char* status,
            ::CORBA::ULongLong logd_request_id);

    virtual void deleteDomainsAfterFailedManualCheck(const char* check_handle);

    virtual char* requestEnqueueingContactCheck(
            ::CORBA::ULongLong contact_id,
            const char* testsuite_handle,
            ::CORBA::ULongLong logd_request_id);

    virtual void confirmEnqueueingContactCheck(
            const char* check_handle,
            ::CORBA::ULongLong logd_request_id);

    virtual char* enqueueContactCheck(
            ::CORBA::ULongLong contact_id,
            const char* testsuite_handle,
            ::CORBA::ULongLong logd_request_id);

    virtual Registry::AdminContactVerification::MessageSeq* getContactCheckMessages(
            ::CORBA::ULongLong contact_id);

    virtual Registry::AdminContactVerification::ContactTestStatusDefSeq* listTestStatusDefs(const char* lang);

    virtual Registry::AdminContactVerification::ContactCheckStatusDefSeq* listCheckStatusDefs(
            const char* lang);

    virtual Registry::AdminContactVerification::ContactTestDefSeq* listTestDefs(
            const char* lang,
            Registry::NullableString* testsuite_handle);

    virtual Registry::AdminContactVerification::ContactTestSuiteDefSeq* listTestSuiteDefs(const char* lang);


};

} // namespace CorbaConversion::AdminContactVerification
} // namespace CorbaConversion

#endif
