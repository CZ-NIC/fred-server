#ifndef MERGE_CONTACT_AUTO_PROC_H__
#define MERGE_CONTACT_AUTO_PROC_H__

/**
 *  @file
 *  contact merge auto procedure
 */

#include "util/optional_value.h"
#include "src/fredlib/logger_client.h"
#include "src/fredlib/mailer.h"
#include "src/fredlib/contact/merge_contact_selection.h"
#include "src/fredlib/contact/merge_contact_email_notification_data.h"



namespace Admin {

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
    std::vector<Fred::MergeContactNotificationEmailWithAddr> exec();

private:
    bool is_set_dry_run() const;

    std::vector<Fred::ContactSelectionFilterType> get_default_selection_filter_order() const;

    unsigned short get_verbose_level() const;

    Fred::Mailer::Manager& mm_;
    Fred::Logger::LoggerClient &logger_client_;

    std::string registrar_;
    Optional<bool> dry_run_;
    std::vector<Fred::ContactSelectionFilterType> selection_filter_order_;
    Optional<unsigned short> verbose_;
};


}


#endif /*MERGE_CONTACT_AUTO_PROC_H__*/

