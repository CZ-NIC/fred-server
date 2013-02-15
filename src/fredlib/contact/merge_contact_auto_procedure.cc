#include "merge_contact_auto_procedure.h"
#include "merge_contact_selection.h"
#include "merge_contact.h"
#include "find_contact_duplicates.h"
#include "poll/create_update_object_poll_message.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <algorithm>


namespace Fred {
namespace Contact {


void logger_merge_contact_transform_output_data(
        const MergeContactOutput &_merge_data,
        Fred::Logger::RequestProperties &_properties,
        Fred::Logger::ObjectReferences &_references)
{
    for (std::vector<MergeContactUpdateDomainRegistrant>::const_iterator i = _merge_data.update_domain_registrant.begin();
            i != _merge_data.update_domain_registrant.end(); ++i)
    {
        _properties.push_back(Fred::Logger::RequestProperty("command", "update_domain", false));
        _properties.push_back(Fred::Logger::RequestProperty("handle", i->fqdn, true));
        _properties.push_back(Fred::Logger::RequestProperty("registrant", i->set_registrant, true));
        _references.push_back(Fred::Logger::ObjectReference("domain", i->domain_id));
    }
    for (std::vector<MergeContactUpdateDomainAdminContact>::const_iterator i = _merge_data.update_domain_admin_contact.begin();
            i != _merge_data.update_domain_admin_contact.end(); ++i)
    {
        _properties.push_back(Fred::Logger::RequestProperty("command", "update_domain", false));
        _properties.push_back(Fred::Logger::RequestProperty("handle", i->fqdn, true));
        _properties.push_back(Fred::Logger::RequestProperty("remAdmin", i->rem_admin_contact, true));
        _properties.push_back(Fred::Logger::RequestProperty("addAdmin", i->add_admin_contact, true));
        _references.push_back(Fred::Logger::ObjectReference("domain", i->domain_id));
    }
    for (std::vector<MergeContactUpdateNssetTechContact>::const_iterator i = _merge_data.update_nsset_tech_contact.begin();
            i != _merge_data.update_nsset_tech_contact.end(); ++i)
    {
        _properties.push_back(Fred::Logger::RequestProperty("command", "update_nsset", false));
        _properties.push_back(Fred::Logger::RequestProperty("handle", i->handle, true));
        _properties.push_back(Fred::Logger::RequestProperty("remTech", i->rem_tech_contact, true));
        _properties.push_back(Fred::Logger::RequestProperty("addTech", i->add_tech_contact, true));
        _references.push_back(Fred::Logger::ObjectReference("nsset", i->nsset_id));
    }
    for (std::vector<MergeContactUpdateKeysetTechContact>::const_iterator i = _merge_data.update_keyset_tech_contact.begin();
            i != _merge_data.update_keyset_tech_contact.end(); ++i)
    {
        _properties.push_back(Fred::Logger::RequestProperty("command", "update_keyset", false));
        _properties.push_back(Fred::Logger::RequestProperty("handle", i->handle, true));
        _properties.push_back(Fred::Logger::RequestProperty("remTech", i->rem_tech_contact, true));
        _properties.push_back(Fred::Logger::RequestProperty("addTech", i->add_tech_contact, true));
        _references.push_back(Fred::Logger::ObjectReference("keyset", i->keyset_id));
    }
}


unsigned long long logger_merge_contact_create_request(
        Fred::Logger::LoggerClient &_logger_client,
        const std::string &_src_contact,
        const std::string &_dst_contact)
{
    unsigned long long req_id = _logger_client.createRequest("", "Admin", "",
            boost::assign::list_of
                (Fred::Logger::RequestProperty("src_contact", _src_contact, false))
                (Fred::Logger::RequestProperty("dst_contact", _dst_contact, false)),
            Fred::Logger::ObjectReferences(),
            "ContactMerge", 0);
    if (req_id == 0) {
        throw std::runtime_error("unable to log merge contact request");
    }
    return req_id;
}


void logger_merge_contact_close(
        Fred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id,
        const Fred::Logger::RequestProperties &_properties,
        const Fred::Logger::ObjectReferences &_references,
        const std::string &_result)
{
    if (_req_id) {
        _logger_client.closeRequest(_req_id, "Admin", "",
                _properties,
                _references,
                _result, 0);
    }
}


void logger_merge_contact_close_request_success(
        Fred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id,
        const MergeContactOutput &_merge_data)
{
    Fred::Logger::RequestProperties props;
    Fred::Logger::ObjectReferences refs;
    logger_merge_contact_transform_output_data(_merge_data, props, refs),
    logger_merge_contact_close(
            _logger_client,
            _req_id,
            props,
            refs,
            "Success");
}


void logger_merge_contact_close_request_fail(
        Fred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id)
{
    logger_merge_contact_close(
            _logger_client,
            _req_id,
            Fred::Logger::RequestProperties(),
            Fred::Logger::ObjectReferences(),
            "Fail");
}


void create_poll_messages(const MergeContactOutput &_merge_data, Fred::OperationContext &_ctx)
{
    for (std::vector<MergeContactUpdateDomainRegistrant>::const_iterator i = _merge_data.update_domain_registrant.begin();
            i != _merge_data.update_domain_registrant.end(); ++i)
    {
        Fred::Poll::CreateUpdateObjectPollMessage(i->history_id).exec(_ctx);
    }
    for (std::vector<MergeContactUpdateDomainAdminContact>::const_iterator i = _merge_data.update_domain_admin_contact.begin();
            i != _merge_data.update_domain_admin_contact.end(); ++i)
    {
        Fred::Poll::CreateUpdateObjectPollMessage(i->history_id).exec(_ctx);
    }
    for (std::vector<MergeContactUpdateNssetTechContact>::const_iterator i = _merge_data.update_nsset_tech_contact.begin();
            i != _merge_data.update_nsset_tech_contact.end(); ++i)
    {
        Fred::Poll::CreateUpdateObjectPollMessage(i->history_id).exec(_ctx);
    }
    for (std::vector<MergeContactUpdateKeysetTechContact>::const_iterator i = _merge_data.update_keyset_tech_contact.begin();
            i != _merge_data.update_keyset_tech_contact.end(); ++i)
    {
        Fred::Poll::CreateUpdateObjectPollMessage(i->history_id).exec(_ctx);
    }
}


std::ostream& print_merge_contact_output(
        const MergeContactOutput &_merge_data,
        const std::string &_src_handle,
        const std::string &_dst_handle,
        const unsigned long long &_merge_counter,
        std::ostream &_ostream)
{
    std::string header = str(boost::format("[%1%] MERGE %2% => %3%") % _merge_counter % _src_handle % _dst_handle);
    _ostream << std::string(header.length(), '-') << std::endl;
    _ostream << header << std::endl;
    _ostream << std::string(header.length(), '-') << std::endl;

    for (std::vector<MergeContactUpdateDomainRegistrant>::const_iterator i = _merge_data.update_domain_registrant.begin();
            i != _merge_data.update_domain_registrant.end(); ++i)
    {
        _ostream << str(boost::format("  %1%  update_domain %2% (id=%3%, hid=%4%) -- new owner: %5%")
                % i->sponsoring_registrar % i->fqdn % i->domain_id % i->history_id % i->set_registrant)
                 << std::endl;
    }
    for (std::vector<MergeContactUpdateDomainAdminContact>::const_iterator i = _merge_data.update_domain_admin_contact.begin();
            i != _merge_data.update_domain_admin_contact.end(); ++i)
    {
        _ostream << str(boost::format("  %1%  update_domain %2% (id=%3%, hid=%4%) -- new admin-c: %5%")
                % i->sponsoring_registrar % i->fqdn % i->domain_id % i->history_id % i->add_admin_contact)
                 << std::endl;
    }
    for (std::vector<MergeContactUpdateNssetTechContact>::const_iterator i = _merge_data.update_nsset_tech_contact.begin();
            i != _merge_data.update_nsset_tech_contact.end(); ++i)
    {
        _ostream << str(boost::format("  %1%  update_nsset %2% (id=%3%, hid=%4%) -- new tech-c: %5%")
                % i->sponsoring_registrar % i->handle % i->nsset_id % i->history_id % i->add_tech_contact)
                 << std::endl;
    }
    for (std::vector<MergeContactUpdateKeysetTechContact>::const_iterator i = _merge_data.update_keyset_tech_contact.begin();
            i != _merge_data.update_keyset_tech_contact.end(); ++i)
    {
        _ostream << str(boost::format("  %1%  update_keyset %2% (id=%3%, hid=%4%) -- new tech-c: %5%")
                % i->sponsoring_registrar % i->handle % i->keyset_id % i->history_id % i->add_tech_contact)
                 << std::endl;
    }

    _ostream << std::endl;
    return _ostream;
}



MergeContactAutoProcedure::MergeContactAutoProcedure(
        Fred::Logger::LoggerClient &_logger_client)
    : logger_client_(_logger_client)
{
}


MergeContactAutoProcedure::MergeContactAutoProcedure(
        Fred::Logger::LoggerClient &_logger_client,
        const optional_string &_registrar,
        const optional_ulonglong &_limit,
        const optional_bool &_dry_run)
    : logger_client_(_logger_client),
      registrar_(_registrar),
      limit_(_limit),
      dry_run_(_dry_run)
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


MergeContactAutoProcedure& MergeContactAutoProcedure::set_dry_run(
        const optional_bool &_dry_run)
{
    dry_run_ = _dry_run;
    return *this;
}


bool MergeContactAutoProcedure::is_set_dry_run() const
{
    return (dry_run_.is_value_set() && dry_run_.get_value() == true);
}


void MergeContactAutoProcedure::exec()
{
    Fred::OperationContext octx;
    /* get system registrar - XXX: should be a parameter?? */
    Database::Result system_registrar_result = octx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system is True");
    if (system_registrar_result.size() != 1) {
        throw std::runtime_error("no system registrar found");
    }
    std::string system_registrar = static_cast<std::string>(system_registrar_result[0][0]);

    /* find any contact duplicates set (optionally for specific registrar only) */
    std::set<std::string> dup_set = FindAnyContactDuplicates().set_registrar(registrar_).exec(octx);

    if (dup_set.empty()) {
        octx.get_log().info("no contact duplicity");
        return;
    }

    unsigned long long merge_counter = 0;
    std::set<std::string> fake_deleted;
    std::set<std::string> global_excluded;
    while (dup_set.size() >= 2)
    {
        merge_counter += 1;
        octx.get_log().debug(boost::format("contact duplicates set: { %1% }")
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
                std::vector<std::string>(dup_set.begin(), dup_set.end()), selection_filter).exec(octx);
        octx.get_log().debug(boost::format("winner handle: %1%") % winner_handle);

        /* remove winner contact from set */
        dup_set.erase(winner_handle);
        global_excluded.insert(global_excluded.end(), winner_handle);
        /* merge first one */
        std::string pick_one = *(dup_set.begin());

        unsigned long long req_id = 0;
        MergeContactOutput merge_data;
        if (this->is_set_dry_run())
        {
            Fred::OperationContext merge_octx;
            merge_data = MergeContact(pick_one, winner_handle, system_registrar)
                            .set_logd_request_id(req_id).exec_dry_run(merge_octx);

            fake_deleted.insert(fake_deleted.end(), pick_one);
            /* do not commit */
            print_merge_contact_output(merge_data, pick_one, winner_handle, merge_counter, std::cout);
        }
        else
        {
            try {
                req_id = logger_merge_contact_create_request(logger_client_, pick_one, winner_handle);

                Fred::OperationContext merge_octx;
                merge_data = MergeContact(pick_one, winner_handle, system_registrar)
                                .set_logd_request_id(req_id).exec(merge_octx);

                /* merge operation notification handling */
                create_poll_messages(merge_data, merge_octx);
                merge_octx.commit_transaction();

                logger_merge_contact_close_request_success(logger_client_, req_id, merge_data);
            }
            catch (...) {
                logger_merge_contact_close_request_fail(logger_client_, req_id);
                /* stop at first error */
                throw;
            }
        }

        /* find contact duplicates for winner contact - if nothing changed in registry data this
         * would be the same list as in previous step but without the merged one */
        dup_set = FindSpecificContactDuplicates(winner_handle).exec(octx);
        if (this->is_set_dry_run())
        {
            std::set<std::string> result;
            std::set_difference(dup_set.begin(), dup_set.end(), fake_deleted.begin(), fake_deleted.end(),
                    std::insert_iterator<std::set<std::string> >(result, result.begin()));
            dup_set = result;
            if (dup_set.size() == 1 && *(dup_set.begin()) == winner_handle) {
                dup_set.clear();
            }
        }
        /* if none go for another contact which has duplicates */
        if (dup_set.empty()) {
               dup_set = FindAnyContactDuplicates()
                        .set_registrar(registrar_)
                        .set_exclude_contacts(global_excluded)
                        .exec(octx);
        }
    }
    if (!this->is_set_dry_run()) {
        octx.commit_transaction();
    }
}


}
}

