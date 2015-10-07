#include "src/mojeid/messages/generate.h"
#include "src/fredlib/common_object.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/public_request/public_request_status.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/mojeid/mojeid_public_request.h"
#include "src/mojeid/mojeid2.h"
#include "src/corba/mailer_manager.h"
#include "util/cfg/config_handler_decl.h"
#include "util/cfg/handle_corbanameservice_args.h"
#include "util/cfg/handle_mojeid_args.h"
#include "util/cfg/handle_registry_args.h"
#include "util/db/query_param.h"
#include "util/corba_wrapper_decl.h"
#include "util/types/birthdate.h"
#include "util/xmlgen.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace MojeID {  //MojeID
namespace Messages {//MojeID::Messages

namespace {         //MojeID::Messages::{anonymous}

template < CommChannel::Value >
struct RequiredStatus
{
    static std::string value(Database::query_param_list &_params)
    {
        typedef Fred::PublicRequest::Status FPRS;
        static const std::string status_new = FPRS(FPRS::NEW).into< std::string >();
        return "$" + _params.add(status_new) + "::TEXT";
    }
};

template < CommChannel::Value COMM_CHANNEL >
struct PossibleRequestTypes;

template < >
struct PossibleRequestTypes< CommChannel::SMS >
{
    typedef Fred::MojeID::PublicRequest::ContactConditionalIdentification PubReqCCI;
    static std::string value(Database::query_param_list &_params)
    {
        static const std::string type[] = {
            Fred::MojeID::PublicRequest::ContactConditionalIdentification::iface().get_public_request_type() };
        return "$" + _params.add(type[0]) + "::TEXT";
    }
    static Generate::MessageId generate_message(
        Fred::OperationContextCreator &_ctx,
        const Multimanager &_multimanager,
        const std::string &_public_request_type,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const Generate::message_checker &_check_message_limits,
        GeneralId _contact_history_id)
    {
        if (PubReqCCI::iface().get_public_request_type() == _public_request_type) {
            const Generate::MessageId message_id = Generate::Into< CommChannel::SMS >::for_given_request< PubReqCCI >(
                _ctx,
                _multimanager,
                _locked_request,
                _locked_contact,
                _check_message_limits,
                _contact_history_id);
            return message_id;
        }
        throw std::runtime_error("unexpected public request type: " + _public_request_type);
    }
};

template < >
struct PossibleRequestTypes< CommChannel::LETTER >
{
    typedef Fred::MojeID::PublicRequest::ContactIdentification   PubReqCI;
    typedef Fred::MojeID::PublicRequest::ContactReidentification PubReqCR;
    static std::string value(Database::query_param_list &_params)
    {
        static const std::string type[] = {
            PubReqCI::iface().get_public_request_type(),
            PubReqCR::iface().get_public_request_type() };
        return "$" + _params.add(type[0]) + "::TEXT,"
               "$" + _params.add(type[1]) + "::TEXT";
    }
    static Generate::MessageId generate_message(
        Fred::OperationContextCreator &_ctx,
        const Multimanager &_multimanager,
        const std::string &_public_request_type,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const Generate::message_checker &_check_message_limits,
        GeneralId _contact_history_id)
    {
        static const CommChannel::Value channel_letter = CommChannel::LETTER;
        if (PubReqCI::iface().get_public_request_type() == _public_request_type) {
            const Generate::MessageId message_id = Generate::Into< channel_letter >::for_given_request< PubReqCI >(
                _ctx,
                _multimanager,
                _locked_request,
                _locked_contact,
                _check_message_limits,
                _contact_history_id);
            return message_id;
        }
        if (PubReqCR::iface().get_public_request_type() == _public_request_type) {
            const Generate::MessageId message_id = Generate::Into< channel_letter >::for_given_request< PubReqCR >(
                _ctx,
                _multimanager,
                _locked_request,
                _locked_contact,
                _check_message_limits,
                _contact_history_id);
            return message_id;
        }
        throw std::runtime_error("unexpected public request type: " + _public_request_type);
    }
};

template < >
struct PossibleRequestTypes< CommChannel::EMAIL >
{
    typedef Fred::MojeID::PublicRequest::ContactConditionalIdentification       PubReqCCI;
    typedef Fred::MojeID::PublicRequest::ConditionallyIdentifiedContactTransfer PubReqCICT;
    typedef Fred::MojeID::PublicRequest::IdentifiedContactTransfer              PubReqICT;
    static std::string value(Database::query_param_list &_params)
    {
        static const std::string type[] = {
            PubReqCCI::iface().get_public_request_type(),
            PubReqCICT::iface().get_public_request_type(),
            PubReqICT::iface().get_public_request_type() };
        return "$" + _params.add(type[0]) + "::TEXT,"
               "$" + _params.add(type[1]) + "::TEXT,"
               "$" + _params.add(type[2]) + "::TEXT";
    }
    static Generate::MessageId generate_message(
        Fred::OperationContextCreator &_ctx,
        const Multimanager &_multimanager,
        const std::string &_public_request_type,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const Generate::message_checker &_check_message_limits,
        GeneralId _contact_history_id)
    {
        static const CommChannel::Value channel_letter = CommChannel::EMAIL;
        if (PubReqCCI::iface().get_public_request_type() == _public_request_type) {
            const Generate::MessageId message_id = Generate::Into< channel_letter >::for_given_request< PubReqCCI >(
                _ctx,
                _multimanager,
                _locked_request,
                _locked_contact,
                _check_message_limits,
                _contact_history_id);
            return message_id;
        }
        if (PubReqCICT::iface().get_public_request_type() == _public_request_type) {
            const Generate::MessageId message_id = Generate::Into< channel_letter >::for_given_request< PubReqCICT >(
                _ctx,
                _multimanager,
                _locked_request,
                _locked_contact,
                _check_message_limits,
                _contact_history_id);
            return message_id;
        }
        if (PubReqICT::iface().get_public_request_type() == _public_request_type) {
            const Generate::MessageId message_id = Generate::Into< channel_letter >::for_given_request< PubReqICT >(
                _ctx,
                _multimanager,
                _locked_request,
                _locked_contact,
                _check_message_limits,
                _contact_history_id);
            return message_id;
        }
        throw std::runtime_error("unexpected public request type: " + _public_request_type);
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
                        "WHERE name IN (" + PossibleRequestTypes< COMM_CHANNEL >::value(_params) + ")"
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
                        "SELECT id AS public_request_id,"
                               "create_time AS public_request_create_time,"
                               "request_type AS public_request_type_id,"
                               "(SELECT object_id FROM public_request_objects_map prom "
                                "WHERE request_id=pr.id AND "
                                      "EXISTS(SELECT * FROM contact WHERE id=prom.object_id)"
                               ") AS contact_id "
                        "FROM public_request pr,"
                             "generation "
                        "WHERE generation.enabled AND "
                              "status=(SELECT id FROM required_status) AND "
                              "(NOW()::DATE-'1DAY'::INTERVAL)<create_time AND "
                              "request_type IN (SELECT id FROM possible_types) AND "
                              "NOT " + Exists< COMM_CHANNEL >::messages_associated_with("pr.id") +
                    ") "
               "SELECT public_request_id,"
                      "(SELECT name FROM enum_public_request_type "
                       "WHERE id=tg.public_request_type_id) AS public_request_type,"
                      "contact_id,"
                      "(SELECT historyid FROM contact_history ch "
                       "WHERE id=tg.contact_id AND "
                             "(SELECT valid_from<=tg.public_request_create_time AND "
                                                "(tg.public_request_create_time<valid_to OR valid_to IS NULL) "
                              "FROM history "
                              "WHERE id=ch.historyid)"
                      ") AS contact_history_id,"
                      "lock_public_request_lock(contact_id) "
               "FROM to_generate tg "
               "WHERE contact_id IS NOT NULL";
    }
};

template < CommChannel::Value COMM_CHANNEL >
struct JoinMessage
{
    static void with_public_request(Fred::OperationContext &_ctx,
                                    const Fred::PublicRequestLockGuard &_locked_request,
                                    GeneralId _message_id)
    {
        _ctx.get_conn().exec_params(
            "INSERT INTO public_request_messages_map (public_request_id,"
                                                     "message_archive_id,"
                                                     "mail_archive_id) "
            "VALUES ($1::BIGINT,"//public_request_id
                    "$2::BIGINT,"//message_archive_id
                    "NULL)",     //mail_archive_id
            Database::query_param_list(_locked_request.get_public_request_id())
                                      (_message_id));
    }
};

template < >
struct JoinMessage< CommChannel::EMAIL >
{
    static void with_public_request(Fred::OperationContext &_ctx,
                                    const Fred::PublicRequestLockGuard &_locked_request,
                                    GeneralId _message_id)
    {
        _ctx.get_conn().exec_params(
            "INSERT INTO public_request_messages_map (public_request_id,"
                                                     "message_archive_id,"
                                                     "mail_archive_id) "
            "VALUES ($1::BIGINT," //public_request_id
                    "NULL,"       //message_archive_id
                    "$2::BIGINT)",//mail_archive_id
            Database::query_param_list(_locked_request.get_public_request_id())
                                      (_message_id));
    }
};

class PublicRequestLocked:public Fred::PublicRequestLockGuard
{
public:
    PublicRequestLocked(Fred::PublicRequestId _locked_request_id)
    :   Fred::PublicRequestLockGuard(_locked_request_id) { }
};

class PublicRequestObjectLocked:public Fred::PublicRequestObjectLockGuard
{
public:
    PublicRequestObjectLocked(Fred::ObjectId _locked_object_id)
    :   Fred::PublicRequestObjectLockGuard(_locked_object_id) { }
};

template < CommChannel::Value COMM_CHANNEL, typename PUBLIC_REQUEST_TYPE >
struct generate_message;

template < >
struct generate_message< CommChannel::SMS, Fred::MojeID::PublicRequest::ContactConditionalIdentification >
{
    static Generate::MessageId for_given_request(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const Generate::message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id)
    {
        const Database::Result dbres = _ctx.get_conn().exec_params(
            "SELECT (SELECT password FROM public_request_auth WHERE id=$1::BIGINT),"
                   "ch.telephone,LOWER(obr.name) "
            "FROM contact_history ch "
            "JOIN object_registry obr ON obr.id=ch.id "
            "WHERE ch.id=$2::BIGINT AND ch.historyid=$3::BIGINT",
            Database::query_param_list(_locked_request.get_public_request_id())
                                      (_locked_contact.get_object_id())
                                      (_contact_history_id.get_value()));
        static const char *const message_type_mojeid_pin2 = "mojeid_pin2";
        const std::string password       = static_cast< std::string >(dbres[0][0]);
        const std::string contact_phone  = static_cast< std::string >(dbres[0][1]);
        const std::string contact_handle = static_cast< std::string >(dbres[0][2]);

        const std::string pin2 = Fred::MojeID::PublicRequest::ContactConditionalIdentification::
                                     get_pin2_part(password);
        const std::string sms_content = "Potvrzujeme uspesne zalozeni uctu mojeID. "
                                        "Pro aktivaci Vaseho uctu je nutne vlozit kody "
                                        "PIN1 a PIN2. PIN1 Vam byl zaslan emailem, PIN2 je: " + pin2;

        const GeneralId message_id = _multimanager.select< Fred::Messages::Manager >()
                                         .save_sms_to_send(contact_handle.c_str(),
                                                           contact_phone.c_str(),
                                                           sms_content.c_str(),
                                                           message_type_mojeid_pin2,
                                                           _locked_contact.get_object_id(),
                                                           _contact_history_id.get_value());
        return message_id;
    }
};

Generate::MessageId send_auth_owner_letter(
    Fred::OperationContext &_ctx,
    Fred::Messages::Manager &_msg_manager,
    Fred::Document::Manager &_doc_manager,
    Fred::Document::GenerationType _doc_type,
    const Fred::InfoContactData &_data,
    const std::string &_pin3,
    bool _validated_contact,
    Fred::PublicRequestId _file_identification,
    const Optional< boost::posix_time::ptime > &_letter_time)
{
    std::string letter_xml("<?xml version='1.0' encoding='utf-8'?>");

    const std::string name = _data.name.get_value_or_default();
    const std::string::size_type name_delimiter_pos = name.find_last_of(' ');
    const std::string firstname = name_delimiter_pos != std::string::npos
                                  ? name.substr(                     0, name_delimiter_pos)
                                  : name;
    const std::string lastname  = name_delimiter_pos != std::string::npos
                                  ? name.substr(name_delimiter_pos + 1, std::string::npos)
                                  : std::string();
    static const char female_suffix[] = "á"; // utf-8 encoded
    enum { FEMALE_SUFFIX_LEN = sizeof(female_suffix) - 1,
           STR_EQUAL = 0 };
    const std::string sex = (FEMALE_SUFFIX_LEN <= name.length()) &&
                            (std::strcmp(name.c_str() + name.length() - FEMALE_SUFFIX_LEN, female_suffix) == STR_EQUAL)
                            ? "female"
                            : "male";

    const Fred::InfoContactData::Address addr = _data.get_address< Fred::ContactAddressType::MAILING >();
    Fred::Messages::PostalAddress pa;
    pa.name    = name;
    pa.org     = _data.organization.get_value_or_default();
    pa.street1 = addr.street1;
    pa.city    = addr.city;
    pa.state   = addr.stateorprovince.get_value_or_default();
    pa.code    = addr.postalcode;
    pa.country = addr.country;

    Database::query_param_list params(pa.country);
    static const std::string sql = "SELECT country FROM enum_country WHERE country=$1::TEXT";
    const Database::Result dbres = _ctx.get_conn().exec_params(sql, params);
    const std::string addr_country = dbres.size() <= 0
                                     ? pa.country
                                     : static_cast< std::string >(dbres[0][0]);
    const std::string contact_handle = _data.handle;
    const boost::gregorian::date letter_date = _letter_time.isset()
                                               ? _letter_time.get_value().date()
                                               : boost::gregorian::day_clock::local_day();
    const std::string letter_date_str = boost::gregorian::to_iso_extended_string(letter_date);
    const std::string contact_state = _validated_contact ? "validated"
                                                         : "";

    Util::XmlTagPair("contact_auth", Util::vector_of<Util::XmlCallback>
        (Util::XmlTagPair("user", Util::vector_of<Util::XmlCallback>
            (Util::XmlTagPair("actual_date", Util::XmlUnparsedCData(letter_date_str)))
            (Util::XmlTagPair("name", Util::XmlUnparsedCData(pa.name)))
            (Util::XmlTagPair("organization", Util::XmlUnparsedCData(pa.org)))
            (Util::XmlTagPair("street", Util::XmlUnparsedCData(pa.street1)))
            (Util::XmlTagPair("city", Util::XmlUnparsedCData(pa.city)))
            (Util::XmlTagPair("stateorprovince", Util::XmlUnparsedCData(pa.state)))
            (Util::XmlTagPair("postal_code", Util::XmlUnparsedCData(pa.code)))
            (Util::XmlTagPair("country", Util::XmlUnparsedCData(addr_country)))
            (Util::XmlTagPair("account", Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair("username", Util::XmlUnparsedCData(contact_handle)))
                (Util::XmlTagPair("first_name", Util::XmlUnparsedCData(firstname)))
                (Util::XmlTagPair("last_name", Util::XmlUnparsedCData(lastname)))
                (Util::XmlTagPair("sex", Util::XmlUnparsedCData(sex)))
                (Util::XmlTagPair("email", Util::XmlUnparsedCData(_data.email.get_value_or_default())))
                (Util::XmlTagPair("mobile", Util::XmlUnparsedCData(_data.telephone.get_value_or_default())))
                (Util::XmlTagPair("state", Util::XmlUnparsedCData(contact_state)))
            ))
            (Util::XmlTagPair("auth", Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair("codes", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("pin3", Util::XmlUnparsedCData(_pin3)))
                ))
            ))
        ))
    )(letter_xml);

    std::stringstream xmldata;
    xmldata << letter_xml;

    enum { FILETYPE_MOJEID_CONTACT_IDENTIFICATION_REQUEST = 7 };
    const std::string file_identification_str = boost::lexical_cast< std::string >(_file_identification);
    const std::string file_name = "identification_request-" + file_identification_str + ".pdf";
    const Fred::ObjectId file_id = _doc_manager.generateDocumentAndSave(
        _doc_type,
        xmldata,
        file_name,
        FILETYPE_MOJEID_CONTACT_IDENTIFICATION_REQUEST, "");

    static const std::string comm_type = "letter";
    static const char *const message_type = "mojeid_card";
    const Generate::MessageId message_id =
        _msg_manager.save_letter_to_send(
            contact_handle.c_str(),
            pa,
            file_id,
            message_type,
            _data.id,
            _data.historyid,
            comm_type,
            true);
    return message_id;
}

