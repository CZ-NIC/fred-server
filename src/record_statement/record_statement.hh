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

#include "src/record_statement/exceptions.hh"
#include "src/record_statement/purpose.hh"

#include "src/fredlib/documents.h"
#include "src/fredlib/mailer.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "src/fredlib/nsset/nsset_dns_host.h"
#include "src/fredlib/contact/place_address.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"
#include "util/timezones.hh"
#include "util/tz/get_psql_handle_of.hh"
#include "util/tz/local_timestamp.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>

namespace Registry {
namespace RecordStatement {

struct PdfBufferImpl
{
    explicit PdfBufferImpl(const std::string& _s);
    const std::string value;
};

class RecordStatementImpl
{
public:
    RecordStatementImpl(
            const std::string& _server_name,
            const boost::shared_ptr<Fred::Document::Manager>& _doc_manager,
            const boost::shared_ptr<Fred::Mailer::Manager>& _mailer_manager,
            const std::string& _handle_of_registry_timezone);//"Europe/Prague" or "UTC"
    virtual ~RecordStatementImpl();

    /**
     * Get server name
     * @return name for logging context
     */
    const std::string& get_server_name()const;

    template <Purpose::Enum _purpose>
    PdfBufferImpl domain_printout(
            const std::string& _fqdn)const;

    PdfBufferImpl nsset_printout(
            const std::string& _handle)const;

    PdfBufferImpl keyset_printout(
            const std::string& _handle)const;

    template <Purpose::Enum _purpose>
    PdfBufferImpl contact_printout(
            const std::string& _handle)const;

    PdfBufferImpl historic_domain_printout(
            const std::string& _fqdn,
            const Tz::LocalTimestamp& _valid_at)const;

    PdfBufferImpl historic_nsset_printout(
            const std::string& _handle,
            const Tz::LocalTimestamp& _valid_at)const;

    PdfBufferImpl historic_keyset_printout(
            const std::string& _handle,
            const Tz::LocalTimestamp& _valid_at)const;

    PdfBufferImpl historic_contact_printout(
            const std::string& _handle,
            const Tz::LocalTimestamp& _valid_at)const;

    template <Purpose::Enum _purpose>
    void send_domain_printout(
            const std::string& _fqdn)const;

    void send_nsset_printout(
            const std::string& _handle)const;

    void send_keyset_printout(
            const std::string& _handle)const;

    template <Purpose::Enum _purpose>
    void send_contact_printout(
            const std::string& _handle)const;

    class WithExternalContext
    {
    public:
        virtual ~WithExternalContext() { }

        virtual PdfBufferImpl domain_printout(
                Fred::OperationContext& _ctx,
                const std::string& _fqdn,
                Purpose::Enum _purpose)const = 0;

        virtual PdfBufferImpl nsset_printout(
                Fred::OperationContext& _ctx,
                const std::string& _handle)const = 0;

        virtual PdfBufferImpl keyset_printout(
                Fred::OperationContext& _ctx,
                const std::string& _handle)const = 0;

        virtual PdfBufferImpl contact_printout(
                Fred::OperationContext& _ctx,
                const std::string& _handle,
                Purpose::Enum _purpose)const = 0;

        virtual PdfBufferImpl historic_domain_printout(
                Fred::OperationContext& _ctx,
                const std::string& _fqdn,
                const Tz::LocalTimestamp& _valid_at)const = 0;

        virtual PdfBufferImpl historic_nsset_printout(
                Fred::OperationContext& _ctx,
                const std::string& _handle,
                const Tz::LocalTimestamp& _valid_at)const = 0;

        virtual PdfBufferImpl historic_keyset_printout(
                Fred::OperationContext& _ctx,
                const std::string& _handle,
                const Tz::LocalTimestamp& _valid_at)const = 0;

        virtual PdfBufferImpl historic_contact_printout(
                Fred::OperationContext& _ctx,
                const std::string& _handle,
                const Tz::LocalTimestamp& _valid_at)const = 0;

        virtual void send_domain_printout(
                Fred::OperationContext& _ctx,
                const std::string& _fqdn,
                Purpose::Enum _purpose)const = 0;

        virtual void send_nsset_printout(
                Fred::OperationContext& _ctx,
                const std::string& _handle)const = 0;

        virtual void send_keyset_printout(
                Fred::OperationContext& _ctx,
                const std::string& _handle)const = 0;

        virtual void send_contact_printout(
                Fred::OperationContext& _ctx,
                const std::string& _handle,
                Purpose::Enum _purpose)const = 0;
    };
private:
    std::string server_name_;
    boost::shared_ptr<WithExternalContext> impl_;
};

}//namespace Registry::RecordStatement
}//namespace Registry

#endif
