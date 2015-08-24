/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file
 *  mojeid2 implementation
 */

#include "src/mojeid/mojeid2.h"
#include "src/mojeid/safe_data_storage.h"
#include "src/mojeid/mojeid_public_request.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/public_request/create_public_request_auth.h"
#include "src/fredlib/public_request/info_public_request_auth.h"
#include "src/fredlib/public_request/update_public_request.h"
#include "src/fredlib/public_request/public_request_status.h"
#include "src/fredlib/public_request/public_request_lock_guard.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "util/random.h"
#include "util/log/context.h"
#include "util/cfg/handle_mojeid_args.h"
#include "util/cfg/config_handler_decl.h"

#include <algorithm>
#include <map>

namespace Registry {
namespace MojeID {

namespace {

std::string create_ctx_name(const std::string &_name)
{
    return str(boost::format("%1%-<%2%>") % _name % Random::integer(0, 10000));
}

std::string create_ctx_function_name(const char *fnc)
{
    std::string name(fnc);
    std::replace(name.begin(), name.end(), '_', '-');
    return name;
}

class LogContext
{
public:
    LogContext(const MojeID2Impl &_impl, const std::string &_op_name)
    :   ctx_server_(create_ctx_name(_impl.get_server_name())),
        ctx_operation_(_op_name)
    {
    }
private:
    Logging::Context ctx_server_;
    Logging::Context ctx_operation_;
};

#define LOGGING_CONTEXT(CTX_VAR, IMPL_OBJ) LogContext CTX_VAR((IMPL_OBJ), create_ctx_function_name(__FUNCTION__))

std::string get_mojeid_registrar_handle()
{
    try {
        const std::string handle =
            CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >()->registrar_handle;
        if (!handle.empty()) {
            return handle;
        }
    }
    catch (...) {
    }
    throw std::runtime_error("missing configuration for dedicated registrar");
}

void set_create_contact_arguments(
    const Fred::InfoContactData &_contact,
    Fred::CreateContact &_arguments)
{
    if (!_contact.name.isnull()) {
        _arguments.set_name(_contact.name.get_value());
    }
    if (!_contact.place.isnull()) {
        _arguments.set_place(_contact.place.get_value());
    }
    if (!_contact.email.isnull()) {
        _arguments.set_email(_contact.email.get_value());
    }
    if (!_contact.telephone.isnull()) {
        _arguments.set_telephone(_contact.telephone.get_value());
    }
    if (!_contact.organization.isnull()) {
        _arguments.set_organization(_contact.organization.get_value());
    }
    if (!_contact.notifyemail.isnull()) {
        _arguments.set_notifyemail(_contact.notifyemail.get_value());
    }
    if (!_contact.fax.isnull()) {
        _arguments.set_fax(_contact.fax.get_value());
    }
    if (!_contact.addresses.empty()) {
        _arguments.set_addresses(_contact.addresses);
    }
    if (!_contact.vat.isnull()) {
        _arguments.set_vat(_contact.vat.get_value());
    }
    if (!_contact.ssntype.isnull()) {
        _arguments.set_ssntype(_contact.ssntype.get_value());
    }
    if (!_contact.ssn.isnull()) {
        _arguments.set_ssn(_contact.ssn.get_value());
    }
}

typedef data_storage< std::string, ContactId >::safe prepare_transaction_storage;
typedef prepare_transaction_storage::object_type::data_not_found prepare_transaction_data_not_found;

}//Registry::MojeID::{anonymous}

MojeID2Impl::MojeID2Impl(const std::string &_server_name)
:   server_name_(_server_name),
    mojeid_registrar_handle_(get_mojeid_registrar_handle())
{
    LogContext log_ctx(*this, "init");
}//MojeID2Impl::MojeID2Impl

MojeID2Impl::~MojeID2Impl()
{
}

const std::string& MojeID2Impl::get_server_name()const
{
    return server_name_;
}

HandleList& MojeID2Impl::get_unregistrable_contact_handles(
        HandleList &_result)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        const Database::Result dbres = ctx.get_conn().exec(
            "WITH static_data AS ("
                "SELECT eot.id AS type_id,"
                       "NOW()-(ep.val||'MONTH')::INTERVAL AS contact_protected_since "
                "FROM enum_object_type eot,enum_parameters ep "
                "WHERE eot.name='contact' AND "
                      "ep.name='handle_registration_protection_period') "
            "SELECT name "
            "FROM object_registry "
            "WHERE type=(SELECT type_id FROM static_data) AND "
                  "COALESCE((SELECT contact_protected_since FROM static_data)<erdate,TRUE) AND "
                  "LOWER(name)~'^[a-z0-9](-?[a-z0-9])*$'");
        _result.clear();
        _result.reserve(dbres.size());
        for (::size_t idx = 0; idx < dbres.size(); ++idx) {
            _result.push_back(static_cast< std::string >(dbres[idx][0]));
        }
        return _result;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

ContactId MojeID2Impl::create_contact_prepare(
        const Fred::InfoContactData &_contact,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        std::string &_ident)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);

