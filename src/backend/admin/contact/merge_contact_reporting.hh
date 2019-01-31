#ifndef MERGE_CONTACT_REPORTING_HH_C8872AAD743842349AD31C756D45F323
#define MERGE_CONTACT_REPORTING_HH_C8872AAD743842349AD31C756D45F323

#include "src/util/output_indenter.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"

#include <map>
#include <string>


namespace Admin {


struct MergeContactOperationSummary
{
    struct OperationCount
    {
        unsigned long long update_domain;
        unsigned long long update_nsset;
        unsigned long long update_keyset;
        unsigned long long delete_contact;

        OperationCount()
            : update_domain(0),
              update_nsset(0),
              update_keyset(0),
              delete_contact(0)

        {
        };

        unsigned long long get_total() const
        {
            return update_domain + update_nsset + update_keyset + delete_contact;
        }
    };
    typedef std::map<std::string, OperationCount> RegistrarOperationMap;
    RegistrarOperationMap ops_by_registrar;

    void add_merge_output(const LibFred::MergeContactOutput& _merge_data);

    std::string format(OutputIndenter _indenter);
};


struct MergeContactSummaryInfo
{
    unsigned long long merge_sets_total;
    unsigned long long merge_operations_total;
    unsigned long long merge_operations_in_current_set;
    unsigned long long invalid_contacts_total;
    unsigned long long invalid_contacts_in_current_set;
    unsigned long long linked_contacts_total;
    unsigned long long linked_contacts_in_current_set;

    MergeContactSummaryInfo()
        : merge_sets_total(0),
          merge_operations_total(0),
          merge_operations_in_current_set(0),
          invalid_contacts_total(0),
          invalid_contacts_in_current_set(0),
          linked_contacts_total(0),
          linked_contacts_in_current_set(0)
    {
    }

    void inc_merge_set()
    {
        merge_sets_total++;
        merge_operations_in_current_set = 0;
    }

    void inc_merge_operation()
    {
        merge_operations_total++;
        merge_operations_in_current_set++;
    }

    void inc_invalid_contacts()
    {
        invalid_contacts_total++;
        invalid_contacts_in_current_set++;
    }

    void inc_linked_contacts()
    {
        linked_contacts_total++;
        linked_contacts_in_current_set++;
    }
};


std::string format_header(const std::string& _text, OutputIndenter _indenter);


std::string format_merge_contact_output(
        const LibFred::MergeContactOutput& _merge_data,
        const std::string& _src_handle,
        const std::string& _dst_handle,
        const MergeContactSummaryInfo& _msi,
        OutputIndenter _indenter);



}


#endif /*ADMIN_MERGE_CONTACT_REPORTING_H__*/

