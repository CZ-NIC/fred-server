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
 *  header of registry record statement implementation
 */

#ifndef RECORD_STATEMENT_HH_72FF9604ED0747FAA4BC91F598728BBC
#define RECORD_STATEMENT_HH_72FF9604ED0747FAA4BC91F598728BBC

#include "src/backend/buffer.hh"
#include "src/backend/record_statement/exceptions.hh"
#include "src/backend/record_statement/purpose.hh"
#include "src/deprecated/libfred/documents.hh"
#include "libfred/mailer.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/place_address.hh"
#include "libfred/registrable_object/domain/enum_validation_extension.hh"
#include "libfred/registrable_object/nsset/nsset_dns_host.hh"
#include "util/db/nullable.hh"
#include "util/optional_value.hh"
#include "src/util/timezones.hh"
#include "src/util/tz/get_psql_handle_of.hh"
#include "src/util/tz/local_timestamp.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace RecordStatement {

class RecordStatementImpl
{
public:
    RecordStatementImpl(
            const std::string& _server_name,
            const std::shared_ptr<LibFred::Document::Manager>& _doc_manager,
            const std::shared_ptr<LibFred::Mailer::Manager>& _mailer_manager,
            const std::string& _handle_of_registry_timezone); //"Europe/Prague" or "UTC"
    virtual ~RecordStatementImpl();

    /**
     * Get server name
     * @return name for logging context
     */
    const std::string& get_server_name() const;

    template <Purpose::Enum _purpose>
    Buffer domain_printout(
            const std::string& _fqdn) const;

    Buffer nsset_printout(
            const std::string& _handle) const;

    Buffer keyset_printout(
            const std::string& _handle) const;

    template <Purpose::Enum _purpose>
    Buffer contact_printout(
            const std::string& _handle) const;

    Buffer historic_domain_printout(
            const std::string& _fqdn,
            const Tz::LocalTimestamp& _valid_at) const;

    Buffer historic_nsset_printout(
            const std::string& _handle,
            const Tz::LocalTimestamp& _valid_at) const;

    Buffer historic_keyset_printout(
            const std::string& _handle,
            const Tz::LocalTimestamp& _valid_at) const;

    Buffer historic_contact_printout(
            const std::string& _handle,
            const Tz::LocalTimestamp& _valid_at) const;

    template <Purpose::Enum _purpose>
    void send_domain_printout(
            const std::string& _fqdn) const;

    void send_nsset_printout(
            const std::string& _handle) const;

    void send_keyset_printout(
            const std::string& _handle) const;

    template <Purpose::Enum _purpose>
    void send_contact_printout(
            const std::string& _handle) const;

    class WithExternalContext
    {
    public:
        virtual ~WithExternalContext()
        {
        }

        virtual Buffer domain_printout(
                LibFred::OperationContext& _ctx,
                const std::string& _fqdn,
                Purpose::Enum _purpose) const = 0;

        virtual Buffer nsset_printout(
                LibFred::OperationContext& _ctx,
                const std::string& _handle) const = 0;

        virtual Buffer keyset_printout(
                LibFred::OperationContext& _ctx,
                const std::string& _handle) const = 0;

        virtual Buffer contact_printout(
                LibFred::OperationContext& _ctx,
                const std::string& _handle,
                Purpose::Enum _purpose) const = 0;

        virtual Buffer historic_domain_printout(
                LibFred::OperationContext& _ctx,
                const std::string& _fqdn,
                const Tz::LocalTimestamp& _valid_at) const = 0;

        virtual Buffer historic_nsset_printout(
                LibFred::OperationContext& _ctx,
                const std::string& _handle,
                const Tz::LocalTimestamp& _valid_at) const = 0;

        virtual Buffer historic_keyset_printout(
                LibFred::OperationContext& _ctx,
                const std::string& _handle,
                const Tz::LocalTimestamp& _valid_at) const = 0;

        virtual Buffer historic_contact_printout(
                LibFred::OperationContext& _ctx,
                const std::string& _handle,
                const Tz::LocalTimestamp& _valid_at) const = 0;

        virtual void send_domain_printout(
                LibFred::OperationContext& _ctx,
                const std::string& _fqdn,
                Purpose::Enum _purpose) const = 0;

        virtual void send_nsset_printout(
                LibFred::OperationContext& _ctx,
                const std::string& _handle) const = 0;

        virtual void send_keyset_printout(
                LibFred::OperationContext& _ctx,
                const std::string& _handle) const = 0;

        virtual void send_contact_printout(
                LibFred::OperationContext& _ctx,
                const std::string& _handle,
                Purpose::Enum _purpose) const = 0;
    };

private:
    std::string server_name_;
    std::shared_ptr<WithExternalContext> impl_;
};

} // namespace Fred::Backend::RecordStatement
} // namespace Fred::Backend
} // namespace Fred

#endif
