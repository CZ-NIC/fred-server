#ifndef ADMIN_MERGE_CONTACT_REPORTING_H__
#define ADMIN_MERGE_CONTACT_REPORTING_H__

#include "util/output_indenter.h"
#include "src/fredlib/contact/merge_contact.h"

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

    void add_merge_output(const Fred::MergeContactOutput &_merge_data);

    std::string format(OutputIndenter _indenter);
};


struct MergeContactSummaryInfo
{
    unsigned long long merge_set_counter;
    unsigned long long merge_counter;
    unsigned long long merge_per_merge_set_counter;

    MergeContactSummaryInfo()
        : merge_set_counter(0),
          merge_counter(0),
          merge_per_merge_set_counter(0)
    {
    }

    void inc_merge_set()
    {
        merge_set_counter += 1;
        merge_per_merge_set_counter = 0;
    }

    void inc_merge_operation()
    {
        merge_counter += 1;
        merge_per_merge_set_counter += 1;
    }
};


std::string format_header(const std::string &_text, OutputIndenter _indenter);


std::string format_merge_contact_output(
        const Fred::MergeContactOutput &_merge_data,
        const std::string &_src_handle,
        const std::string &_dst_handle,
        const MergeContactSummaryInfo &_msi,
        OutputIndenter _indenter);



}


#endif /*ADMIN_MERGE_CONTACT_REPORTING_H__*/

