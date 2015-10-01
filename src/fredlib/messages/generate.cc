#include "src/fredlib/messages/generate.h"
#include "src/fredlib/messages/messages_impl.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/public_request/public_request_status.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/mojeid/mojeid_public_request.h"
#include "src/mojeid/mojeid2.h"
#include "src/corba/mailer_manager.h"
#include "util/cfg/config_handler_decl.h"
#include "util/cfg/handle_mojeid_args.h"
#include "util/db/query_param.h"
#include "util/corba_wrapper_decl.h"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace Fred {
namespace Messages {

namespace {

typedef unsigned long long GeneralId;

template < CommChannel::Value >
struct RequiredStatus
{
    static std::string value(Database::query_param_list &_params)
    {
        static const std::string status_new = PublicRequest::Status(PublicRequest::Status::NEW).into< std::string >();
        return "$" + _params.add(status_new) + "::TEXT";
    }
};

template < CommChannel::Value COMM_CHANNEL >
struct PossibleTypes;

template < >
struct PossibleTypes< CommChannel::SMS >
{
    static std::string value(Database::query_param_list &_params)
    {
        static const std::string type[] = {
            MojeID::PublicRequest::ContactConditionalIdentification::iface().get_public_request_type() };
        return "$" + _params.add(type[0]) + "::TEXT";
    }
};

template < >
struct PossibleTypes< CommChannel::LETTER >
{
    static std::string value(Database::query_param_list &_params)
    {
        static const std::string type[] = {
            MojeID::PublicRequest::ContactIdentification::iface().get_public_request_type(),
            MojeID::PublicRequest::ContactReidentification::iface().get_public_request_type() };
        return "$" + _params.add(type[0]) + "::TEXT,"
               "$" + _params.add(type[1]) + "::TEXT";
    }
};

template < >
struct PossibleTypes< CommChannel::EMAIL >
{
    static std::string value(Database::query_param_list &_params)
    {
        static const std::string type[] = {
            MojeID::PublicRequest::ContactConditionalIdentification::iface().get_public_request_type(),
            MojeID::PublicRequest::ConditionallyIdentifiedContactTransfer::iface().get_public_request_type(),
            MojeID::PublicRequest::IdentifiedContactTransfer::iface().get_public_request_type() };
        return "$" + _params.add(type[0]) + "::TEXT,"
               "$" + _params.add(type[1]) + "::TEXT,"
               "$" + _params.add(type[2]) + "::TEXT";
    }
};

template < CommChannel::Value COMM_CHANNEL >
struct ChannelType;

template < >
struct ChannelType< CommChannel::SMS >
{
    static std::string value(Database::query_param_list &_params)
    {
        return "$" + _params.add("sms") + "::TEXT";
    }
};

template < >
struct ChannelType< CommChannel::LETTER >
{
    static std::string value(Database::query_param_list &_params)
    {
        return "$" + _params.add("letter") + "::TEXT";
    }
};

template < >
struct ChannelType< CommChannel::EMAIL >
{
    static std::string value(Database::query_param_list &_params)
    {
        return "$" + _params.add("email") + "::TEXT";
    }
};

template < CommChannel::Value COMM_CHANNEL >
struct Exists
{
    static std::string messages_associated_with(const std::string &_request_id)
    {
        return "EXISTS(SELECT * FROM public_request_messages_map prmm "
                      "WHERE prmm.public_request_id=" + _request_id + " AND "
                            "EXISTS(SELECT * FROM message_archive ma "
                                   "WHERE ma.id=prmm.message_archive_id AND "
                                         "ma.comm_type_id=(SELECT id FROM channel_type)"
                                  ")"
                     ")";
    }
};

template < >
struct Exists< CommChannel::EMAIL >
{
    static std::string messages_associated_with(const std::string &_request_id)
    {
        return "EXISTS(SELECT * FROM public_request_messages_map prmm "
                      "WHERE prmm.public_request_id=" + _request_id + " AND "
                            "EXISTS(SELECT * FROM mail_archive ma WHERE ma.id=prmm.mail_archive_id)"
                     ")";
    }
};

template < CommChannel::Value COMM_CHANNEL >
struct Parameter;

template < >
struct Parameter< CommChannel::SMS >
{
    static std::string name(Database::query_param_list &_params)
    {
        return "$" + _params.add("async_sms_generation") + "::TEXT";
    }
};

template < >
struct Parameter< CommChannel::LETTER >
{
    static std::string name(Database::query_param_list &_params)
    {
        return "$" + _params.add("async_letter_generation") + "::TEXT";
    }
};

template < >
struct Parameter< CommChannel::EMAIL >
{
    static std::string name(Database::query_param_list &_params)
    {
        return "$" + _params.add("async_email_generation") + "::TEXT";
    }
};

template < CommChannel::Value COMM_CHANNEL >
struct GetData;

template < CommChannel::Value COMM_CHANNEL >
struct CollectFor
{
    static std::string query(Database::query_param_list &_params)
    {
        return "WITH required_status AS ("
                        "SELECT id FROM enum_public_request_status "
                        "WHERE name=" + RequiredStatus< COMM_CHANNEL >::value(_params) +
                    "),"
                    "possible_types AS ("
                        "SELECT id FROM enum_public_request_type "
                        "WHERE name IN (" + PossibleTypes< COMM_CHANNEL >::value(_params) + ")"
                    "),"
                    "channel_type AS ("
                        "SELECT id FROM comm_type "
                        "WHERE type=" + ChannelType< COMM_CHANNEL >::value(_params) +
                    "),"
                    "generation AS ("
                        "SELECT COALESCE((SELECT LOWER(val) NOT IN ('disabled','disable','false','f','0') "
                                         "FROM enum_parameters "
                                         "WHERE name=" + Parameter< COMM_CHANNEL >::name(_params) + "),"
                                        "true) AS enabled"
                    "),"
                    "to_generate AS ("
                        "SELECT prom.request_id AS public_request_id,"
                               "prom.object_id AS contact_id,"
                               "(SELECT ch.historyid "
                                "FROM contact_history ch "
                                "JOIN history h ON h.id=ch.historyid "
                                "JOIN public_request pr ON h.valid_from<=pr.create_time AND "
                                                                       "(pr.create_time<h.valid_to OR "
                                                                        "h.valid_to IS NULL) "
                                "WHERE ch.id=prom.object_id AND "
                                      "pr.id=prom.request_id"
                               ") AS contact_history_id "
                        "FROM public_request_objects_map prom "
                        "WHERE (SELECT enabled FROM generation) AND "
                              "EXISTS(SELECT * FROM contact WHERE id=prom.object_id) AND "
                              "EXISTS(SELECT * FROM public_request "
                                     "WHERE id=prom.request_id AND "
                                           "status=(SELECT id FROM required_status) AND "
                                           "request_type IN (SELECT id FROM possible_types) AND "
                                           "(NOW()::DATE-'1DAY'::INTERVAL)<create_time) AND "
                              "NOT " + Exists< COMM_CHANNEL >::messages_associated_with("prom.request_id") +
                    ") " +
                GetData< COMM_CHANNEL >::finish_query(_params);
    }
};

template < CommChannel::Value COMM_CHANNEL >
struct ProcessFor;

template < CommChannel::Value COMM_CHANNEL >
struct JoinMessage
{
    static void with_public_request(OperationContext &_ctx, GeneralId _public_request_id, GeneralId _message_id)
    {
        _ctx.get_conn().exec_params(
            "INSERT INTO public_request_messages_map (public_request_id,"
                                                     "message_archive_id,"
                                                     "mail_archive_id) "
            "VALUES ($1::BIGINT,"//public_request_id
                    "$2::BIGINT,"//message_archive_id
                    "NULL)",     //mail_archive_id
            Database::query_param_list(_public_request_id)(_message_id));
    }
};

template < >
struct JoinMessage< CommChannel::EMAIL >
{
    static void with_public_request(OperationContext &_ctx, GeneralId _public_request_id, GeneralId _message_id)
    {
        _ctx.get_conn().exec_params(
            "INSERT INTO public_request_messages_map (public_request_id,"
                                                     "message_archive_id,"
                                                     "mail_archive_id) "
            "VALUES ($1::BIGINT," //public_request_id
                    "NULL,"       //message_archive_id
                    "$2::BIGINT)",//mail_archive_id
            Database::query_param_list(_public_request_id)(_message_id));
    }
};

template < >
struct GetData< CommChannel::SMS >
{
    static std::string finish_query(const Database::query_param_list&)
    {
        return "SELECT tg.public_request_id,tg.contact_id,tg.contact_history_id,pra.password,ch.telephone,"
                      "LOWER(obr.name),lock_public_request_lock(tg.contact_id) "
               "FROM to_generate tg "
               "JOIN public_request_auth pra ON pra.id=tg.public_request_id "
               "JOIN contact_history ch ON ch.id=tg.contact_id AND ch.historyid=tg.contact_history_id "
               "JOIN object_registry obr ON obr.id=tg.contact_id";
    }
};

template < >
struct ProcessFor< CommChannel::SMS >
{
    static void data(const Database::Result &_dbres)
    {
        static const char *const message_type_mojeid_pin2 = "mojeid_pin2";
        static ManagerPtr manager_ptr = create_manager();
        for (::size_t idx = 0; idx < _dbres.size(); ++idx) {
            typedef unsigned long long GeneralId;
            const GeneralId   public_request_id  = static_cast< GeneralId   >(_dbres[idx][0]);
            const GeneralId   contact_id         = static_cast< GeneralId   >(_dbres[idx][1]);
            const GeneralId   contact_history_id = static_cast< GeneralId   >(_dbres[idx][2]);
            const std::string password           = static_cast< std::string >(_dbres[idx][3]);
            const std::string contact_phone      = static_cast< std::string >(_dbres[idx][4]);
            const std::string contact_handle     = static_cast< std::string >(_dbres[idx][5]);

            const std::string pin2 = MojeID::PublicRequest::ContactConditionalIdentification::get_pin2_part(password);
            const std::string sms_content = "Potvrzujeme uspesne zalozeni uctu mojeID. "
                                            "Pro aktivaci Vaseho uctu je nutne vlozit kody "
                                            "PIN1 a PIN2. PIN1 Vam byl zaslan emailem, PIN2 je: " + pin2;

            try {
                const GeneralId message_id = manager_ptr->save_sms_to_send(contact_handle.c_str(),
                                                                           contact_phone.c_str(),
                                                                           sms_content.c_str(),
                                                                           message_type_mojeid_pin2,
                                                                           contact_id,
                                                                           contact_history_id);
                OperationContextCreator ctx;
                JoinMessage< CommChannel::LETTER >::with_public_request(ctx, public_request_id, message_id);
                ctx.commit_transaction();
            }
            catch (...) {
            }
        }
    }
};

template < >
struct GetData< CommChannel::LETTER >
{
    static std::string finish_query(Database::query_param_list &_params)
    {
        typedef Fred::Object::State FOS;
        const std::string state_validated_contact = FOS(FOS::VALIDATED_CONTACT).into< std::string >();
        return "SELECT tg.public_request_id,tg.contact_history_id,"
                      "(SELECT name FROM enum_public_request_type WHERE id=pr.request_type) AS public_request_type,"
                      "pr.create_time,"
                      "EXISTS(SELECT * FROM object_state os "
                             "WHERE os.object_id=tg.contact_id AND "
                                   "os.state_id=(SELECT id FROM enum_object_states "
                                                "WHERE name=$" + _params.add(state_validated_contact) + "::TEXT) AND "
                                   "os.valid_from<=pr.create_time AND "
                                                 "(pr.create_time<os.valid_to OR os.valid_to IS NULL)"
                            ") AS is_validated,"
                      "lock_public_request_lock(tg.contact_id) "
               "FROM to_generate tg "
               "JOIN public_request pr ON pr.id=tg.public_request_id";
    }
};

template < >
struct ProcessFor< CommChannel::LETTER >
{
    static void data(const Database::Result &_dbres)
    {
        static const HandleMojeIDArgs *const server_conf_ptr =
            CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >();
        static ManagerPtr manager_ptr = create_manager();
        for (::size_t idx = 0; idx < _dbres.size(); ++idx) {
            try {
                const GeneralId   public_request_id   = static_cast< GeneralId   >(_dbres[idx][0]);
                const GeneralId   contact_history_id  = static_cast< GeneralId   >(_dbres[idx][1]);
                const std::string public_request_type = static_cast< std::string >(_dbres[idx][2]);
                const std::string public_request_time = static_cast< std::string >(_dbres[idx][3]);
                const bool        validated_contact   = static_cast< bool        >(_dbres[idx][4]);
                namespace FMPR = Fred::MojeID::PublicRequest;
                if ((public_request_type != FMPR::ContactIdentification::iface().get_public_request_type()) &&
                    (public_request_type != FMPR::ContactReidentification::iface().get_public_request_type())) {
                    continue;
                }

                OperationContextCreator ctx;
                const InfoContactData contact_data = InfoContactHistoryByHistoryid(contact_history_id)
                                                         .exec(ctx).info_contact_data;
                namespace bptime = boost::posix_time;
                const bptime::ptime letter_time = bptime::time_from_string(public_request_time);
                enum { DONT_USE_LOG_REQUEST_ID = 0 };
                const GeneralId message_id = Registry::MojeID::MojeID2Impl::send_mojeid_card(
                    ctx,
                    manager_ptr.get(),
                    contact_data,
                    server_conf_ptr->letter_limit_count,
                    server_conf_ptr->letter_limit_interval,
                    DONT_USE_LOG_REQUEST_ID,
                    letter_time,
                    validated_contact);
                JoinMessage< CommChannel::LETTER >::with_public_request(ctx, public_request_id, message_id);
                ctx.commit_transaction();
            }
            catch (...) {
            }
        }
    }
};

template < >
struct GetData< CommChannel::EMAIL >
{
    static std::string finish_query(Database::query_param_list &_params)
    {
        return "SELECT tg.public_request_id,UPPER(obr.name),c.email,"
                      "lock_public_request_lock(tg.contact_id) "
               "FROM to_generate tg "
               "JOIN public_request pr ON pr.id=tg.public_request_id "
               "JOIN object_registry obr ON obr.id=tg.contact_id "
               "JOIN contact_history c ON c.historyid=tg.contact_history_id";
    }
};

template < >
struct ProcessFor< CommChannel::EMAIL >
{
    static void data(const Database::Result &_dbres)
    {
        typedef std::auto_ptr< Mailer::Manager > MailerPtr;
        const MailerPtr mailer_ptr(new MailerManager(CorbaContainer::get_instance()->getNS()));
        for (::size_t idx = 0; idx < _dbres.size(); ++idx) {
            try {
                const GeneralId   public_request_id = static_cast< GeneralId   >(_dbres[idx][0]);
                const std::string contact_handle    = static_cast< std::string >(_dbres[idx][1]);
                const std::string contact_email     = static_cast< std::string >(_dbres[idx][2]);

                Mailer::Parameters params;
                Mailer::Handles handles;
                handles.push_back(contact_handle);
                Mailer::Attachments attach;
                const GeneralId message_id = mailer_ptr->sendEmail(
                    "",           //from:         default sender from notification system
                    contact_email,//to:
                    "",           //subject:      default subject is taken from template
                    "",           //mailTemplate:
                    params,       //params:
                    handles,      //handles:
                    attach);      //attach:

                OperationContextCreator ctx;
                JoinMessage< CommChannel::LETTER >::with_public_request(ctx, public_request_id, message_id);
                ctx.commit_transaction();
            }
            catch (...) {
            }
        }
    }
};

}

template < CommChannel::Value COMM_CHANNEL >
void Generate::Into< COMM_CHANNEL >::exec(OperationContext &_ctx)
{
    static Database::query_param_list params;
    static const std::string sql = CollectFor< COMM_CHANNEL >::query(params);
    const Database::Result dbres = _ctx.get_conn().exec_params(sql, params);
    ProcessFor< COMM_CHANNEL >::data(dbres);
}

template void Generate::Into< CommChannel::SMS    >::exec(OperationContext &_ctx);
template void Generate::Into< CommChannel::EMAIL  >::exec(OperationContext &_ctx);
template void Generate::Into< CommChannel::LETTER >::exec(OperationContext &_ctx);

template < CommChannel::Value COMM_CHANNEL >
void Generate::enable(OperationContext &_ctx, bool flag)
{
    Database::query_param_list params;
    const std::string sql = "UPDATE enum_parameters "
                            "SET val=$" + params.add(flag ? "enabled" : "disabled") + "::TEXT "
                            "WHERE name=" + Parameter< COMM_CHANNEL >::name(params);
    _ctx.get_conn().exec_params(sql, params);
}

template void Generate::enable< CommChannel::SMS    >(OperationContext &_ctx, bool flag);
template void Generate::enable< CommChannel::EMAIL  >(OperationContext &_ctx, bool flag);
template void Generate::enable< CommChannel::LETTER >(OperationContext &_ctx, bool flag);

}//namespace Fred::Messages
}//namespace Fred
