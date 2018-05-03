#include "tools/disclose_flags_updater/disclose_value.hh"
#include "tools/disclose_flags_updater/worker.hh"
#include "tools/disclose_flags_updater/thread_safe_output.hh"
#include "src/libfred/registrable_object/contact/update_contact.hh"

#include <thread>

namespace Tools {
namespace DiscloseFlagsUpdater {

namespace {

void update_contact_disclose_flag_impl(
        ::LibFred::OperationContext& _ctx,
        const ContactUpdateTask& _task,
        const std::string& _by_registrar,
        const DiscloseSettings& _discloses,
        const boost::optional<std::uint64_t> _logd_request_id
)
{
    ::LibFred::UpdateContactById update_op(_task.contact_id, _by_registrar);
    if (_logd_request_id != boost::none)
    {
        update_op.set_logd_request_id(_logd_request_id.value());
    }

    if (_discloses.name != DiscloseValue::not_set)
    {
        update_op.set_disclosename(to_db_value(_discloses.name));
    }
    if (_discloses.org != DiscloseValue::not_set)
    {
        update_op.set_discloseorganization(to_db_value(_discloses.org));
    }
    if (_discloses.voice != DiscloseValue::not_set)
    {
        update_op.set_disclosetelephone(to_db_value(_discloses.voice));
    }
    if (_discloses.fax != DiscloseValue::not_set)
    {
        update_op.set_disclosefax(to_db_value(_discloses.fax));
    }
    if (_discloses.email != DiscloseValue::not_set)
    {
        update_op.set_discloseemail(to_db_value(_discloses.email));
    }
    if (_discloses.vat != DiscloseValue::not_set)
    {
        update_op.set_disclosevat(to_db_value(_discloses.vat));
    }
    if (_discloses.ident != DiscloseValue::not_set)
    {
        update_op.set_discloseident(to_db_value(_discloses.ident));
    }
    if (_discloses.notify_email != DiscloseValue::not_set)
    {
        update_op.set_disclosenotifyemail(to_db_value(_discloses.notify_email));
    }
    if (_discloses.addr != DiscloseAddressValue::not_set)
    {
        auto discloseaddress_value = true;
        if (_discloses.addr == DiscloseAddressValue::hide_verified)
        {
            if (_task.contact_hidden_address_allowed)
            {
                discloseaddress_value = false;
            }
            else
            {
                discloseaddress_value = true;
            }
        }
        else
        {
            discloseaddress_value = to_db_value(_discloses.addr);
        }
        update_op.set_discloseaddress(discloseaddress_value);
    }

    update_op.exec(_ctx);
}


} // {anonymous}


void Worker::operator()()
{
    std::ostringstream thread_prefix_format;
    thread_prefix_format << "[thread:" << std::this_thread::get_id() << "] ";
    try
    {

        started = true;
        safe_cout(thread_prefix_format.str() + "started\n");
        safe_cout_flush();

        ctx = std::make_shared<::LibFred::OperationContextCreator>();
        for (auto it = range.first; it != range.second; ++it)
        {
            update_contact_disclose_flag_impl(*ctx, *it, opts.by_registrar, discloses, opts.logd_request_id);
            ++done_count;
        }
    }
    catch (const std::exception& ex)
    {
        safe_cerr(thread_prefix_format.str() + "error: " + ex.what() + "\n");
    }
    catch (...)
    {
        safe_cerr(thread_prefix_format.str() + "unknown error\n");
    }

    safe_cout(thread_prefix_format.str() + "finished\n");
    safe_cout_flush();
    exited = true;
}

}
}
