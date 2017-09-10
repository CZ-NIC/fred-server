/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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
 *  registry record statement corba implementation
 */

//pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/corba ~/workspace_18680/enum/idl/idl/RecordStatement.idl

#include "src/corba/record_statement/record_statement_i.hh"

#include "src/record_statement/record_statement.hh"
#include "src/corba/util/corba_conversions_string.h"
#include "src/corba/util/corba_conversions_datetime.h"
#include "src/corba/util/corba_conversions_nullable_types.h"
#include "src/corba/RecordStatement.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>
#include <stdexcept>

namespace Registry {
namespace RecordStatement {

namespace {

PdfBuffer_var wrap_pdf_buffer(const PdfBufferImpl& pdf)
{
    PdfBuffer_var result(new PdfBuffer());

    result->data.length(pdf.value.size());
    if (!pdf.value.empty())
    {
        std::memcpy(result->data.get_buffer(), pdf.value.c_str(), pdf.value.size());
    }

    return result;
}

}//namespace Registry::RecordStatement::{anonymous}

Server_i::Server_i(
        const std::string &_server_name,
        const boost::shared_ptr<Fred::Document::Manager>& _doc_manager,
        const boost::shared_ptr<Fred::Mailer::Manager>& _mailer_manager,
        const std::string& _registry_timezone)
    : impl_(new Registry::RecordStatement::RecordStatementImpl(
            _server_name,
            _doc_manager,
            _mailer_manager,
            _registry_timezone))
{}

Server_i::~Server_i()
{
    delete impl_;
}

//   Methods corresponding to IDL attributes and operations
PdfBuffer* Server_i::domain_printout(const char* _fqdn, ::CORBA::Boolean _is_private_printout)
{
    try
    {
        const PdfBufferImpl pdf_content = _is_private_printout
                ? impl_->domain_printout<Purpose::private_printout>(Fred::Corba::unwrap_string_from_const_char_ptr(_fqdn))
                : impl_->domain_printout<Purpose::public_printout>(Fred::Corba::unwrap_string_from_const_char_ptr(_fqdn));

        return wrap_pdf_buffer(pdf_content)._retn();
    }
    catch (const ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

PdfBuffer* Server_i::nsset_printout(const char* handle)
{
    try
    {
        const PdfBufferImpl pdf_content = impl_->nsset_printout(
                Fred::Corba::unwrap_string_from_const_char_ptr(handle));

        return wrap_pdf_buffer(pdf_content)._retn();
    }
    catch (const ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

PdfBuffer* Server_i::keyset_printout(const char* handle)
{
    try
    {
        const PdfBufferImpl pdf_content = impl_->keyset_printout(
                Fred::Corba::unwrap_string_from_const_char_ptr(handle));

        return wrap_pdf_buffer(pdf_content)._retn();
    }
    catch (const ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

PdfBuffer* Server_i::contact_printout(const char* _handle, ::CORBA::Boolean _is_private_printout)
{
    try
    {
        const PdfBufferImpl pdf_content = _is_private_printout
                ? impl_->contact_printout<Purpose::private_printout>(Fred::Corba::unwrap_string_from_const_char_ptr(_handle))
                : impl_->contact_printout<Purpose::public_printout>(Fred::Corba::unwrap_string_from_const_char_ptr(_handle));

        return wrap_pdf_buffer(pdf_content)._retn();
    }
    catch (const ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

PdfBuffer* Server_i::historic_domain_printout(const char* fqdn, const Registry::RecordStatement::DateTime& time)
{
    try
    {
        const PdfBufferImpl pdf_content = impl_->historic_domain_printout(
                Fred::Corba::unwrap_string_from_const_char_ptr(fqdn),
                Tz::LocalTimestamp::from_rfc3339_formated_string(Fred::Corba::unwrap_string_from_const_char_ptr(time.value)));

        return wrap_pdf_buffer(pdf_content)._retn();
    }
    catch (const ObjectNotFound&)
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

PdfBuffer* Server_i::historic_nsset_printout(const char* handle, const Registry::RecordStatement::DateTime& time)
{
    try
    {
        const PdfBufferImpl pdf_content = impl_->historic_nsset_printout(
                Fred::Corba::unwrap_string_from_const_char_ptr(handle),
                Tz::LocalTimestamp::from_rfc3339_formated_string(Fred::Corba::unwrap_string_from_const_char_ptr(time.value)));

        return wrap_pdf_buffer(pdf_content)._retn();
    }
    catch (const ObjectNotFound&)
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

PdfBuffer* Server_i::historic_keyset_printout(const char* handle, const Registry::RecordStatement::DateTime& time)
{
    try
    {
        const PdfBufferImpl pdf_content = impl_->historic_keyset_printout(
                Fred::Corba::unwrap_string_from_const_char_ptr(handle),
                Tz::LocalTimestamp::from_rfc3339_formated_string(Fred::Corba::unwrap_string_from_const_char_ptr(time.value)));

        return wrap_pdf_buffer(pdf_content)._retn();
    }
    catch (const ObjectNotFound&)
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

PdfBuffer* Server_i::historic_contact_printout(const char* handle, const Registry::RecordStatement::DateTime& time)
{
    try
    {
        const PdfBufferImpl pdf_content = impl_->historic_contact_printout(
                Fred::Corba::unwrap_string_from_const_char_ptr(handle),
                Tz::LocalTimestamp::from_rfc3339_formated_string(Fred::Corba::unwrap_string_from_const_char_ptr(time.value)));

        return wrap_pdf_buffer(pdf_content)._retn();
    }
    catch (const ObjectNotFound&)
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
        impl_->send_domain_printout<Purpose::private_printout>(Fred::Corba::unwrap_string_from_const_char_ptr(fqdn));
    }
    catch (const ObjectNotFound&)
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
        impl_->send_nsset_printout(Fred::Corba::unwrap_string_from_const_char_ptr(handle));
    }
    catch (const ObjectNotFound&)
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
        impl_->send_keyset_printout(Fred::Corba::unwrap_string_from_const_char_ptr(handle));
    }
    catch (const ObjectNotFound&)
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
        impl_->send_contact_printout<Purpose::private_printout>(Fred::Corba::unwrap_string_from_const_char_ptr(handle));
    }
    catch (const ObjectNotFound&)
    {
        throw OBJECT_NOT_FOUND();
    }
    catch (...)
    {
        throw INTERNAL_SERVER_ERROR();
    }
}

}//namespace Registry::RecordStatement
}//namespace Registry