template < >
struct generate_message< CommChannel::LETTER, Fred::MojeID::PublicRequest::ContactIdentification >
{
    static Generate::MessageId for_given_request(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const Generate::message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id)
    {
        typedef Fred::Object::State FOS;
        const std::string state_validated_contact = FOS(FOS::VALIDATED_CONTACT).into< std::string >();
        static const std::string message_type_mojeid_pin3 = "mojeid_pin3";
        Database::query_param_list params;
        params(_locked_request.get_public_request_id())
              (_locked_contact.get_object_id())
              (state_validated_contact)
              (message_type_mojeid_pin3);
        const Database::Result dbres = _ctx.get_conn().exec_params(
            "SELECT create_time,"
                   "(SELECT password FROM public_request_auth WHERE id=pr.id),"
                   "EXISTS(SELECT * FROM object_state os "
                          "WHERE os.object_id=$2::BIGINT AND "
                                "os.state_id=(SELECT id FROM enum_object_states WHERE name=$3::TEXT) AND "
                                "os.valid_from<=pr.create_time AND (pr.create_time<os.valid_to OR "
                                                                   "os.valid_to IS NULL)"
                         ") AS is_validated,"
                   "(SELECT mtfsm.service_handle "
                    "FROM message_type mt "
                    "JOIN message_type_forwarding_service_map mtfsm ON mtfsm.message_type_id=mt.id "
                    "WHERE mt.type=$4::TEXT) AS message_service_handle "
            "FROM public_request pr "
            "WHERE id=$1::BIGINT", params);
        if (dbres.size() <= 0) {
            throw std::runtime_error("no public request found");
        }
        if (dbres[0][3].isnull()) {
            throw std::runtime_error("no message service handle asociated with '" + 
                                     message_type_mojeid_pin3 + "' message type");
        }

        const std::string public_request_time    = static_cast< std::string >(dbres[0][0]);
        const std::string pin3                   = static_cast< std::string >(dbres[0][1]);
        const bool        validated_contact      = static_cast< bool        >(dbres[0][2]);
        const std::string message_service_handle = static_cast< std::string >(dbres[0][3]);

        const bool send_via_optys      =                     message_service_handle == "OPTYS";
        const bool send_via_postservis = !send_via_optys && (message_service_handle == "POSTSERVIS");
        if (!(send_via_optys || send_via_postservis)) {
            throw std::runtime_error("unexpected service handle: " + message_service_handle);
        }

        const Fred::Document::GenerationType doc_type = send_via_optys
                                                        ? Fred::Document::GT_CONTACT_IDENTIFICATION_LETTER_PIN3_OPTYS
                                                        : Fred::Document::GT_CONTACT_IDENTIFICATION_LETTER_PIN3;
        const bool use_historic_data = _contact_history_id.isset();
        const Fred::InfoContactData contact_data = use_historic_data
                                                   ? Fred::InfoContactHistoryByHistoryid(
                                                         _contact_history_id.get_value()).exec(_ctx).info_contact_data
                                                   : Fred::InfoContactById(
                                                         _locked_contact.get_object_id()).exec(_ctx).info_contact_data;
        namespace bptime = boost::posix_time;
        const bptime::ptime letter_time = bptime::time_from_string(public_request_time);
        _check_message_limits(_ctx, contact_data.id);

        const Generate::MessageId message_id = send_auth_owner_letter(
            _ctx,
            _multimanager.select< Fred::Messages::Manager >(),
            _multimanager.select< Fred::Document::Manager >(),
            doc_type,
            contact_data,
            pin3,
            validated_contact,
            _locked_request.get_public_request_id(),
            letter_time);
        return message_id;
    }
};