        {
            const CheckCreateContactPrepare check_contact_data(Fred::make_args(_contact),
                                                               Fred::make_args(_contact, ctx));
            if (!check_contact_data.success()) {
                throw check_contact_data;
            }
        }

        Fred::CreateContact op_create_contact(_contact.handle, mojeid_registrar_handle_);
        set_create_contact_arguments(_contact, op_create_contact);
        const Fred::CreateContact::Result new_contact = op_create_contact.exec(ctx);
        Fred::CreatePublicRequestAuth op_create_pub_req(
            Fred::MojeID::PublicRequest::ContactConditionalIdentification::iface());
        Fred::PublicRequestObjectLockGuard locked_contact(ctx, new_contact.object_id);
        {
            const Fred::CreatePublicRequestAuth::Result result = op_create_pub_req.exec(ctx, locked_contact);
            _ident = result.identification;
        }
        prepare_transaction_storage()->store(_trans_id, new_contact.object_id);
        ctx.commit_transaction();
        return new_contact.object_id;
    }
    catch (const CreateContactPrepareError&) {
        LOGGER(PACKAGE).error("request failed (incorrect input data)");
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

Fred::InfoContactData& MojeID2Impl::transfer_contact_prepare(
        const std::string &_handle,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        Fred::InfoContactData &_contact,
        std::string &_ident)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);

        _contact = Fred::InfoContactByHandle(_handle).exec(ctx).info_contact_data;
        Fred::PublicRequestObjectLockGuard locked_contact(ctx, _contact.id);
        const TransferContactPrepareRelatedStatesPresence states_presence =
            GetContact(_contact.id).states< TransferContactPrepareRelatedStates >().presence(ctx);
        const CheckTransferContactPrepare check_result(Fred::make_args(_contact),
                                                       Fred::make_args(states_presence));
        if (!check_result.success()) {
            throw check_result;
        }

        struct GetPublicRequestAuthType
        {
            static const Fred::PublicRequestAuthTypeIface& iface(
                bool has_conditionally_identified_state,
                bool has_identified_state)
            {
                switch ((has_conditionally_identified_state ? (0x01 << 0) : 0x00) |
                        (has_identified_state               ? (0x01 << 1) : 0x00)) {
                case 0x00://..
                    return Fred::MojeID::PublicRequest::ContactConditionalIdentification::iface();
                case 0x01://C.
                    return Fred::MojeID::PublicRequest::ConditionallyIdentifiedContactTransfer::iface();
                case 0x02://.I
                    break;
                case 0x03://CI
                    return Fred::MojeID::PublicRequest::IdentifiedContactTransfer::iface();
                }
                throw std::runtime_error("unsupported combination of contact identification states");
            }
        };
        Fred::CreatePublicRequestAuth op_create_pub_req(
            GetPublicRequestAuthType::iface(
                states_presence.get< Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT >(),
                states_presence.get< Fred::Object::State::IDENTIFIED_CONTACT >()));
        prepare_transaction_storage()->store(_trans_id, _contact.id);
        ctx.commit_transaction();
        return _contact;
    }
    catch (const TransferContactPrepareError&) {
        LOGGER(PACKAGE).error("request failed (incorrect input data)");
        throw;
    }
    catch (const Fred::InfoContactByHandle::Exception &e) {
        if (e.is_set_unknown_contact_handle()) {
            LOGGER(PACKAGE).error("request failed (incorrect input data)");
            throw Fred::Object::Get< Fred::Object::Type::CONTACT >::object_doesnt_exist();
        }
        throw;
    }
    catch (const GetContact::object_doesnt_exist &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

namespace {

struct PubReqType
{
    enum Value
    {
        CONTACT_CONDITIONAL_IDENTIFICATION,
        CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER,
        IDENTIFIED_CONTACT_TRANSFER,
    };
    static Value from(const std::string &_type)
    {
        typedef std::map< std::string, Value > StrToValue;
        static StrToValue convert;
        if (convert.empty()) {
            using namespace Fred::MojeID::PublicRequest;
            convert[into_string< ContactConditionalIdentification >()]       = CONTACT_CONDITIONAL_IDENTIFICATION;
            convert[into_string< ConditionallyIdentifiedContactTransfer >()] = CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER;
            convert[into_string< IdentifiedContactTransfer >()]              = IDENTIFIED_CONTACT_TRANSFER;
        }
        StrToValue::const_iterator value_ptr = convert.find(_type);
        if (value_ptr != convert.end()) {
            return value_ptr->second;
        }
        throw std::runtime_error("unexpected public request type " + _type);
    }
    template < class PUB_REQ >
    static std::string into_string()
    {
        return PUB_REQ::iface().get_public_request_type();
    }
};

enum { INVALID_LOG_REQUEST_ID = 0 };

Fred::UpdatePublicRequest::Result set_status(
    Fred::OperationContext &_ctx,
    const Fred::PublicRequestLockGuard &_locked_request,
    Fred::PublicRequest::Status::Value _status,
    const std::string &_reason,
    LogRequestId _log_request_id)
{
    Fred::UpdatePublicRequest op_update_public_request;
    op_update_public_request.set_status(_status);
    if (!_reason.empty()) {
        op_update_public_request.set_reason(_reason);
    }
    if (_log_request_id == INVALID_LOG_REQUEST_ID) {
        return op_update_public_request.exec(_ctx, _locked_request);
    }
    return op_update_public_request.exec(_ctx, _locked_request, _log_request_id);
}

Fred::UpdatePublicRequest::Result answer(
    Fred::OperationContext &_ctx,
    const Fred::PublicRequestLockGuard &_locked_request,
    const std::string &_reason = "",
    LogRequestId _log_request_id = INVALID_LOG_REQUEST_ID)
{
    return set_status(_ctx, _locked_request, Fred::PublicRequest::Status::ANSWERED, _reason, _log_request_id);
}

Fred::UpdatePublicRequest::Result invalidate(
    Fred::OperationContext &_ctx,
    const Fred::PublicRequestLockGuard &_locked_request,
    const std::string &_reason = "",
    LogRequestId _log_request_id = INVALID_LOG_REQUEST_ID)
{
    return set_status(_ctx, _locked_request, Fred::PublicRequest::Status::INVALIDATED, _reason, _log_request_id);
}

}//namespace Registry::MojeID::{anonymous}

ContactId MojeID2Impl::process_registration_request(
        const std::string &_ident_request_id,
        const std::string &_password,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        Fred::PublicRequestLockGuardByIdentification locked_request(ctx, _ident_request_id);
        const Fred::PublicRequestAuthInfo pub_req_info(ctx, locked_request);

        switch (pub_req_info.get_status()) {
        case Fred::PublicRequest::Status::NEW:
            break;
        case Fred::PublicRequest::Status::ANSWERED:
            throw IdentificationAlreadyProcessed("identification already processed");
        case Fred::PublicRequest::Status::INVALIDATED:
            throw IdentificationAlreadyInvalidated("identification already invalidated");
        }

        const PubReqType::Value pub_req_type = PubReqType::from(pub_req_info.get_type());
        switch (pub_req_type) {
        case PubReqType::CONTACT_CONDITIONAL_IDENTIFICATION:
        case PubReqType::CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER:
        case PubReqType::IDENTIFIED_CONTACT_TRANSFER:
            break;
        default:
            throw std::runtime_error("unexpected public request type " + pub_req_info.get_type());
        }

        if (pub_req_info.get_object_id().isnull()) {
            static const std::string msg = "no object associated with this public request";
            invalidate(ctx, locked_request, msg, _log_request_id);
            ctx.commit_transaction();
            throw IdentificationFailed(msg);
        }

        const Fred::ObjectId object_id = pub_req_info.get_object_id().get_value();
        const Database::Result dbres = ctx.get_conn().exec_params(
              "SELECT EXISTS(SELECT 1 FROM public_request "
                            "WHERE id=$1::BIGINT AND "
                                  "create_time<(SELECT GREATEST(update,trdate) FROM object "
                                               "WHERE id=$2::BIGINT)"
                           ") AS object_changed",
              Database::query_param_list(locked_request.get_public_request_id())//$1::BIGINT
                                        (object_id));                           //$2::BIGINT

        if (dbres.size() != 1) {
            throw std::runtime_error("something wrong happened database is crazy");
        }

        const bool contact_changed = static_cast< bool >(dbres[0][0]);
        if (contact_changed) {
            static const std::string msg = "contact data changed after the public request had been created";
            invalidate(ctx, locked_request, msg, _log_request_id);
            ctx.commit_transaction();
            throw ContactChanged(msg);
        }

        if (!pub_req_info.check_password(_password)) {
            throw IdentificationFailed("password doesn't match");
        }

        const Fred::InfoContactData contact = Fred::InfoContactById(object_id).exec(ctx).info_contact_data;

        try {
            switch (pub_req_type) {
            case PubReqType::CONTACT_CONDITIONAL_IDENTIFICATION:
                this->process_contact_conditional_identification(ctx, contact, _log_request_id);
                break;
            case PubReqType::CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER:
                this->process_conditionally_identified_contact_transfer(ctx, contact, _log_request_id);
                break;
            case PubReqType::IDENTIFIED_CONTACT_TRANSFER:
                this->process_identified_contact_transfer(ctx, contact, _log_request_id);
                break;
            }
        }
        catch (const IdentificationAlreadyProcessed &e) {
            invalidate(ctx, locked_request, e.what(), _log_request_id);
            ctx.commit_transaction();
            throw;
        }
        catch (const AlreadyMojeidContact &e) {
            invalidate(ctx, locked_request, e.what(), _log_request_id);
            ctx.commit_transaction();
            throw;
        }
        catch (const ObjectAdminBlocked &e) {
            invalidate(ctx, locked_request, e.what(), _log_request_id);
            ctx.commit_transaction();
            throw;
        }
        catch (const ObjectUserBlocked &e) {
            invalidate(ctx, locked_request, e.what(), _log_request_id);
            ctx.commit_transaction();
            throw;
        }
        answer(ctx, locked_request, "process_registration_request call", _log_request_id);
        ctx.commit_transaction();
        return contact.id;
    }
    catch (const Fred::PublicRequestLockGuardByIdentification::Exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        if (e.is_set_public_request_doesnt_exist()) {
            throw PublicRequestDoesntExist(e.what());
        }
        throw std::runtime_error(e.what());
    }
    catch (const Fred::InfoContactById::Exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        if (e.is_set_unknown_object_id()) {
            throw IdentificationFailed("no contact associated with this public request");
        }
        throw;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

void MojeID2Impl::process_identification_request(
        ContactId _contact_id,
        const std::string &_password,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

void MojeID2Impl::commit_prepared_transaction(const std::string &_trans_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::commit_transaction(_trans_id);
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
    try {
        const ContactId contact_id = prepare_transaction_storage()->get(_trans_id);
        Fred::OperationContextCreator ctx;
        Fred::PerformObjectStateRequest(contact_id).exec(ctx);
        ctx.commit_transaction();
        prepare_transaction_storage()->release(_trans_id);
    }
    catch (const prepare_transaction_data_not_found&) {
        LOGGER(PACKAGE).error("request failed (cannot retrieve saved transaction data "
                              "using transaction identifier " + _trans_id + ")");
    }
}

void MojeID2Impl::rollback_prepared_transaction(const std::string &_trans_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::rollback_transaction(_trans_id);
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
    try {
        prepare_transaction_storage()->release(_trans_id);
    }
    catch (const prepare_transaction_data_not_found&) {
        LOGGER(PACKAGE).error("request failed (cannot retrieve saved transaction data "
                              "using transaction identifier " + _trans_id + ")");
    }
}

void MojeID2Impl::process_contact_conditional_identification(
        Fred::OperationContext &_ctx,
        const Fred::InfoContactData &_contact,
        LogRequestId _log_request_id)const
{
    {
        const CheckCreateContactPrepare check_contact_data(Fred::make_args(_contact),
                                                           Fred::make_args(_contact, _ctx));
        if (!check_contact_data.success()) {
            throw check_contact_data;
        }
    }

    using namespace Fred::Object;
    typedef State::set<
                State::SERVER_TRANSFER_PROHIBITED,
                State::SERVER_UPDATE_PROHIBITED,
                State::SERVER_DELETE_PROHIBITED,
                State::SERVER_BLOCKED,
                State::MOJEID_CONTACT,
                State::CONDITIONALLY_IDENTIFIED_CONTACT >::type RelatedStates;
    const Get< Type::CONTACT >::States< RelatedStates >::Presence presence =
        Get< Type::CONTACT >(_contact.id).states< RelatedStates >().presence(_ctx);

    if (presence.get< State::CONDITIONALLY_IDENTIFIED_CONTACT >()) {
        throw IdentificationAlreadyProcessed("contact already conditionally identified");
    }

    if (presence.get< State::MOJEID_CONTACT >()) {
        throw AlreadyMojeidContact("contact mustn't be in mojeidContact state");
    }

    if (presence.get< State::SERVER_BLOCKED >()) {
        throw ObjectAdminBlocked("contact administratively protected against changes");
    }

    if (presence.get< State::SERVER_TRANSFER_PROHIBITED >() ||
        presence.get< State::SERVER_UPDATE_PROHIBITED >() ||
        presence.get< State::SERVER_DELETE_PROHIBITED >()) {
        throw ObjectUserBlocked("contact protected against changes");
    }

    if (_contact.sponsoring_registrar_handle != mojeid_registrar_handle_) {
        Fred::UpdateContactById op_update_contact(_contact.id, mojeid_registrar_handle_);
        op_update_contact.set_sponsoring_registrar(mojeid_registrar_handle_);
        if (_log_request_id != INVALID_LOG_REQUEST_ID) {
            op_update_contact.set_logd_request_id(_log_request_id);
        }
        op_update_contact.exec(_ctx);
    }
}

void MojeID2Impl::process_conditionally_identified_contact_transfer(
        Fred::OperationContext &_ctx,
        const Fred::InfoContactData &_contact,
        LogRequestId _log_request_id)const
{
}

void MojeID2Impl::process_identified_contact_transfer(
        Fred::OperationContext &_ctx,
        const Fred::InfoContactData &_contact,
        LogRequestId _log_request_id)const
{
}

}//namespace Registry::MojeID
}//namespace Registry
