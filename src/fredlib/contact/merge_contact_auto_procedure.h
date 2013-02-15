#ifndef MERGE_CONTACT_AUTO_PROC_H__
#define MERGE_CONTACT_AUTO_PROC_H__

#include "util/types/optional.h"
#include "fredlib/opcontext.h"
#include "fredlib/logger_client.h"


namespace Fred {
namespace Contact {


class MergeContactAutoProcedure
{
public:
    MergeContactAutoProcedure(Fred::Logger::LoggerClient &_logger_client);

    MergeContactAutoProcedure(
            Fred::Logger::LoggerClient &_logger_client,
            const optional_string &_registrar,
            const optional_ulonglong &_limit,
            const optional_bool &_dry_run);

    MergeContactAutoProcedure& set_registrar(const optional_string &_registrar);

    MergeContactAutoProcedure& set_limit(const optional_ulonglong &_limit);

    MergeContactAutoProcedure& set_dry_run(const optional_bool &_dry_run);

    void exec();


private:
    bool is_set_dry_run() const;

    Fred::Logger::LoggerClient &logger_client_;

    optional_string registrar_;
    optional_ulonglong limit_;
    optional_bool dry_run_;
};


}
}


#endif /*MERGE_CONTACT_AUTO_PROC_H__*/