template < >
struct generate_message< CommChannel::LETTER, Fred::MojeID::PublicRequest::ContactReidentification >
{
    static Generate::MessageId for_given_request(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const Generate::message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id)
    {
        typedef Fred::Object::State FOS;
        const std::string state_validated_contact = FOS(FOS::VALIDATED_CONTACT).into< std::string >();
        Database::query_param_list params;
        params(_locked_request.get_public_request_id())
              (_locked_contact.get_object_id())
              (state_validated_contact);
        const Database::Result dbres = _ctx.get_conn().exec_params(
            "SELECT create_time,"
                   "(SELECT password FROM public_request_auth WHERE id=pr.id),"
                   "EXISTS(SELECT * FROM object_state os "
                          "WHERE os.object_id=$2::BIGINT AND "
                                "os.state_id=(SELECT id FROM enum_object_states WHERE name=$3::TEXT) AND "
                                "os.valid_from<=pr.create_time AND (pr.create_time<os.valid_to OR "
                                                                   "os.valid_to IS NULL)"
                         ") AS is_validated "
            "FROM public_request pr "
            "WHERE id=$1::BIGINT", params);
        if (dbres.size() <= 0) {
            throw std::runtime_error("no public request found");
        }

        const std::string public_request_time    = static_cast< std::string >(dbres[0][0]);
        const std::string pin3                   = static_cast< std::string >(dbres[0][1]);
        const bool        validated_contact      = static_cast< bool        >(dbres[0][2]);

        const Fred::Document::GenerationType doc_type = Fred::Document::GT_CONTACT_REIDENTIFICATION_LETTER_PIN3;
        const bool use_historic_data = _contact_history_id.isset();
        const Fred::InfoContactData contact_data = use_historic_data
                                                   ? Fred::InfoContactHistoryByHistoryid(
                                                         _contact_history_id.get_value()).exec(_ctx).info_contact_data
                                                   : Fred::InfoContactById(
                                                         _locked_contact.get_object_id()).exec(_ctx).info_contact_data;
        namespace bptime = boost::posix_time;
        const bptime::ptime letter_time = bptime::time_from_string(public_request_time);
        _check_message_limits(_ctx, contact_data.id);

        const Generate::MessageId message_id = send_auth_owner_letter(
            _ctx,
            _multimanager.select< Fred::Messages::Manager >(),
            _multimanager.select< Fred::Document::Manager >(),
            doc_type,
            contact_data,
            pin3,
            validated_contact,
            _locked_request.get_public_request_id(),
            letter_time);
        return message_id;
    }
};

