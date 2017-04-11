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

#include "src/record_statement/record_statement_exception.hh"

#include "src/fredlib/documents.h"
#include "src/fredlib/mailer.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "src/fredlib/nsset/nsset_dns_host.h"
#include "src/fredlib/contact/place_address.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>

namespace Registry
{
namespace RecordStatement
{

    struct PdfBufferImpl
    {
        explicit PdfBufferImpl(const std::string& s);
        const std::string value;
    };

    class RecordStatementImpl
    {
    private:
        std::string server_name_;
        boost::shared_ptr<Fred::Document::Manager> doc_manager_;
        boost::shared_ptr<Fred::Mailer::Manager> mailer_manager_;
        std::string registry_timezone_;

    public:
        RecordStatementImpl(
            const std::string& server_name,
            boost::shared_ptr<Fred::Document::Manager> doc_manager,
            boost::shared_ptr<Fred::Mailer::Manager> mailer_manager,
            const std::string& registry_timezone_);
        virtual ~RecordStatementImpl();

        /**
         * Get server name
         * @return name for logging context
         */
        std::string get_server_name() const;

        PdfBufferImpl domain_printout(const std::string& fqdn,
                bool is_private_printout,
                Fred::OperationContext* ext_ctx = NULL/**< optional test context */
            ) const;

        PdfBufferImpl nsset_printout(const std::string& handle,
                Fred::OperationContext* ext_ctx = NULL/**< optional test context */
            ) const;

        PdfBufferImpl keyset_printout(const std::string& handle,
                Fred::OperationContext* ext_ctx = NULL/**< optional test context */
            ) const;

        PdfBufferImpl contact_printout(const std::string& handle,
                bool is_private_printout,
                Fred::OperationContext* ext_ctx = NULL/**< optional test context */
            ) const;

        PdfBufferImpl historic_domain_printout(
                const std::string& fqdn,
                const std::string& timestamp,/**< RFC3339 format: YYYY-MM-DDTHH:MM:SS+ZZ:ZZ, in case of missing zone offset, it is considered to be time in UTC */
                Fred::OperationContext* ex_ctx = NULL /**< optional test context */
            ) const;

        PdfBufferImpl historic_nsset_printout(
                const std::string& handle,
                const std::string& timestamp,/**< RFC3339 format: YYYY-MM-DDTHH:MM:SS+ZZ:ZZ, in case of missing zone offset, it is considered to be time in UTC */
                Fred::OperationContext* ext_ctx = NULL/**< optional test context */
            ) const;

        PdfBufferImpl historic_keyset_printout(
                const std::string& handle,
                const std::string& timestamp,/**< RFC3339 format: YYYY-MM-DDTHH:MM:SS+ZZ:ZZ, in case of missing zone offset, it is considered to be time in UTC */
                Fred::OperationContext* ext_ctx = NULL/**< optional test context */
            ) const;

        PdfBufferImpl historic_contact_printout(
                const std::string& handle,
                const std::string& timestamp,/**< RFC3339 format: YYYY-MM-DDTHH:MM:SS+ZZ:ZZ, in case of missing zone offset, it is considered to be time in UTC */
                Fred::OperationContext* ext_ctx = NULL/**< optional test context */
            ) const;

        void send_domain_printout(
                const std::string& fqdn,
                bool is_private_printout,
                Fred::OperationContext* ext_ctx = NULL/**< optional test context */
            ) const;

        void send_nsset_printout(
                const std::string& handle,
                Fred::OperationContext* ext_ctx = NULL/**< optional test context */
            ) const;

        void send_keyset_printout(
                const std::string& handle,
                Fred::OperationContext* ext_ctx = NULL/**< optional test context */
            ) const;

        void send_contact_printout(
                const std::string& handle,
                bool is_private_printout,
                Fred::OperationContext* ex_ctx = NULL/**< optional test context */
            ) const;

    };

}//namespace RecordStatement
}//namespace Registry

#endif
