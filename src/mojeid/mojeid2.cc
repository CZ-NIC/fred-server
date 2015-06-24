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
#include "src/mojeid/mojeid2_checkers.h"
#include "src/fredlib/contact/create_contact.h"
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

Fred::Contact::PlaceAddress& convert(const Fred::MojeID::Address &_src, Fred::Contact::PlaceAddress &_dst)
{
    _dst.street1    = _src.street1;
    _dst.city       = _src.city;
    _dst.postalcode = _src.postal_code;
    _dst.country    = _src.country;
    if (!_src.street2.get_value_or_default().empty()) {
        _dst.street2 = _src.street2.get_value();
    }
    if (!_src.street3.get_value_or_default().empty()) {
        _dst.street3 = _src.street3.get_value();
    }
    if (!_src.state.get_value_or_default().empty()) {
        _dst.stateorprovince = _src.state.get_value();
    }
    return _dst;
}

Fred::ContactAddress& convert(const Fred::MojeID::ShippingAddress &_src, Fred::ContactAddress &_dst)
{
    if (!_src.company_name.get_value_or_default().empty()) {
        _dst.company_name = _src.company_name.get_value();
    }
    _dst.street1    = _src.street1;
    _dst.city       = _src.city;
    _dst.postalcode = _src.postal_code;
    _dst.country    = _src.country;
    if (!_src.street2.get_value_or_default().empty()) {
        _dst.street2 = _src.street2.get_value();
    }
    if (!_src.street3.get_value_or_default().empty()) {
        _dst.street3 = _src.street3.get_value();
    }
    if (!_src.state.get_value_or_default().empty()) {
        _dst.stateorprovince = _src.state.get_value();
    }
    return _dst;
}

typedef bool ValueWasSet;

ValueWasSet set_create_contact_ssn(
    const Nullable< std::string > &_ssn,
    const char *_ssn_type,
    Fred::CreateContact &_arguments)
{
    if (_ssn.isnull()) {
        return false;
    }
    _arguments.set_ssntype(_ssn_type);
    _arguments.set_ssn(_ssn.get_value());
    return true;
}