class to_string
{
public:
    to_string(std::string &_dst):dst_(_dst) { }
    const to_string& concat(const std::string &_delimiter, const std::string &_untrimmed_data)const
    {
        const std::string trimmed_data = boost::algorithm::trim_copy(_untrimmed_data);
        if (!trimmed_data.empty()) {
            if (!dst_.empty()) {
                dst_.append(_delimiter);
            }
            dst_.append(trimmed_data);
        }
        return *this;
    }
    const to_string& concat(const std::string &_delimiter, const Optional< std::string > &_untrimmed_data)const
    {
        if (_untrimmed_data.isset()) {
            this->concat(_delimiter, _untrimmed_data.get_value());
        }
        return *this;
    }
private:
    std::string &dst_;
};

std::string collect_address(const Fred::Contact::PlaceAddress &_addr)
{
    std::string result;
    to_string(result).concat(", ", _addr.street1)
                     .concat(", ", _addr.street2)
                     .concat(", ", _addr.street3)
                     .concat(", ", _addr.postalcode)
                     .concat(", ", _addr.city)
                     .concat(", ", _addr.country);
    return result;
}

std::string collect_address(const Nullable< Fred::Contact::PlaceAddress > &_addr)
{
    if (_addr.isnull()) {
        return collect_address(_addr.get_value());
    }
    return std::string();
}

