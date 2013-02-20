#ifndef MERGE_CONTACT_AUTO_PROC_H__
#define MERGE_CONTACT_AUTO_PROC_H__

#include "util/types/optional.h"
#include "fredlib/opcontext.h"
#include "fredlib/logger_client.h"
#include "merge_contact_selection.h"
#include "mailer.h"



namespace Fred {
namespace Contact {


class MergeContactAutoProcedure
{
public:
    MergeContactAutoProcedure(Fred::Mailer::Manager& mm, Fred::Logger::LoggerClient &_logger_client);

    MergeContactAutoProcedure(
            Fred::Mailer::Manager& mm,
            Fred::Logger::LoggerClient &_logger_client,
            const optional_string &_registrar,
            const optional_ulonglong &_limit,
            const optional_bool &_dry_run);

    MergeContactAutoProcedure& set_registrar(const optional_string &_registrar);

    MergeContactAutoProcedure& set_limit(const optional_ulonglong &_limit);

    MergeContactAutoProcedure& set_dry_run(const optional_bool &_dry_run);

    MergeContactAutoProcedure& set_selection_filter_order(
            const std::vector<ContactSelectionFilterType> &_selection_filter_order);

    void exec();


private:
    bool is_set_dry_run() const;

    std::vector<ContactSelectionFilterType> get_default_selection_filter_order() const;

    Fred::Mailer::Manager& mm_;
    Fred::Logger::LoggerClient &logger_client_;

    optional_string registrar_;
    optional_ulonglong limit_;
    optional_bool dry_run_;
    std::vector<ContactSelectionFilterType> selection_filter_order_;
};


}
}


#endif /*MERGE_CONTACT_AUTO_PROC_H__*/

