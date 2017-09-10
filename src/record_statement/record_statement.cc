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
 *  registry record statement implementation
 */

#include "src/record_statement/record_statement.hh"

#include "src/record_statement/impl/record_statement_xml.hh"
#include "src/record_statement/impl/factory.hh"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/zone/zone.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/log/context.h"
#include "util/util.h"
#include "util/subprocess.h"

#include <boost/lexical_cast.hpp>

#include <cstring>
#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <sstream>
#include <stdexcept>


namespace Registry {
namespace RecordStatement {

PdfBufferImpl::PdfBufferImpl(const std::string& s)
    : value(s)
{ }

RecordStatementImpl::RecordStatementImpl(
        const std::string &_server_name,
        const boost::shared_ptr<Fred::Document::Manager>& _doc_manager,
        const boost::shared_ptr<Fred::Mailer::Manager>& _mailer_manager,
        const std::string& _registry_timezone)
    : server_name_(_server_name),
      impl_(Fred::RecordStatement::Impl::Factory::produce(
              _registry_timezone,
              _doc_manager,
              _mailer_manager))
{ }

RecordStatementImpl::~RecordStatementImpl()
{ }

const std::string& RecordStatementImpl::get_server_name()const
{
    return server_name_;
}

template <Purpose::Enum _purpose>
PdfBufferImpl RecordStatementImpl::domain_printout(
        const std::string& _fqdn)const
{
    Fred::OperationContextCreator ctx;
    return impl_->domain_printout(ctx, _fqdn, _purpose);
}

template PdfBufferImpl RecordStatementImpl::domain_printout<Purpose::private_printout>(const std::string&)const;
template PdfBufferImpl RecordStatementImpl::domain_printout<Purpose::public_printout>(const std::string&)const;

PdfBufferImpl RecordStatementImpl::nsset_printout(
        const std::string& _handle)const
{
    Fred::OperationContextCreator ctx;
    return impl_->nsset_printout(ctx, _handle);
}

PdfBufferImpl RecordStatementImpl::keyset_printout(
        const std::string& _handle)const
{
    Fred::OperationContextCreator ctx;
    return impl_->keyset_printout(ctx, _handle);
}

template <Purpose::Enum _purpose>
PdfBufferImpl RecordStatementImpl::contact_printout(
        const std::string& _handle)const
{
    Fred::OperationContextCreator ctx;
    return impl_->contact_printout(ctx, _handle, _purpose);
}

template PdfBufferImpl RecordStatementImpl::contact_printout<Purpose::private_printout>(const std::string&)const;
template PdfBufferImpl RecordStatementImpl::contact_printout<Purpose::public_printout>(const std::string&)const;

PdfBufferImpl RecordStatementImpl::historic_domain_printout(
        const std::string& _fqdn,
        const Tz::LocalTimestamp& _valid_at)const
{
    Fred::OperationContextCreator ctx;
    return impl_->historic_domain_printout(ctx, _fqdn, _valid_at);
}

PdfBufferImpl RecordStatementImpl::historic_nsset_printout(
        const std::string& _handle,
        const Tz::LocalTimestamp& _valid_at)const
{
    Fred::OperationContextCreator ctx;
    return impl_->historic_nsset_printout(ctx, _handle, _valid_at);
}

PdfBufferImpl RecordStatementImpl::historic_keyset_printout(
        const std::string& _handle,
        const Tz::LocalTimestamp& _valid_at)const
{
    Fred::OperationContextCreator ctx;
    return impl_->historic_keyset_printout(ctx, _handle, _valid_at);
}

PdfBufferImpl RecordStatementImpl::historic_contact_printout(
        const std::string& _handle,
        const Tz::LocalTimestamp& _valid_at)const
{
    Fred::OperationContextCreator ctx;
    return impl_->historic_contact_printout(ctx, _handle, _valid_at);
}

template <Purpose::Enum _purpose>
void RecordStatementImpl::send_domain_printout(
        const std::string& _fqdn)const
{
    Fred::OperationContextCreator ctx;
    impl_->send_domain_printout(ctx, _fqdn, _purpose);
}

template void RecordStatementImpl::send_domain_printout<Purpose::private_printout>(const std::string&)const;
template void RecordStatementImpl::send_domain_printout<Purpose::public_printout>(const std::string&)const;

void RecordStatementImpl::send_nsset_printout(
        const std::string& _handle)const
{
    Fred::OperationContextCreator ctx;
    impl_->send_nsset_printout(ctx, _handle);
}

void RecordStatementImpl::send_keyset_printout(
        const std::string& _handle)const
{
    Fred::OperationContextCreator ctx;
    impl_->send_keyset_printout(ctx, _handle);
}

template <Purpose::Enum _purpose>
void RecordStatementImpl::send_contact_printout(
        const std::string& _handle)const
{
    Fred::OperationContextCreator ctx;
    impl_->send_contact_printout(ctx, _handle, _purpose);
}

template void RecordStatementImpl::send_contact_printout<Purpose::private_printout>(const std::string&)const;
template void RecordStatementImpl::send_contact_printout<Purpose::public_printout>(const std::string&)const;

}//namespace Registry::RecordStatement
}//namespace Registry
