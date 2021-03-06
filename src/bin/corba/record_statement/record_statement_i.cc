/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
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
 *  registry record statement corba implementation
 */

//pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/bin/corba ~/workspace_18680/enum/idl/idl/RecordStatement.idl

#include "src/bin/corba/record_statement/record_statement_i.hh"

#include "src/backend/record_statement/record_statement.hh"
#include "corba/RecordStatement.hh"
#include "src/bin/corba/util/corba_conversions_buffer.hh"
#include "src/bin/corba/util/corba_conversions_nullable_types.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <stdexcept>
#include <string>

namespace Registry {
namespace RecordStatement {

Server_i::Server_i(
        const std::string& _server_name,
        const std::shared_ptr<LibFred::Document::Manager>& _doc_manager,
        const std::shared_ptr<LibFred::Mailer::Manager>& _mailer_manager,
        const std::string& _registry_timezone)
    : impl_(new Fred::Backend::RecordStatement::RecordStatementImpl(
              _server_name,
              _doc_manager,
              _mailer_manager,
              _registry_timezone))
{
}

Server_i::~Server_i()
{
    delete impl_;
}

//   Methods corresponding to IDL attributes and operations
Buffer* Server_i::domain_printout(const char* _fqdn, ::CORBA::Boolean _is_private_printout)
{
    try
    {
        const Fred::Backend::Buffer pdf_content =
                _is_private_printout
                        ? impl_->domain_printout<Fred::Backend::RecordStatement::Purpose::private_printout>(LibFred::Corba::unwrap_string_from_const_char_ptr(_fqdn))
                        : impl_->domain_printout<Fred::Backend::RecordStatement::Purpose::public_printout>(LibFred::Corba::unwrap_string_from_const_char_ptr(_fqdn));

        return CorbaConversion::Util::wrap_Buffer(pdf_content)._retn();
    }
    catch (const Fred::Backend::RecordStatement::ObjectDeleteCandidate&)
    {
        throw OBJECT_DELETE_CANDIDATE();
    }
    catch (const Fred::Backend::RecordStatement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

Buffer* Server_i::nsset_printout(const char* handle)
{
    try
    {
        const Fred::Backend::Buffer pdf_content = impl_->nsset_printout(
                LibFred::Corba::unwrap_string_from_const_char_ptr(handle));

        return CorbaConversion::Util::wrap_Buffer(pdf_content)._retn();
    }
    catch (const Fred::Backend::RecordStatement::ObjectDeleteCandidate&)
    {
        throw OBJECT_DELETE_CANDIDATE();
    }
    catch (const Fred::Backend::RecordStatement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

Buffer* Server_i::keyset_printout(const char* handle)
{
    try
    {
        const Fred::Backend::Buffer pdf_content = impl_->keyset_printout(
                LibFred::Corba::unwrap_string_from_const_char_ptr(handle));

        return CorbaConversion::Util::wrap_Buffer(pdf_content)._retn();
    }
    catch (const Fred::Backend::RecordStatement::ObjectDeleteCandidate&)
    {
        throw OBJECT_DELETE_CANDIDATE();
    }
    catch (const Fred::Backend::RecordStatement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

Buffer* Server_i::contact_printout(const char* _handle, ::CORBA::Boolean _is_private_printout)
{
    try
    {
        const Fred::Backend::Buffer pdf_content =
                _is_private_printout
                        ? impl_->contact_printout<Fred::Backend::RecordStatement::Purpose::private_printout>(LibFred::Corba::unwrap_string_from_const_char_ptr(_handle))
                        : impl_->contact_printout<Fred::Backend::RecordStatement::Purpose::public_printout>(LibFred::Corba::unwrap_string_from_const_char_ptr(_handle));

        return CorbaConversion::Util::wrap_Buffer(pdf_content)._retn();
    }
    catch (const Fred::Backend::RecordStatement::ObjectDeleteCandidate&)
    {
        throw OBJECT_DELETE_CANDIDATE();
    }
    catch (const Fred::Backend::RecordStatement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

Buffer* Server_i::historic_domain_printout(const char* fqdn, const Registry::IsoDateTime& time)
{
    try
    {
        const Fred::Backend::Buffer pdf_content =
                impl_->historic_domain_printout(
                        LibFred::Corba::unwrap_string_from_const_char_ptr(fqdn),
                        Tz::LocalTimestamp::from_rfc3339_formated_string(LibFred::Corba::unwrap_string_from_const_char_ptr(time.value)));

        return CorbaConversion::Util::wrap_Buffer(pdf_content)._retn();
    }
    catch (const Fred::Backend::RecordStatement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (const Tz::LocalTimestamp::NotRfc3339Compliant&)
    {
        throw INVALID_TIMESTAMP();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

Buffer* Server_i::historic_nsset_printout(const char* handle, const Registry::IsoDateTime& time)
{
    try
    {
        const Fred::Backend::Buffer pdf_content =
                impl_->historic_nsset_printout(
                        LibFred::Corba::unwrap_string_from_const_char_ptr(handle),
                        Tz::LocalTimestamp::from_rfc3339_formated_string(LibFred::Corba::unwrap_string_from_const_char_ptr(time.value)));

        return CorbaConversion::Util::wrap_Buffer(pdf_content)._retn();
    }
    catch (const Fred::Backend::RecordStatement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (const Tz::LocalTimestamp::NotRfc3339Compliant&)
    {
        throw INVALID_TIMESTAMP();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

Buffer* Server_i::historic_keyset_printout(const char* handle, const Registry::IsoDateTime& time)
{
    try
    {
        const Fred::Backend::Buffer pdf_content =
                impl_->historic_keyset_printout(
                        LibFred::Corba::unwrap_string_from_const_char_ptr(handle),
                        Tz::LocalTimestamp::from_rfc3339_formated_string(LibFred::Corba::unwrap_string_from_const_char_ptr(time.value)));

        return CorbaConversion::Util::wrap_Buffer(pdf_content)._retn();
    }
    catch (const Fred::Backend::RecordStatement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (const Tz::LocalTimestamp::NotRfc3339Compliant&)
    {
        throw INVALID_TIMESTAMP();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

Buffer* Server_i::historic_contact_printout(const char* handle, const Registry::IsoDateTime& time)
{
    try
    {
        const Fred::Backend::Buffer pdf_content =
                impl_->historic_contact_printout(
                        LibFred::Corba::unwrap_string_from_const_char_ptr(handle),
                        Tz::LocalTimestamp::from_rfc3339_formated_string(LibFred::Corba::unwrap_string_from_const_char_ptr(time.value)));

        return CorbaConversion::Util::wrap_Buffer(pdf_content)._retn();
    }
    catch (const Fred::Backend::RecordStatement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (const Tz::LocalTimestamp::NotRfc3339Compliant&)
    {
        throw INVALID_TIMESTAMP();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

void Server_i::send_domain_printout(const char* fqdn)
{
    try
    {
        impl_->send_domain_printout<Fred::Backend::RecordStatement::Purpose::private_printout>(LibFred::Corba::unwrap_string_from_const_char_ptr(fqdn));
    }
    catch (const Fred::Backend::RecordStatement::ObjectDeleteCandidate&)
    {
        throw OBJECT_DELETE_CANDIDATE();
    }
    catch (const Fred::Backend::RecordStatement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

void Server_i::send_nsset_printout(const char* handle)
{
    try
    {
        impl_->send_nsset_printout(LibFred::Corba::unwrap_string_from_const_char_ptr(handle));
    }
    catch (const Fred::Backend::RecordStatement::ObjectDeleteCandidate&)
    {
        throw OBJECT_DELETE_CANDIDATE();
    }
    catch (const Fred::Backend::RecordStatement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

void Server_i::send_keyset_printout(const char* handle)
{
    try
    {
        impl_->send_keyset_printout(LibFred::Corba::unwrap_string_from_const_char_ptr(handle));
    }
    catch (const Fred::Backend::RecordStatement::ObjectDeleteCandidate&)
    {
        throw OBJECT_DELETE_CANDIDATE();
    }
    catch (const Fred::Backend::RecordStatement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

void Server_i::send_contact_printout(const char* handle)
{
    try
    {
        impl_->send_contact_printout<Fred::Backend::RecordStatement::Purpose::private_printout>(LibFred::Corba::unwrap_string_from_const_char_ptr(handle));
    }
    catch (const Fred::Backend::RecordStatement::ObjectDeleteCandidate&)
    {
        throw OBJECT_DELETE_CANDIDATE();
    }
    catch (const Fred::Backend::RecordStatement::ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

} // namespace Registry::RecordStatement
} // namespace Registry
