#ifndef MERGE_CONTACT_AUTO_PROC_H__
#define MERGE_CONTACT_AUTO_PROC_H__

/**
 *  @file
 *  contact merge auto procedure
 */

#include "src/admin/contact/merge_contact_reporting.h"
#include "src/fredlib/contact/merge_contact_email_notification_data.h"
#include "src/fredlib/contact/merge_contact_selection.h"
#include "src/fredlib/logger_client.h"
#include "src/fredlib/mailer.h"
#include "util/optional_value.h"

#include <ostream>

namespace Admin {

struct MergeContactDryRunInfo
{
    std::set<std::string> any_search_excluded;

    MergeContactDryRunInfo()
        : any_search_excluded()
    { }

    void add_search_excluded(const std::string&_handle)
    {
        any_search_excluded.insert(any_search_excluded.end(), _handle);
    }

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
    MergeContactAutoProcedure(Fred::Mailer::Manager& mm, Fred::Logger::LoggerClient &_logger_client, const std::string& _registrar);

    /**
     * Constructor.
     * @param mm legacy mailer interface
     * @param _logger_client legacy logger interface
     * @param _registrar merged contacts registrar handle
     * @param _dry_run just write what could be done; don't actually touch data
     * @param _verbose output verbosity level: 0,1,2,3 if _dry_run is set then 3
     */
    MergeContactAutoProcedure(
            Fred::Mailer::Manager& mm,
            Fred::Logger::LoggerClient &_logger_client,
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
            const std::vector<Fred::ContactSelectionFilterType> &_selection_filter_order);

    /**
     * Output verbosity level: 0,1,2,3 if dry_run is set then 3
     */
    MergeContactAutoProcedure& set_verbose(const Optional<unsigned short> &_verbose);

    /**
     * Execute automatic merge of duplicate contacts.
     * @return email notification data about object changes for testing purposes only
     */
    std::vector<Fred::MergeContactNotificationEmailWithAddr> exec(std::ostream& _output_stream);

private:
    bool is_set_dry_run() const;

    std::vector<Fred::ContactSelectionFilterType> get_default_selection_filter_order() const;

    Fred::MergeContactOutput merge_contact(
        const std::string& _src_contact,
        const std::string& _dst_contact,
        const std::string& _system_registrar
    );

    unsigned short get_verbose_level() const;

    Fred::Mailer::Manager& mm_;
    Fred::Logger::LoggerClient& logger_client_;

    std::string registrar_;
    Optional<bool> dry_run_;
    std::vector<Fred::ContactSelectionFilterType> selection_filter_order_;
    Optional<unsigned short> verbose_;

    MergeContactDryRunInfo dry_run_info_;
    MergeContactSummaryInfo summary_info_;
    MergeContactOperationSummary all_merge_operation_info_;

};

}

#endif /*MERGE_CONTACT_AUTO_PROC_H__*/
