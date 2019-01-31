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

#include "libfred/public_request/public_request_auth_type_iface.hh"

namespace Fred {
namespace Backend {

namespace MojeId {

std::string contact_transfer_request_generate_passwords(const LibFred::LockedPublicRequestsOfObjectForUpdate& _locked_contact);

std::string contact_identification_generate_passwords();

namespace PublicRequest {

class ContactConditionalIdentification : public LibFred::PublicRequestAuthTypeIface
{
public:
    ~ContactConditionalIdentification()
    {
    }
    std::string get_public_request_type() const;
    const LibFred::PublicRequestAuthTypeIface& iface() const
    {
        return *this;
    }
    static std::string get_pin1_part(const std::string& _summary_password);
    static std::string get_pin2_part(const std::string& _summary_password);

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum _old_status,
            LibFred::PublicRequest::Status::Enum _new_status) const;
    std::string generate_passwords(const LibFred::LockedPublicRequestsOfObjectForUpdate& _locked_contact) const;
};

class ContactIdentification : public LibFred::PublicRequestAuthTypeIface
{
public:
    ~ContactIdentification()
    {
    }
    const LibFred::PublicRequestAuthTypeIface& iface() const
    {
        return *this;
    }
    std::string get_public_request_type() const;

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum _old_status,
            LibFred::PublicRequest::Status::Enum _new_status) const;
    std::string generate_passwords(const LibFred::LockedPublicRequestsOfObjectForUpdate& _locked_contact) const;
};

class ContactReidentification : public LibFred::PublicRequestAuthTypeIface
{
public:
    ~ContactReidentification()
    {
    }
    const LibFred::PublicRequestAuthTypeIface& iface() const
    {
        return *this;
    }
    std::string get_public_request_type() const;

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum _old_status,
            LibFred::PublicRequest::Status::Enum _new_status) const;
    std::string generate_passwords(const LibFred::LockedPublicRequestsOfObjectForUpdate& _locked_contact) const;
};

class ContactValidation : public LibFred::PublicRequestTypeIface
{
public:
    ~ContactValidation()
    {
    }
    const PublicRequestTypeIface& iface() const
    {
        return *this;
    }
    std::string get_public_request_type() const;

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum _old_status,
            LibFred::PublicRequest::Status::Enum _new_status) const;
};

class ConditionallyIdentifiedContactTransfer : public LibFred::PublicRequestAuthTypeIface
{
public:
    ~ConditionallyIdentifiedContactTransfer()
    {
    }
    const LibFred::PublicRequestAuthTypeIface& iface() const
    {
        return *this;
    }
    std::string get_public_request_type() const;

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum _old_status,
            LibFred::PublicRequest::Status::Enum _new_status) const;
    std::string generate_passwords(const LibFred::LockedPublicRequestsOfObjectForUpdate& _locked_contact) const;
};

class IdentifiedContactTransfer : public LibFred::PublicRequestAuthTypeIface
{
public:
    ~IdentifiedContactTransfer()
    {
    }
    const LibFred::PublicRequestAuthTypeIface& iface() const
    {
        return *this;
    }
    std::string get_public_request_type() const;

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum _old_status,
            LibFred::PublicRequest::Status::Enum _new_status) const;
    std::string generate_passwords(const LibFred::LockedPublicRequestsOfObjectForUpdate& _locked_contact) const;
};

class PrevalidatedUnidentifiedContactTransfer : public LibFred::PublicRequestAuthTypeIface
{
public:
    ~PrevalidatedUnidentifiedContactTransfer()
    {
    }
    const LibFred::PublicRequestAuthTypeIface& iface() const
    {
        return *this;
    }
    std::string get_public_request_type() const;

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum _old_status,
            LibFred::PublicRequest::Status::Enum _new_status) const;
    std::string generate_passwords(const LibFred::LockedPublicRequestsOfObjectForUpdate& _locked_contact) const;
};

class PrevalidatedContactTransfer : public LibFred::PublicRequestAuthTypeIface
{
public:
    ~PrevalidatedContactTransfer()
    {
    }
    const LibFred::PublicRequestAuthTypeIface& iface() const
    {
        return *this;
    }
    std::string get_public_request_type() const;

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const;
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum _old_status,
            LibFred::PublicRequest::Status::Enum _new_status) const;
    std::string generate_passwords(const LibFred::LockedPublicRequestsOfObjectForUpdate& _locked_contact) const;
};

} // namespace Fred::Backend::MojeId::PublicRequest
} // namespace Fred::Backend::MojeId
} // namespace Fred::Backend
} // namespace Fred

#endif
