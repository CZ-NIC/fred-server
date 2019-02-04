#include "src/backend/admin/contact/merge_contact_auto_procedure.hh"
#include "src/backend/admin/contact/merge_contact.hh"
#include "src/backend/admin/contact/merge_contact_reporting.hh"
#include "libfred/registrable_object/contact/find_contact_duplicates.hh"
#include "libfred/registrable_object/contact/merge_contact_email_notification_data.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "util/util.hh"

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

boost::optional<std::string> get_system_registrar_handle(LibFred::OperationContext& ctx) {
    const Database::Result db_result = ctx.get_conn().exec(
        "SELECT handle FROM registrar WHERE system is True"
    );
    return db_result.size() > 0
        ?
        static_cast<std::string>(db_result[0][0])
        :
        boost::optional<std::string>();
}

bool registrar_exists(LibFred::OperationContext& ctx, std::string _registrar_handle) {
    const Database::Result db_result = ctx.get_conn().exec_params(
        "SELECT id FROM registrar WHERE handle = UPPER($1::text)"
        , Database::query_param_list(_registrar_handle)
    );
    return db_result.size() != 0;
}

} // namespace Admin::{anonymous}

std::vector<LibFred::MergeContactNotificationEmailWithAddr> email_notification(
    LibFred::Mailer::Manager& mm,
    const std::vector<LibFred::MergeContactEmailNotificationInput>& email_notification_input_vector
)
{
    LibFred::OperationContextCreator enctx;
    std::vector<LibFred::MergeContactNotificationEmailWithAddr> notif_emails
          = LibFred::MergeContactNotificationEmailAddr(LibFred::MergeContactEmailNotificationData(email_notification_input_vector)
        .exec(enctx)).exec(enctx);

    for(std::vector<LibFred::MergeContactNotificationEmailWithAddr>::const_iterator ci = notif_emails.begin()
            ;ci != notif_emails.end() ; ++ci)
    {
        LibFred::Mailer::Attachments attach;
        LibFred::Mailer::Handles handles;
        LibFred::Mailer::Parameters params;
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
    LibFred::Mailer::Manager& mm,
    LibFred::Logger::LoggerClient& _logger_client,
    const std::string& _registrar)
    : mm_(mm)
    , logger_client_(_logger_client)
    , registrar_(_registrar)
{ }

MergeContactAutoProcedure::MergeContactAutoProcedure(
    LibFred::Mailer::Manager& mm,
    LibFred::Logger::LoggerClient& _logger_client,
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
    const std::vector<LibFred::ContactSelectionFilterType>& _selection_filter_order)
{
    /* XXX: this throws - should do better error reporting */
    FactoryHaveSupersetOfKeysChecker<LibFred::ContactSelectionFilterFactory>(_selection_filter_order).check();
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


std::vector<LibFred::ContactSelectionFilterType> MergeContactAutoProcedure::get_default_selection_filter_order() const
{
    std::vector<LibFred::ContactSelectionFilterType> tmp = boost::assign::list_of
        (LibFred::MCS_FILTER_IDENTIFIED_CONTACT)
        (LibFred::MCS_FILTER_CONDITIONALLY_IDENTIFIED_CONTACT)
        (LibFred::MCS_FILTER_HANDLE_MOJEID_SYNTAX)
        (LibFred::MCS_FILTER_MAX_DOMAINS_BOUND)
        (LibFred::MCS_FILTER_MAX_OBJECTS_BOUND)
        (LibFred::MCS_FILTER_RECENTLY_UPDATED)
        (LibFred::MCS_FILTER_NOT_REGCZNIC)
        (LibFred::MCS_FILTER_RECENTLY_CREATED);
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

LibFred::MergeContactOutput MergeContactAutoProcedure::merge_contact(
    const std::string& _src_contact,
    const std::string& _dst_contact,
    const std::string& _system_registrar
)
{
    Admin::MergeContact merge_op = Admin::MergeContact(_src_contact, _dst_contact, _system_registrar);

    if (is_set_dry_run()) {
        dry_run_info_.add_search_excluded(_src_contact);
        dry_run_info_.add_search_excluded(_dst_contact);
        return merge_op.exec_dry_run();
    }

    return merge_op.exec(logger_client_);
}

std::vector<LibFred::MergeContactNotificationEmailWithAddr> MergeContactAutoProcedure::exec(std::ostream& _output_stream)
{
    LibFred::OperationContextCreator ctx;

    std::vector<LibFred::MergeContactNotificationEmailWithAddr> ret_email_notifications;

    const boost::optional<std::string> system_registrar = get_system_registrar_handle(ctx);
    if (!system_registrar) {
        throw std::runtime_error("no system registrar found");
    }

    if (!registrar_exists(ctx, registrar_)) {
        throw std::runtime_error(std::string("registrar: '") + registrar_ + "' not found");
    }

    const std::vector<LibFred::ContactSelectionFilterType> selection_filter =
        !selection_filter_order_.empty()
        ? selection_filter_order_
        : get_default_selection_filter_order();

    OutputIndenter indenter(2, 0, ' ');

    if (get_verbose_level() > 0) {
        _output_stream << format_header(boost::str(boost::format("REGISTRAR: %1%") % registrar_), indenter);
    }

    std::set<std::string> duplicate_contacts;
    std::set<std::string> invalid_contacts;

    while (true) {

        duplicate_contacts = LibFred::Contact::FindContactDuplicates()
            .set_registrar(Optional<std::string>(registrar_))
            .set_exclude_contacts(invalid_contacts)
            .exec(ctx);

        if(is_set_dry_run()) {
            duplicate_contacts = set_diff(duplicate_contacts, dry_run_info_.any_search_excluded);
        }

        if (duplicate_contacts.size() < 2) break;

        summary_info_.inc_merge_set();
        MergeContactOperationSummary merge_set_operation_info;
        std::vector<LibFred::MergeContactEmailNotificationInput> email_notification_input_vector;

        boost::optional<LibFred::MergeContactSelectionOutput> dst_contact;

        while (duplicate_contacts.size()) {

            if (!dst_contact) {
                dst_contact = LibFred::MergeContactSelection(
                                  std::vector<std::string>(duplicate_contacts.begin(), duplicate_contacts.end()),
                                  selection_filter
                              ).exec(ctx);

                if(!dst_contact) {
                    throw std::runtime_error("dst_contact not identified\n");
                }

                ctx.get_log().debug(boost::format("contact duplicates set: { %1% }") % boost::algorithm::join(duplicate_contacts, ", "));

                ctx.get_log().debug(boost::format("winner handle: %1%") % (*dst_contact).handle);

                duplicate_contacts.erase((*dst_contact).handle);

            }

            if(duplicate_contacts.size() < 1) break;

            const std::string src_contact = *(duplicate_contacts.begin());

            try {
                MergeContactOperationSummary merge_operation_info;

                summary_info_.inc_merge_operation();

                const LibFred::MergeContactOutput merge_data =
                    merge_contact(src_contact, (*dst_contact).handle, *system_registrar);

                if(!is_set_dry_run()) {
                    email_notification_input_vector.push_back(
                        LibFred::MergeContactEmailNotificationInput(src_contact, (*dst_contact).handle, merge_data)
                    );
                }

                if (get_verbose_level() > 0) {
                    merge_operation_info.add_merge_output(merge_data);
                    merge_set_operation_info.add_merge_output(merge_data);
                    all_merge_operation_info_.add_merge_output(merge_data);
                }

                if (get_verbose_level() > 2) {
                    _output_stream << format_merge_contact_output(merge_data, src_contact, (*dst_contact).handle, summary_info_, indenter);
                    _output_stream << merge_operation_info.format(indenter.dive());
                }
            }
            catch(const LibFred::MergeContact::Exception& ex)
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
                    dst_contact = boost::optional<LibFred::MergeContactSelectionOutput>(); // invalidate dst_contact
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
            _output_stream << format_header(
                boost::str(boost::format("[-/%1%/-] MERGE SET SUMMARY") % summary_info_.merge_sets_total),
                indenter);
            _output_stream << merge_set_operation_info.format(indenter);
        }

        if (!is_set_dry_run()) {
            ret_email_notifications = email_notification(mm_, email_notification_input_vector);
        }

    }

    if (get_verbose_level() > 0) {
        _output_stream << format_header(
            boost::str(boost::format("[%1%/%2%/-] PROCEDURE RUN SUMMARY") % summary_info_.merge_operations_total % summary_info_.merge_sets_total),
            indenter);
        _output_stream << all_merge_operation_info_.format(indenter);
    }
    if (!is_set_dry_run()) {
        ctx.commit_transaction();
    }
    return ret_email_notifications;
}


}

