#include "merge_contact_auto_procedure.h"
#include "merge_contact.h"
#include "find_contact_duplicates.h"
#include "poll/create_update_object_poll_message.h"
#include "merge_contact_email_notification_data.h"
#include "mailer_manager.h"
#include "mailer.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <algorithm>
#include <sstream>


namespace Fred {
namespace Contact {

void email_notification(Fred::Mailer::Manager& mm
        , const std::vector<Fred::MergeContactEmailNotificationInput>& email_notification_input_vector)
{
    Fred::OperationContext enctx;
    std::vector<Fred::MergeContactNotificationEmailWithAddr> notif_emails
          = Fred::MergeContactNotificationEmailAddr(Fred::MergeContactEmailNotificationData(email_notification_input_vector)
        .exec(enctx)).exec(enctx);

    for(std::vector<Fred::MergeContactNotificationEmailWithAddr>::const_iterator ci = notif_emails.begin()
            ;ci != notif_emails.end() ; ++ci)
    {
        Fred::Mailer::Attachments attach;
        Fred::Mailer::Handles handles;
        Fred::Mailer::Parameters params;
        int counter=0;

        params["email"] = ci->notification_email_addr;

        params["dst_contact_handle"] = ci->email_data.dst_contact_handle;

        counter = 0;//reset counter
        for (std::vector<std::string>::const_iterator listci = ci->email_data.domain_registrant_list.begin()
                ; listci != ci->email_data.domain_registrant_list.end() ; ++listci)
        {
            std::stringstream list_key;
            list_key << "domain_registrant_list." << counter; ++counter;
            params[list_key.str()] = *listci;
        }

        counter = 0;//reset counter
        for (std::vector<std::string>::const_iterator listci = ci->email_data.domain_admin_list.begin()
                ; listci != ci->email_data.domain_admin_list.end() ; ++listci)
        {
            std::stringstream list_key;
            list_key << "domain_admin_list." << counter; ++counter;
            params[list_key.str()] = *listci;
        }

        counter = 0;//reset counter
        for (std::vector<std::string>::const_iterator listci = ci->email_data.nsset_tech_list.begin()
                ; listci != ci->email_data.nsset_tech_list.end() ; ++listci)
        {
            std::stringstream list_key;
            list_key << "nsset_tech_list." << counter; ++counter;
            params[list_key.str()] = *listci;
        }

        counter = 0;//reset counter
        for (std::vector<std::string>::const_iterator listci = ci->email_data.keyset_tech_list.begin()
                ; listci != ci->email_data.keyset_tech_list.end() ; ++listci)
        {
            std::stringstream list_key;
            list_key << "keyset_tech_list." << counter; ++counter;
            params[list_key.str()] = *listci;
        }

        counter = 0;//reset counter
        for (std::vector<std::string>::const_iterator listci = ci->email_data.removed_list.begin()
                ; listci != ci->email_data.removed_list.end() ; ++listci)
        {
            std::stringstream list_key;
            list_key << "removed_list." << counter; ++counter;
            params[list_key.str()] = *listci;
        }

        try
        {
            mm.sendEmail(
                "",// default sender
                params["email"],
                "",// default subject
                "merge_contacts_auto",//mail template
                params,handles,attach);
        }
        catch(std::exception& ex)
        {
            std::stringstream errmsg;
            errmsg << "merge_contacts_auto sendEmail failed - email: " << params["email"] << " what: " << ex.what();
            enctx.get_log().error(errmsg.str());
        }
    }//for emails
}

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


struct MergeContactDryRunInfo
{
    unsigned long long merge_counter;
    std::set<std::string> fake_deleted;
    std::set<std::string> any_search_excluded;

    MergeContactDryRunInfo()
        : merge_counter(0),
          fake_deleted(),
          any_search_excluded()
    {
    }

    void add_fake_deleted(const std::string &_handle)
    {
        fake_deleted.insert(fake_deleted.end(), _handle);
    }

    void add_search_excluded(const std::string &_handle)
    {
        any_search_excluded.insert(any_search_excluded.end(), _handle);
    }

