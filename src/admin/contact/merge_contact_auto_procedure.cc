#include "src/admin/contact/merge_contact_auto_procedure.h"
#include "src/admin/contact/merge_contact.h"
#include "src/admin/contact/merge_contact_reporting.h"
#include "src/fredlib/contact/find_contact_duplicates.h"
#include "src/fredlib/contact/merge_contact_email_notification_data.h"
#include "src/fredlib/contact/merge_contact.h"
#include "util/util.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>

#include <algorithm>
#include <sstream>

namespace Admin {

namespace {

std::set<std::string> set_diff(const std::set<std::string>& _set, const std::set<std::string>& _subset) {
    std::set<std::string> result;
    std::set_difference(
        _set.begin(), _set.end(),
        _subset.begin(), _subset.end(),
        std::insert_iterator<std::set<std::string> >(result, result.begin())
    );
    return result;
}

boost::optional<std::string> get_system_registrar_handle(Fred::OperationContext& ctx) {
    const Database::Result db_result = ctx.get_conn().exec(
        "SELECT handle FROM registrar WHERE system is True"
    );
    return db_result.size() > 0
        ?
        static_cast<std::string>(db_result[0][0])
        :
        boost::optional<std::string>();
}

bool registrar_exists(Fred::OperationContext& ctx, std::string _registrar_handle) {
    const Database::Result db_result = ctx.get_conn().exec_params(
        "SELECT id FROM registrar WHERE handle = UPPER($1::text)"
        , Database::query_param_list(_registrar_handle)
    );
    return db_result.size() != 0;
}

} // namespace Admin::{anonymous}

std::vector<Fred::MergeContactNotificationEmailWithAddr> email_notification(
    Fred::Mailer::Manager& mm,
    const std::vector<Fred::MergeContactEmailNotificationInput>& email_notification_input_vector
)
{
    Fred::OperationContextCreator enctx;
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

        counter = 0;
        for (std::vector<std::string>::const_iterator listci = ci->email_data.domain_registrant_list.begin()
                ; listci != ci->email_data.domain_registrant_list.end() ; ++listci)
        {
            std::stringstream list_key;
            list_key << "domain_registrant_list." << counter; ++counter;
            params[list_key.str()] = *listci;
        }

        counter = 0;
        for (std::vector<std::string>::const_iterator listci = ci->email_data.domain_admin_list.begin()
                ; listci != ci->email_data.domain_admin_list.end() ; ++listci)
        {
            std::stringstream list_key;
            list_key << "domain_admin_list." << counter; ++counter;
            params[list_key.str()] = *listci;
        }

        counter = 0;
        for (std::vector<std::string>::const_iterator listci = ci->email_data.nsset_tech_list.begin()
                ; listci != ci->email_data.nsset_tech_list.end() ; ++listci)
        {
            std::stringstream list_key;
            list_key << "nsset_tech_list." << counter; ++counter;
            params[list_key.str()] = *listci;
        }

        counter = 0;
        for (std::vector<std::string>::const_iterator listci = ci->email_data.keyset_tech_list.begin()
                ; listci != ci->email_data.keyset_tech_list.end() ; ++listci)
        {
            std::stringstream list_key;
            list_key << "keyset_tech_list." << counter; ++counter;
            params[list_key.str()] = *listci;
        }

        counter = 0;
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
                "", // default sender
                params["email"],
                "", // default subject
                "merge_contacts_auto", // mail template
                params,handles,attach);
        }
        catch(std::exception& ex)
        {
            std::stringstream errmsg;
            errmsg << "merge_contacts_auto sendEmail failed - email: " << params["email"] << " what: " << ex.what();
            enctx.get_log().error(errmsg.str());
        }
    }//for emails
    return notif_emails;
}

MergeContactAutoProcedure::MergeContactAutoProcedure(
    Fred::Mailer::Manager& mm,
    Fred::Logger::LoggerClient& _logger_client,
    const std::string& _registrar)
    : mm_(mm)
    , logger_client_(_logger_client)
    , registrar_(_registrar)
{ }

MergeContactAutoProcedure::MergeContactAutoProcedure(
    Fred::Mailer::Manager& mm,
    Fred::Logger::LoggerClient& _logger_client,
    const std::string& _registrar,
    const Optional<bool>& _dry_run,
    const Optional<unsigned short>& _verbose)
    : mm_(mm),
      logger_client_(_logger_client),
      registrar_(_registrar),
      dry_run_(_dry_run),
      verbose_(_verbose)
{ }

MergeContactAutoProcedure& MergeContactAutoProcedure::set_dry_run(
    const Optional<bool>& _dry_run
)
{
    dry_run_ = _dry_run;
    return *this;
}

MergeContactAutoProcedure& MergeContactAutoProcedure::set_selection_filter_order(
    const std::vector<Fred::ContactSelectionFilterType>& _selection_filter_order)
{
    /* XXX: this throws - should do better error reporting */
    FactoryHaveSupersetOfKeysChecker<Fred::ContactSelectionFilterFactory>(_selection_filter_order).check();
    selection_filter_order_ = _selection_filter_order;
    return *this;
}


MergeContactAutoProcedure& MergeContactAutoProcedure::set_verbose(
    const Optional<unsigned short>& _verbose)
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
    if (is_set_dry_run()) {
        return 3;
    }
    return 0;
}

Fred::MergeContactOutput MergeContactAutoProcedure::merge_contact(
    const std::string& src_contact_,
    const std::string& dst_contact_,
    const std::string& system_registrar_
)
{
    Admin::MergeContact merge_op = Admin::MergeContact(src_contact_, dst_contact_, system_registrar_);

    if (is_set_dry_run()) {
        dry_run_info_.add_search_excluded(src_contact_);
        dry_run_info_.add_search_excluded(dst_contact_);
        return merge_op.exec_dry_run();
    }

    return merge_op.exec(logger_client_);
}