Generate::MessageId send_email(
    const std::string &_mail_template,
    Fred::OperationContext &_ctx,
    const Multimanager &_multimanager,
    const Fred::PublicRequestLockGuard &_locked_request,
    const Fred::PublicRequestObjectLockGuard &_locked_contact,
    const Generate::message_checker &_check_message_limits,
    const Optional< GeneralId > &_contact_history_id)
{
    Database::query_param_list params;
    params(_locked_request.get_public_request_id())
          (_locked_contact.get_object_id());
    const Database::Result dbres = _ctx.get_conn().exec_params(
        "SELECT pr.create_time,"
               "pra.identification,"
               "pra.password,"
               "(SELECT UPPER(name) FROM object_registry WHERE id=$2::BIGINT) "
        "FROM public_request pr "
        "JOIN public_request_auth pra ON pra.id=pr.id "
        "WHERE pr.id=$1::BIGINT", params);
    if (dbres.size() <= 0) {
        throw std::runtime_error("no public request found");
    }

    const std::string public_request_time = static_cast< std::string >(dbres[0][0]);
    const std::string identification      = static_cast< std::string >(dbres[0][1]);
    const std::string password            = static_cast< std::string >(dbres[0][2]);
    const std::string contact_handle      = static_cast< std::string >(dbres[0][3]);

    const bool use_historic_data = _contact_history_id.isset();
    const Fred::InfoContactData contact_data = use_historic_data
                                               ? Fred::InfoContactHistoryByHistoryid(
                                                     _contact_history_id.get_value()).exec(_ctx).info_contact_data
                                               : Fred::InfoContactById(
                                                     _locked_contact.get_object_id()).exec(_ctx).info_contact_data;
    const std::string pin1 = Fred::MojeID::PublicRequest::ContactConditionalIdentification::
                                 get_pin1_part(password);

    const std::string sender;//default sender from notification system
    const std::string recipient = contact_data.email.get_value_or_default();
    const std::string subject;//default subject is taken from template

    Fred::Mailer::Parameters mail_params;
    namespace bptime = boost::posix_time;
    const bptime::ptime email_time = bptime::time_from_string(public_request_time);
    const std::string contact_name = contact_data.name.get_value_or_default();
    const std::string::size_type name_delimiter_pos = contact_name.find_last_of(' ');
    const std::string firstname = name_delimiter_pos != std::string::npos
                                  ? contact_name.substr(                     0, name_delimiter_pos)
                                  : contact_name;
    const std::string lastname  = name_delimiter_pos != std::string::npos
                                  ? contact_name.substr(name_delimiter_pos + 1, std::string::npos)
                                  : std::string();

    mail_params["reqdate"]        = boost::gregorian::to_iso_extended_string(email_time.date());
    mail_params["reqid"]          = boost::lexical_cast< std::string >(_locked_request.get_public_request_id());
    mail_params["type"]           = boost::lexical_cast< std::string >(Fred::FT_CONTACT);
    mail_params["handle"]         = contact_handle;
    mail_params["name"]           = contact_data.name.get_value_or_default();
    mail_params["org"]            = contact_data.organization.get_value_or_default();
    mail_params["ic"]             = contact_data.ssntype.get_value_or_default() == "ICO"
                                    ? contact_data.ssn.get_value_or_default()
                                    : std::string();
    mail_params["birthdate"]      = contact_data.ssntype.get_value_or_default() == "BIRTHDAY"
                                    ? boost::gregorian::to_iso_extended_string(
                                          birthdate_from_string_to_date(contact_data.ssn.get_value_or_default()))
                                    : std::string();
    mail_params["address"]        = collect_address(contact_data.place);
    mail_params["status"]         = "2";//public_request.status == "answered" ? "1" : "2"
    mail_params["hostname"]       = CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >()->hostname;
    mail_params["firstname"]      = firstname;
    mail_params["lastname"]       = lastname;
    mail_params["email"]          = recipient;
    mail_params["identification"] = identification;
    mail_params["passwd"]         = pin1;

    Fred::Mailer::Handles handles;
    handles.push_back(contact_handle);
    const GeneralId message_id = _multimanager.select< Fred::Mailer::Manager >().sendEmail(
        sender,                          
        recipient,
        subject,
        _mail_template,
        mail_params,
        handles,
        Fred::Mailer::Attachments());
    return message_id;
}

