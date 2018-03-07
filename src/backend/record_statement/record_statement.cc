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

#include "src/backend/record_statement/record_statement.hh"

#include "src/backend/buffer.hh"
#include "src/backend/record_statement/impl/factory.hh"
#include "src/backend/record_statement/impl/record_statement_xml.hh"
#include "src/libfred/object/object_type.hh"
#include "src/libfred/object_state/get_object_states.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/domain/info_domain.hh"
#include "src/libfred/registrable_object/keyset/info_keyset.hh"
#include "src/libfred/registrable_object/nsset/info_nsset.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/libfred/zone/zone.hh"
#include "src/util/log/context.hh"
#include "src/util/random.hh"
#include "src/util/subprocess.hh"
#include "src/util/util.hh"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <cstring>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


namespace Fred {
namespace Backend {
namespace RecordStatement {

namespace {

std::string create_ctx_name(const std::string& _name)
{
    return str(boost::format("%1%-<%2%>") % _name % Random::integer(0, 10000));
}

std::string create_ctx_function_name(const char* fnc)
{
    std::string name(fnc);
    std::replace(name.begin(), name.end(), '_', '-');
    return name;
}

class LogContext
{
public:
    LogContext(const RecordStatementImpl& _impl, const std::string& _op_name)
        : ctx_server_(create_ctx_name(_impl.get_server_name())),
          ctx_interface_("record-statement"),
          ctx_operation_(_op_name)
    {
    }

private:
    Logging::Context ctx_server_;
    Logging::Context ctx_interface_;
    Logging::Context ctx_operation_;
};

#define LOGGING_CONTEXT(CTX_VAR, IMPL_OBJ) LogContext CTX_VAR((IMPL_OBJ), create_ctx_function_name(__FUNCTION__))
}


RecordStatementImpl::RecordStatementImpl(
        const std::string& _server_name,
        const std::shared_ptr<LibFred::Document::Manager>& _doc_manager,
        const std::shared_ptr<LibFred::Mailer::Manager>& _mailer_manager,
        const std::string& _registry_timezone)
    : server_name_(_server_name),
      impl_(Impl::Factory::produce(
              _registry_timezone,
              _doc_manager,
              _mailer_manager))
{
}

RecordStatementImpl::~RecordStatementImpl()
{
}

const std::string& RecordStatementImpl::get_server_name() const
{
    return server_name_;
}

template <Purpose::Enum _purpose>
Buffer RecordStatementImpl::domain_printout(
        const std::string& _fqdn) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        return impl_->domain_printout(ctx, _fqdn, _purpose);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

template Buffer RecordStatementImpl::domain_printout<Purpose::private_printout>(const std::string&) const;
template Buffer RecordStatementImpl::domain_printout<Purpose::public_printout>(const std::string&) const;

Buffer RecordStatementImpl::nsset_printout(
        const std::string& _handle) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        return impl_->nsset_printout(ctx, _handle);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

Buffer RecordStatementImpl::keyset_printout(
        const std::string& _handle) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        return impl_->keyset_printout(ctx, _handle);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

template <Purpose::Enum _purpose>
Buffer RecordStatementImpl::contact_printout(
        const std::string& _handle) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        return impl_->contact_printout(ctx, _handle, _purpose);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

template Buffer RecordStatementImpl::contact_printout<Purpose::private_printout>(const std::string&) const;
template Buffer RecordStatementImpl::contact_printout<Purpose::public_printout>(const std::string&) const;

Buffer RecordStatementImpl::historic_domain_printout(
        const std::string& _fqdn,
        const Tz::LocalTimestamp& _valid_at) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        return impl_->historic_domain_printout(ctx, _fqdn, _valid_at);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

Buffer RecordStatementImpl::historic_nsset_printout(
        const std::string& _handle,
        const Tz::LocalTimestamp& _valid_at) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        return impl_->historic_nsset_printout(ctx, _handle, _valid_at);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

Buffer RecordStatementImpl::historic_keyset_printout(
        const std::string& _handle,
        const Tz::LocalTimestamp& _valid_at) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        return impl_->historic_keyset_printout(ctx, _handle, _valid_at);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

Buffer RecordStatementImpl::historic_contact_printout(
        const std::string& _handle,
        const Tz::LocalTimestamp& _valid_at) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        return impl_->historic_contact_printout(ctx, _handle, _valid_at);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

template <Purpose::Enum _purpose>
void RecordStatementImpl::send_domain_printout(
        const std::string& _fqdn) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        impl_->send_domain_printout(ctx, _fqdn, _purpose);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

template void RecordStatementImpl::send_domain_printout<Purpose::private_printout>(const std::string&) const;
template void RecordStatementImpl::send_domain_printout<Purpose::public_printout>(const std::string&) const;

void RecordStatementImpl::send_nsset_printout(
        const std::string& _handle) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        impl_->send_nsset_printout(ctx, _handle);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

void RecordStatementImpl::send_keyset_printout(
        const std::string& _handle) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        impl_->send_keyset_printout(ctx, _handle);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

template <Purpose::Enum _purpose>
void RecordStatementImpl::send_contact_printout(
        const std::string& _handle) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        impl_->send_contact_printout(ctx, _handle, _purpose);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

template void RecordStatementImpl::send_contact_printout<Purpose::private_printout>(const std::string&) const;
template void RecordStatementImpl::send_contact_printout<Purpose::public_printout>(const std::string&) const;

} // namespace Fred::Backend::RecordStatement
} // namespace Fred::Backend
} // namespace Fred
