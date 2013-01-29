#include "merge_contact_auto_procedure.h"
#include "merge_contact_selection.h"
#include "merge_contact.h"
#include "find_contact_duplicates.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <algorithm>


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
    /* get system registrar - XXX: should be a parameter?? */
    Database::Result system_registrar_result = _ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system is True");
    if (system_registrar_result.size() != 1) {
        throw std::runtime_error("no system registrar found");
    }
    std::string system_registrar = static_cast<std::string>(system_registrar_result[0][0]);

    /* find any contact duplicates set (optionally for specific registrar only) */
    std::set<std::string> dup_set = FindAnyContactDuplicates().set_registrar(registrar_).exec(_ctx);

    if (dup_set.empty()) {
        _ctx.get_log().info("no contact duplicity");
        return;
    }

    while (dup_set.size() >= 2)
    {
        _ctx.get_log().debug(boost::format("contact duplicates set: { %1% }")
                % boost::algorithm::join(dup_set, ", "));

        /* filter for best contact selection */
        std::vector<ContactSelectionFilterType> selection_filter = boost::assign::list_of
            (MCS_FILTER_IDENTIFIED_CONTACT)
            (MCS_FILTER_CONDITIONALLY_IDENTIFIED_CONTACT)
            (MCS_FILTER_HANDLE_MOJEID_SYNTAX)
            (MCS_FILTER_MAX_DOMAINS_BOUND)
            (MCS_FILTER_MAX_OBJECTS_BOUND)
            (MCS_FILTER_RECENTLY_UPDATED)
            (MCS_FILTER_NOT_REGCZNIC)
            (MCS_FILTER_RECENTLY_CREATED);

        /* compute best handle to merge all others onto */
        std::string winner_handle = MergeContactSelection(
                std::vector<std::string>(dup_set.begin(), dup_set.end()), selection_filter).exec(_ctx);
        _ctx.get_log().debug(boost::format("winner handle: %1%") % winner_handle);

        /* remove winner contact from set */
        dup_set.erase(winner_handle);
        /* merge first one */
        std::string pick_one = *(dup_set.begin());

        Fred::OperationContext merge_octx;
        MergeContact(pick_one, winner_handle, system_registrar).exec(merge_octx);

        /* merge operation notification handling */

        merge_octx.commit_transaction();

        /* find contact duplicates for winner contact - if nothing changed in registry data this
         * would be the same list as in previous step but without the merged one */
        dup_set = FindSpecificContactDuplicates(winner_handle).exec(_ctx);
        /* if none go for another contact which has duplicates */
        if (dup_set.empty()) {
            dup_set = FindAnyContactDuplicates().set_registrar(registrar_).exec(_ctx);
        }
    }
}


}
}

