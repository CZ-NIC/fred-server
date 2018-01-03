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
 *  header of mojeid public request classes
 */

#ifndef MOJEID_PUBLIC_REQUEST_HH_84FE8D19EBEB44C998CD55B92E3E2ABE
#define MOJEID_PUBLIC_REQUEST_HH_84FE8D19EBEB44C998CD55B92E3E2ABE

#include "src/libfred/public_request/public_request_auth_type_iface.hh"

namespace LibFred {

std::string conditional_contact_identification_generate_passwords();

namespace MojeID {

std::string contact_transfer_request_generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact);

std::string contact_identification_generate_passwords();

namespace PublicRequest {

class ContactConditionalIdentification:public PublicRequestAuthTypeIface
{
public:
    ~ContactConditionalIdentification() { }
    std::string get_public_request_type()const;
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    static std::string get_pin1_part(const std::string &_summary_password);
    static std::string get_pin2_part(const std::string &_summary_password);
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create()const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        LibFred::PublicRequest::Status::Enum _old_status, LibFred::PublicRequest::Status::Enum _new_status)const;
    std::string generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const;
};

class ContactIdentification:public PublicRequestAuthTypeIface
{
public:
    ~ContactIdentification() { }
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create()const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        LibFred::PublicRequest::Status::Enum _old_status, LibFred::PublicRequest::Status::Enum _new_status)const;
    std::string generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const;
};

class ContactReidentification:public PublicRequestAuthTypeIface
{
public:
    ~ContactReidentification() { }
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create()const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        LibFred::PublicRequest::Status::Enum _old_status, LibFred::PublicRequest::Status::Enum _new_status)const;
    std::string generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const;
};

class ContactValidation:public PublicRequestTypeIface
{
public:
    ~ContactValidation() { }
    const PublicRequestTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create()const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        LibFred::PublicRequest::Status::Enum _old_status, LibFred::PublicRequest::Status::Enum _new_status)const;
};

class ConditionallyIdentifiedContactTransfer:public PublicRequestAuthTypeIface
{
public:
    ~ConditionallyIdentifiedContactTransfer() { }
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create()const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        LibFred::PublicRequest::Status::Enum _old_status, LibFred::PublicRequest::Status::Enum _new_status)const;
    std::string generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const;
};

class IdentifiedContactTransfer:public PublicRequestAuthTypeIface
{
public:
    ~IdentifiedContactTransfer() { }
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create()const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        LibFred::PublicRequest::Status::Enum _old_status, LibFred::PublicRequest::Status::Enum _new_status)const;
    std::string generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const;
};

class PrevalidatedUnidentifiedContactTransfer:public PublicRequestAuthTypeIface
{
public:
    ~PrevalidatedUnidentifiedContactTransfer() { }
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create()const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        LibFred::PublicRequest::Status::Enum _old_status, LibFred::PublicRequest::Status::Enum _new_status)const;
    std::string generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const;
};

class PrevalidatedContactTransfer:public PublicRequestAuthTypeIface
{
public:
    ~PrevalidatedContactTransfer() { }
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create()const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        LibFred::PublicRequest::Status::Enum _old_status, LibFred::PublicRequest::Status::Enum _new_status)const;
    std::string generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const;
};

}//Fred::MojeID::PublicRequest
}//Fred::MojeID
}//Fred

#endif