    std::set<std::string> remove_fake_deleted_from_set(const std::set<std::string> &_set)
    {
        std::set<std::string> result;
        std::set_difference(_set.begin(), _set.end(), fake_deleted.begin(), fake_deleted.end(),
                std::insert_iterator<std::set<std::string> >(result, result.begin()));
        return result;
    }
};



MergeContactAutoProcedure::MergeContactAutoProcedure(
        Fred::Mailer::Manager& mm,
        Fred::Logger::LoggerClient &_logger_client)
    : mm_(mm)
    , logger_client_(_logger_client)
{
}


MergeContactAutoProcedure::MergeContactAutoProcedure(
        Fred::Mailer::Manager& mm,
        Fred::Logger::LoggerClient &_logger_client,
        const optional_string &_registrar,
        const optional_ulonglong &_limit,
        const optional_bool &_dry_run)
    : mm_(mm),
      logger_client_(_logger_client),
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


MergeContactAutoProcedure& MergeContactAutoProcedure::set_selection_filter_order(
        const std::vector<ContactSelectionFilterType> &_selection_filter_order)
{
    /* XXX: this throws - should do better error reporting */
    FactoryHaveSupersetOfKeysChecker<ContactSelectionFilterFactory>(_selection_filter_order).check();
    selection_filter_order_ = _selection_filter_order;
    return *this;
}


bool MergeContactAutoProcedure::is_set_dry_run() const
{
    return (dry_run_.is_value_set() && dry_run_.get_value() == true);
}


std::vector<ContactSelectionFilterType> MergeContactAutoProcedure::get_default_selection_filter_order() const
{
    std::vector<ContactSelectionFilterType> tmp = boost::assign::list_of
        (MCS_FILTER_IDENTIFIED_CONTACT)
        (MCS_FILTER_CONDITIONALLY_IDENTIFIED_CONTACT)
        (MCS_FILTER_HANDLE_MOJEID_SYNTAX)
        (MCS_FILTER_MAX_DOMAINS_BOUND)
        (MCS_FILTER_MAX_OBJECTS_BOUND)
        (MCS_FILTER_RECENTLY_UPDATED)
        (MCS_FILTER_NOT_REGCZNIC)
        (MCS_FILTER_RECENTLY_CREATED);
    return tmp;
}


void MergeContactAutoProcedure::exec()
{
    Fred::OperationContext octx;
    /* get system registrar - XXX: should be a parameter?? */
    Database::Result system_registrar_result = octx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system is True");
    if (system_registrar_result.size() == 0) {
        throw std::runtime_error("no system registrar found");
    }
    std::string system_registrar = static_cast<std::string>(system_registrar_result[0][0]);

    /* find any contact duplicates set (optionally for specific registrar only) */
    std::set<std::string> dup_set = FindAnyContactDuplicates().set_registrar(registrar_).exec(octx);

    if (dup_set.empty()) {
        octx.get_log().info("no contact duplicity");
        return;
    }

    /* filter for best contact selection */
    std::vector<ContactSelectionFilterType> selection_filter = selection_filter_order_;
    if (selection_filter_order_.empty()) {
        selection_filter = this->get_default_selection_filter_order();
    }

    std::vector<Fred::MergeContactEmailNotificationInput> email_notification_input_vector;

    MergeContactDryRunInfo dry_run_info;
    while (dup_set.size() >= 2)
    {
        dry_run_info.merge_counter += 1;

        octx.get_log().debug(boost::format("contact duplicates set: { %1% }")
                % boost::algorithm::join(dup_set, ", "));

        /* compute best handle to merge all others onto */
        std::string winner_handle = MergeContactSelection(
                std::vector<std::string>(dup_set.begin(), dup_set.end()), selection_filter).exec(octx);
        octx.get_log().debug(boost::format("winner handle: %1%") % winner_handle);

        /* remove winner contact from set */
        dup_set.erase(winner_handle);
        /* merge first one */
        std::string pick_one = *(dup_set.begin());

        unsigned long long req_id = 0;
        MergeContactOutput merge_data;
        if (this->is_set_dry_run())
        {
            Fred::OperationContext merge_octx;
            merge_data = MergeContact(pick_one, winner_handle, system_registrar)
                            .set_logd_request_id(req_id).exec_dry_run(merge_octx);

            dry_run_info.add_fake_deleted(pick_one);
            dry_run_info.add_search_excluded(winner_handle);
            /* do not commit */
            print_merge_contact_output(merge_data, pick_one, winner_handle, dry_run_info.merge_counter, std::cout);
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
        //save merge output for email notification
        email_notification_input_vector.push_back(Fred::MergeContactEmailNotificationInput(
                pick_one, winner_handle, merge_data));

        /* find contact duplicates for winner contact - if nothing changed in registry data this
         * would be the same list as in previous step but without the merged one */
        dup_set = FindSpecificContactDuplicates(winner_handle).exec(octx);
        if (this->is_set_dry_run())
        {
            dup_set = dry_run_info.remove_fake_deleted_from_set(dup_set);
            if (dup_set.size() == 1 && *(dup_set.begin()) == winner_handle) {
                dup_set.clear();
            }
        }
        /* if none go for another contact which has duplicates */
        if (dup_set.empty()) {
            FindAnyContactDuplicates new_dup_search = FindAnyContactDuplicates().set_registrar(registrar_);
            if (this->is_set_dry_run()) {
                dup_set = new_dup_search.set_exclude_contacts(dry_run_info.any_search_excluded).exec(octx);
            }
            else {
                email_notification(mm_, email_notification_input_vector);
                email_notification_input_vector.clear();

                dup_set = new_dup_search.exec(octx);
            }
        }
    }
    if (!this->is_set_dry_run()) {
        octx.commit_transaction();
    }





}


}
}

