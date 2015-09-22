#include "src/fredlib/messages/generate.h"
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
    static std::string messages_associated_with_request(const std::string &_request_id)
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
    static std::string messages_associated_with_request(const std::string &_request_id)
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

}

template < CommChannel::Value COMM_CHANNEL >
void Generate::Into< COMM_CHANNEL >::exec(OperationContext &_ctx)
{
    static std::string sql;
    static Database::query_param_list params;
    if (sql.empty()) {
        std::ostringstream ssql;
        ssql << "WITH required_status AS ("
                         "SELECT id FROM enum_public_request_status "
                         "WHERE name=" << RequiredStatus< COMM_CHANNEL >::value(params) <<
                     "),"
                     "possible_types AS ("
                         "SELECT id FROM enum_public_request_type "
                         "WHERE name IN (" << PossibleTypes< COMM_CHANNEL >::value(params) << ")"
                     "),"
                     "channel_type AS ("
                         "SELECT id FROM comm_type "
                         "WHERE type=" << ChannelType< COMM_CHANNEL >::value(params) <<
                     "),"
                     "generation AS ("
                         "SELECT COALESCE((SELECT LOWER(val) NOT IN ('disabled','disable','false','f','0') "
                                          "FROM enum_parameters "
                                          "WHERE name=" << Parameters< COMM_CHANNEL >::name(params) << "),"
                                         "true) AS enabled"
                     ") "
                "SELECT prom.request_id,prom.object_id,"
                       "(SELECT ch.historyid "
                        "FROM contact_history ch "
                        "JOIN history h ON h.id=ch.historyid "
                        "JOIN public_request pr ON h.valid_from<=pr.create_time AND "
                                                               "(pr.create_time<h.valid_to OR h.valid_to IS NULL)"
                        "WHERE ch.id=prom.object_id AND "
                              "pr.id=prom.request_id"
                       "),"
                       "lock_public_request_lock(prom.object_id) "
                "FROM public_request_objects_map prom "
                "WHERE (SELECT enabled FROM generation) AND "
                      "EXISTS(SELECT * FROM contact WHERE id=prom.object_id) AND "
                      "EXISTS(SELECT * FROM public_request WHERE id=prom.request_id AND "
                                                                "status=(SELECT id FROM required_status) AND "
                                                                "request_type IN (SELECT id FROM possible_types)) AND "
                      "NOT " << Exists< COMM_CHANNEL >::messages_associated_with_request("prom.request_id");
        sql = ssql.str();
    }
    const Database::Result dbres = _ctx.get_conn().exec_params(sql, params);
    for (::size_t idx = 0; idx < dbres.size(); ++idx) {
        typedef unsigned long long PublicRequestId;
        typedef unsigned long long ContactId;
        typedef unsigned long long ContactHistoryId;
        const PublicRequestId  public_request_id  = static_cast< PublicRequestId  >(dbres[idx][0]);
        const ContactId        contact_id         = static_cast< ContactId        >(dbres[idx][1]);
        const ContactHistoryId contact_history_id = static_cast< ContactHistoryId >(dbres[idx][2]);
    }
}

template void Generate::Into< CommChannel::SMS    >::exec(OperationContext &_ctx);
template void Generate::Into< CommChannel::EMAIL  >::exec(OperationContext &_ctx);
template void Generate::Into< CommChannel::LETTER >::exec(OperationContext &_ctx);

}//namespace Fred::Messages
}//namespace Fred
