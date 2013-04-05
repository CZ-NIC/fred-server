#include "admin/contact/merge_contact_auto_procedure.h"
#include "fredlib/contact/merge_contact.h"
#include "fredlib/contact/find_contact_duplicates.h"
#include "fredlib/contact/merge_contact_email_notification_data.h"
#include "fredlib/poll/create_update_object_poll_message.h"
#include "fredlib/poll/create_delete_contact_poll_message.h"
#include "util/util.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <algorithm>
#include <sstream>


namespace Admin {



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
        const Fred::MergeContactOutput &_merge_data,
        Fred::Logger::RequestProperties &_properties,
        Fred::Logger::ObjectReferences &_references)
{
    for (std::vector<Fred::MergeContactUpdateDomainRegistrant>::const_iterator i = _merge_data.update_domain_registrant.begin();
            i != _merge_data.update_domain_registrant.end(); ++i)
    {
        _properties.push_back(Fred::Logger::RequestProperty("command", "update_domain", false));
        _properties.push_back(Fred::Logger::RequestProperty("handle", i->fqdn, true));
        _properties.push_back(Fred::Logger::RequestProperty("registrant", i->set_registrant, true));
        _references.push_back(Fred::Logger::ObjectReference("domain", i->domain_id));
    }
    for (std::vector<Fred::MergeContactUpdateDomainAdminContact>::const_iterator i = _merge_data.update_domain_admin_contact.begin();
            i != _merge_data.update_domain_admin_contact.end(); ++i)
    {
        _properties.push_back(Fred::Logger::RequestProperty("command", "update_domain", false));
        _properties.push_back(Fred::Logger::RequestProperty("handle", i->fqdn, true));
        _properties.push_back(Fred::Logger::RequestProperty("remAdmin", i->rem_admin_contact, true));
        _properties.push_back(Fred::Logger::RequestProperty("addAdmin", i->add_admin_contact, true));
        _references.push_back(Fred::Logger::ObjectReference("domain", i->domain_id));
    }
    for (std::vector<Fred::MergeContactUpdateNssetTechContact>::const_iterator i = _merge_data.update_nsset_tech_contact.begin();
            i != _merge_data.update_nsset_tech_contact.end(); ++i)
    {
        _properties.push_back(Fred::Logger::RequestProperty("command", "update_nsset", false));
        _properties.push_back(Fred::Logger::RequestProperty("handle", i->handle, true));
        _properties.push_back(Fred::Logger::RequestProperty("remTech", i->rem_tech_contact, true));
        _properties.push_back(Fred::Logger::RequestProperty("addTech", i->add_tech_contact, true));
        _references.push_back(Fred::Logger::ObjectReference("nsset", i->nsset_id));
    }
    for (std::vector<Fred::MergeContactUpdateKeysetTechContact>::const_iterator i = _merge_data.update_keyset_tech_contact.begin();
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
        Fred::Logger::RequestProperties &_properties,
        Fred::Logger::ObjectReferences &_references,
        const std::string &_result)
{
    if (_req_id) {
        _properties.push_back(Fred::Logger::RequestProperty("opTRID", Util::make_svtrid(_req_id), false));
        _logger_client.closeRequest(_req_id, "Admin", "",
                _properties,
                _references,
                _result, 0);
    }
}


void logger_merge_contact_close_request_success(
        Fred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id,
        const Fred::MergeContactOutput &_merge_data)
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
    Fred::Logger::RequestProperties props;
    Fred::Logger::ObjectReferences refs;
    logger_merge_contact_close(
            _logger_client,
            _req_id,
            props,
            refs,
            "Fail");
}


void create_poll_messages(const Fred::MergeContactOutput &_merge_data, Fred::OperationContext &_ctx)
{
    for (std::vector<Fred::MergeContactUpdateDomainRegistrant>::const_iterator i = _merge_data.update_domain_registrant.begin();
            i != _merge_data.update_domain_registrant.end(); ++i)
    {
        Fred::Poll::CreateUpdateObjectPollMessage(i->history_id).exec(_ctx);
    }
    for (std::vector<Fred::MergeContactUpdateDomainAdminContact>::const_iterator i = _merge_data.update_domain_admin_contact.begin();
            i != _merge_data.update_domain_admin_contact.end(); ++i)
    {
        Fred::Poll::CreateUpdateObjectPollMessage(i->history_id).exec(_ctx);
    }
    for (std::vector<Fred::MergeContactUpdateNssetTechContact>::const_iterator i = _merge_data.update_nsset_tech_contact.begin();
            i != _merge_data.update_nsset_tech_contact.end(); ++i)
    {
        Fred::Poll::CreateUpdateObjectPollMessage(i->history_id).exec(_ctx);
    }
    for (std::vector<Fred::MergeContactUpdateKeysetTechContact>::const_iterator i = _merge_data.update_keyset_tech_contact.begin();
            i != _merge_data.update_keyset_tech_contact.end(); ++i)
    {
        Fred::Poll::CreateUpdateObjectPollMessage(i->history_id).exec(_ctx);
    }
    Fred::Poll::CreateDeleteContactPollMessage(_merge_data.contactid.src_contact_historyid).exec(_ctx);
}


