#include "src/admin/contact/merge_contact.h"
#include "src/admin/contact/merge_contact_logger.h"


namespace Admin {

std::string get_system_registrar(Fred::OperationContext &_ctx)
{
    Database::Result system_registrar_result = _ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system is True");
    if (system_registrar_result.size() == 0) {
        throw std::runtime_error("no system registrar found");
    }
    return static_cast<std::string>(system_registrar_result[0][0]);
}



MergeContact::MergeContact(
        const std::string &_src_contact_handle,
        const std::string &_dst_contact_handle)
    : src_contact_handle_(_src_contact_handle),
      dst_contact_handle_(_dst_contact_handle)
{
}



MergeContact::MergeContact(
        const std::string &_src_contact_handle,
        const std::string &_dst_contact_handle,
        const Optional<std::string> &_registrar)
    : src_contact_handle_(_src_contact_handle),
      dst_contact_handle_(_dst_contact_handle),
      registrar_(_registrar)
{
}



Fred::MergeContactOutput MergeContact::exec(Fred::Logger::LoggerClient &_logger_client)
{
    Fred::OperationContext ctx;

    if (!registrar_.isset()) {
        registrar_ = get_system_registrar(ctx);
    }

    Fred::MergeContactOutput merge_data;
    unsigned long long req_id = 0;
    try {
        req_id = logger_merge_contact_create_request(_logger_client, src_contact_handle_, dst_contact_handle_);

        merge_data = Fred::MergeContact(src_contact_handle_, dst_contact_handle_, registrar_.get_value())
                            .set_logd_request_id(req_id)
                            .exec(ctx);

        Fred::create_poll_messages(merge_data, ctx);
        ctx.commit_transaction();

        logger_merge_contact_close_request_success(_logger_client, req_id, merge_data);
    }
    catch (...) {
        logger_merge_contact_close_request_fail(_logger_client, req_id);
        throw;
    }
    return merge_data;
}


Fred::MergeContactOutput MergeContact::exec_dry_run()
{
    Fred::OperationContext ctx;

    if (!registrar_.isset()) {
        registrar_ = get_system_registrar(ctx);
    }

    return Fred::MergeContact(src_contact_handle_, dst_contact_handle_, registrar_.get_value()).exec_dry_run(ctx);
}


}

