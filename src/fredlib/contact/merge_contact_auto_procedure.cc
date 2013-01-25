#include "merge_contact_auto_procedure.h"

namespace Fred {
namespace Contact {


MergeContactAutoProcedure::MergeContactAutoProcedure()
{
}


MergeContactAutoProcedure::MergeContactAutoProcedure(
        const optional_string &_registrar,
        const optional_ulonglong &_limit)
    : registrar_(_registrar),
      limit_(_limit)
{
}


MergeContactAutoProcedure& MergeContactAutoProcedure::set_registrar(
        const optional_string &_registrar)
{
    registrar_ = _registrar;
    return *this;
}


MergeContactAutoProcedure& MergeContactAutoProcedure::set_limit(
        const optional_ulonglong &_limit)
{
    limit_ = _limit;
    return *this;
}


void MergeContactAutoProcedure::exec(Fred::OperationContext &_ctx)
{
}



}
}

