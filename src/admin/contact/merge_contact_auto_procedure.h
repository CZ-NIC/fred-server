#ifndef MERGE_CONTACT_AUTO_PROC_H__
#define MERGE_CONTACT_AUTO_PROC_H__

#include "util/types/optional.h"
#include "src/fredlib/logger_client.h"
#include "src/fredlib/mailer.h"
#include "src/fredlib/contact/merge_contact_selection.h"



namespace Admin {


class MergeContactAutoProcedure
{
public:
    MergeContactAutoProcedure(Fred::Mailer::Manager& mm, Fred::Logger::LoggerClient &_logger_client);

    MergeContactAutoProcedure(
            Fred::Mailer::Manager& mm,
            Fred::Logger::LoggerClient &_logger_client,
            const optional_string &_registrar,
            const optional_ulonglong &_limit,
            const optional_bool &_dry_run,
            const optional_ushort &_verbose);

    MergeContactAutoProcedure& set_registrar(const optional_string &_registrar);

    MergeContactAutoProcedure& set_limit(const optional_ulonglong &_limit);

    MergeContactAutoProcedure& set_dry_run(const optional_bool &_dry_run);

    MergeContactAutoProcedure& set_selection_filter_order(
            const std::vector<Fred::ContactSelectionFilterType> &_selection_filter_order);

    MergeContactAutoProcedure& set_verbose(const optional_ushort &_verbose);

    void exec();


private:
    bool is_set_dry_run() const;

    std::vector<Fred::ContactSelectionFilterType> get_default_selection_filter_order() const;

    unsigned short get_verbose_level() const;

    Fred::Mailer::Manager& mm_;
    Fred::Logger::LoggerClient &logger_client_;

    optional_string registrar_;
    optional_ulonglong limit_;
    optional_bool dry_run_;
    std::vector<Fred::ContactSelectionFilterType> selection_filter_order_;
    optional_ushort verbose_;
};


}


#endif /*MERGE_CONTACT_AUTO_PROC_H__*/