struct OutputIndenter
{
    unsigned short indent;
    unsigned short level;
    char c;

    OutputIndenter dive() const
    {
        return OutputIndenter(indent, level + 1, c);
    }

    OutputIndenter(unsigned short _i, unsigned short _l, char _c)
        : indent(_i), level(_l), c(_c)
    {
    }

    friend std::ostream& operator<<(std::ostream &_ostream, const OutputIndenter &_oi);
};

std::ostream& operator<<(std::ostream &_ostream, const OutputIndenter &_oi)
{
    return _ostream << std::string(_oi.indent * _oi.level, _oi.c);
}



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

    void add_merge_output(const Fred::MergeContactOutput &_merge_data)
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


    std::string format(OutputIndenter _indenter)
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


std::string format_header(const std::string &_text, OutputIndenter _indenter)
{
    std::stringstream output;
    output << _indenter << std::string(_text.length(), '-') << std::endl;
    output << _indenter << _text << std::endl;
    output << _indenter << std::string(_text.length(), '-') << std::endl;
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


struct MergeContactDryRunInfo
{
    std::set<std::string> fake_deleted;
    std::set<std::string> any_search_excluded;

    MergeContactDryRunInfo()
        : fake_deleted(),
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
        const optional_bool &_dry_run,
        const optional_ushort &_verbose)
    : mm_(mm),
      logger_client_(_logger_client),
      registrar_(_registrar),
      limit_(_limit),
      dry_run_(_dry_run),
      verbose_(_verbose)
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
        const std::vector<Fred::ContactSelectionFilterType> &_selection_filter_order)
{
    /* XXX: this throws - should do better error reporting */
    FactoryHaveSupersetOfKeysChecker<Fred::ContactSelectionFilterFactory>(_selection_filter_order).check();
    selection_filter_order_ = _selection_filter_order;
    return *this;
}


MergeContactAutoProcedure& MergeContactAutoProcedure::set_verbose(
        const optional_ushort &_verbose)
{
    verbose_ = _verbose;
    return *this;
}


bool MergeContactAutoProcedure::is_set_dry_run() const
{
    return (dry_run_.is_value_set() && dry_run_.get_value() == true);
}


std::vector<Fred::ContactSelectionFilterType> MergeContactAutoProcedure::get_default_selection_filter_order() const
{
    std::vector<Fred::ContactSelectionFilterType> tmp = boost::assign::list_of
        (Fred::MCS_FILTER_IDENTIFIED_CONTACT)
        (Fred::MCS_FILTER_CONDITIONALLY_IDENTIFIED_CONTACT)
        (Fred::MCS_FILTER_HANDLE_MOJEID_SYNTAX)
        (Fred::MCS_FILTER_MAX_DOMAINS_BOUND)
        (Fred::MCS_FILTER_MAX_OBJECTS_BOUND)
        (Fred::MCS_FILTER_RECENTLY_UPDATED)
        (Fred::MCS_FILTER_NOT_REGCZNIC)
        (Fred::MCS_FILTER_RECENTLY_CREATED);
    return tmp;
}


