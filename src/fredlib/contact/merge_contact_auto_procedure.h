#ifndef MERGE_CONTACT_AUTO_PROC_H__
#define MERGE_CONTACT_AUTO_PROC_H__

#include "util/types/optional.h"
#include "fredlib/opcontext.h"


namespace Fred {
namespace Contact {


class MergeContactAutoProcedure
{
public:
    MergeContactAutoProcedure();

    MergeContactAutoProcedure(
            const optional_string &_registrar,
            const optional_ulonglong &_limit);

    MergeContactAutoProcedure& set_registrar(const optional_string &_registrar);

    MergeContactAutoProcedure& set_limit(const optional_ulonglong &_limit);

    void exec(Fred::OperationContext &_ctx);


private:
    optional_string registrar_;
    optional_ulonglong limit_;
};


}
}


#endif /*MERGE_CONTACT_AUTO_PROC_H__*/

