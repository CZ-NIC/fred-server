/*
 * Copyright (C) 2013-2022  CZ.NIC, z. s. p. o.
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

#ifndef MERGE_CONTACT_AUTO_PROCEDURE_HH_2C54E9D7D6834B688C1326BB111C34DC
#define MERGE_CONTACT_AUTO_PROCEDURE_HH_2C54E9D7D6834B688C1326BB111C34DC

#include "src/backend/admin/contact/merge_contact_reporting.hh"
#include "libfred/registrable_object/contact/merge_contact_email_notification_data.hh"
#include "libfred/registrable_object/contact/merge_contact_selection.hh"
#include "src/deprecated/libfred/logger_client.hh"
#include "libfred/mailer.hh"
#include "util/optional_value.hh"

#include <ostream>

namespace Admin {

struct MergeContactDryRunInfo
{
    MergeContactDryRunInfo()
        : any_search_excluded()
    { }
    void add_search_excluded(const std::string&_handle)
    {
        any_search_excluded.insert(any_search_excluded.end(), _handle);
    }
    std::set<std::string> any_search_excluded;
};

/**
 * Contact duplicates merge procedure.
 */
class MergeContactAutoProcedure
{
public:
    /**
     * Minimal constructor.
     * @param mm legacy mailer interface
     * @param _logger_client legacy logger interface
     * @param _registrar merged contacts registrar handle
     */
    MergeContactAutoProcedure(LibFred::Mailer::Manager& mm, LibFred::Logger::LoggerClient &_logger_client, const std::string& _registrar);

    /**
     * Constructor.
     * @param mm legacy mailer interface
     * @param _logger_client legacy logger interface
     * @param _registrar merged contacts registrar handle
     * @param _dry_run just write what could be done; don't actually touch data
     * @param _verbose output verbosity level: 0,1,2,3 if _dry_run is set then 3
     */
    MergeContactAutoProcedure(
            LibFred::Mailer::Manager& mm,
            LibFred::Logger::LoggerClient &_logger_client,
            const std::string& _registrar,
            const Optional<bool> &_dry_run,
            const Optional<unsigned short> &_verbose);

    /**
     * Just print not do actions
     */
    MergeContactAutoProcedure& set_dry_run(const Optional<bool> &_dry_run);

    /**
     * Specify custom order of filters for selection of destination contact.
     * If not set, there is default in @ref get_default_selection_filter_order()
     */
    MergeContactAutoProcedure& set_selection_filter_order(
            const std::vector<std::string> &_selection_filter_order);

    /**
     * Output verbosity level: 0,1,2,3 if dry_run is set then 3
     */
    MergeContactAutoProcedure& set_verbose(const Optional<unsigned short> &_verbose);

    /**
     * Execute automatic merge of duplicate contacts.
     * @return email notification data about object changes for testing purposes only
     */
    std::vector<LibFred::MergeContactNotificationEmailWithAddr> exec(std::ostream& _output_stream);
private:
    bool is_set_dry_run() const;

    std::vector<std::string> get_default_selection_filter_order() const;

    LibFred::MergeContactOutput merge_contact(
        const std::string& _src_contact,
        const std::string& _dst_contact,
        const std::string& _system_registrar
    );

    unsigned short get_verbose_level() const;

    LibFred::Mailer::Manager& mm_;
    LibFred::Logger::LoggerClient& logger_client_;

    std::string registrar_;
    Optional<bool> dry_run_;
    std::vector<std::string> selection_filter_order_;
    Optional<unsigned short> verbose_;

    MergeContactDryRunInfo dry_run_info_;
    MergeContactSummaryInfo summary_info_;
    MergeContactOperationSummary all_merge_operation_info_;
};

}//namespace Admin

#endif//MERGE_CONTACT_AUTO_PROCEDURE_HH_2C54E9D7D6834B688C1326BB111C34DC