unsigned short MergeContactAutoProcedure::get_verbose_level() const
{
    if (verbose_.is_value_set()) {
        return verbose_.get_value();
    }
    if (this->is_set_dry_run()) {
        return 3;
    }
    return 0;
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


    /* filter for best contact selection */
    std::vector<Fred::ContactSelectionFilterType> selection_filter = selection_filter_order_;
    if (selection_filter_order_.empty()) {
        selection_filter = this->get_default_selection_filter_order();
    }

    MergeContactDryRunInfo dry_run_info;
    MergeContactSummaryInfo summary_info;
    MergeContactOperationSummary all_merge_operation_info;
    OutputIndenter indenter(2, 0, ' ');
    std::ostream &out_stream = std::cout;

    std::set<std::string> any_dup_set = Fred::Contact::FindAnyContactDuplicates().set_registrar(registrar_).exec(octx);
    while (any_dup_set.size() >= 2)
    {
        /* one specific contact set merges scope */
        summary_info.inc_merge_set();
        MergeContactOperationSummary merge_set_operation_info;

        std::vector<Fred::MergeContactEmailNotificationInput> email_notification_input_vector;
        std::set<std::string> dup_set = any_dup_set;
        do
        {
            /* one contact merge scope */
            summary_info.inc_merge_operation();
            MergeContactOperationSummary merge_operation_info;

            octx.get_log().debug(boost::format("contact duplicates set: { %1% }")
                    % boost::algorithm::join(dup_set, ", "));

            /* compute best handle to merge all others onto */
            Fred::MergeContactSelectionOutput contact_select = Fred::MergeContactSelection(
                    std::vector<std::string>(dup_set.begin(), dup_set.end()), selection_filter).exec(octx);
            std::string winner_handle = contact_select.handle;
            octx.get_log().debug(boost::format("winner handle: %1%") % winner_handle);

            /* remove winner contact from set */
            dup_set.erase(winner_handle);
            /* merge first one */
            std::string pick_one = *(dup_set.begin());

            Fred::MergeContactOutput merge_data;
            Fred::MergeContact merge_op = Fred::MergeContact(pick_one, winner_handle, system_registrar);
            if (this->is_set_dry_run())
            {
                Fred::OperationContext merge_octx;
                merge_data = merge_op.exec_dry_run(merge_octx);

                dry_run_info.add_fake_deleted(pick_one);
                dry_run_info.add_search_excluded(winner_handle);
                /* do not commit */
            }
            else
            {
                /* MERGE ONE CONTACT */
                unsigned long long req_id = 0;
                try {
                    req_id = logger_merge_contact_create_request(logger_client_, pick_one, winner_handle);

                    Fred::OperationContext merge_octx;
                    merge_data = merge_op.set_logd_request_id(req_id).exec(merge_octx);

                    /* merge operation notification handling */
                    create_poll_messages(merge_data, merge_octx);
                    merge_octx.commit_transaction();
                    /* save merge output for email notification */
                    email_notification_input_vector.push_back(
                            Fred::MergeContactEmailNotificationInput(pick_one, winner_handle, merge_data));

                    logger_merge_contact_close_request_success(logger_client_, req_id, merge_data);
                }
                catch (...) {
                    logger_merge_contact_close_request_fail(logger_client_, req_id);
                    /* stop at first error */
                    throw;
                }
            }

            if (this->get_verbose_level() > 0) {
                merge_operation_info.add_merge_output(merge_data);
                merge_set_operation_info.add_merge_output(merge_data);
                all_merge_operation_info.add_merge_output(merge_data);
            }

            if (this->get_verbose_level() > 2) {
                out_stream << format_merge_contact_output(merge_data, pick_one, winner_handle, summary_info, indenter);
                out_stream << merge_operation_info.format(indenter.dive());
            }

            /* find contact duplicates for winner contact - if nothing changed in registry data this
             * would be the same list as in previous step but without the merged one */
            dup_set = Fred::Contact::FindSpecificContactDuplicates(winner_handle).exec(octx);
            if (this->is_set_dry_run())
            {
                dup_set = dry_run_info.remove_fake_deleted_from_set(dup_set);
            }
        }
        while (dup_set.size() >= 2);

        if (this->get_verbose_level() > 1) {
            out_stream << format_header(str(boost::format("[-/%1%/-] MERGE SET SUMMARY")
                        % summary_info.merge_set_counter), indenter);
            out_stream << merge_set_operation_info.format(indenter);
        }

        Fred::Contact::FindAnyContactDuplicates new_dup_search = Fred::Contact::FindAnyContactDuplicates().set_registrar(registrar_);
        if (this->is_set_dry_run()) {
            new_dup_search.set_exclude_contacts(dry_run_info.any_search_excluded);
        }
        else {
            /* send email notifications */
            email_notification(mm_, email_notification_input_vector);
        }
        any_dup_set = new_dup_search.exec(octx);
    }

    if (this->get_verbose_level() > 0) {
        out_stream << format_header(str(boost::format("[%1%/%2%/-] PROCEDURE RUN SUMMARY")
                    % summary_info.merge_counter % summary_info.merge_set_counter), indenter);
        out_stream << all_merge_operation_info.format(indenter);
    }
    if (!this->is_set_dry_run()) {
        octx.commit_transaction();
    }
}


}

