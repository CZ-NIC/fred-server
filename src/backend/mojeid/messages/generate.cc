/*
 * Copyright (C) 2015-2022  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/backend/mojeid/messages/generate.hh"
#include "src/backend/mojeid/mojeid.hh"
#include "src/backend/mojeid/mojeid_public_request.hh"
#include "src/deprecated/libfred/common_object.hh"
#include "libfred/object/object_state.hh"
#include "libfred/public_request/public_request_status.hh"
#include "libfred/public_request/update_public_request.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_mojeid_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "util/db/query_param.hh"
#include "src/util/types/birthdate.hh"
#include "src/util/xmlgen.hh"

#include "libhermes/libhermes.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <chrono>
#include <utility>

namespace Fred {
namespace Backend {
namespace MojeId {
namespace Messages {

namespace {

struct DbCommand
{
    std::string query;
    Database::query_param_list params;

};

class PublicRequestLockedForUpdate
    : public LibFred::LockedPublicRequestForUpdate
{
public:
    PublicRequestLockedForUpdate(LibFred::OperationContext& _ctx, LibFred::PublicRequestId _locked_request_id)
        : ctx_(_ctx),
          public_request_id_(_locked_request_id)
    {
    }

    virtual ~PublicRequestLockedForUpdate()
    {
    }

private:
    virtual LibFred::PublicRequestId get_id() const
    {
        return public_request_id_;
    }

    virtual LibFred::OperationContext& get_ctx() const
    {
        return ctx_;
    }

    LibFred::OperationContext& ctx_;
    const LibFred::PublicRequestId public_request_id_;
};

class PublicRequestObjectLocked
    : public LibFred::LockedPublicRequestsOfObject
{
public:

    explicit PublicRequestObjectLocked(LibFred::ObjectId _locked_object_id)
        : object_id_(_locked_object_id)
    {
    }

    virtual ~PublicRequestObjectLocked()
    {
    }

private:
    virtual LibFred::ObjectId get_id() const
    {
        return object_id_;
    }

    const LibFred::ObjectId object_id_;
};

boost::uuids::uuid get_public_request_uuid(
        LibFred::OperationContext& _ctx,
        unsigned long long _public_request_id)
{
        const Database::Result dbres = _ctx.get_conn().exec_params(
                // clang-format off
                "SELECT uuid "
                  "FROM public_request "
                 "WHERE id = $1::BIGINT",
                // clang-format on
                Database::query_param_list(_public_request_id));
        if (dbres.size() <= 0)
        {
            throw std::runtime_error("no public request found");
        }
        if (dbres.size() > 1)
        {
            throw std::runtime_error("unexpected number of rows");
        }
        auto uuid = boost::uuids::string_generator{}(static_cast<std::string>(dbres[0][0]));
        return uuid;
}

template <CommChannel::Enum COMM_CHANNEL, typename PUBLIC_REQUEST_TYPE>
struct generate_message;

template <>
struct generate_message<CommChannel::sms,
                        Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification>
{
    static void for_given_request(
            LibFred::OperationContext& _ctx,
            const MojeId::MessengerConfiguration& _messenger_configuration,
            const LibFred::LockedPublicRequest& _locked_request,
            const LibFred::LockedPublicRequestsOfObject& _locked_contact,
            const std::string& _link_hostname_part [[gnu::unused]],
            const Optional<GeneralId>& _contact_history_id)
    {
        const Database::Result dbres = _ctx.get_conn().exec_params(
                // clang-format off
                "SELECT (SELECT password FROM public_request_auth WHERE id=$1::BIGINT),"
                       "ch.telephone,LOWER(obr.name),obr.uuid "
                "FROM contact_history ch "
                "JOIN object_registry obr ON obr.id=ch.id "
                "WHERE ch.id=$2::BIGINT AND ch.historyid=$3::BIGINT",
                // clang-format on
                Database::query_param_list(_locked_request.get_id())(_locked_contact.get_id())(
                        _contact_history_id.get_value()));
        const std::string password       = static_cast<std::string>(dbres[0][0]);
        const std::string contact_phone  = static_cast<std::string>(dbres[0][1]);
        const std::string contact_handle = static_cast<std::string>(dbres[0][2]);
        const auto contact_uuid = boost::uuids::string_generator{}(static_cast<std::string>(dbres[0][3]));
        const auto public_request_uuid = get_public_request_uuid(_ctx, _locked_request.get_id());

        const std::string pin2 = Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification::
                                 get_pin2_part(password);

        LibHermes::Struct template_parameters{{LibHermes::StructKey{"pin"}, LibHermes::StructValue{pin2}}};

        auto message_data =
                LibHermes::Sms::make_minimal_message(
                        LibHermes::Sms::RecipientPhoneNumber(contact_phone),
                        LibHermes::Sms::BodyTemplate{std::string{"mojeid-pin2-cs.txt"}});

        message_data.recipient_uuids = std::vector<LibHermes::Sms::RecipientUuid>{LibHermes::Sms::RecipientUuid{contact_uuid}};
        message_data.type = LibHermes::Sms::Type{std::string{"mojeid_pin2"}};
        message_data.context = template_parameters;

        LibHermes::Connection<LibHermes::Service::SmsMessenger> connection{
                LibHermes::Connection<LibHermes::Service::SmsMessenger>::ConnectionString{
                        _messenger_configuration.endpoint}};
        connection.set_timeout(std::chrono::seconds{10});

        try
        {
            LibHermes::Sms::send(
                    connection,
                    message_data,
                    LibHermes::Sms::Archive{_messenger_configuration.archive},
                    {LibHermes::Reference{
                            LibHermes::Reference::Type{"public-request"},
                            LibHermes::Reference::Value{boost::uuids::to_string(public_request_uuid)}}});
        }
        catch (const LibHermes::Sms::SendFailed& e)
        {
            _ctx.get_log().warning(boost::str(boost::format("gRPC exception caught while sending sms about public request with uuid %1%: gRPC error code: %2%, error message: %3%, grpc_message_json: %4%") % boost::uuids::to_string(public_request_uuid) % e.error_code() % e.error_message() % e.grpc_message_json()));
            throw;
        }
        catch (const std::exception& e)
        {
            _ctx.get_log().warning(boost::str(boost::format("std::exception caught while sending sms about public request with uuid %1%: %2%") % boost::uuids::to_string(public_request_uuid) % e.what()));
            throw;
        }
        catch (...)
        {
            _ctx.get_log().warning(boost::str(boost::format("exception caught while sending sms about public request with uuid %1%") % boost::uuids::to_string(public_request_uuid)));
            throw;
        }
    }

};

template <>
struct generate_message<CommChannel::sms,
                        Fred::Backend::MojeId::PublicRequest::PrevalidatedUnidentifiedContactTransfer>
{
    static void for_given_request(
            LibFred::OperationContext& _ctx,
            const MojeId::MessengerConfiguration& _messenger_configuration,
            const LibFred::LockedPublicRequest& _locked_request,
            const LibFred::LockedPublicRequestsOfObject& _locked_contact,
            const std::string& _link_hostname_part,
            const Optional<GeneralId>& _contact_history_id)
    {
        generate_message<CommChannel::sms,
                Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification>::
                for_given_request(
                        _ctx,
                        _messenger_configuration,
                        _locked_request,
                        _locked_contact,
                        _link_hostname_part,
                        _contact_history_id);
    }

};

void send_auth_owner_letter(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const LibFred::InfoContactData& _data,
        const std::string& _pin3,
        bool _validated_contact,
        LibFred::PublicRequestId _public_request_id,
        const Optional<boost::posix_time::ptime>& _letter_time)
{
    // std::string letter_xml("<?xml version='1.0' encoding='utf-8'?>");

    const std::string name = _data.name.get_value_or_default();
    const std::string::size_type name_delimiter_pos = name.find_last_of(' ');
    const std::string firstname = name.substr(
                                          0,
                                          name_delimiter_pos);
    const std::string lastname  = name_delimiter_pos != std::string::npos
                                  ? name.substr(
                                          name_delimiter_pos + 1,
                                          std::string::npos)
                                  : std::string{};

    const LibFred::InfoContactData::Address addr = _data.get_address<LibFred::ContactAddressType::MAILING>();

    Database::query_param_list params(addr.country);
    const std::string sql =
            // clang-format off
            "SELECT (SELECT country_cs FROM enum_country WHERE id=$1::TEXT OR country=$1::TEXT),"
                   "(SELECT country FROM enum_country WHERE id=$1::TEXT)";
            // clang-format on
    const Database::Result dbres = _ctx.get_conn().exec_params(sql, params);
    const boost::optional<std::string> country_name_cs = !dbres[0][0].isnull() ? boost::optional<std::string>{static_cast<std::string>(dbres[0][0])} : boost::none;
    const std::string country_name_fallback_code = !dbres[0][1].isnull() ? static_cast<std::string>(dbres[0][1]) : addr.country;

    std::vector<LibHermes::Letter::RecipientAddress::Street> recipient_address_street_fields{LibHermes::Letter::RecipientAddress::Street{addr.street1}};
    if (addr.street2.isset())
    {
        recipient_address_street_fields.push_back(LibHermes::Letter::RecipientAddress::Street{addr.street2.get_value()});
    }
    if (addr.street3.isset())
    {
        recipient_address_street_fields.push_back(LibHermes::Letter::RecipientAddress::Street{addr.street3.get_value()});
    }
    const LibHermes::Letter::RecipientAddress recipient_address{
            LibHermes::Letter::RecipientAddress::Name{name},
            LibHermes::Letter::RecipientAddress::Organization{_data.organization.get_value_or_default()},
            recipient_address_street_fields,
            LibHermes::Letter::RecipientAddress::City{addr.city},
            LibHermes::Letter::RecipientAddress::StateOrProvince{addr.stateorprovince.get_value_or_default()},
            LibHermes::Letter::RecipientAddress::PostalCode{addr.postalcode},
            LibHermes::Letter::RecipientAddress::CountryName{country_name_cs != boost::none ? *country_name_cs : addr.country}};


    const std::string contact_handle = _data.handle;
    const boost::gregorian::date letter_date = _letter_time.isset()
                                               ? _letter_time.get_value().date()
                                               : boost::gregorian::day_clock::local_day();
    const std::string contact_state = _validated_contact ? "validated" : "";
    const auto contact_uuid = get_raw_value_from(_data.uuid);
    const auto public_request_uuid = get_public_request_uuid(_ctx, _public_request_id);

    LibHermes::Struct template_parameters{
            {LibHermes::StructKey{"date"}, LibHermes::StructValue{boost::gregorian::to_iso_extended_string(letter_date)}},
            {LibHermes::StructKey{"address"},
                    LibHermes::StructValue{LibHermes::Struct{
                            {LibHermes::StructKey{"name"}, LibHermes::StructValue{*recipient_address.name}},
                            {LibHermes::StructKey{"organization"}, LibHermes::StructValue{*recipient_address.organization}},
                            {LibHermes::StructKey{"streets"}, {LibHermes::StructValue{*(recipient_address.street_field[0])}}},
                            {LibHermes::StructKey{"postal_code"}, LibHermes::StructValue{*recipient_address.postal_code}},
                            {LibHermes::StructKey{"city"}, LibHermes::StructValue{*recipient_address.city}},
                            {LibHermes::StructKey{"country_code"}, LibHermes::StructValue{country_name_fallback_code}},
                            {LibHermes::StructKey{"state_or_province"}, LibHermes::StructValue{*recipient_address.state_or_province}}}}},
            {LibHermes::StructKey{"username"}, LibHermes::StructValue{boost::algorithm::to_lower_copy(contact_handle)}},
            {LibHermes::StructKey{"pin"}, LibHermes::StructValue{_pin3}}};

    auto message_data =
            LibHermes::Letter::make_minimal_message(
                    recipient_address,
                    LibHermes::Letter::BodyTemplate{"mojeid-pin3.html"});

    message_data.recipient_uuids = std::vector<LibHermes::Letter::RecipientUuid>{LibHermes::Letter::RecipientUuid(contact_uuid)};
    message_data.type = LibHermes::Letter::Type{std::string{"mojeid_pin3"}};
    message_data.context = template_parameters;

    LibHermes::Connection<LibHermes::Service::LetterMessenger> connection{
            LibHermes::Connection<LibHermes::Service::LetterMessenger>::ConnectionString{
                    _messenger_configuration.endpoint}};
    connection.set_timeout(std::chrono::seconds{10});

    try
    {
        LibHermes::Letter::send(
                connection,
                message_data,
                LibHermes::Letter::Archive{_messenger_configuration.archive},
                LibHermes::Letter::ArchiveRendered{_messenger_configuration.archive_rendered},
                {LibHermes::Reference{
                        LibHermes::Reference::Type{"public-request"},
                        LibHermes::Reference::Value{boost::uuids::to_string(public_request_uuid)}}});
    }
    catch (const LibHermes::Letter::SendFailed& e)
    {
        _ctx.get_log().warning(boost::str(boost::format("gRPC exception caught while sending letter about public request with uuid %1%: gRPC error code: %2%, error message: %3%, grpc_message_json: %4%") % boost::uuids::to_string(public_request_uuid) % e.error_code() % e.error_message() % e.grpc_message_json()));
        throw;
    }
    catch (const std::exception& e)
    {
        _ctx.get_log().warning(boost::str(boost::format("std::exception caught while sending letter about public request with uuid %1%: %2%") % boost::uuids::to_string(public_request_uuid) % e.what()));
        throw;
    }
    catch (...)
    {
        _ctx.get_log().warning(boost::str(boost::format("exception caught while sending letter about public_request with uuid %1%") % boost::uuids::to_string(public_request_uuid)));
        throw;
    }
}


template <>
struct generate_message<CommChannel::letter, Fred::Backend::MojeId::PublicRequest::ContactIdentification>
{
    static void for_given_request(
            LibFred::OperationContext& _ctx,
            const MojeId::MessengerConfiguration& _messenger_configuration,
            const LibFred::LockedPublicRequest& _locked_request,
            const LibFred::LockedPublicRequestsOfObject& _locked_contact,
            const std::string& _link_hostname_part [[gnu::unused]],
            const Optional<GeneralId>& _contact_history_id)
    {

        typedef LibFred::Object_State FOS;

        const std::string state_validated_contact = Conversion::Enums::to_db_handle(FOS::validated_contact);
        const std::string message_type_mojeid_pin3 = "mojeid_pin3";
        Database::query_param_list params;
        params(_locked_request.get_id())(_locked_contact.get_id())(state_validated_contact)(
                message_type_mojeid_pin3);
        const Database::Result dbres = _ctx.get_conn().exec_params(
                // clang-format off
                "SELECT create_time,"
                       "(SELECT password FROM public_request_auth WHERE id=pr.id),"
                       "EXISTS(SELECT * FROM object_state os "
                              "WHERE os.object_id=$2::BIGINT AND "
                                    "os.state_id=(SELECT id FROM enum_object_states WHERE name=$3::TEXT) AND "
                                    "os.valid_from<=pr.create_time AND (pr.create_time<os.valid_to OR "
                                                                       "os.valid_to IS NULL)"
                             ") AS is_validated, "
                       "(SELECT mtfsm.service_handle "
                        "FROM message_type mt "
                        "JOIN message_type_forwarding_service_map mtfsm ON mtfsm.message_type_id=mt.id "
                        "WHERE mt.type=$4::TEXT) AS message_service_handle "
                "FROM public_request pr "
                "WHERE id=$1::BIGINT",
                // clang-format on
                params);
        if (dbres.size() <= 0)
        {
            throw std::runtime_error("no public request found");
        }
        if (dbres[0][3].isnull())
        {
            throw std::runtime_error(
                    "no message service handle asociated with '" +
                    message_type_mojeid_pin3 + "' message type");
        }

        const std::string public_request_time = static_cast<std::string>(dbres[0][0]);
        const std::string pin3 = static_cast<std::string>(dbres[0][1]);
        const bool validated_contact = static_cast<bool>(dbres[0][2]);
        const std::string message_service_handle = static_cast<std::string>(dbres[0][3]);

        const bool send_via_optys = message_service_handle == "OPTYS";
        const bool send_via_postservis = !send_via_optys && (message_service_handle == "POSTSERVIS");
        if (!(send_via_optys || send_via_postservis))
        {
            throw std::runtime_error("unexpected service handle: " + message_service_handle);
        }

        const bool use_historic_data = _contact_history_id.isset();
        const LibFred::InfoContactData contact_data = use_historic_data
                                                      ? LibFred::InfoContactHistoryByHistoryid(
                _contact_history_id.get_value()).exec(_ctx).info_contact_data
                                                      : LibFred::InfoContactById(_locked_contact.get_id()).
                                                      exec(_ctx).info_contact_data;
        namespace bptime = boost::posix_time;
        const bptime::ptime letter_time = bptime::time_from_string(public_request_time);

        send_auth_owner_letter(
                _ctx,
                _messenger_configuration,
                contact_data,
                pin3,
                validated_contact,
                _locked_request.get_id(),
                letter_time);
    }

};

template <>
struct generate_message<CommChannel::letter, Fred::Backend::MojeId::PublicRequest::ContactReidentification>
{
    static void for_given_request(
            LibFred::OperationContext& _ctx,
            const MojeId::MessengerConfiguration& _messenger_configuration,
            const LibFred::LockedPublicRequest& _locked_request,
            const LibFred::LockedPublicRequestsOfObject& _locked_contact,
            const std::string& _link_hostname_part [[gnu::unused]],
            const Optional<GeneralId>& _contact_history_id)
    {

        typedef LibFred::Object_State FOS;

        const std::string state_validated_contact = Conversion::Enums::to_db_handle(FOS::validated_contact);
        Database::query_param_list params;
        params(_locked_request.get_id())(_locked_contact.get_id())(state_validated_contact);
        const Database::Result dbres = _ctx.get_conn().exec_params(
                // clang-format off
                "SELECT create_time,"
                       "(SELECT password FROM public_request_auth WHERE id=pr.id),"
                       "EXISTS(SELECT * FROM object_state os "
                              "WHERE os.object_id=$2::BIGINT AND "
                                    "os.state_id=(SELECT id FROM enum_object_states WHERE name=$3::TEXT) AND "
                                    "os.valid_from<=pr.create_time AND (pr.create_time<os.valid_to OR "
                                                                       "os.valid_to IS NULL)"
                             ") AS is_validated "
                "FROM public_request pr "
                "WHERE id=$1::BIGINT",
                // clang-format on
                params);
        if (dbres.size() <= 0)
        {
            throw std::runtime_error("no public request found");
        }

        const std::string public_request_time = static_cast<std::string>(dbres[0][0]);
        const std::string pin3 = static_cast<std::string>(dbres[0][1]);
        const bool validated_contact = static_cast<bool>(dbres[0][2]);

        const bool use_historic_data = _contact_history_id.isset();
        const LibFred::InfoContactData contact_data =
                use_historic_data
                        ? LibFred::InfoContactHistoryByHistoryid(
                                  _contact_history_id.get_value()).exec(_ctx).info_contact_data
                        : LibFred::InfoContactById(_locked_contact.get_id()).exec(_ctx).info_contact_data;
        namespace bptime = boost::posix_time;
        const bptime::ptime letter_time = bptime::time_from_string(public_request_time);

        send_auth_owner_letter(
                _ctx,
                _messenger_configuration,
                contact_data,
                pin3,
                validated_contact,
                _locked_request.get_id(),
                letter_time);
    }

};

class to_string
{
public:

    explicit to_string(std::string& _dst)
        : dst_(_dst)
    {
    }

    const to_string& concat(
            const std::string& _delimiter,
            const std::string& _untrimmed_data) const
    {
        const std::string trimmed_data = boost::algorithm::trim_copy(_untrimmed_data);
        if (!trimmed_data.empty())
        {
            if (!dst_.empty())
            {
                dst_.append(_delimiter);
            }
            dst_.append(trimmed_data);
        }
        return *this;
    }

    const to_string& concat(
            const std::string& _delimiter,
            const Optional<std::string>& _untrimmed_data) const
    {
        if (_untrimmed_data.isset())
        {
            this->concat(
                    _delimiter,
                    _untrimmed_data.get_value());
        }
        return *this;
    }

private:
    std::string& dst_;
};

std::string collect_address(const LibFred::Contact::PlaceAddress& _addr)
{
    std::string result;
    to_string(result)
            .concat(", ", _addr.street1)
            .concat(", ", _addr.street2)
            .concat(", ", _addr.street3)
            .concat(", ", _addr.postalcode)
            .concat(", ", _addr.city)
            .concat(", ", _addr.country);
    return result;
}


std::string collect_address(const Nullable<LibFred::Contact::PlaceAddress>& _addr)
{
    if (_addr.isnull())
    {
        return collect_address(_addr.get_value());
    }
    return std::string();
}

LibHermes::Email::SubjectTemplate get_libhermes_email_subject_template(const std::string& _mail_type)
{
    if (_mail_type == "mojeid_identification")
    {
        return LibHermes::Email::SubjectTemplate{"mojeid-identification-subject.txt"};
    }
    if (_mail_type == "mojeid_verified_contact_transfer")
    {
        return LibHermes::Email::SubjectTemplate{"mojeid-verified-contact-transfer-subject.txt"};
    }
    throw std::runtime_error{"unexpected _mail_type"};
}

LibHermes::Email::BodyTemplate get_libhermes_email_body_template(const std::string& _mail_type)
{
    if (_mail_type == "mojeid_identification")
    {
        return LibHermes::Email::BodyTemplate{"mojeid-identification-body.txt"};
    }
    if (_mail_type == "mojeid_verified_contact_transfer")
    {
        return LibHermes::Email::BodyTemplate{"mojeid-verified-contact-transfer-body.txt"};
    }
    throw std::runtime_error{"unexpected _mail_type"};
}

void send_email(
        const std::string& _mail_type,
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const LibFred::LockedPublicRequest& _locked_request,
        const LibFred::LockedPublicRequestsOfObject& _locked_contact,
        const std::string& _link_hostname_part,
        const Optional<GeneralId>& _contact_history_id)
{
    const Database::Result dbres =
            _ctx.get_conn().exec_params(
                    // clang-format off
                    "SELECT pr.create_time,pra.identification,pra.password,eprt.name,UPPER(obr.name),pr.uuid,obr.uuid "
                    "FROM public_request pr "
                    "JOIN public_request_auth pra ON pra.id=pr.id "
                    "JOIN enum_public_request_type eprt ON eprt.id=pr.request_type,"
                         "object_registry obr "
                    "WHERE pr.id=$1::BIGINT AND "
                          "obr.id=$2::BIGINT",
                    // clang-format on
                    Database::query_param_list(_locked_request.get_id())(_locked_contact.get_id()));
    if (dbres.size() <= 0)
    {
        throw std::runtime_error("no public request found");
    }

    const std::string public_request_time = static_cast<std::string>(dbres[0][0]);
    const std::string identification = static_cast<std::string>(dbres[0][1]);
    const std::string password = static_cast<std::string>(dbres[0][2]);
    const std::string pub_req_type = static_cast<std::string>(dbres[0][3]);
    const std::string contact_handle = static_cast<std::string>(dbres[0][4]);
    const auto public_request_uuid = boost::uuids::string_generator{}(static_cast<std::string>(dbres[0][5]));
    const auto contact_uuid = boost::uuids::string_generator{}(static_cast<std::string>(dbres[0][6]));

    const bool use_historic_data = _contact_history_id.isset();
    const LibFred::InfoContactData contact_data = use_historic_data
                                                  ? LibFred::InfoContactHistoryByHistoryid(
            _contact_history_id.get_value()).exec(_ctx).info_contact_data
                                                  : LibFred::InfoContactById(_locked_contact.get_id()).exec(
            _ctx).info_contact_data;
    // mojeid_contact_conditional_identification and mojeid_prevalidated_unidentified_contact_transfer public
    // requests password is composed of PIN1 and PIN2, other public requests password contains only PIN1
    const std::string pin1 =
        (pub_req_type ==
         Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification()
         .iface().get_public_request_type())
        ||
        (pub_req_type ==
         Fred::Backend::MojeId::PublicRequest::PrevalidatedUnidentifiedContactTransfer()
         .iface().get_public_request_type())
                ? Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification::get_pin1_part(password) // password = PIN1 & PIN2
                : password; // password = PIN1

    const std::string recipient = contact_data.email.get_value_or_default();

    namespace bptime = boost::posix_time;
    const bptime::ptime email_time = bptime::time_from_string(public_request_time);
    const std::string contact_name = contact_data.name.get_value_or_default();
    const std::string::size_type name_delimiter_pos = contact_name.find_last_of(' ');
    const std::string firstname =
            name_delimiter_pos != std::string::npos
                    ? contact_name.substr(0, name_delimiter_pos)
                    : contact_name;
    const std::string lastname =
            name_delimiter_pos != std::string::npos
                    ? contact_name.substr(name_delimiter_pos + 1, std::string::npos)
                    : std::string{};
    const auto ic =
            contact_data.ssntype.get_value_or_default() == "ICO"
                    ? contact_data.ssn.get_value_or_default()
                    : std::string{};
    const auto birthdate =
            contact_data.ssntype.get_value_or_default() == "BIRTHDAY"
                    ? boost::gregorian::to_iso_extended_string(
                              birthdate_from_string_to_date(contact_data.ssn.get_value_or_default()))
                    : std::string{};
    const std::string public_request_status{"2"};

    LibHermes::Struct template_parameters{
            {LibHermes::StructKey{"reqdate"}, LibHermes::StructValue{boost::gregorian::to_iso_extended_string(email_time.date())}},
            {LibHermes::StructKey{"reqid"}, LibHermes::StructValue{boost::lexical_cast<std::string>(_locked_request.get_id())}},
            {LibHermes::StructKey{"type"}, LibHermes::StructValue{boost::lexical_cast<std::string>(LibFred::FT_CONTACT)}},
            {LibHermes::StructKey{"handle"}, LibHermes::StructValue{contact_handle}},
            {LibHermes::StructKey{"name"}, LibHermes::StructValue{contact_data.name.get_value_or_default()}},
            {LibHermes::StructKey{"org"}, LibHermes::StructValue{contact_data.organization.get_value_or_default()}},
            {LibHermes::StructKey{"ic"}, LibHermes::StructValue{ic}},
            {LibHermes::StructKey{"birthdate"}, LibHermes::StructValue{birthdate}},
            {LibHermes::StructKey{"address"}, LibHermes::StructValue{collect_address(contact_data.place)}},
            {LibHermes::StructKey{"status"}, LibHermes::StructValue{public_request_status}},
            {LibHermes::StructKey{"hostname"}, LibHermes::StructValue{_link_hostname_part}},
            {LibHermes::StructKey{"firstname"}, LibHermes::StructValue{firstname}},
            {LibHermes::StructKey{"lastname"}, LibHermes::StructValue{lastname}},
            {LibHermes::StructKey{"email"}, LibHermes::StructValue{recipient}},
            {LibHermes::StructKey{"identification"}, LibHermes::StructValue{identification}},
            {LibHermes::StructKey{"passwd"}, LibHermes::StructValue{pin1}}};

    auto message_data =
            LibHermes::Email::make_minimal_email(
                    {{LibHermes::Email::RecipientEmail{recipient}, {LibHermes::Email::RecipientUuid{contact_uuid}}}},
                    get_libhermes_email_subject_template(_mail_type),
                    get_libhermes_email_body_template(_mail_type));
    message_data.type = LibHermes::Email::Type{_mail_type};
    message_data.context = template_parameters;

    LibHermes::Connection<LibHermes::Service::EmailMessenger> connection{
            LibHermes::Connection<LibHermes::Service::EmailMessenger>::ConnectionString{
                    _messenger_configuration.endpoint}};
    connection.set_timeout(std::chrono::seconds{10});

    try
    {
        LibHermes::Email::send(
                connection,
                message_data,
                LibHermes::Email::Archive{_messenger_configuration.archive},
                {LibHermes::Reference{
                        LibHermes::Reference::Type{"public-request"},
                        LibHermes::Reference::Value{boost::uuids::to_string(public_request_uuid)}}});
    }
    catch (const LibHermes::Email::SendFailed& e)
    {
        _ctx.get_log().warning(boost::str(boost::format("gRPC exception caught while sending email about public request with uuid %1%: gRPC error code: %2%, error message: %3%, grpc_message_json: %4%") % boost::uuids::to_string(public_request_uuid) % e.error_code() % e.error_message() % e.grpc_message_json()));
        throw;
    }
    catch (const std::exception& e)
    {
        _ctx.get_log().warning(boost::str(boost::format("std::exception caught while sending email about public request with uuid %1%: %2%") % boost::uuids::to_string(public_request_uuid) % e.what()));
        throw;
    }
    catch (...)
    {
        _ctx.get_log().warning(boost::str(boost::format("exception caught while sending email about public_request with uuid %1%") % boost::uuids::to_string(public_request_uuid)));
        throw;
    }
}


template <>
struct generate_message<CommChannel::email,
                        Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification>
{
    static void for_given_request(
            LibFred::OperationContext& _ctx,
            const MojeId::MessengerConfiguration& _messenger_configuration,
            const LibFred::LockedPublicRequest& _locked_request,
            const LibFred::LockedPublicRequestsOfObject& _locked_contact,
            const std::string& _link_hostname_part,
            const Optional<GeneralId>& _contact_history_id)
    {
        // db table mail_type: 21,'mojeid_identification','[mojeID] Založení účtu - PIN1 pro aktivaci mojeID'
        const std::string mail_type = "mojeid_identification";
        send_email(
                mail_type,
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
    }

};

template <>
struct generate_message<CommChannel::email,
                        Fred::Backend::MojeId::PublicRequest::ConditionallyIdentifiedContactTransfer>
{
    static void for_given_request(
            LibFred::OperationContext& _ctx,
            const MojeId::MessengerConfiguration& _messenger_configuration,
            const LibFred::LockedPublicRequest& _locked_request,
            const LibFred::LockedPublicRequestsOfObject& _locked_contact,
            const std::string& _link_hostname_part,
            const Optional<GeneralId>& _contact_history_id)
    {
        // db table mail_type: 27,'mojeid_verified_contact_transfer','Založení účtu mojeID'
        const std::string mail_type = "mojeid_verified_contact_transfer";
        return send_email(
                mail_type,
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
    }

};

template <>
struct generate_message<CommChannel::email, Fred::Backend::MojeId::PublicRequest::IdentifiedContactTransfer>
{
    static void for_given_request(
            LibFred::OperationContext& _ctx,
            const MojeId::MessengerConfiguration& _messenger_configuration,
            const LibFred::LockedPublicRequest& _locked_request,
            const LibFred::LockedPublicRequestsOfObject& _locked_contact,
            const std::string& _link_hostname_part,
            const Optional<GeneralId>& _contact_history_id)
    {
        // db table mail_type: 27,'mojeid_verified_contact_transfer','Založení účtu mojeID'
        const std::string mail_type = "mojeid_verified_contact_transfer";
        send_email(
                mail_type,
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
    }

};

template <>
struct generate_message<CommChannel::email,
                        Fred::Backend::MojeId::PublicRequest::PrevalidatedUnidentifiedContactTransfer>
{
    static void for_given_request(
            LibFred::OperationContext& _ctx,
            const MojeId::MessengerConfiguration& _messenger_configuration,
            const LibFred::LockedPublicRequest& _locked_request,
            const LibFred::LockedPublicRequestsOfObject& _locked_contact,
            const std::string& _link_hostname_part,
            const Optional<GeneralId>& _contact_history_id)
    {
        // db table mail_type: 21,'mojeid_identification','[mojeID] Založení účtu - PIN1 pro aktivaci mojeID'
        const std::string mail_type = "mojeid_identification";
        send_email(
                mail_type,
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
    }

};

template <>
struct generate_message<CommChannel::email, Fred::Backend::MojeId::PublicRequest::PrevalidatedContactTransfer>
{
    static void for_given_request(
            LibFred::OperationContext& _ctx,
            const MojeId::MessengerConfiguration& _messenger_configuration,
            const LibFred::LockedPublicRequest& _locked_request,
            const LibFred::LockedPublicRequestsOfObject& _locked_contact,
            const std::string& _link_hostname_part,
            const Optional<GeneralId>& _contact_history_id)
    {
        // db table mail_type: 27,'mojeid_verified_contact_transfer','Založení účtu mojeID'
        const std::string mail_type = "mojeid_verified_contact_transfer";
        send_email(
                mail_type,
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
    }

};

const LibFred::PublicRequestTypeIface& get_public_request_type_iface(const std::string& _public_request_type)
{
    typedef Fred::Backend::MojeId::PublicRequest::ConditionallyIdentifiedContactTransfer PubReqCICT;
    typedef Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification PubReqCCI;
    typedef Fred::Backend::MojeId::PublicRequest::ContactIdentification PubReqCI;
    typedef Fred::Backend::MojeId::PublicRequest::ContactReidentification PubReqCR;
    typedef Fred::Backend::MojeId::PublicRequest::IdentifiedContactTransfer PubReqICT;
    typedef Fred::Backend::MojeId::PublicRequest::PrevalidatedContactTransfer PubReqPCT;
    typedef Fred::Backend::MojeId::PublicRequest::PrevalidatedUnidentifiedContactTransfer PubReqPUCT;

    if (PubReqCCI().get_public_request_type() == _public_request_type)
    {
        return PubReqCCI().iface();
    }
    if (PubReqPUCT().get_public_request_type() == _public_request_type)
    {
        return PubReqPUCT().iface();
    }
    if (PubReqCICT().get_public_request_type() == _public_request_type)
    {
        return PubReqCICT().iface();
    }
    if (PubReqICT().get_public_request_type() == _public_request_type)
    {
        return PubReqICT().iface();
    }
    if (PubReqPCT().get_public_request_type() == _public_request_type)
    {
        return PubReqPCT().iface();
    }
    if (PubReqCI().get_public_request_type() == _public_request_type)
    {
        return PubReqCI().iface();
    }
    if (PubReqCR().get_public_request_type() == _public_request_type)
    {
        return PubReqCR().iface();
    }
    throw std::runtime_error("unexpected public requesty type");
}


void generate_messages_for_given_request(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const std::string& _public_request_type,
        const LibFred::LockedPublicRequest& _locked_request,
        const LibFred::LockedPublicRequestsOfObject& _locked_contact,
        const std::string& _link_hostname_part,
        GeneralId _contact_history_id)
{
    typedef Fred::Backend::MojeId::PublicRequest::ConditionallyIdentifiedContactTransfer PubReqCICT;
    typedef Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification PubReqCCI;
    typedef Fred::Backend::MojeId::PublicRequest::ContactIdentification PubReqCI;
    typedef Fred::Backend::MojeId::PublicRequest::ContactReidentification PubReqCR;
    typedef Fred::Backend::MojeId::PublicRequest::IdentifiedContactTransfer PubReqICT;
    typedef Fred::Backend::MojeId::PublicRequest::PrevalidatedContactTransfer PubReqPCT;
    typedef Fred::Backend::MojeId::PublicRequest::PrevalidatedUnidentifiedContactTransfer PubReqPUCT;

    if (PubReqCCI().get_public_request_type() == _public_request_type)
    {
        Generate::Into<CommChannel::sms>::for_given_request<PubReqCCI>(
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
        Generate::Into<CommChannel::email>::for_given_request<PubReqCCI>(
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
    }
    else if (PubReqPUCT().get_public_request_type() == _public_request_type)
    {
        Generate::Into<CommChannel::sms>::for_given_request<PubReqPUCT>(
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
        Generate::Into<CommChannel::email>::for_given_request<PubReqPUCT>(
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
    }
    else if (PubReqCICT().get_public_request_type() == _public_request_type)
    {
        Generate::Into<CommChannel::email>::for_given_request<PubReqCICT>(
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
    }
    else if (PubReqICT().get_public_request_type() == _public_request_type)
    {
        Generate::Into<CommChannel::email>::for_given_request<PubReqICT>(
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
    }
    else if (PubReqPCT().get_public_request_type() == _public_request_type)
    {
        Generate::Into<CommChannel::email>::for_given_request<PubReqPCT>(
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
    }
    else if (PubReqCI().get_public_request_type() == _public_request_type)
    {
        Generate::Into<CommChannel::letter>::for_given_request<PubReqCI>(
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
    }
    else if (PubReqCR().get_public_request_type() == _public_request_type)
    {
        Generate::Into<CommChannel::letter>::for_given_request<PubReqCR>(
                _ctx,
                _messenger_configuration,
                _locked_request,
                _locked_contact,
                _link_hostname_part,
                _contact_history_id);
    }
}

} // namespace Fred::Backend::MojeId::Messages::{anonymous}

void Generate::for_new_requests(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const std::string& _link_hostname_part)
{
    DbCommand cmd;
    cmd.query =
        // clang-format off
        "WITH possible_types AS ("
                 "SELECT id FROM enum_public_request_type "
                 "WHERE name IN ("
                        "'mojeid_contact_conditional_identification', "
                        "'mojeid_contact_identification', "
                        "'mojeid_contact_reidentification', "
                        //"'mojeid_contact_validation', "
                        "'mojeid_conditionally_identified_contact_transfer', "
                        "'mojeid_identified_contact_transfer', "
                        "'mojeid_prevalidated_unidentified_contact_transfer', "
                        "'mojeid_prevalidated_contact_transfer'"
             "), "
             "to_generate AS ("
                 "SELECT id AS public_request_id, "
                        "create_time AS public_request_create_time, "
                        "request_type AS public_request_type_id, "
                        "(SELECT object_id FROM public_request_objects_map prom "
                         "WHERE request_id = pr.id AND "
                               "EXISTS(SELECT * FROM contact WHERE id = prom.object_id)"
                        ") AS contact_id "
                 "FROM public_request pr "
                 "WHERE status = (SELECT id FROM enum_public_request_status WHERE name = 'opened'::TEXT) AND "
                       "on_status_action = 'scheduled'::ENUM_ON_STATUS_ACTION_TYPE AND "
                       "(NOW()::DATE - '1 DAY'::INTERVAL) < create_time AND "
                       "request_type IN (SELECT id FROM possible_types)"
                 ") "
        "SELECT public_request_id, "
               "(SELECT name FROM enum_public_request_type "
                "WHERE id = tg.public_request_type_id) AS public_request_type, "
               "contact_id, "
               "(SELECT historyid FROM contact_history ch "
                "WHERE id = tg.contact_id AND "
                      "(SELECT valid_from <= tg.public_request_create_time AND "
                                           "(tg.public_request_create_time < valid_to OR valid_to IS NULL) "
                       "FROM history "
                       "WHERE id = ch.historyid)"
               ") AS contact_history_id, "
               "lock_public_request_lock(contact_id) "
        "FROM to_generate tg "
        "WHERE contact_id IS NOT NULL";
        // clang-format on

    const Database::Result dbres = _ctx.get_conn().exec_params(
            cmd.query,
            cmd.params);
    for (::size_t idx = 0; idx < dbres.size(); ++idx)
    {
        try
        {
            const PublicRequestLockedForUpdate locked_request_for_update(_ctx, static_cast<GeneralId>(dbres[idx][0]));
            const std::string public_request_type = static_cast<std::string>(dbres[idx][1]);
            const PublicRequestObjectLocked locked_contact(static_cast<GeneralId>(dbres[idx][2]));
            const GeneralId contact_history_id  = static_cast<GeneralId>(dbres[idx][3]);
            generate_messages_for_given_request(
                    _ctx,
                    _messenger_configuration,
                    public_request_type,
                    locked_request_for_update,
                    locked_contact,
                    _link_hostname_part,
                    contact_history_id);
            LibFred::UpdatePublicRequest()
                    .set_on_status_action(LibFred::PublicRequest::OnStatusAction::processed)
                    .exec(locked_request_for_update, get_public_request_type_iface(public_request_type), Optional<MojeIdImpl::LogRequestId>()); // TODO log_request_id
        }
        catch (const std::exception& e)
        {
            _ctx.get_log().error(e.what());
        }
        catch (...)
        {
            _ctx.get_log().error("unexpected error");
        }
    }
}

template <CommChannel::Enum COMM_CHANNEL>
template <typename PUBLIC_REQUEST_TYPE>
void Generate::Into<COMM_CHANNEL>::for_given_request(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const LibFred::LockedPublicRequest& _locked_request,
        const LibFred::LockedPublicRequestsOfObject& _locked_contact,
        const std::string& _link_hostname_part,
        const Optional<GeneralId>& _contact_history_id)
{
    generate_message<COMM_CHANNEL, PUBLIC_REQUEST_TYPE>::for_given_request(
            _ctx,
            _messenger_configuration,
            _locked_request,
            _locked_contact,
            _link_hostname_part,
            _contact_history_id);
}

template void Generate::Into<CommChannel::sms>::
for_given_request<Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification>(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const LibFred::LockedPublicRequest& _locked_request,
        const LibFred::LockedPublicRequestsOfObject& _locked_contact,
        const std::string& _link_hostname_part,
        const Optional<GeneralId>& _contact_history_id);


template void Generate::Into<CommChannel::sms>::
for_given_request<Fred::Backend::MojeId::PublicRequest::PrevalidatedUnidentifiedContactTransfer>(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const LibFred::LockedPublicRequest& _locked_request,
        const LibFred::LockedPublicRequestsOfObject& _locked_contact,
        const std::string& _link_hostname_part,
        const Optional<GeneralId>& _contact_history_id);


template void Generate::Into<CommChannel::letter>::
for_given_request<Fred::Backend::MojeId::PublicRequest::ContactIdentification>(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const LibFred::LockedPublicRequest& _locked_request,
        const LibFred::LockedPublicRequestsOfObject& _locked_contact,
        const std::string& _link_hostname_part,
        const Optional<GeneralId>& _contact_history_id);


template void Generate::Into<CommChannel::letter>::
for_given_request<Fred::Backend::MojeId::PublicRequest::ContactReidentification>(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const LibFred::LockedPublicRequest& _locked_request,
        const LibFred::LockedPublicRequestsOfObject& _locked_contact,
        const std::string& _link_hostname_part,
        const Optional<GeneralId>& _contact_history_id);


template void Generate::Into<CommChannel::email>::
for_given_request<Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification>(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const LibFred::LockedPublicRequest& _locked_request,
        const LibFred::LockedPublicRequestsOfObject& _locked_contact,
        const std::string& _link_hostname_part,
        const Optional<GeneralId>& _contact_history_id);


template void Generate::Into<CommChannel::email>::
for_given_request<Fred::Backend::MojeId::PublicRequest::ConditionallyIdentifiedContactTransfer>(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const LibFred::LockedPublicRequest& _locked_request,
        const LibFred::LockedPublicRequestsOfObject& _locked_contact,
        const std::string& _link_hostname_part,
        const Optional<GeneralId>& _contact_history_id);


template void Generate::Into<CommChannel::email>::
for_given_request<Fred::Backend::MojeId::PublicRequest::IdentifiedContactTransfer>(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const LibFred::LockedPublicRequest& _locked_request,
        const LibFred::LockedPublicRequestsOfObject& _locked_contact,
        const std::string& _link_hostname_part,
        const Optional<GeneralId>& _contact_history_id);


template void Generate::Into<CommChannel::email>::
for_given_request<Fred::Backend::MojeId::PublicRequest::PrevalidatedUnidentifiedContactTransfer>(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const LibFred::LockedPublicRequest& _locked_request,
        const LibFred::LockedPublicRequestsOfObject& _locked_contact,
        const std::string& _link_hostname_part,
        const Optional<GeneralId>& _contact_history_id);


template void Generate::Into<CommChannel::email>::
for_given_request<Fred::Backend::MojeId::PublicRequest::PrevalidatedContactTransfer>(
        LibFred::OperationContext& _ctx,
        const MojeId::MessengerConfiguration& _messenger_configuration,
        const LibFred::LockedPublicRequest& _locked_request,
        const LibFred::LockedPublicRequestsOfObject& _locked_contact,
        const std::string& _link_hostname_part,
        const Optional<GeneralId>& _contact_history_id);


} // namespace Fred::Backend::MojeId::Messages
} // namespace Fred::Backend::MojeId
} // namespace Fred::Backend
} // namespace Fred