template < >
struct generate_message< CommChannel::EMAIL, Fred::MojeID::PublicRequest::ContactConditionalIdentification >
{
    static Generate::MessageId for_given_request(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const Generate::message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id)
    {
        //db table mail_type: 21,'mojeid_identification','[mojeID] Založení účtu - PIN1 pro aktivaci mojeID'
        static const std::string mail_template = "mojeid_identification";
        return send_email(mail_template,
                         _ctx,
                         _multimanager,
                         _locked_request,
                         _locked_contact,
                         _check_message_limits,
                         _contact_history_id);
    }
};

template < >
struct generate_message< CommChannel::EMAIL, Fred::MojeID::PublicRequest::ConditionallyIdentifiedContactTransfer >
{
    static Generate::MessageId for_given_request(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const Generate::message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id)
    {
        //db table mail_type: 27,'mojeid_verified_contact_transfer','Založení účtu mojeID'
        static const std::string mail_template = "mojeid_verified_contact_transfer";
        return send_email(mail_template,
                         _ctx,
                         _multimanager,
                         _locked_request,
                         _locked_contact,
                         _check_message_limits,
                         _contact_history_id);
    }
};

template < >
struct generate_message< CommChannel::EMAIL, Fred::MojeID::PublicRequest::IdentifiedContactTransfer >
{
    static Generate::MessageId for_given_request(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const Generate::message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id)
    {
        //db table mail_type: 27,'mojeid_verified_contact_transfer','Založení účtu mojeID'
        static const std::string mail_template = "mojeid_verified_contact_transfer";
        return send_email(mail_template,
                         _ctx,
                         _multimanager,
                         _locked_request,
                         _locked_contact,
                         _check_message_limits,
                         _contact_history_id);
    }
};

}//namespace MojeID::Messages::{anonymous}

