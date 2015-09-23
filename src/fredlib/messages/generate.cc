#include "src/fredlib/messages/generate.h"
#include "src/fredlib/messages/messages_impl.h"
#include "src/fredlib/public_request/public_request_status.h"
#include "src/mojeid/mojeid_public_request.h"
#include "util/db/query_param.h"

namespace Fred {
namespace Messages {

namespace {

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
                GetData< COMM_CHANNEL >::finish_query();
    }
};

template < >
struct GetData< CommChannel::SMS >
{
    static std::string finish_query()
    {
        return "SELECT tg.public_request_id,tg.contact_id,tg.contact_history_id,pra.password,ch.telephone,obr.name,"
                      "lock_public_request_lock(tg.contact_id) "
               "FROM to_generate tg "
               "JOIN public_request_auth pra ON pra.id=tg.public_request_id "
               "JOIN contact_history ch ON ch.id=tg.contact_id AND ch.historyid=tg.contact_history_id "
               "JOIN object_registry obr ON obr.id=tg.contact_id";
    }
};

template < >
struct GetData< CommChannel::LETTER >
{
    static std::string finish_query()
    {
        return "";
    }
};

template < >
struct GetData< CommChannel::EMAIL >
{
    static std::string finish_query()
    {
        return "";
    }
};

template < CommChannel::Value COMM_CHANNEL >
struct ProcessFor;

template < >
struct ProcessFor< CommChannel::SMS >
{
    static void data(OperationContext &_ctx, const Database::Result &_dbres)
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

            const GeneralId message_id = manager_ptr->save_sms_to_send(contact_handle.c_str(),
                                                                       contact_phone.c_str(),
                                                                       sms_content.c_str(),
                                                                       message_type_mojeid_pin2,
                                                                       contact_id,
                                                                       contact_history_id);
            _ctx.get_conn().exec_params(
                "INSERT INTO public_request_messages_map (public_request_id,"
                                                         "message_archive_id,"
                                                         "mail_archive_id) "
                "VALUES ($1::BIGINT,"//public_request_id
                        "$2::BIGINT,"//message_archive_id
                        "NULL)",     //mail_archive_id
                Database::query_param_list(public_request_id)(message_id));
        }
    }
};

template < >
struct ProcessFor< CommChannel::LETTER >
{
    static void data(OperationContext &_ctx, const Database::Result &_dbres)
    {
    }
};

template < >
struct ProcessFor< CommChannel::EMAIL >
{
    static void data(OperationContext &_ctx, const Database::Result &_dbres)
    {
    }
};

}

template < CommChannel::Value COMM_CHANNEL >
void Generate::Into< COMM_CHANNEL >::exec(OperationContext &_ctx)
{
    static Database::query_param_list params;
    static const std::string sql = CollectFor< COMM_CHANNEL >::query(params);
    const Database::Result dbres = _ctx.get_conn().exec_params(sql, params);
    ProcessFor< COMM_CHANNEL >::data(_ctx, dbres);
}

template void Generate::Into< CommChannel::SMS    >::exec(OperationContext &_ctx);
template void Generate::Into< CommChannel::EMAIL  >::exec(OperationContext &_ctx);
template void Generate::Into< CommChannel::LETTER >::exec(OperationContext &_ctx);

}//namespace Fred::Messages
}//namespace Fred
