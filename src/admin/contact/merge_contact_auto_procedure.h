#ifndef MERGE_CONTACT_AUTO_PROC_H__
#define MERGE_CONTACT_AUTO_PROC_H__

#include "util/optional_value.h"
#include "src/fredlib/logger_client.h"
#include "src/fredlib/mailer.h"
#include "src/fredlib/contact/merge_contact_selection.h"
#include "src/fredlib/contact/merge_contact_email_notification_data.h"



namespace Admin {


class MergeContactAutoProcedure
{
public:
    MergeContactAutoProcedure(Fred::Mailer::Manager& mm, Fred::Logger::LoggerClient &_logger_client);

    MergeContactAutoProcedure(
            Fred::Mailer::Manager& mm,
            Fred::Logger::LoggerClient &_logger_client,
            const Optional<std::string> &_registrar,
            const Optional<unsigned long long> &_limit,
            const Optional<bool> &_dry_run,
            const Optional<unsigned short> &_verbose);

    MergeContactAutoProcedure& set_registrar(const Optional<std::string> &_registrar);

    MergeContactAutoProcedure& set_limit(const Optional<unsigned long long> &_limit);

    MergeContactAutoProcedure& set_dry_run(const Optional<bool> &_dry_run);

    MergeContactAutoProcedure& set_selection_filter_order(
            const std::vector<Fred::ContactSelectionFilterType> &_selection_filter_order);

    MergeContactAutoProcedure& set_verbose(const Optional<unsigned short> &_verbose);

    std::vector<Fred::MergeContactNotificationEmailWithAddr> exec();


private:
    bool is_set_dry_run() const;

    std::vector<Fred::ContactSelectionFilterType> get_default_selection_filter_order() const;

    unsigned short get_verbose_level() const;

    Fred::Mailer::Manager& mm_;
    Fred::Logger::LoggerClient &logger_client_;

    Optional<std::string> registrar_;
    Optional<unsigned long long> limit_;
    Optional<bool> dry_run_;
    std::vector<Fred::ContactSelectionFilterType> selection_filter_order_;
    Optional<unsigned short> verbose_;
};


}


#endif /*MERGE_CONTACT_AUTO_PROC_H__*/