template < bool X >
struct Multimanager::traits< Fred::Document::Manager, X >
{
    static Fred::Document::Manager* get(const Multimanager *mm_ptr) { return mm_ptr->document(); }
};

template < bool X >
struct Multimanager::traits< Fred::Mailer::Manager, X >
{
    static Fred::Mailer::Manager* get(const Multimanager *mm_ptr) { return mm_ptr->mailer(); }
};

template < bool X >
struct Multimanager::traits< Fred::Messages::Manager, X >
{
    static Fred::Messages::Manager* get(const Multimanager *mm_ptr) { return mm_ptr->messages(); }
};

template < typename MANAGER >
MANAGER& Multimanager::select()const
{
    return *traits< MANAGER, false >::get(this);
}

Fred::Document::Manager* DefaultMultimanager::document()const
{
    static const HandleRegistryArgs *const rconf =
        CfgArgs::instance()->get_handler_ptr_by_type< HandleRegistryArgs >();
    typedef std::auto_ptr< Fred::Document::Manager > ManagerPtr;
    static const ManagerPtr manager_ptr =
        Fred::Document::Manager::create(
            rconf->docgen_path,
            rconf->docgen_template_path,
            rconf->fileclient_path,
            CfgArgs::instance()->get_handler_ptr_by_type< HandleCorbaNameServiceArgs >()
                ->get_nameservice_host_port());
    return manager_ptr.get();
}

Fred::Mailer::Manager* DefaultMultimanager::mailer()const
{
    typedef std::auto_ptr< Fred::Mailer::Manager > ManagerPtr;
    static const ManagerPtr manager_ptr(new MailerManager(CorbaContainer::get_instance()->getNS()));
    return manager_ptr.get();
}