std::vector<Fred::MergeContactNotificationEmailWithAddr> MergeContactAutoProcedure::exec(std::ostream& _output_stream)
{
    Fred::OperationContextCreator ctx;

    std::vector<Fred::MergeContactNotificationEmailWithAddr> ret_email_notifications;

    const boost::optional<std::string> system_registrar = get_system_registrar_handle(ctx);
    if (!system_registrar) {
        throw std::runtime_error("no system registrar found");
    }

    if (!registrar_exists(ctx, registrar_)) {
        throw std::runtime_error(std::string("registrar: '") + registrar_ + "' not found");
    }

    const std::vector<Fred::ContactSelectionFilterType> selection_filter =
        !selection_filter_order_.empty()
        ? selection_filter_order_
        : get_default_selection_filter_order();

    OutputIndenter indenter(2, 0, ' ');

    if (get_verbose_level() > 0) {
        _output_stream << format_header(str(boost::format("REGISTRAR: %1%") % registrar_), indenter);
    }

    std::set<std::string> duplicate_contacts;
    std::set<std::string> invalid_contacts;

    while (true) {

        duplicate_contacts = Fred::Contact::FindContactDuplicates()
            .set_registrar(Optional<std::string>(registrar_))
            .set_exclude_contacts(invalid_contacts)
            .exec(ctx);

        if(is_set_dry_run()) {
            duplicate_contacts = set_diff(duplicate_contacts, dry_run_info_.any_search_excluded);
        }

        if (duplicate_contacts.size() < 2) break;

        summary_info_.inc_merge_set();
        MergeContactOperationSummary merge_set_operation_info;
        std::vector<Fred::MergeContactEmailNotificationInput> email_notification_input_vector;

        boost::optional<Fred::MergeContactSelectionOutput> dst_contact;

        while (duplicate_contacts.size()) {

            if (!dst_contact) {
                dst_contact = Fred::MergeContactSelection(
                                  std::vector<std::string>(duplicate_contacts.begin(), duplicate_contacts.end()),
                                  selection_filter
                              ).exec(ctx);

                if(!dst_contact) {
                    throw std::runtime_error("dst_contact not identified\n");
                }

                ctx.get_log().debug(boost::format("contact duplicates set: { %1% }") % boost::algorithm::join(duplicate_contacts, ", "));

                ctx.get_log().debug(boost::format("winner handle: %1%") % dst_contact.value().handle);

                duplicate_contacts.erase(dst_contact.value().handle);

            }

            if(duplicate_contacts.size() < 1) break;

            const std::string src_contact = *(duplicate_contacts.begin());

            try {
                MergeContactOperationSummary merge_operation_info;

                summary_info_.inc_merge_operation();

                const Fred::MergeContactOutput merge_data =
                    merge_contact(src_contact, dst_contact.value().handle, system_registrar.value());

                if(!is_set_dry_run()) {
                    email_notification_input_vector.push_back(
                        Fred::MergeContactEmailNotificationInput(src_contact, dst_contact.value().handle, merge_data)
                    );
                }

                if (get_verbose_level() > 0) {
                    merge_operation_info.add_merge_output(merge_data);
                    merge_set_operation_info.add_merge_output(merge_data);
                    all_merge_operation_info_.add_merge_output(merge_data);
                }

                if (get_verbose_level() > 2) {
                    _output_stream << format_merge_contact_output(merge_data, src_contact, dst_contact.value().handle, summary_info_, indenter);
                    _output_stream << merge_operation_info.format(indenter.dive());
                }
            }
            catch(const Fred::MergeContact::Exception& ex)
            {
                bool exception_handled = false;

                if (ex.is_set_src_contact_invalid()) {
                    invalid_contacts.insert(ex.get_src_contact_invalid());
                    exception_handled = true;
                    summary_info_.inc_invalid_contacts();
                }

                if (ex.is_set_object_blocked()) { // object linked to src contact blocked, skip src contact
                    invalid_contacts.insert(src_contact);
                    exception_handled = true;
                    summary_info_.inc_linked_contacts();
                }

                if (ex.is_set_dst_contact_invalid()) {
                    invalid_contacts.insert(ex.get_dst_contact_invalid());
                    dst_contact = boost::optional<Fred::MergeContactSelectionOutput>(); // invalidate dst_contact
                    exception_handled = true;
                    summary_info_.inc_invalid_contacts();
                }

                if (!exception_handled) {
                    throw;
                }
            }

            ctx.get_log().debug(boost::format("skip invalid contacts set: { %1% }") % boost::algorithm::join(invalid_contacts, ", "));

            duplicate_contacts = set_diff(duplicate_contacts, invalid_contacts);

            duplicate_contacts.erase(src_contact); // remove if not already removed as invalid

        }

        if (get_verbose_level() > 1) {
            _output_stream << format_header(str(boost::format("[-/%1%/-] MERGE SET SUMMARY")
                        % summary_info_.merge_sets_total), indenter);
            _output_stream << merge_set_operation_info.format(indenter);
        }

        if (!is_set_dry_run()) {
            ret_email_notifications = email_notification(mm_, email_notification_input_vector);
        }

    }

    if (get_verbose_level() > 0) {
        _output_stream << format_header(str(boost::format("[%1%/%2%/-] PROCEDURE RUN SUMMARY")
                    % summary_info_.merge_operations_total % summary_info_.merge_sets_total), indenter);
        _output_stream << all_merge_operation_info_.format(indenter);
    }
    if (!is_set_dry_run()) {
        ctx.commit_transaction();
    }
    return ret_email_notifications;
}


}

