#include "merge_contact_reporting.h"

namespace Admin {


std::string format_header(const std::string &_text, OutputIndenter _indenter)
{
    std::stringstream output;
    output << _indenter << std::string(_text.length(), '-') << std::endl;
    output << _indenter << _text << std::endl;
    output << _indenter << std::string(_text.length(), '-') << std::endl;
    return output.str();
}



void MergeContactOperationSummary::add_merge_output(const Fred::MergeContactOutput &_merge_data)
{
    for (std::vector<Fred::MergeContactUpdateDomainRegistrant>::const_iterator i = _merge_data.update_domain_registrant.begin();
            i != _merge_data.update_domain_registrant.end(); ++i)
    {
        ops_by_registrar[i->sponsoring_registrar].update_domain += 1;
    }
    for (std::vector<Fred::MergeContactUpdateDomainAdminContact>::const_iterator i = _merge_data.update_domain_admin_contact.begin();
            i != _merge_data.update_domain_admin_contact.end(); ++i)
    {
        ops_by_registrar[i->sponsoring_registrar].update_domain += 1;
    }
    for (std::vector<Fred::MergeContactUpdateNssetTechContact>::const_iterator i = _merge_data.update_nsset_tech_contact.begin();
            i != _merge_data.update_nsset_tech_contact.end(); ++i)
    {
        ops_by_registrar[i->sponsoring_registrar].update_nsset += 1;
    }
    for (std::vector<Fred::MergeContactUpdateKeysetTechContact>::const_iterator i = _merge_data.update_keyset_tech_contact.begin();
            i != _merge_data.update_keyset_tech_contact.end(); ++i)
    {
        ops_by_registrar[i->sponsoring_registrar].update_keyset += 1;
    }
    ops_by_registrar[_merge_data.contactid.src_contact_sponsoring_registrar].delete_contact += 1;
}



std::string MergeContactOperationSummary::format(OutputIndenter _indenter)
{
    std::stringstream output;
    std::string fmt_str("%1% %20t%2% %35t%3% %50t%4% %65t%5% %80t%6%");
    std::string header = str(boost::format(fmt_str)
            % "registrar" % "total" % "update_domain" % "update_nsset" % "update_keyset" % "delete_contact");

    output << _indenter << std::string(header.length(), '-') << std::endl;
    output << _indenter <<  header << std::endl;
    output << _indenter << std::string(header.length(), '-') << std::endl;

    for (RegistrarOperationMap::const_iterator i = ops_by_registrar.begin();
            i != ops_by_registrar.end(); ++i)
    {
        output << _indenter << str(boost::format(fmt_str) % i->first % i->second.get_total()
                % i->second.update_domain % i->second.update_nsset % i->second.update_keyset
                % i->second.delete_contact) << std::endl;
    }
    output << std::endl;
    return output.str();
}



std::string format_merge_contact_output(
        const Fred::MergeContactOutput &_merge_data,
        const std::string &_src_handle,
        const std::string &_dst_handle,
        const MergeContactSummaryInfo &_msi,
        OutputIndenter _indenter)
{
    std::stringstream output;
    std::string header = str(boost::format("[%1%/%2%/%3%] MERGE %4% => %5%")
            % _msi.merge_counter % _msi.merge_set_counter % _msi.merge_per_merge_set_counter
            % _src_handle % _dst_handle);

    output << format_header(header, _indenter);

    for (std::vector<Fred::MergeContactUpdateDomainRegistrant>::const_iterator i = _merge_data.update_domain_registrant.begin();
            i != _merge_data.update_domain_registrant.end(); ++i)
    {
        output << str(boost::format("  %1%  update_domain %2% (id=%3%, hid=%4%) -- new owner: %5%")
                % i->sponsoring_registrar % i->fqdn % i->domain_id % i->history_id % i->set_registrant)
                 << std::endl;
    }
    for (std::vector<Fred::MergeContactUpdateDomainAdminContact>::const_iterator i = _merge_data.update_domain_admin_contact.begin();
            i != _merge_data.update_domain_admin_contact.end(); ++i)
    {
        output << str(boost::format("  %1%  update_domain %2% (id=%3%, hid=%4%) -- new admin-c: %5%")
                % i->sponsoring_registrar % i->fqdn % i->domain_id % i->history_id % i->add_admin_contact)
                 << std::endl;
    }
    for (std::vector<Fred::MergeContactUpdateNssetTechContact>::const_iterator i = _merge_data.update_nsset_tech_contact.begin();
            i != _merge_data.update_nsset_tech_contact.end(); ++i)
    {
        output << str(boost::format("  %1%  update_nsset %2% (id=%3%, hid=%4%) -- new tech-c: %5%")
                % i->sponsoring_registrar % i->handle % i->nsset_id % i->history_id % i->add_tech_contact)
                 << std::endl;
    }
    for (std::vector<Fred::MergeContactUpdateKeysetTechContact>::const_iterator i = _merge_data.update_keyset_tech_contact.begin();
            i != _merge_data.update_keyset_tech_contact.end(); ++i)
    {
        output << str(boost::format("  %1%  update_keyset %2% (id=%3%, hid=%4%) -- new tech-c: %5%")
                % i->sponsoring_registrar % i->handle % i->keyset_id % i->history_id % i->add_tech_contact)
                 << std::endl;
    }

    output << std::endl;
    return output.str();
}



}

