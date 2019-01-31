#include "src/backend/admin/contact/verification/delete_domains_of_invalid_contact.hh"

#include "src/backend/admin/contact/verification/contact_states/enum.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/poll/create_poll_message.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "src/deprecated/libfred/registrable_object/contact/verification/enum_testsuite_handle.hh"
#include "libfred/registrable_object/contact/verification/info_check.hh"
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_diff.hh"
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "util/log/context.hh"

#include <boost/foreach.hpp>

#include <set>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

static std::set<unsigned long long> get_owned_domains_locking(
        LibFred::OperationContext& _ctx,
        unsigned long long _contact_id);

static void store_check_poll_message_relation(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle,
        unsigned long long _poll_msg_id);

static bool related_delete_domain_poll_message_exists(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle);


void delete_domains_of_invalid_contact(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle)
{
    Logging::Context log("delete_domains_of_invalid_contact");

    try
    {
        LibFred::InfoContactCheckOutput check_info = LibFred::InfoContactCheck(_check_handle).exec(_ctx);

        if (check_info.testsuite_handle != LibFred::TestsuiteHandle::MANUAL)
        {
            throw ExceptionIncorrectTestsuite();
        }

        if (check_info.check_state_history.rbegin()->status_handle != LibFred::ContactCheckStatus::FAIL)
        {
            throw ExceptionIncorrectCheckStatus();
        }

        LibFred::InfoContactOutput contact_info = LibFred::InfoContactHistoryByHistoryid(
                check_info.contact_history_id).exec(_ctx);

        {
            using namespace ContactStates;
            bool has_state_failed_verification = false;

            BOOST_FOREACH(
                    const LibFred::ObjectStateData& state,
                    LibFred::GetObjectStates(contact_info.info_contact_data.id).exec(_ctx)) {

                if (state.state_name == CONTACT_FAILED_MANUAL_VERIFICATION)
                {
                    has_state_failed_verification = true;
                }
                else if (state.state_name == CONTACT_IN_MANUAL_VERIFICATION)
                {
                    throw ExceptionIncorrectContactStatus();
                }
                else if (state.state_name == CONTACT_PASSED_MANUAL_VERIFICATION)
                {
                    throw ExceptionIncorrectContactStatus();
                }
            }
            if (!has_state_failed_verification)
            {
                throw ExceptionIncorrectContactStatus();
            }
        }

        if (related_delete_domain_poll_message_exists(_ctx, _check_handle))
        {
            throw ExceptionDomainsAlreadyDeleted();
        }

        std::set<unsigned long long> domain_ids_to_delete =
            get_owned_domains_locking(
                    _ctx,
                    contact_info.info_contact_data.id);

        for (std::set<unsigned long long>::const_iterator it = domain_ids_to_delete.begin();
             it != domain_ids_to_delete.end();
             ++it)
        {
            // beware - need to get info before deleting
            LibFred::InfoDomainData info_domain = LibFred::InfoDomainById(*it).exec(_ctx).info_domain_data;
            LibFred::DeleteDomainById(*it).exec(_ctx);

            store_check_poll_message_relation(
                    _ctx,
                    _check_handle,
                    LibFred::Poll::CreatePollMessage<LibFred::Poll::MessageType::delete_domain>()
                    .exec(_ctx, info_domain.historyid));
        }
    }
    catch (const LibFred::ExceptionUnknownCheckHandle&)
    {
        throw ExceptionUnknownCheckHandle();
    }
}


std::set<unsigned long long> get_owned_domains_locking(
        LibFred::OperationContext& _ctx,
        unsigned long long _contact_id)
{
    Database::Result owned_domains_res = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT o_r.id AS id_ "
            "   FROM object_registry AS o_r "
            "       JOIN domain AS d USING(id) "
            "   WHERE d.registrant = $1::integer "
            "   FOR UPDATE OF o_r ",
            // clang-format on
            Database::query_param_list(_contact_id));

    std::set<unsigned long long> result;

    for (Database::Result::Iterator it = owned_domains_res.begin();
         it != owned_domains_res.end();
         ++it
         )
    {
        result.insert(static_cast<unsigned long long>((*it)["id_"]));
    }

    return result;
}


void store_check_poll_message_relation(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle,
        unsigned long long _poll_msg_id)
{
    _ctx.get_conn().exec_params(
            // clang-format off
            "INSERT "
            "   INTO contact_check_poll_message_map "
            "   (contact_check_id, poll_message_id) "
            "   VALUES("
            "       (SELECT id FROM contact_check WHERE handle=$1::uuid), "
            "       $2::bigint"
            "   ) ",
            // clang-format on
            Database::query_param_list(_check_handle)(_poll_msg_id));
}


bool related_delete_domain_poll_message_exists(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle)
{
    return 0 < _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT 1 "  // ...whatever, just to keep it smaller than "*"
            "FROM contact_check_poll_message_map p_m_map "
            "JOIN message AS m ON p_m_map.poll_message_id=m.id "
            "JOIN messagetype AS mtype ON m.msgtype=mtype.id "
            "JOIN contact_check AS c_ch ON p_m_map.contact_check_id=c_ch.id "
            "WHERE c_ch.handle=$1::UUID AND mtype.name=$2::TEXT "
            "LIMIT 1", // going for bool value eventually, one would be enough
            // clang-format on
            Database::query_param_list(_check_handle)(
                    Conversion::Enums::to_db_handle(
                            LibFred::Poll::
                            MessageType::delete_domain))).size();
}


} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