Fred::Messages::Manager* DefaultMultimanager::messages()const
{
    static Fred::Messages::ManagerPtr const manager_ptr = Fred::Messages::create_manager();
    return manager_ptr.get();
}

template < CommChannel::Value COMM_CHANNEL >
void Generate::Into< COMM_CHANNEL >::for_new_requests(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const message_checker &_check_message_limits)
{
    static Database::query_param_list params;
    static const std::string sql = CollectFor< COMM_CHANNEL >::query(params);
    const Database::Result dbres = _ctx.get_conn().exec_params(sql, params);
    for (::size_t idx = 0; idx < dbres.size(); ++idx) {
        try {
            const PublicRequestLocked       locked_request       (static_cast< GeneralId   >(dbres[idx][0]));
            const std::string               public_request_type = static_cast< std::string >(dbres[idx][1]);
            const PublicRequestObjectLocked locked_contact       (static_cast< GeneralId   >(dbres[idx][2]));
            const GeneralId                 contact_history_id  = static_cast< GeneralId   >(dbres[idx][3]);
            Fred::OperationContextCreator ctx;
            const MessageId message_id = PossibleRequestTypes< COMM_CHANNEL >::generate_message(
                ctx,
                _multimanager,
                public_request_type,
                locked_request,
                locked_contact,
                _check_message_limits,
                contact_history_id);
            JoinMessage< COMM_CHANNEL >::with_public_request(ctx, locked_request, message_id);
            ctx.commit_transaction();
        }
        catch (const std::exception &e) {
            _ctx.get_log().error(e.what());
        }
        catch (...) {
            _ctx.get_log().error("unexpected error");
        }
    }
}

template void Generate::Into< CommChannel::SMS    >::for_new_requests(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const message_checker &_check_message_limits);
template void Generate::Into< CommChannel::EMAIL  >::for_new_requests(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const message_checker &_check_message_limits);
template void Generate::Into< CommChannel::LETTER >::for_new_requests(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const message_checker &_check_message_limits);

template < CommChannel::Value COMM_CHANNEL >
void Generate::enable(Fred::OperationContext &_ctx, bool flag)
{
    Database::query_param_list params;
    const std::string sql = "UPDATE enum_parameters "
                            "SET val=$" + params.add(flag ? "enabled" : "disabled") + "::TEXT "
                            "WHERE name=" + Parameter< COMM_CHANNEL >::name(params);
    _ctx.get_conn().exec_params(sql, params);
}

template void Generate::enable< CommChannel::SMS    >(Fred::OperationContext &_ctx, bool flag);
template void Generate::enable< CommChannel::EMAIL  >(Fred::OperationContext &_ctx, bool flag);
template void Generate::enable< CommChannel::LETTER >(Fred::OperationContext &_ctx, bool flag);

template < CommChannel::Value COMM_CHANNEL >
template < typename PUBLIC_REQUEST_TYPE >
Generate::MessageId Generate::Into< COMM_CHANNEL >::for_given_request(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id)
{
    return generate_message< COMM_CHANNEL, PUBLIC_REQUEST_TYPE >::for_given_request(
        _ctx,
        _multimanager,
        _locked_request,
        _locked_contact,
        _check_message_limits,
        _contact_history_id);
}

template Generate::MessageId Generate::Into< CommChannel::SMS >::
                             for_given_request< Fred::MojeID::PublicRequest::ContactConditionalIdentification >(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id);

template Generate::MessageId Generate::Into< CommChannel::LETTER >::
                             for_given_request< Fred::MojeID::PublicRequest::ContactIdentification >(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id);

template Generate::MessageId Generate::Into< CommChannel::LETTER >::
                             for_given_request< Fred::MojeID::PublicRequest::ContactReidentification >(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id);

template Generate::MessageId Generate::Into< CommChannel::EMAIL >::
                             for_given_request< Fred::MojeID::PublicRequest::ContactConditionalIdentification >(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id);

template Generate::MessageId Generate::Into< CommChannel::EMAIL >::
                             for_given_request< Fred::MojeID::PublicRequest::ConditionallyIdentifiedContactTransfer >(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id);

template Generate::MessageId Generate::Into< CommChannel::EMAIL >::
                             for_given_request< Fred::MojeID::PublicRequest::IdentifiedContactTransfer >(
        Fred::OperationContext &_ctx,
        const Multimanager &_multimanager,
        const Fred::PublicRequestLockGuard &_locked_request,
        const Fred::PublicRequestObjectLockGuard &_locked_contact,
        const message_checker &_check_message_limits,
        const Optional< GeneralId > &_contact_history_id);

}//namespace MojeID::Messages
}//namespace MojeID
