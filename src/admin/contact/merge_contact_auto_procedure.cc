#include "src/admin/contact/merge_contact_auto_procedure.h"
#include "src/admin/contact/merge_contact.h"
#include "src/admin/contact/merge_contact_reporting.h"
#include "src/fredlib/contact/merge_contact.h"
#include "src/fredlib/contact/find_contact_duplicates.h"
#include "src/fredlib/contact/merge_contact_email_notification_data.h"
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
        const Optional<std::string> &_registrar,
        const Optional<unsigned long long> &_limit,
        const Optional<bool> &_dry_run,
        const Optional<unsigned short> &_verbose)
    : mm_(mm),
      logger_client_(_logger_client),
      registrar_(_registrar),
      limit_(_limit),
      dry_run_(_dry_run),
      verbose_(_verbose)
{
}


MergeContactAutoProcedure& MergeContactAutoProcedure::set_registrar(
        const Optional<std::string> &_registrar)
{
    registrar_ = _registrar;
    return *this;
}


MergeContactAutoProcedure& MergeContactAutoProcedure::set_limit(
        const Optional<unsigned long long> &_limit)
{
    limit_ = _limit;
    return *this;
}


MergeContactAutoProcedure& MergeContactAutoProcedure::set_dry_run(
        const Optional<bool> &_dry_run)
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
        const Optional<unsigned short> &_verbose)
{
    verbose_ = _verbose;
    return *this;
}


bool MergeContactAutoProcedure::is_set_dry_run() const
{
    return (dry_run_.isset() && dry_run_.get_value() == true);
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
    if (verbose_.isset()) {
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

    //check registrar
    Database::Result registrar_res = octx.get_conn().exec_params(
        "SELECT id FROM registrar WHERE handle = UPPER($1::text)"
        , Database::query_param_list(registrar_.get_value()));

    if(registrar_res.size() == 0)//registrar not found
    {
        throw std::runtime_error(std::string("registrar: '")
            + registrar_.get_value()+"' not found");
    }

    std::string system_registrar = static_cast<std::string>(system_registrar_result[0][0]);

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

    if (this->get_verbose_level() > 0) {
        out_stream << format_header(str(boost::format("REGISTRAR: %1%")
            % (registrar_.isset() ? registrar_.get_value(): std::string("n/a"))), indenter);
    }



    /* find any contact duplicates set (optionally for specific registrar only) */
    std::set<std::string> any_dup_set = Fred::Contact::FindContactDuplicates().set_registrar(registrar_).exec(octx);
    std::set<std::string> skip_invalid_contact_set;

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

            //skip invalid contacts
            {
                octx.get_log().debug(boost::format("skip invalid contacts set: { %1% }")
                        % boost::algorithm::join(skip_invalid_contact_set, ", "));

                std::set<std::string> tmp_dup_set;
                    std::set_difference(dup_set.begin(), dup_set.end(),skip_invalid_contact_set.begin(), skip_invalid_contact_set.end(),
                        std::insert_iterator<std::set<std::string> >(tmp_dup_set, tmp_dup_set.begin()));
                    dup_set = tmp_dup_set;
            }

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
            Admin::MergeContact merge_op = Admin::MergeContact(pick_one, winner_handle, system_registrar);

            try
            {
                if (this->is_set_dry_run())
                {
                    merge_data = merge_op.exec_dry_run();

                    dry_run_info.add_fake_deleted(pick_one);
                    dry_run_info.add_search_excluded(winner_handle);
                    /* do not commit */
                }
                else
                {
                    /* MERGE ONE CONTACT */
                    merge_data = merge_op.exec(logger_client_);
                    /* save output for later email notification */
                    email_notification_input_vector.push_back(
                            Fred::MergeContactEmailNotificationInput(pick_one, winner_handle, merge_data));
                }
            }
            catch(const Fred::MergeContact::Exception& ex)
            {
                if(ex.is_set_src_contact_invalid())
                {
                    skip_invalid_contact_set.insert(ex.get_src_contact_invalid());
                }

                if(ex.is_set_object_blocked())//linked object of src contact blocked, skip src contact
                {
                    skip_invalid_contact_set.insert(pick_one);
                }

                if(ex.is_set_dst_contact_invalid())
                {
                    skip_invalid_contact_set.insert(ex.get_dst_contact_invalid());
                }

                if(!(ex.is_set_src_contact_invalid()
                    || ex.is_set_object_blocked()
                    || ex.is_set_dst_contact_invalid()))
                {
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
            dup_set = Fred::Contact::FindContactDuplicates().set_specific_contact(winner_handle).exec(octx);
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

        Fred::Contact::FindContactDuplicates new_dup_search = Fred::Contact::FindContactDuplicates().set_registrar(registrar_);
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

