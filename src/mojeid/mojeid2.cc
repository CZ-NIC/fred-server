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
#include "src/mojeid/mojeid_public_request.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/public_request/create_public_request_auth.h"
#include "util/random.h"
#include "util/log/context.h"
#include "util/cfg/handle_mojeid_args.h"
#include "util/cfg/config_handler_decl.h"

#include <algorithm>

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

}//Registry::MojeID::{anonymous}

MojeID2Impl::MojeID2Impl(const std::string &_server_name)
:   server_name_(_server_name),
    mojeid_registrar_handle_(Registry::MojeID::get_mojeid_registrar_handle())
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

        const CheckCreateContactPrepare check_result(Fred::make_args(_contact),
                                                     Fred::make_args(_contact, ctx));
        if (!check_result.success()) {
            throw check_result;
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

ContactId MojeID2Impl::process_registration_request(
        const std::string &_ident_request_id,
        const std::string &_password,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        const Database::Result dbres = ctx.get_conn().exec_params(
              "SELECT pra.password=$2::TEXT AS auth_successful,"
                     "pr.id AS pub_req_id,"
                     "obr.id AS contact_id,"
                     "(SELECT name FROM enum_public_request_type WHERE id=pr.request_type) AS pub_req_type,"
                     "(SELECT name FROM enum_public_request_status WHERE id=pr.status) AS pub_req_status "
              "FROM public_request_auth pra "
              "LEFT JOIN public_request pr ON pr.id=pra.id "
              "LEFT JOIN public_request_objects_map prom ON prom.request_id=pr.id "
              "LEFT JOIN object_registry obr ON obr.id=prom.object_id AND "
                                               "obr.type=(SELECT id FROM enum_object_type WHERE name='contact') "
              "WHERE pra.identification=$1::TEXT",
              Database::query_param_list(_ident_request_id)
                                        (_password));
        if (dbres.size() <= 0) {
            throw IdentificationFailed("no public request found");
        }
        if (!static_cast< bool >(dbres[0][0])) {
            throw IdentificationFailed("password doesn't match");
        }
        if (dbres[0][1].isnull()) {
            throw std::runtime_error("no public request associated with this identification");
        }
        const std::string pub_req_status = static_cast< std::string >(dbres[0][4]);
        if (pub_req_status != "new") {
            if (pub_req_status == "answered") {
                throw IdentificationAlreadyProcessed("identification already processed");
            }
            if (pub_req_status == "invalidated") {
                throw IdentificationAlreadyInvalidated("identification already invalidated");
            }
            throw std::runtime_error("unexpected public request status '" + pub_req_status + "'");
        }
        if (dbres[0][2].isnull()) {
            throw std::runtime_error("no contact associated with this public request");
        }
        const ContactId contact_id = static_cast< ContactId >(dbres[0][2]);
        const std::string pub_req_type = static_cast< std::string >(dbres[0][3]);
        return contact_id;
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
}

}//namespace Registry::MojeID
}//namespace Registry