void set_create_contact_arguments(
    const Fred::MojeID::CreateContact &_contact,
    Fred::CreateContact &_arguments)
{
    _arguments.set_name(_contact.first_name + " " + _contact.last_name);
    Fred::Contact::PlaceAddress place;
    _arguments.set_place(convert(_contact.permanent, place));
    _arguments.set_email(_contact.email);
    _arguments.set_telephone(_contact.telephone);

    const bool contact_is_organization = !_contact.organization.isnull();
    if (contact_is_organization) {
        _arguments.set_organization(_contact.organization.get_value());
    }
    if (!_contact.notify_email.isnull()) {
        _arguments.set_notifyemail(_contact.notify_email.get_value());
    }
    if (!_contact.fax.isnull()) {
        _arguments.set_fax(_contact.fax.get_value());
    }

    {
        Fred::ContactAddressList addresses;
        if (!_contact.mailing.isnull()) {
            addresses[Fred::ContactAddressType::MAILING] = convert(_contact.mailing.get_value(), place);
        }
        if (!_contact.billing.isnull()) {
            addresses[Fred::ContactAddressType::BILLING] = convert(_contact.billing.get_value(), place);
        }

        Fred::ContactAddress shipping;
        if (!_contact.shipping.isnull()) {
            addresses[Fred::ContactAddressType::SHIPPING] = convert(_contact.shipping.get_value(), shipping);
        }
        if (!_contact.shipping2.isnull()) {
            addresses[Fred::ContactAddressType::SHIPPING_2] = convert(_contact.shipping2.get_value(), shipping);
        }
        if (!_contact.shipping3.isnull()) {
            addresses[Fred::ContactAddressType::SHIPPING_3] = convert(_contact.shipping3.get_value(), shipping);
        }

        if (!addresses.empty()) {
            _arguments.set_addresses(addresses);
        }
    }

    if (!_contact.vat_reg_num.isnull()) {
        _arguments.set_vat(_contact.vat_reg_num.get_value());
    }

    if (contact_is_organization) {
        if (set_create_contact_ssn(_contact.vat_id_num, "ICO", _arguments)) {
            return;
        }
        if (set_create_contact_ssn(_contact.birth_date, "BIRTHDAY", _arguments)) {
            return;
        }
    }
    else {
        if (set_create_contact_ssn(_contact.birth_date, "BIRTHDAY", _arguments)) {
            return;
        }
        if (set_create_contact_ssn(_contact.vat_id_num, "ICO", _arguments)) {
            return;
        }
    }
    if (set_create_contact_ssn(_contact.id_card_num, "OP", _arguments)) {
        return;
    }
    if (set_create_contact_ssn(_contact.passport_num, "PASS", _arguments)) {
        return;
    }
    if (set_create_contact_ssn(_contact.ssn_id_num, "MPSV", _arguments)) {
        return;
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
        ::size_t _chunk_size,
        ContactId &_start_from,
        HandleList &_result)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextCreator ctx;
        const Database::Result dbres = ctx.get_conn().exec_params(
            "WITH static_data AS ("
                "SELECT eot.id AS type_id,"
                       "NOW()::DATE-(ep.val||'MONTH')::INTERVAL AS handle_protected_to "
                "FROM enum_object_type eot,enum_parameters ep "
                "WHERE eot.name='contact' AND "
                      "ep.name='handle_registration_protection_period') "
            "SELECT obr.id,obr.name "
            "FROM static_data sd "
            "JOIN object_registry obr ON obr.type=sd.type_id AND "
                                        "COALESCE(sd.handle_protected_to<obr.erdate,TRUE) "
            "WHERE LOWER(obr.name)~'^[a-z0-9](-?[a-z0-9])*$' AND "
                  "$2::BIGINT<obr.id "
            "ORDER BY obr.id "
            "LIMIT $1::BIGINT",
            Database::query_param_list
                (_chunk_size + 1)
                (_start_from));
        const bool data_continues = _chunk_size < dbres.size();
        const ::size_t items = data_continues ? _chunk_size
                                              : dbres.size();
        _result.clear();
        _result.reserve(items);
        enum
        {
            CONTACT_ID_IDX = 0,
            CONTACT_HANDLE_IDX = 1,
        };
        for (::size_t idx = 0; idx < items; ++idx) {
            _result.push_back(static_cast< std::string >(dbres[idx][CONTACT_HANDLE_IDX]));
        }
        _start_from = data_continues ? static_cast< ContactId >(dbres[_chunk_size - 1][CONTACT_ID_IDX])
                                     : contact_handles_end_reached;
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
        const Fred::MojeID::CreateContact &_contact,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        std::string &_ident)
{
    LOGGING_CONTEXT(log_ctx, *this);

    try {
        Fred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        typedef boost::mpl::list< Fred::MojeID::Check::contact_name,
                                  Fred::MojeID::Check::contact_permanent_address,
                                  Fred::MojeID::Check::contact_email_presence,
                                  Fred::MojeID::Check::contact_email_validity,
                                  Fred::MojeID::Check::contact_phone_presence,
                                  Fred::MojeID::Check::contact_phone_validity > check_contact;
        typedef boost::mpl::list< Fred::MojeID::Check::contact_username_availability,
                                  Fred::MojeID::Check::contact_email_availability,
                                  Fred::MojeID::Check::contact_phone_availability > check_contact_ctx;
        typedef Fred::Check< boost::mpl::list< check_contact,
                                               check_contact_ctx > > Check;

        Check check_result(Fred::make_args(_contact),
                           Fred::make_args(_contact, ctx));

        Fred::CreateContact create_contact(_contact.username, mojeid_registrar_handle_);
        set_create_contact_arguments(_contact, create_contact);
        const Fred::CreateContact::Result new_contact = create_contact.exec(ctx);
        return new_contact.object_id;
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
