/*
 * Copyright (C) 2010-2022  CZ.NIC, z. s. p. o.
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

#include "src/backend/mojeid/mojeid.hh"
#include "src/backend/buffer.hh"
#include "src/backend/mojeid/messenger_configuration.hh"
#include "src/backend/mojeid/messages/generate.hh"
#include "src/backend/mojeid/mojeid_impl_internal.hh"
#include "src/backend/mojeid/mojeid_public_request.hh"
#include "src/backend/mojeid/safe_data_storage.hh"

#include "src/bin/corba/mojeid/mojeid_corba_conversion.hh"

#include "src/deprecated/libfred/documents.hh"
#include "src/deprecated/libfred/messages/messages_impl.hh"
#include "src/deprecated/libfred/registrable_object/contact/ssntype.hh"

#include "libfred/notifier/enqueue_notification.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/object/store_authinfo.hh"
#include "libfred/object_state/cancel_object_state_request_id.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/poll/create_poll_message.hh"
#include "libfred/public_request/create_public_request.hh"
#include "libfred/public_request/create_public_request_auth.hh"
#include "libfred/public_request/get_opened_public_request.hh"
#include "libfred/public_request/info_public_request_auth.hh"
#include "libfred/public_request/public_request_lock_guard.hh"
#include "libfred/public_request/public_request_status.hh"
#include "libfred/public_request/update_public_request.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/transfer_contact.hh"
#include "libfred/registrable_object/contact/undisclose_address.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/check_registrar.hh"

#include "util/case_insensitive.hh"
#include "util/random/random.hh"
#include "util/log/context.hh"

#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_messenger_args.hh"
#include "src/util/cfg/handle_mojeid_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/types/birthdate.hh"
#include "src/util/xmlgen.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/copy_if.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/set.hpp>

#include <algorithm>
#include <functional>
#include <iomanip>
#include <limits>
#include <map>
#include <type_traits>
#include <utility>

namespace Fred {
namespace Backend {
namespace MojeId {

namespace {

unsigned long long get_registrar_id(const std::string& _registrar_handle)
{
    LibFred::OperationContextCreator ctx;

    try
    {
        return LibFred::InfoRegistrarByHandle(_registrar_handle)
                .exec(ctx)
                .info_registrar_data.id;
    }
    catch (const LibFred::InfoRegistrarByHandle::Exception& e)
    {
        if (e.is_set_unknown_registrar_handle())
        {
            throw std::runtime_error(boost::str(boost::format(
                    "registrar with handle \"%1%\" not found in database") % _registrar_handle));
        }
        throw;
    }
}

auto make_authinfo_password()
{
    Random::Generator generator;
    const auto get_random_char = [&]()
    {
        return generator.get(std::numeric_limits<char>::min(), std::numeric_limits<char>::max());
    };
    static constexpr auto authinfo_length = 16;
    std::string authinfo_password;
    authinfo_password.reserve(authinfo_length);
    std::generate_n(std::back_inserter(authinfo_password), authinfo_length, get_random_char);
    return authinfo_password;
}

auto set_contact_authinfo_password(
        LibFred::OperationContext& ctx,
        const LibFred::Object::ObjectId& contact_id,
        unsigned long long system_registrar_id)
{
    auto authinfo_password = make_authinfo_password();
    static constexpr auto authinfo_ttl = std::chrono::seconds{1};
    LibFred::Object::StoreAuthinfo{contact_id, system_registrar_id, authinfo_ttl}
            .exec(ctx, authinfo_password);
    return authinfo_password;
}

} // namespace Fred::Backend::MojeId::{anonymous}

ConfiguredRegistrar::ConfiguredRegistrar(const std::string& _handle)
    : handle_(_handle),
      id_(get_registrar_id(_handle))
{
}

std::string ConfiguredRegistrar::handle() const
{
    return handle_;
}

unsigned long long ConfiguredRegistrar::id() const
{
    return id_;
}

namespace {

std::string create_ctx_name(const std::string& _name)
{
    return str(boost::format("%1%-<%2%>") % _name % Random::Generator().get(0, 10000));
}

std::string create_ctx_function_name(const char* fnc)
{
    std::string name(fnc);
    std::replace(name.begin(), name.end(), '_', '-');
    return name;
}

class LogContext
{
public:
    LogContext(const MojeIdImpl& _impl, const std::string& _op_name)
        : ctx_server_(create_ctx_name(_impl.get_server_name())),
          ctx_operation_(_op_name)
    {
    }

private:
    Logging::Context ctx_server_;
    Logging::Context ctx_operation_;
};

#define LOGGING_CONTEXT(CTX_VAR, IMPL_OBJ) LogContext CTX_VAR((IMPL_OBJ), create_ctx_function_name(__FUNCTION__))

ConfiguredRegistrar get_mojeid_registrar()
{
    const std::string mojeid_registrar_handle =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>()->registrar_handle;
    if (mojeid_registrar_handle.empty())
    {
        throw std::runtime_error("missing configuration for dedicated registrar");
    }
    return ConfiguredRegistrar(mojeid_registrar_handle);
}

ConfiguredRegistrar get_system_registrar()
{
    const std::string system_registrar_handle =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()->system_registrar;
    if (system_registrar_handle.empty())
    {
        throw std::runtime_error("missing configuration for system registrar");
    }
    return ConfiguredRegistrar(system_registrar_handle);
}

void set_create_contact_arguments(
        const MojeIdImplData::CreateContact& _contact,
        LibFred::CreateContact& _arguments)
{
    _arguments.set_name(_contact.name);
    LibFred::Contact::PlaceAddress permanent;
    from_into(_contact.permanent, permanent);
    _arguments.set_place(permanent);
    _arguments.set_email(_contact.email);
    _arguments.set_telephone(_contact.telephone);
    if (!_contact.organization.isnull())
    {
        _arguments.set_organization(_contact.organization.get_value());
    }
    if (!_contact.notify_email.isnull())
    {
        _arguments.set_notifyemail(_contact.notify_email.get_value());
    }
    if (!_contact.fax.isnull())
    {
        _arguments.set_fax(_contact.fax.get_value());
    }
    {
        LibFred::ContactAddressList addresses;
        if (!_contact.mailing.isnull())
        {
            from_into(_contact.mailing.get_value(), addresses[LibFred::ContactAddressType::MAILING]);
        }
        if (!_contact.billing.isnull())
        {
            from_into(_contact.billing.get_value(), addresses[LibFred::ContactAddressType::BILLING]);
        }
        if (!_contact.shipping.isnull())
        {
            from_into(_contact.shipping.get_value(), addresses[LibFred::ContactAddressType::SHIPPING]);
        }
        if (!_contact.shipping2.isnull())
        {
            from_into(_contact.shipping2.get_value(), addresses[LibFred::ContactAddressType::SHIPPING_2]);
        }
        if (!_contact.shipping3.isnull())
        {
            from_into(_contact.shipping3.get_value(), addresses[LibFred::ContactAddressType::SHIPPING_3]);
        }
        if (!addresses.empty())
        {
            _arguments.set_addresses(addresses);
        }
    }

    if (_contact.organization.isnull())
    {
        if (!_contact.birth_date.isnull())
        {
            _arguments.set_ssntype(Conversion::Enums::to_db_handle(LibFred::SSNType::birthday));
            _arguments.set_ssn(_contact.birth_date.get_value().value);
        }
    }
    else
    {
        if (!_contact.vat_id_num.isnull())
        {
            _arguments.set_ssntype(Conversion::Enums::to_db_handle(LibFred::SSNType::ico));
            _arguments.set_ssn(_contact.vat_id_num.get_value());
        }
        else if (!_contact.id_card_num.isnull())
        {
            _arguments.set_ssntype(Conversion::Enums::to_db_handle(LibFred::SSNType::op));
            _arguments.set_ssn(_contact.id_card_num.get_value());
        }
        else if (!_contact.passport_num.isnull())
        {
            _arguments.set_ssntype(Conversion::Enums::to_db_handle(LibFred::SSNType::pass));
            _arguments.set_ssn(_contact.passport_num.get_value());
        }
        else if (!_contact.ssn_id_num.isnull())
        {
            _arguments.set_ssntype(Conversion::Enums::to_db_handle(LibFred::SSNType::mpsv));
            _arguments.set_ssn(_contact.ssn_id_num.get_value());
        }
    }
    //default mojeID disclose policy
    _arguments.set_disclosename(true);
    _arguments.set_discloseorganization(true);
    _arguments.set_discloseaddress(true);
    _arguments.set_disclosetelephone(false);
    _arguments.set_disclosefax(false);
    _arguments.set_discloseemail(false);
    _arguments.set_disclosevat(false);
    _arguments.set_discloseident(false);
    _arguments.set_disclosenotifyemail(false);
}

void check_sent_letters_limit(LibFred::OperationContext& _ctx,
        MojeIdImpl::ContactId _contact_id,
        unsigned _max_sent_letters,
        unsigned _watched_period_in_days)
{
    const Database::Result result = _ctx.get_conn().exec_params(
            "WITH comm_type_letter AS (SELECT id FROM comm_type WHERE type='letter'),"
            "message_types AS (SELECT id FROM message_type WHERE type IN ('mojeid_pin3',"
            "'mojeid_card')),"
            "send_states_ignore AS (SELECT id FROM enum_send_status WHERE status_name='no_processing') "
            "SELECT ma.moddate+($3::TEXT||'DAYS')::INTERVAL "
            "FROM message_archive ma "
            "JOIN message_contact_history_map mc ON mc.message_archive_id=ma.id "
            "WHERE ma.message_type_id IN (SELECT id FROM message_types) AND "
            "ma.comm_type_id=(SELECT id FROM comm_type_letter) AND "
            "ma.status_id NOT IN (SELECT id FROM send_states_ignore) AND "
            "(NOW()-($3::TEXT||'DAYS')::INTERVAL)::DATE<ma.moddate::DATE AND "
            "mc.contact_object_registry_id=$1::BIGINT "
            "ORDER BY 1 DESC OFFSET ($2::INTEGER-1) LIMIT 1",
            Database::query_param_list(_contact_id) // used as $1::BIGINT
            (_max_sent_letters) // used as $2::INTEGER
            (_watched_period_in_days)); // used as $3::TEXT
    if (0 < result.size())
    {
        MojeIdImplData::MessageLimitExceeded e;
        e.limit_expire_datetime = boost::posix_time::time_from_string(static_cast<std::string>(result[0][0]));
        throw e;
    }
}

template <typename T>
auto missing_value_differs_from_existing(T&&);

template <typename C>
class MissingValueDiffersFromExisting
{
public:
    MissingValueDiffersFromExisting(MissingValueDiffersFromExisting&& src)
        : equal_to_(std::move(src.equal_to_)) { }
    template <typename T>
    bool operator()(const Nullable<T>& lhs, const Nullable<T>& rhs)const
    {
        return (lhs.isnull() == rhs.isnull()) &&
               (lhs.isnull() || equal_to_(lhs.get_value(), rhs.get_value()));
    }
    template <typename T>
    bool operator()(const Optional<T>& lhs, const Optional<T>& rhs)const
    {
        return (lhs.is_set() == rhs.is_set()) &&
               (!lhs.is_set() || equal_to_(lhs.get_value(), rhs.get_value()));
    }
private:
    explicit MissingValueDiffersFromExisting(C&& comparator)
        : equal_to_(std::move(comparator)) { }
    C equal_to_;
    template <typename T>
    friend auto missing_value_differs_from_existing(T&&);
};

template <typename T>
auto missing_value_as_empty(T&&);

template <typename C>
class MissingValueAsEmpty
{
public:
    MissingValueAsEmpty(MissingValueAsEmpty&& src)
        : equal_to_(std::move(src.equal_to_)) { }
    template <typename T>
    bool operator()(const Nullable<T>& lhs, const Nullable<T>& rhs)const
    {
        if (lhs.isnull() == rhs.isnull())
        {
            return lhs.isnull() || equal_to_(lhs.get_value(), rhs.get_value());
        }
        if (lhs.isnull())
        {
            return rhs.get_value().empty();
        }
        return lhs.get_value().empty();
    }
    template <typename T>
    bool operator()(const Optional<T>& lhs, const Optional<T>& rhs)const
    {
        if (lhs.is_set() == rhs.is_set())
        {
            return !lhs.is_set() || equal_to_(lhs.get_value(), rhs.get_value());
        }
        if (lhs.is_set())
        {
            return lhs.get_value().empty();
        }
        return rhs.get_value().empty();
    }
private:
    explicit MissingValueAsEmpty(C&& comparator)
        : equal_to_(std::move(comparator)) { }
    C equal_to_;
    template <typename T>
    friend auto missing_value_as_empty(T&&);
};

template <typename T>
auto missing_value_differs_from_existing(T&& comparator)
{
    return MissingValueDiffersFromExisting<std::remove_reference_t<T>>(std::forward<T>(comparator));
}

template <typename T>
auto missing_value_as_empty(T&& comparator)
{
    return MissingValueAsEmpty<std::remove_reference_t<T>>(std::forward<T>(comparator));
}

template <typename L, typename R, typename C>
bool does_it_differ(L&& a, R&& b, C&& equal_to)
{
    return !std::forward<C>(equal_to)(std::forward<L>(a), std::forward<R>(b));
}

template <typename L, typename R>
bool does_it_differ(L&& a, R&& b)
{
    return does_it_differ(std::forward<L>(a), std::forward<R>(b), std::equal_to<void>());
}

bool does_it_differ(const Nullable<std::string>& a, const Nullable<std::string>& b)
{
    return does_it_differ(a, b, missing_value_as_empty(std::equal_to<void>()));
}

bool does_it_differ(const Optional<std::string>& a, const Optional<std::string>& b)
{
    return does_it_differ(a, b, missing_value_as_empty(std::equal_to<void>()));
}

template <typename T>
auto case_insensitive(T&& db_conn)
{
    return Util::case_insensitive_equal_to(std::forward<T>(db_conn));
}

Nullable<boost::gregorian::date> convert_as_birthdate(const Nullable<std::string>& _birth_date)
{
    if (!_birth_date.isnull())
    {
        return Nullable<boost::gregorian::date>(birthdate_from_string_to_date(_birth_date.get_value()));
    }
    return Nullable<boost::gregorian::date>();
}

template <typename C>
bool validated_data_changed(C&& db_conn, const LibFred::InfoContactData& _c1, const LibFred::InfoContactData& _c2)
{
    if (does_it_differ(_c1.name, _c2.name, missing_value_as_empty(case_insensitive(std::forward<C>(db_conn)))))
    {
        LOGGER.debug("case insensitive change of name detected");
        return true;
    }

    if (does_it_differ(_c1.organization, _c2.organization))
    {
        LOGGER.debug("change of organization detected");
        return true;
    }

    const LibFred::InfoContactData::Address a1 = _c1.get_permanent_address();
    const LibFred::InfoContactData::Address a2 = _c2.get_permanent_address();
    if (does_it_differ(a1.street1, a2.street1) ||
        does_it_differ(a1.street2, a2.street2) ||
        does_it_differ(a1.street3, a2.street3) ||
        does_it_differ(a1.city, a2.city) ||
        does_it_differ(a1.stateorprovince, a2.stateorprovince) ||
        does_it_differ(a1.country, a2.country) ||
        does_it_differ(a1.postalcode, a2.postalcode))
    {
        LOGGER.debug("change of address detected");
        return true;
    }

    if (does_it_differ(_c1.ssntype, _c2.ssntype))
    {
        LOGGER.debug("change of ssntype detected");
        return true;
    }

    if (does_it_differ(_c1.ssn, _c2.ssn))
    {
        if (_c1.ssntype.get_value_or_default() != Conversion::Enums::to_db_handle(LibFred::SSNType::birthday))
        {
            LOGGER.debug("unexpected ssntype detected");
            return true;
        }
        const Nullable<boost::gregorian::date> bd1 = convert_as_birthdate(_c1.ssn);
        const Nullable<boost::gregorian::date> bd2 = convert_as_birthdate(_c2.ssn);
        if (does_it_differ(bd1, bd2))
        {
            LOGGER.debug("change of birthdate detected");
            return true;
        }
    }
    return false;
}

bool notification_enabled()
{
    return CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>()->notify_commands;
}

void notify(LibFred::OperationContext& _ctx,
        const Notification::notified_event _event,
        unsigned long long _done_by_registrar,
        unsigned long long _object_historyid_post_change,
        MojeIdImpl::LogRequestId _log_request_id)
{
    if (notification_enabled())
    {
        Notification::enqueue_notification(_ctx, _event, _done_by_registrar, _object_historyid_post_change, ::Util::make_svtrid(_log_request_id));
    }
}

struct MessageType
{
    enum Enum //message_type table
    {
        domain_expiration,
        mojeid_pin2,
        mojeid_pin3,
        mojeid_sms_change,
        monitoring,
        contact_verification_pin2,
        contact_verification_pin3,
        mojeid_pin3_reminder,
        contact_check_notice,
        contact_check_thank_you,
        mojeid_card
    };
};

struct CommType
{
    enum Enum //comm_type table
    {
        email,
        letter,
        sms,
        registered_letter
    };
};

struct SendStatus
{
    enum Enum //enum_send_status table
    {
        ready,
        waiting_confirmation,
        no_processing,
        send_failed,
        sent,
        being_sent,
        undelivered
    };
};

struct PubReqType
{
    enum Enum //subset of enum_public_request_type table
    {
        contact_conditional_identification,
        conditionally_identified_contact_transfer,
        identified_contact_transfer,
        prevalidated_unidentified_contact_transfer,
        prevalidated_contact_transfer
    };
};

} // namespace Fred::Backend::MojeId::{anonymous}
} // namespace Fred::Backend::MojeId
} // namespace Fred::Backend
} // namespace Fred

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(Fred::Backend::MojeId::MessageType::Enum value)
{
    switch (value)
    {
        case Fred::Backend::MojeId::MessageType::domain_expiration:
            return "domain_expiration";
        case Fred::Backend::MojeId::MessageType::mojeid_pin2:
            return "mojeid_pin2";
        case Fred::Backend::MojeId::MessageType::mojeid_pin3:
            return "mojeid_pin3";
        case Fred::Backend::MojeId::MessageType::mojeid_sms_change:
            return "mojeid_sms_change";
        case Fred::Backend::MojeId::MessageType::monitoring:
            return "monitoring";
        case Fred::Backend::MojeId::MessageType::contact_verification_pin2:
            return "contact_verification_pin2";
        case Fred::Backend::MojeId::MessageType::contact_verification_pin3:
            return "contact_verification_pin3";
        case Fred::Backend::MojeId::MessageType::mojeid_pin3_reminder:
            return "mojeid_pin3_reminder";
        case Fred::Backend::MojeId::MessageType::contact_check_notice:
            return "contact_check_notice";
        case Fred::Backend::MojeId::MessageType::contact_check_thank_you:
            return "contact_check_thank_you";
        case Fred::Backend::MojeId::MessageType::mojeid_card:
            return "mojeid_card";
    }
    throw std::invalid_argument("value doesn't exist in Fred::Backend::MojeId::MessageType::{anonymous}::Enum");
}

template <>
inline Fred::Backend::MojeId::MessageType::Enum from_db_handle<Fred::Backend::MojeId::MessageType>(const std::string& db_handle)
{
    if (to_db_handle(Fred::Backend::MojeId::MessageType::domain_expiration) == db_handle)
    {
        return Fred::Backend::MojeId::MessageType::domain_expiration;
    }
    if (to_db_handle(Fred::Backend::MojeId::MessageType::mojeid_pin2) == db_handle)
    {
        return Fred::Backend::MojeId::MessageType::mojeid_pin2;
    }
    if (to_db_handle(Fred::Backend::MojeId::MessageType::mojeid_pin3) == db_handle)
    {
        return Fred::Backend::MojeId::MessageType::mojeid_pin3;
    }
    if (to_db_handle(Fred::Backend::MojeId::MessageType::mojeid_sms_change) == db_handle)
    {
        return Fred::Backend::MojeId::MessageType::mojeid_sms_change;
    }
    if (to_db_handle(Fred::Backend::MojeId::MessageType::monitoring) == db_handle)
    {
        return Fred::Backend::MojeId::MessageType::monitoring;
    }
    if (to_db_handle(Fred::Backend::MojeId::MessageType::contact_verification_pin2) == db_handle)
    {
        return Fred::Backend::MojeId::MessageType::contact_verification_pin2;
    }
    if (to_db_handle(Fred::Backend::MojeId::MessageType::contact_verification_pin3) == db_handle)
    {
        return Fred::Backend::MojeId::MessageType::contact_verification_pin3;
    }
    if (to_db_handle(Fred::Backend::MojeId::MessageType::mojeid_pin3_reminder) == db_handle)
    {
        return Fred::Backend::MojeId::MessageType::mojeid_pin3_reminder;
    }
    if (to_db_handle(Fred::Backend::MojeId::MessageType::contact_check_notice) == db_handle)
    {
        return Fred::Backend::MojeId::MessageType::contact_check_notice;
    }
    if (to_db_handle(Fred::Backend::MojeId::MessageType::contact_check_thank_you) == db_handle)
    {
        return Fred::Backend::MojeId::MessageType::contact_check_thank_you;
    }
    if (to_db_handle(Fred::Backend::MojeId::MessageType::mojeid_card) == db_handle)
    {
        return Fred::Backend::MojeId::MessageType::mojeid_card;
    }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Fred::Backend::MojeId::MessageType::{anonymous}::Enum");
}

inline std::string to_db_handle(Fred::Backend::MojeId::CommType::Enum value)
{
    switch (value)
    {
        case Fred::Backend::MojeId::CommType::email:
            return "email";
        case Fred::Backend::MojeId::CommType::letter:
            return "letter";
        case Fred::Backend::MojeId::CommType::sms:
            return "sms";
        case Fred::Backend::MojeId::CommType::registered_letter:
            return "registered_letter";
    }
    throw std::invalid_argument("value doesn't exist in Fred::Backend::MojeId::CommType::{anonymous}::Enum");
}

template <>
inline Fred::Backend::MojeId::CommType::Enum from_db_handle<Fred::Backend::MojeId::CommType>(const std::string& db_handle)
{
    if (to_db_handle(Fred::Backend::MojeId::CommType::email) == db_handle)
    {
        return Fred::Backend::MojeId::CommType::email;
    }
    if (to_db_handle(Fred::Backend::MojeId::CommType::letter) == db_handle)
    {
        return Fred::Backend::MojeId::CommType::letter;
    }
    if (to_db_handle(Fred::Backend::MojeId::CommType::sms) == db_handle)
    {
        return Fred::Backend::MojeId::CommType::sms;
    }
    if (to_db_handle(Fred::Backend::MojeId::CommType::registered_letter) == db_handle)
    {
        return Fred::Backend::MojeId::CommType::registered_letter;
    }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Fred::Backend::MojeId::CommType::{anonymous}::Enum");
}

inline std::string to_db_handle(Fred::Backend::MojeId::SendStatus::Enum value)
{
    switch (value)
    {
        case Fred::Backend::MojeId::SendStatus::ready:
            return "ready";
        case Fred::Backend::MojeId::SendStatus::waiting_confirmation:
            return "waiting_confirmation";
        case Fred::Backend::MojeId::SendStatus::no_processing:
            return "no_processing";
        case Fred::Backend::MojeId::SendStatus::send_failed:
            return "send_failed";
        case Fred::Backend::MojeId::SendStatus::sent:
            return "sent";
        case Fred::Backend::MojeId::SendStatus::being_sent:
            return "being_sent";
        case Fred::Backend::MojeId::SendStatus::undelivered:
            return "undelivered";
    }
    throw std::invalid_argument("value doesn't exist in Fred::Backend::MojeId::SendStatus::{anonymous}::Enum");
}

template <>
inline Fred::Backend::MojeId::SendStatus::Enum from_db_handle<Fred::Backend::MojeId::SendStatus>(const std::string& db_handle)
{
    if (to_db_handle(Fred::Backend::MojeId::SendStatus::ready) == db_handle)
    {
        return Fred::Backend::MojeId::SendStatus::ready;
    }
    if (to_db_handle(Fred::Backend::MojeId::SendStatus::waiting_confirmation) == db_handle)
    {
        return Fred::Backend::MojeId::SendStatus::waiting_confirmation;
    }
    if (to_db_handle(Fred::Backend::MojeId::SendStatus::no_processing) == db_handle)
    {
        return Fred::Backend::MojeId::SendStatus::no_processing;
    }
    if (to_db_handle(Fred::Backend::MojeId::SendStatus::send_failed) == db_handle)
    {
        return Fred::Backend::MojeId::SendStatus::send_failed;
    }
    if (to_db_handle(Fred::Backend::MojeId::SendStatus::sent) == db_handle)
    {
        return Fred::Backend::MojeId::SendStatus::sent;
    }
    if (to_db_handle(Fred::Backend::MojeId::SendStatus::being_sent) == db_handle)
    {
        return Fred::Backend::MojeId::SendStatus::being_sent;
    }
    if (to_db_handle(Fred::Backend::MojeId::SendStatus::undelivered) == db_handle)
    {
        return Fred::Backend::MojeId::SendStatus::undelivered;
    }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Fred::Backend::MojeId::SendStatus::{anonymous}::Enum");
}

inline std::string to_db_handle(Fred::Backend::MojeId::PubReqType::Enum value)
{
    switch (value)
    {
        case Fred::Backend::MojeId::PubReqType::contact_conditional_identification:
            return Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification().get_public_request_type();
        case Fred::Backend::MojeId::PubReqType::conditionally_identified_contact_transfer:
            return Fred::Backend::MojeId::PublicRequest::ConditionallyIdentifiedContactTransfer().get_public_request_type();
        case Fred::Backend::MojeId::PubReqType::identified_contact_transfer:
            return Fred::Backend::MojeId::PublicRequest::IdentifiedContactTransfer().get_public_request_type();
        case Fred::Backend::MojeId::PubReqType::prevalidated_contact_transfer:
            return Fred::Backend::MojeId::PublicRequest::PrevalidatedContactTransfer().get_public_request_type();
        case Fred::Backend::MojeId::PubReqType::prevalidated_unidentified_contact_transfer:
            return Fred::Backend::MojeId::PublicRequest::PrevalidatedUnidentifiedContactTransfer().get_public_request_type();
    }
    throw std::invalid_argument("value doesn't exist in Fred::Backend::MojeId::PubReqType::{anonymous}::Enum");
}

template <>
inline Fred::Backend::MojeId::PubReqType::Enum from_db_handle<Fred::Backend::MojeId::PubReqType>(const std::string& db_handle)
{
    if (to_db_handle(Fred::Backend::MojeId::PubReqType::contact_conditional_identification) == db_handle)
    {
        return Fred::Backend::MojeId::PubReqType::contact_conditional_identification;
    }
    if (to_db_handle(Fred::Backend::MojeId::PubReqType::conditionally_identified_contact_transfer) == db_handle)
    {
        return Fred::Backend::MojeId::PubReqType::conditionally_identified_contact_transfer;
    }
    if (to_db_handle(Fred::Backend::MojeId::PubReqType::identified_contact_transfer) == db_handle)
    {
        return Fred::Backend::MojeId::PubReqType::identified_contact_transfer;
    }
    if (to_db_handle(Fred::Backend::MojeId::PubReqType::prevalidated_contact_transfer) == db_handle)
    {
        return Fred::Backend::MojeId::PubReqType::prevalidated_contact_transfer;
    }
    if (to_db_handle(Fred::Backend::MojeId::PubReqType::prevalidated_unidentified_contact_transfer) == db_handle)
    {
        return Fred::Backend::MojeId::PubReqType::prevalidated_unidentified_contact_transfer;
    }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Fred::Backend::MojeId::PubReqType::{anonymous}::Enum");
}

} // namespace Conversion::Enums
} // namespace Conversion

namespace Fred {
namespace Backend {
namespace MojeId {

namespace {

template <MessageType::Enum MT, CommType::Enum CT>
::size_t cancel_message_sending(LibFred::OperationContext& _ctx, MojeIdImpl::ContactId _contact_id)
{
    const Database::Result result = _ctx.get_conn().exec_params(
            // clang-format off
            "UPDATE message_archive ma "
            "SET moddate=NOW(),"
                "status_id=(SELECT id FROM enum_send_status WHERE status_name=$4::TEXT) "
            "FROM message_contact_history_map mchm "
            "JOIN letter_archive la ON la.id=mchm.message_archive_id "
            "WHERE mchm.message_archive_id=ma.id AND "
                  "mchm.contact_object_registry_id=$1::BIGINT AND "
                  "ma.status_id IN (SELECT id FROM enum_send_status "
                                   "WHERE status_name IN ($5::TEXT,$6::TEXT)) AND "
                  "ma.comm_type_id=(SELECT id FROM comm_type WHERE type=$2::TEXT) AND "
                  "ma.message_type_id=(SELECT id FROM message_type WHERE type=$3::TEXT) "
            "RETURNING ma.id",
            // clang-format on
            Database::query_param_list
                    (_contact_id) //$1::BIGINT
                    (Conversion::Enums::to_db_handle(CT)) //$2::TEXT
                    (Conversion::Enums::to_db_handle(MT)) //$3::TEXT
                    (Conversion::Enums::to_db_handle(SendStatus::no_processing)) //$4::TEXT
                    (Conversion::Enums::to_db_handle(SendStatus::send_failed)) //$5::TEXT
                    (Conversion::Enums::to_db_handle(SendStatus::ready))); //$6::TEXT
    return result.size();
}

template <typename C>
bool identified_data_changed(C&& db_conn, const LibFred::InfoContactData& _c1, const LibFred::InfoContactData& _c2)
{
    if (does_it_differ(_c1.name, _c2.name, missing_value_as_empty(case_insensitive(std::forward<C>(db_conn)))))
    {
        return true;
    }

    const LibFred::InfoContactData::Address a1 = _c1.get_address<LibFred::ContactAddressType::MAILING>();
    const LibFred::InfoContactData::Address a2 = _c2.get_address<LibFred::ContactAddressType::MAILING>();
    if (does_it_differ(a1.name, a2.name, missing_value_differs_from_existing(case_insensitive(std::forward<C>(db_conn)))))
    {
        const std::string name1 = a1.name.isset() ? a1.name.get_value() : _c1.name.get_value_or_default();
        const std::string name2 = a2.name.isset() ? a2.name.get_value() : _c2.name.get_value_or_default();
        if (does_it_differ(name1, name2, case_insensitive(std::forward<C>(db_conn))))
        {
            return true;
        }
    }
    if (does_it_differ(a1.street1, a2.street1) ||
        does_it_differ(a1.street2, a2.street2) ||
        does_it_differ(a1.street3, a2.street3) ||
        does_it_differ(a1.city, a2.city) ||
        does_it_differ(a1.stateorprovince, a2.stateorprovince) ||
        does_it_differ(a1.country, a2.country) ||
        does_it_differ(a1.postalcode, a2.postalcode))
    {
        return true;
    }
    return false;
}

typedef data_storage<std::string, MojeIdImpl::ContactId>::safe prepare_transaction_storage;
typedef prepare_transaction_storage::object_type::data_not_found prepare_transaction_data_not_found;

} // namespace Fred::Backend::MojeId::{anonymous}

MojeIdImpl::MojeIdImpl(const std::string& _server_name)
    : server_name_(_server_name),
      mojeid_registrar_(get_mojeid_registrar()),
      system_registrar_(get_system_registrar())
{
    LogContext log_ctx(*this, "init");
}

MojeIdImpl::~MojeIdImpl()
{
}

const std::string& MojeIdImpl::get_server_name() const
{
    return server_name_;
}

void MojeIdImpl::get_unregistrable_contact_handles(
        MojeIdImplData::ContactHandleList& _result) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        const Database::Result dbres =
                ctx.get_conn().exec(
                        // clang-format off
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
                        // clang-format on
        _result.clear();
        _result.reserve(dbres.size());
        for (::size_t idx = 0; idx < dbres.size(); ++idx)
        {
            _result.push_back(static_cast<std::string>(dbres[idx][0]));
        }
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

namespace {

Optional<MojeIdImpl::LogRequestId> get_optional_log_request_id(MojeIdImpl::LogRequestId _log_request_id)
{
    if (0 < _log_request_id)
    {
        return _log_request_id;
    }
    return Optional<MojeIdImpl::LogRequestId>();
}

} // namespace Fred::Backend::MojeId::{anonymous}

MojeIdImpl::ContactId MojeIdImpl::create_contact_prepare(
        const MojeIdImplData::CreateContact& _contact,
        const std::string& _trans_id,
        MojeIdImpl::LogRequestId _log_request_id,
        std::string& _ident) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);

        LibFred::InfoContactData info_contact_data;
        from_into(_contact, info_contact_data);
        {
            const MojeIdImplInternal::CheckCreateContactPrepare check_contact_data(
                    make_args(info_contact_data),
                    make_args(info_contact_data, ctx));

            if (!check_contact_data.success())
            {
                MojeIdImplInternal::raise(check_contact_data);
            }
        }

        LibFred::CreateContact op_create_contact(_contact.username, mojeid_registrar_.handle());
        set_create_contact_arguments(_contact, op_create_contact);
        if (0 < _log_request_id)
        {
            op_create_contact.set_logd_request_id(_log_request_id);
        }
        const LibFred::CreateContact::Result new_contact = op_create_contact.exec(ctx);
        LibFred::CreatePublicRequestAuth op_create_pub_req;
        op_create_pub_req.set_registrar_id(mojeid_registrar_.id());
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, new_contact.create_object_result.object_id);
        {
            const LibFred::CreatePublicRequestAuth::Result result = op_create_pub_req.exec(
                    locked_contact, Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification().iface(), get_optional_log_request_id(_log_request_id));
            _ident = result.identification;
            notify(ctx, Notification::created, mojeid_registrar_.id(), new_contact.create_object_result.history_id, _log_request_id);
        }
        prepare_transaction_storage()->store(_trans_id, new_contact.create_object_result.object_id);
        ctx.commit_transaction();
        return new_contact.create_object_result.object_id;
    }
    catch (const MojeIdImplData::RegistrationValidationResult& e)
    {
        LOGGER.info("request failed (incorrect input data)");
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

namespace {

LibFred::CreatePublicRequestAuth::Result action_transfer_contact_prepare(
        const LibFred::PublicRequestAuthTypeIface& _iface,
        const std::string& _trans_id,
        const LibFred::InfoContactData& _contact,
        const LibFred::LockedPublicRequestsOfObjectForUpdate& _locked_contact,
        unsigned long long _registrar_id,
        MojeIdImpl::LogRequestId _log_request_id)
{
    LibFred::CreatePublicRequestAuth op_create_pub_req;
    if (!_contact.notifyemail.isnull())
    {
        op_create_pub_req.set_email_to_answer(_contact.notifyemail.get_value());
    }
    op_create_pub_req.set_registrar_id(_registrar_id);
    const LibFred::CreatePublicRequestAuth::Result result =
            op_create_pub_req.exec(_locked_contact, _iface, get_optional_log_request_id(_log_request_id));
    prepare_transaction_storage()->store(_trans_id, _contact.id);
    return result;
}

bool is_identity_attached(const LibFred::OperationContext& ctx, unsigned long long contact_id)
{
    const auto db_res = ctx.get_conn().exec_params(
            "SELECT EXISTS(SELECT 0 "
                          "FROM contact_identity ci "
                          "WHERE ci.contact_id = obr.id AND "
                                "ci.valid_to IS NULL) "
            "FROM object_registry obr "
            "WHERE obr.id = $1::BIGINT AND "
                  "obr.erdate IS NULL AND "
                  "obr.type = get_object_type_id('contact') "
            "FOR UPDATE OF obr",
            Database::query_param_list{contact_id});
    if (db_res.size() == 0)
    {
        throw MojeIdImplData::ObjectDoesntExist{};
    }
    if (1 < db_res.size())
    {
        throw std::runtime_error{"too many contacts with a given id"};
    }
    return static_cast<bool>(db_res[0][0]);
}

} // namespace Fred::Backend::MojeId::{anonymous}

void MojeIdImpl::transfer_contact_prepare(
        const std::string& _handle,
        const std::string& _trans_id,
        MojeIdImpl::LogRequestId _log_request_id,
        MojeIdImplData::InfoContact& _contact,
        std::string& _ident) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        const LibFred::InfoContactData contact = LibFred::InfoContactByHandle(_handle).exec(ctx).info_contact_data;
        if (is_identity_attached(ctx, contact.id))
        {
            throw MojeIdImplData::IdentityAttached{};
        }
        const LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, contact.id);
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(contact.id).exec(ctx));
        {
            const MojeIdImplInternal::CheckTransferContactPrepareStates check_result(states);
            if (!check_result.success())
            {
                MojeIdImplInternal::raise(check_result);
            }
        }
        {
            const MojeIdImplInternal::CheckMojeIdRegistration check_result(
                    make_args(contact), make_args(contact, ctx));
            if (!check_result.success())
            {
                MojeIdImplInternal::raise(check_result);
            }
        }

        LibFred::CreatePublicRequestAuth::Result pub_req_result;
        if (states.absents(LibFred::Object_State::conditionally_identified_contact) &&
                states.absents(LibFred::Object_State::identified_contact) &&
                states.absents(LibFred::Object_State::validated_contact))
        {
            pub_req_result = action_transfer_contact_prepare(
                    Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification(),
                    _trans_id,
                    contact,
                    locked_contact,
                    mojeid_registrar_.id(),
                    _log_request_id);
        }
        else if (states.presents(LibFred::Object_State::conditionally_identified_contact) &&
                 states.absents(LibFred::Object_State::identified_contact) &&
                 states.absents(LibFred::Object_State::validated_contact))
        {
            pub_req_result = action_transfer_contact_prepare(
                    Fred::Backend::MojeId::PublicRequest::ConditionallyIdentifiedContactTransfer(),
                    _trans_id,
                    contact,
                    locked_contact,
                    mojeid_registrar_.id(),
                    _log_request_id);
        }
        else if (states.presents(LibFred::Object_State::conditionally_identified_contact) &&
                 states.presents(LibFred::Object_State::identified_contact) &&
                 states.absents(LibFred::Object_State::validated_contact))
        {
            pub_req_result = action_transfer_contact_prepare(
                    Fred::Backend::MojeId::PublicRequest::IdentifiedContactTransfer(),
                    _trans_id,
                    contact,
                    locked_contact,
                    mojeid_registrar_.id(),
                    _log_request_id);
        }

        from_into(contact, _contact);
        ctx.commit_transaction();
        _ident = pub_req_result.identification;
        return;
    }
    catch (const MojeIdImplData::AlreadyMojeidContact&)
    {
        LOGGER.info("request failed (incorrect input data - AlreadyMojeidContact)");
        throw;
    }
    catch (const MojeIdImplData::IdentityAttached&)
    {
        LOGGER.info("request failed (incorrect input data - IdentityAttached)");
        throw;
    }
    catch (const MojeIdImplData::ObjectAdminBlocked&)
    {
        LOGGER.info("request failed (incorrect input data - ObjectAdminBlocked)");
        throw;
    }
    catch (const MojeIdImplData::ObjectUserBlocked&)
    {
        LOGGER.info("request failed (incorrect input data - ObjectUserBlocked)");
        throw;
    }
    catch (const MojeIdImplData::RegistrationValidationResult&)
    {
        LOGGER.info("request failed (incorrect input data - RegistrationValidationResult)");
        throw;
    }
    catch (const LibFred::InfoContactByHandle::Exception& e)
    {
        if (e.is_set_unknown_contact_handle())
        {
            LOGGER.info("request failed (incorrect input data)");
            throw MojeIdImplData::ObjectDoesntExist();
        }
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

namespace {

template <LibFred::ContactAddressType::Value ADDRESS_TYPE, typename UPDATE_CONTACT>
void update_address(const LibFred::ContactAddressList& _old_addresses,
        const LibFred::ContactAddressList& _new_addresses,
        LibFred::UpdateContact<UPDATE_CONTACT>& _update_op)
{
    LibFred::ContactAddressList::const_iterator old_ptr = _old_addresses.find(ADDRESS_TYPE);
    const bool old_presents = old_ptr != _old_addresses.end();
    LibFred::ContactAddressList::const_iterator new_ptr = _new_addresses.find(ADDRESS_TYPE);
    const bool new_presents = new_ptr != _new_addresses.end();
    if (new_presents)
    {
        if (!old_presents || (old_ptr->second != new_ptr->second))
        {
            _update_op.template set_address<ADDRESS_TYPE>(new_ptr->second);
        }
        return;
    }
    if (old_presents)
    {
        _update_op.template reset_address<ADDRESS_TYPE>();
    }
}

template <LibFred::ContactAddressType::Value ADDRESS_TYPE, typename UPDATE_CONTACT>
void update_address(const LibFred::InfoContactDiff& _data_changes,
        LibFred::UpdateContact<UPDATE_CONTACT>& _update_op)
{
    update_address<ADDRESS_TYPE>(_data_changes.addresses.get_value().first,
            _data_changes.addresses.get_value().second,
            _update_op);
}

template <typename UPDATE_CONTACT>
void set_update_contact_op(const LibFred::InfoContactDiff& _data_changes,
        LibFred::UpdateContact<UPDATE_CONTACT>& _update_op)
{
    if (_data_changes.name.isset())
    {
        _update_op.set_name(_data_changes.name.get_value().second);
    }
    if (_data_changes.organization.isset())
    {
        _update_op.set_organization(_data_changes.organization.get_value().second);
    }
    if (_data_changes.vat.isset())
    {
        _update_op.set_vat(_data_changes.vat.get_value().second);
    }
    if (_data_changes.personal_id.isset())
    {
        _update_op.set_personal_id(_data_changes.personal_id.get_value().second);
    }
    if (_data_changes.place.isset())
    {
        _update_op.set_place(_data_changes.place.get_value().second);
    }
    if (_data_changes.addresses.isset())
    {
        update_address<LibFred::ContactAddressType::MAILING>(_data_changes, _update_op);
        update_address<LibFred::ContactAddressType::BILLING>(_data_changes, _update_op);
        update_address<LibFred::ContactAddressType::SHIPPING>(_data_changes, _update_op);
        update_address<LibFred::ContactAddressType::SHIPPING_2>(_data_changes, _update_op);
        update_address<LibFred::ContactAddressType::SHIPPING_3>(_data_changes, _update_op);
    }
    if (_data_changes.email.isset())
    {
        _update_op.set_email(_data_changes.email.get_value().second);
    }
    if (_data_changes.notifyemail.isset())
    {
        _update_op.set_notifyemail(_data_changes.notifyemail.get_value().second);
    }
    if (_data_changes.telephone.isset())
    {
        _update_op.set_telephone(_data_changes.telephone.get_value().second);
    }
    if (_data_changes.fax.isset())
    {
        _update_op.set_fax(_data_changes.fax.get_value().second);
    }
}

} // namespace Fred::Backend::MojeId::{anonymous}

void MojeIdImpl::update_contact_prepare(
        ContactId _contact_id,
        const MojeIdImplData::UpdateContact& _new_data,
        const std::string& _trans_id,
        MojeIdImpl::LogRequestId _log_request_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::InfoContactData new_data;
        from_into(_new_data, new_data);
        new_data.id = _contact_id;
        LibFred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(new_data.id).exec(ctx));
        if (states.absents(LibFred::Object_State::mojeid_contact))
        {
            throw MojeIdImplData::ObjectDoesntExist();
        }
        const LibFred::InfoContactData current_data = LibFred::InfoContactById(new_data.id).exec(ctx).info_contact_data;
        const LibFred::InfoContactDiff data_changes = LibFred::diff_contact_data(current_data, new_data);
        if (!(data_changes.name.isset() ||
              data_changes.organization.isset() ||
              data_changes.vat.isset() ||
              data_changes.personal_id.isset() ||
              data_changes.place.isset() ||
              data_changes.addresses.isset() ||
              data_changes.email.isset() ||
              data_changes.notifyemail.isset() ||
              data_changes.telephone.isset() ||
              data_changes.fax.isset()))
        {
            ctx.commit_transaction();
            return;
        }
        const LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, new_data.id);
        LibFred::StatusList to_cancel;
        bool drop_validation = false;
        if (states.presents(LibFred::Object_State::validated_contact))
        {
            drop_validation = validated_data_changed(ctx.get_conn(), current_data, new_data);
            if (drop_validation)
            {
                to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::validated_contact));
            }
        }
        const bool drop_identification = identified_data_changed(ctx.get_conn(), current_data, new_data);
        if (drop_identification || does_it_differ(current_data.email, new_data.email))
        {
            cancel_message_sending<MessageType::mojeid_card, CommType::letter>(ctx, new_data.id);
            cancel_message_sending<MessageType::mojeid_pin3, CommType::letter>(ctx, new_data.id);
        }
        if (drop_identification)
        {
            bool reidentification_needed = states.presents(LibFred::Object_State::identified_contact);
            if (reidentification_needed)
            {
                to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::identified_contact));
            }
            else
            {
                const Database::Result dbres =
                        ctx.get_conn().exec_params(
                                // clang-format off
                                "SELECT 1 "
                                "FROM object_registry obr "
                                "JOIN object_state os ON os.object_id=obr.id AND "
                                                        "os.state_id=(SELECT id FROM enum_object_states WHERE name=$2::TEXT) "
                                "WHERE obr.id=$1::BIGINT AND "
                                      "os.valid_to IS NULL AND "
                                      "EXISTS(SELECT * FROM object_state "
                                             "WHERE object_id=obr.id AND "
                                                   "state_id=(SELECT id FROM enum_object_states WHERE name=$3::TEXT) AND "
                                                   "(os.valid_from<=valid_from OR "
                                                   " os.valid_from<valid_to))",
                                // clang-format on
                                Database::query_param_list
                                        (new_data.id) //$1::BIGINT
                                        (Conversion::Enums::to_db_handle(LibFred::Object_State::mojeid_contact)) //$2::TEXT
                                        (Conversion::Enums::to_db_handle(LibFred::Object_State::identified_contact))); //$3::TEXT
                const bool contact_was_identified_in_the_past = 0 < dbres.size();
                if (contact_was_identified_in_the_past)
                {
                    reidentification_needed = true;
                }
            }
            const HandleMojeIdArgs* const server_conf_ptr = CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>();
            if (server_conf_ptr->auto_pin3_sending)
            {
                check_sent_letters_limit(
                        ctx,
                        new_data.id,
                        server_conf_ptr->letter_limit_count,
                        server_conf_ptr->letter_limit_interval);
                LibFred::CreatePublicRequestAuth create_public_request_op;
                create_public_request_op.set_reason("data changed");
                create_public_request_op.set_registrar_id(mojeid_registrar_.id());
                create_public_request_op.exec(
                        locked_contact,
                        reidentification_needed ? Fred::Backend::MojeId::PublicRequest::ContactReidentification().iface()
                                                : Fred::Backend::MojeId::PublicRequest::ContactIdentification().iface(),
                        get_optional_log_request_id(_log_request_id));
            }
        }
        {
            const MojeIdImplInternal::CheckUpdateContactPrepare check_contact_data(new_data);
            if (!check_contact_data.success())
            {
                throw check_contact_data;
            }
        }
        const bool manual_verification_done = states.presents(LibFred::Object_State::contact_failed_manual_verification) ||
                                              states.presents(LibFred::Object_State::contact_passed_manual_verification);
        if (manual_verification_done)
        {
            const bool name_changed = data_changes.name.isset() &&
                                      does_it_differ(
                                              current_data.name,
                                              new_data.name,
                                              missing_value_as_empty(case_insensitive(ctx.get_conn())));
            const bool cancel_manual_verification = name_changed ||
                                                    data_changes.organization.isset() ||
                                                    data_changes.personal_id.isset() ||
                                                    data_changes.place.isset() ||
                                                    data_changes.email.isset() ||
                                                    data_changes.notifyemail.isset() ||
                                                    data_changes.telephone.isset() ||
                                                    data_changes.fax.isset();
            if (cancel_manual_verification)
            {
                if (states.presents(LibFred::Object_State::contact_failed_manual_verification))
                {
                    to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::contact_failed_manual_verification));
                }
                if (states.presents(LibFred::Object_State::contact_passed_manual_verification))
                {
                    to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::contact_passed_manual_verification));
                }
            }
        }
        const bool object_states_changed = !to_cancel.empty();
        if (object_states_changed)
        {
            LibFred::CancelObjectStateRequestId(new_data.id, to_cancel).exec(ctx);
        }
        LibFred::UpdateContactById update_contact_op(new_data.id, mojeid_registrar_.handle());
        set_update_contact_op(data_changes, update_contact_op);
        const bool is_identified = states.presents(LibFred::Object_State::identified_contact) && !drop_identification;
        const bool is_validated = states.presents(LibFred::Object_State::validated_contact) && !drop_validation;
        const bool addr_can_be_hidden = (is_identified || is_validated) &&
                                        new_data.organization.get_value_or_default().empty();
        if (!addr_can_be_hidden)
        {
            update_contact_op.set_discloseaddress(true);
        }
        if (0 < _log_request_id)
        {
            update_contact_op.set_logd_request_id(_log_request_id);
        }
        const unsigned long long history_id = update_contact_op.exec(ctx);

        LibFred::Poll::CreateUpdateOperationPollMessage<LibFred::Object_Type::contact>().exec(ctx, history_id);
        notify(ctx, Notification::updated, mojeid_registrar_.id(), history_id, _log_request_id);

        if (object_states_changed)
        {
            prepare_transaction_storage()->store(_trans_id, new_data.id);
        }

        ctx.commit_transaction();
        return;
    }
    catch (const LibFred::InfoContactById::Exception& e)
    {
        if (e.is_set_unknown_object_id())
        {
            LOGGER.info("request failed (InfoContactById::Exception - unknown_object_id)");
            throw MojeIdImplData::ObjectDoesntExist();
        }
        LOGGER.error("request failed (InfoContactById::Exception)");
        throw;
    }
    catch (const MojeIdImplData::ObjectDoesntExist& e)
    {
        LOGGER.info("request failed (ObjectDoesntExist)");
        throw;
    }
    catch (const MojeIdImplData::MessageLimitExceeded& e)
    {
        LOGGER.info(e.as_string());
        throw;
    }
    catch (const MojeIdImplInternal::CheckUpdateContactPrepare& e)
    {
        LOGGER.info("request failed (CheckUpdateContactPrepare)");
        MojeIdImplInternal::raise(e);
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

void MojeIdImpl::update_validated_contact_prepare(
        ContactId contact_id,
        const MojeIdImplData::ValidatedContactData& verified_data,
        const std::string& _trans_id,
        LogRequestId _log_request_id)const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextTwoPhaseCommitCreator ctx{_trans_id};
        const LibFred::ObjectStatesInfo states{LibFred::GetObjectStates(contact_id).exec(ctx)};
        if (states.absents(LibFred::Object_State::mojeid_contact))
        {
            throw MojeIdImplData::ObjectDoesntExist{};
        }
        LibFred::InfoContactData verified_contact_data_subset;
        from_into(verified_data, verified_contact_data_subset);
        {
            const MojeIdImplInternal::CheckUpdateValidatedContactPrepare check_verified_data(verified_contact_data_subset);
            if (!check_verified_data.success())
            {
                throw check_verified_data;
            }
        }
        LibFred::StatusList to_cancel;
        const bool manual_verification_done = states.presents(LibFred::Object_State::contact_failed_manual_verification) ||
                                              states.presents(LibFred::Object_State::contact_passed_manual_verification);
        if (manual_verification_done)
        {
            if (states.presents(LibFred::Object_State::contact_failed_manual_verification))
            {
                to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::contact_failed_manual_verification));
            }
            if (states.presents(LibFred::Object_State::contact_passed_manual_verification))
            {
                to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::contact_passed_manual_verification));
            }
        }
        const bool object_states_changed = !to_cancel.empty();
        if (object_states_changed)
        {
            LibFred::CancelObjectStateRequestId(contact_id, to_cancel).exec(ctx);
        }
        LibFred::UpdateContactById update_contact_op(contact_id, mojeid_registrar_.handle());
        update_contact_op.set_name(verified_data.name);
        update_contact_op.set_personal_id(LibFred::PersonalIdUnion::get_BIRTHDAY(verified_data.birth_date.value));
        update_contact_op.set_place(verified_contact_data_subset.place);
        if (0 < _log_request_id)
        {
            update_contact_op.set_logd_request_id(_log_request_id);
        }
        const unsigned long long history_id = update_contact_op.exec(ctx);

        LibFred::Poll::CreateUpdateOperationPollMessage<LibFred::Object_Type::contact>().exec(ctx, history_id);
        notify(ctx, Notification::updated, mojeid_registrar_.id(), history_id, _log_request_id);

        if (object_states_changed)
        {
            prepare_transaction_storage()->store(_trans_id, contact_id);
        }

        ctx.commit_transaction();
        return;
    }
    catch (const LibFred::InfoContactById::Exception& e)
    {
        if (e.is_set_unknown_object_id())
        {
            LOGGER.info("request failed (InfoContactById::Exception - unknown_object_id)");
            throw MojeIdImplData::ObjectDoesntExist();
        }
        LOGGER.error("request failed (InfoContactById::Exception)");
        throw;
    }
    catch (const MojeIdImplData::ObjectDoesntExist& e)
    {
        LOGGER.info("request failed (ObjectDoesntExist)");
        throw;
    }
    catch (const MojeIdImplInternal::CheckUpdateValidatedContactPrepare& e)
    {
        LOGGER.info("request failed (CheckUpdateValidatedContactPrepare)");
        MojeIdImplInternal::raise(e);
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

MojeIdImplData::InfoContact MojeIdImpl::update_transfer_contact_prepare(
        const std::string& _username,
        const MojeIdImplData::UpdateTransferContact& _new_data,
        const std::string& _trans_id,
        MojeIdImpl::LogRequestId _log_request_id,
        std::string& _ident) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::InfoContactData new_data;
        from_into(_new_data, new_data);
        LibFred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        //check contact is registered
        const LibFred::InfoContactData current_data = LibFred::InfoContactByHandle(_username).exec(ctx).info_contact_data;
        if (is_identity_attached(ctx, current_data.id))
        {
            throw MojeIdImplData::IdentityAttached{};
        }
        new_data.id = current_data.id;
        new_data.handle = current_data.handle;
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(new_data.id).exec(ctx));
        {
            const MojeIdImplInternal::CheckTransferContactPrepareStates check_result(states);
            if (!check_result.success())
            {
                MojeIdImplInternal::raise(check_result);
            }
        }
        const LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, new_data.id);
        bool drop_identification = false;
        bool drop_cond_identification = false;
        bool drop_validation = false;
        unsigned long long history_id;
        {
            if (states.presents(LibFred::Object_State::identified_contact))
            {
                const LibFred::InfoContactData& c1 = current_data;
                const LibFred::InfoContactData& c2 = new_data;
                drop_identification = does_it_differ(
                        c1.name,
                        c2.name,
                        missing_value_as_empty(case_insensitive(ctx.get_conn())));
                if (!drop_identification)
                {
                    const LibFred::InfoContactData::Address a1 = c1.get_address<LibFred::ContactAddressType::MAILING>();
                    const LibFred::InfoContactData::Address a2 = c2.get_address<LibFred::ContactAddressType::MAILING>();
                    drop_identification =
                            does_it_differ(a1.name, a2.name, missing_value_as_empty(case_insensitive(ctx.get_conn()))) ||
                            does_it_differ(a1.organization, a2.organization) ||
                            does_it_differ(a1.company_name, a2.company_name) ||
                            does_it_differ(a1.street1, a2.street1) ||
                            does_it_differ(a1.street2, a2.street2) ||
                            does_it_differ(a1.street3, a2.street3) ||
                            does_it_differ(a1.city, a2.city) ||
                            does_it_differ(a1.stateorprovince, a2.stateorprovince) ||
                            does_it_differ(a1.postalcode, a2.postalcode) ||
                            does_it_differ(a1.country, a2.country);
                }
            }
            if (states.presents(LibFred::Object_State::conditionally_identified_contact) ||
                (!drop_identification && states.presents(LibFred::Object_State::identified_contact)))
            {
                const LibFred::InfoContactData& c1 = current_data;
                const LibFred::InfoContactData& c2 = new_data;
                drop_cond_identification =
                        does_it_differ(c1.telephone, c2.telephone) ||
                        does_it_differ(c1.email, c2.email);
                drop_identification |= drop_cond_identification;
            }
            if (states.presents(LibFred::Object_State::validated_contact))
            {
                drop_validation = validated_data_changed(ctx.get_conn(), current_data, new_data);
            }
            if (drop_cond_identification || drop_identification || drop_validation)
            {
                LibFred::StatusList to_cancel;
                //drop conditionally identified flag if e-mail or mobile changed
                if (drop_cond_identification &&
                    states.presents(LibFred::Object_State::conditionally_identified_contact))
                {
                    to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::conditionally_identified_contact));
                }
                //drop identified flag if name, mailing address, e-mail or mobile changed
                if (drop_identification &&
                    states.presents(LibFred::Object_State::identified_contact))
                {
                    to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::identified_contact));
                }
                if (drop_validation)
                {
                    to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::validated_contact));
                }
                if (!to_cancel.empty())
                {
                    try
                    {
                        LibFred::CancelObjectStateRequestId(current_data.id, to_cancel).exec(ctx);
                    }
                    catch (const LibFred::CancelObjectStateRequestId::Exception& e)
                    {
                        if (e.is_set_object_id_not_found())
                        {
                            throw MojeIdImplData::ObjectDoesntExist();
                        }
                        if (e.is_set_state_not_found())
                        {
                            LOGGER.info("unable clear state " + e.get_state_not_found());
                        }
                        else
                        {
                            throw;
                        }
                    }
                }
            }

            {
                const MojeIdImplInternal::CheckUpdateTransferContactPrepare result_of_check(new_data);

                if (!result_of_check.success())
                {
                    MojeIdImplInternal::raise(result_of_check);
                }
            }
            if (current_data.sponsoring_registrar_handle != mojeid_registrar_.handle())
            {
                const auto authinfo_password = set_contact_authinfo_password(
                        ctx,
                        LibFred::Object::ObjectId{current_data.id},
                        system_registrar_.id());
                LibFred::TransferContact transfer_contact_op(current_data.id,
                        mojeid_registrar_.handle(),
                        authinfo_password,
                        0 < _log_request_id ? _log_request_id
                                            : Nullable<MojeIdImpl::LogRequestId>());
                //transfer contact to 'REG-MOJEID' sponsoring registrar
                const unsigned long long history_id = transfer_contact_op.exec(ctx);
                notify(ctx, Notification::transferred, mojeid_registrar_.id(), history_id, _log_request_id);
                LibFred::Poll::CreatePollMessage<LibFred::Poll::MessageType::transfer_contact>()
                        .exec(ctx, history_id);
            }
            //perform changes
            LibFred::UpdateContactById update_contact_op(new_data.id, mojeid_registrar_.handle());
            set_update_contact_op(LibFred::diff_contact_data(current_data, new_data), update_contact_op);
            const bool is_identified = states.presents(LibFred::Object_State::identified_contact) && !drop_identification;
            const bool is_validated = states.presents(LibFred::Object_State::validated_contact) && !drop_validation;
            const bool addr_can_be_hidden = (is_identified || is_validated) &&
                                            new_data.organization.get_value_or_default().empty();
            if (!addr_can_be_hidden)
            {
                update_contact_op.set_discloseaddress(true);
            }
            if (0 < _log_request_id)
            {
                update_contact_op.set_logd_request_id(_log_request_id);
            }
            history_id = update_contact_op.exec(ctx);
            LibFred::Poll::CreateUpdateOperationPollMessage<LibFred::Object_Type::contact>().exec(ctx, history_id);
        }
        const bool is_cond_identified = states.presents(LibFred::Object_State::conditionally_identified_contact) &&
                                        !drop_cond_identification;
        LibFred::CreatePublicRequestAuth op_create_pub_req;
        if (!current_data.notifyemail.get_value_or_default().empty())
        {
            op_create_pub_req.set_email_to_answer(current_data.notifyemail.get_value());
        }
        op_create_pub_req.set_registrar_id(mojeid_registrar_.id());
        const LibFred::CreatePublicRequestAuth::Result result =
                op_create_pub_req.exec(
                        locked_contact,
                        //for 'conditionallyIdentifiedContact' or 'identifiedContact' create 'mojeid_prevalidated_contact_transfer' public request
                        is_cond_identified ? Fred::Backend::MojeId::PublicRequest::PrevalidatedContactTransfer().iface()
                                           //in other cases create 'mojeid_prevalidated_unidentified_contact_transfer' public request
                                           : Fred::Backend::MojeId::PublicRequest::PrevalidatedUnidentifiedContactTransfer().iface(),
                        get_optional_log_request_id(_log_request_id));

        notify(ctx, Notification::updated, mojeid_registrar_.id(), history_id, _log_request_id);
        //second phase commit will change contact states
        prepare_transaction_storage()->store(_trans_id, current_data.id);

        MojeIdImplData::InfoContact changed_data;
        from_into(LibFred::InfoContactById(current_data.id).exec(ctx).info_contact_data, changed_data);
        ctx.commit_transaction();
        _ident = result.identification;
        return changed_data;
    }
    catch (const LibFred::InfoContactByHandle::Exception& e)
    {
        //check contact is registered, throw OBJECT_NOT_EXISTS if isn't
        if (e.is_set_unknown_contact_handle())
        {
            LOGGER.info("request failed (InfoContactByHandle::Exception) - unknown_contact_handle");
            throw MojeIdImplData::ObjectDoesntExist();
        }
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const MojeIdImplData::RegistrationValidationResult&)
    {
        LOGGER.info("request failed (RegistrationValidationResult)");
        throw;
    }
    catch (const MojeIdImplData::AlreadyMojeidContact&)
    {
        LOGGER.info("request failed (AlreadyMojeidContact)");
        throw;
    }
    catch (const MojeIdImplData::IdentityAttached&)
    {
        LOGGER.info("request failed (IdentityAttached)");
        throw;
    }
    catch (const MojeIdImplData::ObjectAdminBlocked&)
    {
        LOGGER.info("request failed (ObjectAdminBlocked)");
        throw;
    }
    catch (const MojeIdImplData::ObjectUserBlocked&)
    {
        LOGGER.info("request failed (ObjectUserBlocked)");
        throw;
    }
    catch (const MojeIdImplData::MessageLimitExceeded& e)
    {
        LOGGER.info(e.as_string());
        throw;
    }
    catch (const MojeIdImplData::ObjectDoesntExist& e)
    {
        LOGGER.info("request failed (ObjectDoesntExist)");
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

namespace {

LibFred::UpdatePublicRequest::Result set_status(
        const LibFred::LockedPublicRequestForUpdate& _locked_request,
        const LibFred::PublicRequestTypeIface& _request_type,
        LibFred::PublicRequest::Status::Enum _status,
        const std::string& _reason,
        MojeIdImpl::LogRequestId _log_request_id)
{
    LibFred::UpdatePublicRequest op_update_public_request;
    op_update_public_request.set_status(_status);
    if (!_reason.empty())
    {
        op_update_public_request.set_reason(_reason);
    }
    return op_update_public_request.exec(_locked_request, _request_type, get_optional_log_request_id(_log_request_id));
}

LibFred::UpdatePublicRequest::Result answer(
        const LibFred::LockedPublicRequestForUpdate& _locked_request,
        const LibFred::PublicRequestTypeIface& _request_type,
        const std::string& _reason,
        MojeIdImpl::LogRequestId _log_request_id)
{
    return set_status(_locked_request, _request_type, LibFred::PublicRequest::Status::resolved, _reason, _log_request_id);
}

LibFred::UpdatePublicRequest::Result invalidate(
        const LibFred::LockedPublicRequestForUpdate& _locked_request,
        const LibFred::PublicRequestTypeIface& _request_type,
        const std::string& _reason,
        MojeIdImpl::LogRequestId _log_request_id)
{
    return set_status(_locked_request, _request_type, LibFred::PublicRequest::Status::invalidated, _reason, _log_request_id);
}

//ticket #15587 hack
void invalid_birthday_looks_like_no_birthday(MojeIdImplData::InfoContact& _data)
{
    if (!_data.birth_date.isnull())
    {
        const boost::gregorian::date invalid_date(boost::gregorian::not_a_date_time);
        const std::string invalid_date_str = boost::gregorian::to_iso_extended_string(invalid_date);
        if (_data.birth_date.get_value().value == invalid_date_str)
        { //make believe that invalid birthday
            _data.birth_date = Nullable<MojeIdImplData::Birthdate>(); //is no birthday
        }
    }
}

} // namespace Fred::Backend::MojeId::{anonymous}

void MojeIdImpl::info_contact(
        const std::string& _username,
        MojeIdImplData::InfoContact& _result) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        from_into(LibFred::InfoContactByHandle(_username).exec(ctx).info_contact_data, _result);
        invalid_birthday_looks_like_no_birthday(_result); //ticket #15587 hack
        ctx.commit_transaction();
        return;
    }
    catch (const LibFred::InfoContactByHandle::Exception& e)
    {
        if (e.is_set_unknown_contact_handle())
        {
            LOGGER.info("request failed (ObjectDoesntExist)");
            throw MojeIdImplData::ObjectDoesntExist();
        }
        LOGGER.error("request failed (LibFred::InfoContactByHandle failure)");
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

void MojeIdImpl::get_contact_info_publish_flags(
        ContactId _contact_id,
        MojeIdImplData::InfoContactPublishFlags& _flags) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::InfoContactData data = LibFred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(_contact_id).exec(ctx));

        if (states.presents(LibFred::Object_State::linked))
        {
            _flags.name = data.disclosename;
            _flags.organization = data.discloseorganization;
            _flags.vat_reg_num = data.discloseident;
            _flags.birth_date = data.discloseident;
            _flags.id_card_num = data.discloseident;
            _flags.passport_num = data.discloseident;
            _flags.ssn_id_num = data.discloseident;
            _flags.vat_id_num = data.discloseident;
            _flags.email = data.discloseemail;
            _flags.notify_email = data.disclosenotifyemail;
            _flags.telephone = data.disclosetelephone;
            _flags.fax = data.disclosefax;
            _flags.permanent = data.discloseaddress;
            _flags.mailing = false;
            _flags.billing = false;
            _flags.shipping = false;
            _flags.shipping2 = false;
            _flags.shipping3 = false;
        }
        else
        {
            _flags.name = data.disclosename;
            _flags.organization = false;
            _flags.vat_reg_num = false;
            _flags.birth_date = false;
            _flags.id_card_num = false;
            _flags.passport_num = false;
            _flags.ssn_id_num = false;
            _flags.vat_id_num = false;
            _flags.email = false;
            _flags.notify_email = false;
            _flags.telephone = false;
            _flags.fax = false;
            _flags.permanent = false;
            _flags.mailing = false;
            _flags.billing = false;
            _flags.shipping = false;
            _flags.shipping2 = false;
            _flags.shipping3 = false;
        }
        ctx.commit_transaction();
        return;
    }
    catch (const LibFred::InfoContactById::Exception& e)
    {
        if (e.is_set_unknown_object_id())
        {
            LOGGER.info("request failed (incorrect input data)");
            throw MojeIdImplData::ObjectDoesntExist();
        }
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

MojeIdImpl::ContactId MojeIdImpl::process_registration_request(
        const std::string& _ident_request_id,
        const std::string& _password,
        MojeIdImpl::LogRequestId _log_request_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestLockGuardByIdentification locked_request(ctx, _ident_request_id);
        const LibFred::PublicRequestAuthInfo pub_req_info(ctx, locked_request);
        if (pub_req_info.get_object_id().isnull())
        {
            invalidate(locked_request,
                    LibFred::FakePublicRequestForInvalidating(pub_req_info.get_type()).iface(),
                    "no object associated with this public request",
                    _log_request_id);
            throw MojeIdImplData::IdentificationRequestDoesntExist();
        }
        const LibFred::ObjectId contact_id = pub_req_info.get_object_id().get_value();
        if (is_identity_attached(ctx, contact_id))
        {
            invalidate(locked_request,
                    LibFred::FakePublicRequestForInvalidating(pub_req_info.get_type()).iface(),
                    "contact is attached to another identity",
                    _log_request_id);
            throw MojeIdImplData::IdentityAttached{};
        }
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(contact_id).exec(ctx));
        const PubReqType::Enum pub_req_type(Conversion::Enums::from_db_handle<PubReqType>(pub_req_info.get_type()));
        try
        {
            switch (pub_req_info.get_status())
            {
                case LibFred::PublicRequest::Status::opened:
                    break;
                case LibFred::PublicRequest::Status::resolved:
                    throw MojeIdImplData::IdentificationAlreadyProcessed();
                case LibFred::PublicRequest::Status::invalidated:
                    throw MojeIdImplData::IdentificationAlreadyInvalidated();
            }

            LibFred::StatusList to_set;
            bool validated_contact_state_was_set = false;
            switch (pub_req_type)
            {
                case PubReqType::contact_conditional_identification:
                    to_set.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::conditionally_identified_contact));
                    break;
                case PubReqType::prevalidated_unidentified_contact_transfer:
                    to_set.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::conditionally_identified_contact));
                    if (states.absents(LibFred::Object_State::validated_contact))
                    {
                        to_set.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::validated_contact));
                        validated_contact_state_was_set = true;
                    }
                    break;
                case PubReqType::conditionally_identified_contact_transfer:
                case PubReqType::identified_contact_transfer:
                    break;
                case PubReqType::prevalidated_contact_transfer:
                    if (states.absents(LibFred::Object_State::validated_contact))
                    {
                        to_set.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::validated_contact));
                        validated_contact_state_was_set = true;
                    }
                    break;
                default:
                    const std::string error_message = "unexpected public request type ";
                    throw std::runtime_error(error_message + pub_req_info.get_type());
            }

            const Database::Result dbres = ctx.get_conn().exec_params(
                    "SELECT EXISTS(SELECT 1 FROM public_request "
                    "WHERE id=$1::BIGINT AND "
                    "create_time<(SELECT GREATEST(update,trdate) FROM object "
                    "WHERE id=$2::BIGINT)"
                    ") AS object_changed",
                    Database::query_param_list(static_cast<const LibFred::LockedPublicRequest&>(locked_request).get_id()) //$1::BIGINT
                    (contact_id)); //$2::BIGINT

            if (dbres.size() != 1)
            {
                throw std::runtime_error("something wrong happened, database looks to be crazy, this query has to return exactly one row");
            }

            const bool contact_changed = static_cast<bool>(dbres[0][0]);
            if (contact_changed)
            {
                invalidate(locked_request,
                        LibFred::FakePublicRequestForInvalidating(pub_req_info.get_type()).iface(),
                        "contact data changed after the public request had been created",
                        _log_request_id);
                throw MojeIdImplData::ContactChanged();
            }

            if (!pub_req_info.check_password(_password))
            {
                throw MojeIdImplData::IdentificationFailed();
            }

            to_set.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_delete_prohibited));
            to_set.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_transfer_prohibited));
            to_set.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_update_prohibited));
            to_set.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::mojeid_contact));
            LibFred::CreateObjectStateRequestId(contact_id, to_set).exec(ctx);
            LibFred::PerformObjectStateRequest(contact_id).exec(ctx);
            if (validated_contact_state_was_set)
            {
                const LibFred::InfoContactData info_contact_data = LibFred::InfoContactById(contact_id).exec(ctx).info_contact_data;
                const bool address_is_disclosed = info_contact_data.discloseaddress;
                const bool address_can_be_undisclosed = info_contact_data.organization.get_value_or("").empty();
                if (address_is_disclosed && address_can_be_undisclosed)
                {
                    LibFred::Contact::undisclose_address(ctx, contact_id, system_registrar_.handle()); // #21767
                }
            }

            const LibFred::InfoContactData contact = LibFred::InfoContactById(contact_id).exec(ctx).info_contact_data;
            {
                const MojeIdImplInternal::CheckProcessRegistrationValidation check_result(contact);
                if (!check_result.success())
                {
                    MojeIdImplInternal::raise(check_result);
                }
            }
            if (contact.sponsoring_registrar_handle != mojeid_registrar_.handle())
            {
                const auto authinfo_password = set_contact_authinfo_password(
                        ctx,
                        LibFred::Object::ObjectId{contact.id},
                        system_registrar_.id());
                LibFred::TransferContact transfer_contact_op(contact.id,
                        mojeid_registrar_.handle(),
                        authinfo_password,
                        0 < _log_request_id ? _log_request_id
                                            : Nullable<MojeIdImpl::LogRequestId>());
                //transfer contact to 'REG-MOJEID' sponsoring registrar
                const unsigned long long history_id = transfer_contact_op.exec(ctx);
                notify(ctx, Notification::transferred, mojeid_registrar_.id(), history_id, _log_request_id);
                LibFred::Poll::CreatePollMessage<LibFred::Poll::MessageType::transfer_contact>()
                        .exec(ctx, history_id);
            }
            answer(locked_request,
                    pub_req_type == PubReqType::contact_conditional_identification
                            ? Fred::Backend::MojeId::PublicRequest::ContactConditionalIdentification().iface()
                            : pub_req_type == PubReqType::prevalidated_unidentified_contact_transfer
                                      ? Fred::Backend::MojeId::PublicRequest::PrevalidatedUnidentifiedContactTransfer().iface()
                                      : pub_req_type == PubReqType::conditionally_identified_contact_transfer
                                                ? Fred::Backend::MojeId::PublicRequest::ConditionallyIdentifiedContactTransfer().iface()
                                                : pub_req_type == PubReqType::identified_contact_transfer
                                                          ? Fred::Backend::MojeId::PublicRequest::IdentifiedContactTransfer().iface()
                                                          : Fred::Backend::MojeId::PublicRequest::PrevalidatedContactTransfer().iface(),
                    "successfully processed",
                    _log_request_id);

            const HandleMojeIdArgs* const server_conf_ptr = CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>();
            if (server_conf_ptr->auto_pin3_sending)
            {
                if (LibFred::ObjectStatesInfo(LibFred::GetObjectStates(contact_id).exec(ctx))
                                .absents(LibFred::Object_State::identified_contact))
                {
                    LibFred::CreatePublicRequestAuth op_create_pub_req;
                    op_create_pub_req.set_registrar_id(mojeid_registrar_.id());
                    LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, contact_id);
                    const LibFred::CreatePublicRequestAuth::Result result = op_create_pub_req.exec(
                            locked_contact, Fred::Backend::MojeId::PublicRequest::ContactIdentification().iface(), get_optional_log_request_id(_log_request_id));
                }
            }
            ctx.commit_transaction();

            return contact_id;
        }
        catch (const MojeIdImplData::IdentificationFailed&)
        {
            ctx.commit_transaction();
            throw;
        }
        catch (const MojeIdImplData::ContactChanged&)
        {
            ctx.commit_transaction();
            throw;
        }
        catch (const MojeIdImplData::IdentityAttached&)
        {
            ctx.commit_transaction();
            throw;
        }
    }
    catch (const MojeIdImplData::IdentificationRequestDoesntExist&)
    {
        LOGGER.info("request failed (identification request doesn't exist)");
        throw;
    }
    catch (const MojeIdImplData::IdentificationFailed&)
    {
        LOGGER.info("request failed (identification failed)");
        throw;
    }
    catch (const MojeIdImplData::ContactChanged&)
    {
        LOGGER.info("request failed (contact changed)");
        throw;
    }
    catch (const MojeIdImplData::IdentityAttached&)
    {
        LOGGER.info("request failed (contact attached to another identity)");
        throw;
    }
    catch (const MojeIdImplData::ObjectDoesntExist& e)
    {
        LOGGER.info(boost::format("request failed (%1%)") % e.what());
        throw MojeIdImplData::IdentificationFailed{};
    }
    catch (const MojeIdImplData::ProcessRegistrationValidationResult&)
    {
        LOGGER.info("request failed (incorrect data)");
        throw;
    }
    catch (const MojeIdImplData::IdentificationAlreadyProcessed&)
    {
        LOGGER.info("request failed (IdentificationAlreadyProcessed)");
        throw;
    }
    catch (const LibFred::PublicRequestLockGuardByIdentification::Exception& e)
    {
        if (e.is_set_public_request_doesnt_exist())
        {
            LOGGER.info(boost::format("request failed (%1%)") % e.what());
            throw MojeIdImplData::IdentificationRequestDoesntExist();
        }
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw std::runtime_error(e.what());
    }
    catch (const LibFred::InfoContactById::Exception& e)
    {
        if (e.is_set_unknown_object_id())
        {
            LOGGER.info(boost::format("request failed (%1%)") % e.what());
            throw MojeIdImplData::IdentificationFailed();
        }
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

void MojeIdImpl::process_identification_request(
        ContactId _contact_id,
        const std::string& _password,
        MojeIdImpl::LogRequestId _log_request_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, _contact_id);
        LibFred::PublicRequestId public_request_id;
        bool reidentification;
        try
        {
            public_request_id = LibFred::GetOpenedPublicRequest(
                    Fred::Backend::MojeId::PublicRequest::ContactIdentification())
                                        .exec(ctx, locked_contact, _log_request_id);
            reidentification = false;
        }
        catch (const LibFred::GetOpenedPublicRequest::Exception& e)
        {
            if (!e.is_set_no_request_found())
            {
                throw;
            }
            try
            {
                public_request_id = LibFred::GetOpenedPublicRequest(
                        Fred::Backend::MojeId::PublicRequest::ContactReidentification())
                                            .exec(ctx, locked_contact, _log_request_id);
                reidentification = true;
            }
            catch (const LibFred::GetOpenedPublicRequest::Exception& e)
            {
                if (e.is_set_no_request_found())
                {
                    throw MojeIdImplData::IdentificationRequestDoesntExist();
                }
                throw;
            }
        }
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(_contact_id).exec(ctx));
        if (states.absents(LibFred::Object_State::mojeid_contact))
        {
            throw MojeIdImplData::ObjectDoesntExist();
        }
        if (states.absents(LibFred::Object_State::conditionally_identified_contact))
        {
            throw std::runtime_error("state conditionallyIdentifiedContact missing");
        }
        if (states.presents(LibFred::Object_State::identified_contact))
        {
            throw MojeIdImplData::IdentificationAlreadyProcessed();
        }
        if (states.presents(LibFred::Object_State::server_blocked))
        {
            throw MojeIdImplData::ObjectAdminBlocked();
        }
        if (states.absents(LibFred::Object_State::server_transfer_prohibited) ||
                states.absents(LibFred::Object_State::server_update_prohibited) ||
                states.absents(LibFred::Object_State::server_delete_prohibited))
        {
            throw std::runtime_error("contact not protected against changes");
        }

        LibFred::PublicRequestLockGuardById locked_request(ctx, public_request_id);
        if (!LibFred::PublicRequestAuthInfo(ctx, locked_request).check_password(_password))
        {
            throw MojeIdImplData::IdentificationFailed();
        }
        LibFred::StatusList to_set;
        to_set.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::identified_contact));
        LibFred::CreateObjectStateRequestId(_contact_id, to_set).exec(ctx);
        LibFred::PerformObjectStateRequest(_contact_id).exec(ctx);
        const LibFred::InfoContactData info_contact_data = LibFred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        const bool address_is_disclosed = info_contact_data.discloseaddress;
        const bool address_can_be_undisclosed = info_contact_data.organization.get_value_or("").empty();
        if (address_is_disclosed && address_can_be_undisclosed)
        {
            LibFred::Contact::undisclose_address(ctx, _contact_id, system_registrar_.handle()); // #21767
        }
        answer(locked_request,
                reidentification ? Fred::Backend::MojeId::PublicRequest::ContactReidentification().iface()
                                 : Fred::Backend::MojeId::PublicRequest::ContactIdentification().iface(),
                "successfully processed",
                _log_request_id);
        ctx.commit_transaction();
    }
    catch (const MojeIdImplData::IdentificationRequestDoesntExist&)
    {
        LOGGER.info("request failed (IdentificationRequestDoesntExist)");
        throw;
    }
    catch (const MojeIdImplData::ObjectDoesntExist&)
    {
        LOGGER.info("request failed (ObjectDoesntExist)");
        throw;
    }
    catch (const MojeIdImplData::IdentificationAlreadyProcessed&)
    {
        LOGGER.info("request failed (IdentificationAlreadyProcessed)");
        throw;
    }
    catch (const MojeIdImplData::IdentificationFailed&)
    {
        LOGGER.info("request failed (IdentificationFailed)");
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

void MojeIdImpl::commit_prepared_transaction(const std::string& _trans_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::commit_transaction(_trans_id);
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }

    try
    {
        const ContactId contact_id = prepare_transaction_storage()->get(_trans_id);
        LibFred::OperationContextCreator ctx;
        LibFred::PerformObjectStateRequest(contact_id).exec(ctx);
        ctx.commit_transaction();
        prepare_transaction_storage()->release(_trans_id);
    }
    catch (const prepare_transaction_data_not_found&)
    {
        LOGGER.info("no saved transaction data for " + _trans_id + " identifier)");
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }

    try
    {
        const HandleMojeIdArgs* const server_conf_ptr = CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>();
        if (server_conf_ptr->auto_messages_generation)
        {
            this->generate_public_request_messages();
        }
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

void MojeIdImpl::rollback_prepared_transaction(const std::string& _trans_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::rollback_transaction(_trans_id);
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
    try
    {
        prepare_transaction_storage()->release(_trans_id);
    }
    catch (const prepare_transaction_data_not_found&)
    {
        LOGGER.info("no saved transaction data for " + _trans_id + " identifier)");
    }
}

namespace {

std::string birthdate_into_czech_date(const std::string& _birthdate)
{
    const boost::gregorian::date d = birthdate_from_string_to_date(_birthdate);
    std::ostringstream out;
    if (!d.is_special())
    {
        const boost::gregorian::date::ymd_type ymd = d.year_month_day();
        out << std::setw(2) << std::setfill('0') << std::right << ymd.day.as_number() << "." //dd.
            << std::setw(2) << std::setfill('0') << std::right << ymd.month.as_number() << "." //dd.mm.
            << std::setw(0) << ymd.year; //dd.mm.yyyy
    }
    return out.str();
}

} // namespace Fred::Backend::MojeId::{anonymous}

Fred::Backend::Buffer MojeIdImpl::get_validation_pdf(ContactId _contact_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;

        const LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, _contact_id);

        Database::Result res = ctx.get_conn().exec_params(
                // clang-format off
                "SELECT pr.id,c.name,c.organization,c.ssn,"
                       "(SELECT type FROM enum_ssntype WHERE id=c.ssntype),"
                       "CONCAT_WS(', ',"
                           "NULLIF(BTRIM(c.street1),''),NULLIF(BTRIM(c.street2),''),NULLIF(BTRIM(c.street3),''),"
                           "BTRIM(NULLIF(BTRIM(c.postalcode),'')||E'\\u2007'||NULLIF(BTRIM(c.city),'')),"
                           "CASE WHEN c.country='CZ' THEN NULL "
                                "ELSE (SELECT country_cs FROM enum_country WHERE id=c.country) END),"
                       "(SELECT name FROM object_registry WHERE id=c.id) "
                "FROM public_request pr,"
                     "contact c "
                "WHERE pr.resolve_time IS NULL AND "
                      "pr.status=(SELECT id FROM enum_public_request_status WHERE name=$1::TEXT) AND "
                      "pr.request_type=(SELECT id FROM enum_public_request_type WHERE name=$2::TEXT) AND "
                      "c.id=$3::BIGINT AND "
                      "EXISTS(SELECT 1 FROM public_request_objects_map WHERE request_id=pr.id AND object_id=c.id)",
                // clang-format on
                Database::query_param_list
                        (Conversion::Enums::to_db_handle(LibFred::PublicRequest::Status::opened))
                        (Fred::Backend::MojeId::PublicRequest::ContactValidation().get_public_request_type())
                        (_contact_id));
        if (res.size() <= 0)
        {
            throw MojeIdImplData::ObjectDoesntExist();
        }
        const HandleRegistryArgs* const reg_conf =
                CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>();
        HandleCorbaNameServiceArgs* const cn_conf =
                CfgArgs::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        std::unique_ptr<LibFred::Document::Manager> doc_manager(
                LibFred::Document::Manager::create(
                        reg_conf->docgen_path,
                        reg_conf->docgen_template_path,
                        reg_conf->fileclient_path,
                        // doc_manager config dependence
                        cn_conf->get_nameservice_host_port()));
        const std::string czech_language = "cs";
        std::ostringstream pdf_document;
        std::unique_ptr<LibFred::Document::Generator> doc_gen(
                doc_manager->createOutputGenerator(LibFred::Document::GT_CONTACT_VALIDATION_REQUEST_PIN3,
                        pdf_document,
                        czech_language));
        const std::string request_id = static_cast<std::string>(res[0][0]);
        const std::string name = static_cast<std::string>(res[0][1]);
        const std::string organization = static_cast<std::string>(res[0][2]);
        const std::string ssn_value = static_cast<std::string>(res[0][3]);
        const LibFred::SSNType::Enum ssn_type = Conversion::Enums::from_db_handle<LibFred::SSNType>(
                static_cast<std::string>(res[0][4]));
        const std::string address = static_cast<std::string>(res[0][5]);
        const std::string handle = static_cast<std::string>(res[0][6]);
        const bool is_ssn_ico =
                ssn_type == LibFred::SSNType::ico;
        const std::string birthday =
                ssn_type == LibFred::SSNType::birthday
                        ? birthdate_into_czech_date(ssn_value)
                        : "";
        std::string letter_xml("<?xml version='1.0' encoding='utf-8'?>");
        // clang-format off
        ::Util::XmlTagPair("mojeid_valid", ::Util::vector_of< ::Util::XmlCallback >
            (::Util::XmlTagPair("request_id",   ::Util::XmlUnparsedCData(request_id)))
            (::Util::XmlTagPair("handle",       ::Util::XmlUnparsedCData(handle)))
            (::Util::XmlTagPair("name",         ::Util::XmlUnparsedCData(name)))
            (::Util::XmlTagPair("organization", ::Util::XmlUnparsedCData(organization)))
            (::Util::XmlTagPair("ic",           ::Util::XmlUnparsedCData(is_ssn_ico ? ssn_value : "")))
            (::Util::XmlTagPair("birth_date",   ::Util::XmlUnparsedCData(birthday)))
            (::Util::XmlTagPair("address",      ::Util::XmlUnparsedCData(address)))
        )(letter_xml);
        // clang-format on

        doc_gen->getInput() << letter_xml;
        doc_gen->closeInput();
        return Fred::Backend::Buffer(pdf_document.str());
    }
    catch (const LibFred::PublicRequestsOfObjectLockGuardByObjectId::Exception& e)
    {
        if (e.is_set_object_doesnt_exist())
        {
            LOGGER.info(boost::format("contact doesn't exist (%1%)") % e.what());
            throw MojeIdImplData::ObjectDoesntExist();
        }
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const MojeIdImplData::ObjectDoesntExist&)
    {
        LOGGER.info("request doesn't exist (ObjectDoesntExist)");
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
} //MojeIdImpl::get_validation_pdf

void MojeIdImpl::create_validation_request(
        ContactId _contact_id,
        MojeIdImpl::LogRequestId _log_request_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, _contact_id);
        try
        {
            LibFred::GetOpenedPublicRequest(Fred::Backend::MojeId::PublicRequest::ContactValidation())
                    .exec(ctx, locked_contact, _log_request_id);
            throw MojeIdImplData::ValidationRequestExists();
        }
        catch (const LibFred::GetOpenedPublicRequest::Exception& e)
        {
            if (!e.is_set_no_request_found())
            {
                throw;
            }
        }
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(_contact_id).exec(ctx));
        if (states.presents(LibFred::Object_State::validated_contact))
        {
            throw MojeIdImplData::ValidationAlreadyProcessed();
        }
        if (states.absents(LibFred::Object_State::mojeid_contact))
        {
            throw MojeIdImplData::ObjectDoesntExist();
        }
        const LibFred::InfoContactData contact_data = LibFred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        {
            const MojeIdImplInternal::CheckCreateValidationRequest check_create_validation_request(contact_data);
            if (!check_create_validation_request.success())
            {
                MojeIdImplInternal::raise(check_create_validation_request);
            }
        }
        LibFred::CreatePublicRequest().set_registrar_id(mojeid_registrar_.id()).exec(
                locked_contact,
                Fred::Backend::MojeId::PublicRequest::ContactValidation().iface(),
                get_optional_log_request_id(_log_request_id));
        ctx.commit_transaction();
        return;
    }
    catch (const LibFred::PublicRequestsOfObjectLockGuardByObjectId::Exception& e)
    {
        if (e.is_set_object_doesnt_exist())
        {
            LOGGER.info(boost::format("contact doesn't exist (%1%)") % e.what());
            throw MojeIdImplData::ObjectDoesntExist();
        }
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const MojeIdImplData::ObjectDoesntExist&)
    {
        LOGGER.info("contact doesn't exist (ObjectDoesntExist)");
        throw;
    }
    catch (const MojeIdImplData::ValidationRequestExists&)
    {
        LOGGER.info("unable to create new request (ValidationRequestExists)");
        throw;
    }
    catch (const MojeIdImplData::ValidationAlreadyProcessed&)
    {
        LOGGER.info("contact already validated (ValidationAlreadyProcessed)");
        throw;
    }
    catch (const MojeIdImplData::CreateValidationRequestValidationResult&)
    {
        LOGGER.info("request failed (CreateValidationRequestValidationResult)");
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

void MojeIdImpl::validate_contact(
        ContactId _contact_id,
        MojeIdImpl::LogRequestId _log_request_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, _contact_id);
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(_contact_id).exec(ctx));
        if (states.presents(LibFred::Object_State::validated_contact))
        {
            throw MojeIdImplData::ValidationAlreadyProcessed();
        }
        if (states.absents(LibFred::Object_State::mojeid_contact))
        {
            throw MojeIdImplData::ObjectDoesntExist();
        }
        const LibFred::InfoContactData contact_data = LibFred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        {
            const MojeIdImplInternal::CheckCreateValidationRequest check_create_validation_request(contact_data);
            if (!check_create_validation_request.success())
            {
                MojeIdImplInternal::raise(check_create_validation_request);
            }
        }
        const LibFred::PublicRequestId public_request_id =
                LibFred::CreatePublicRequest().set_registrar_id(mojeid_registrar_.id()).exec(
                        locked_contact,
                        Fred::Backend::MojeId::PublicRequest::ContactValidation().iface(),
                        get_optional_log_request_id(_log_request_id));
        const LibFred::PublicRequestLockGuardById locked_request(ctx, public_request_id);
        LibFred::UpdatePublicRequest().set_registrar_id(mojeid_registrar_.id()).set_status(
                LibFred::PublicRequest::Status::resolved).set_reason("MojeId validate_contact function has been called").exec(locked_request,
                Fred::Backend::MojeId::PublicRequest::ContactValidation().iface(),
                get_optional_log_request_id(_log_request_id));
        LibFred::StatusList to_set;
        to_set.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::validated_contact));
        LibFred::CreateObjectStateRequestId(_contact_id, to_set).exec(ctx);
        LibFred::PerformObjectStateRequest(_contact_id).exec(ctx);
        const bool address_is_disclosed = contact_data.discloseaddress;
        const bool address_can_be_undisclosed = contact_data.organization.get_value_or("").empty();
        if (address_is_disclosed && address_can_be_undisclosed)
        {
            LibFred::Contact::undisclose_address(ctx, _contact_id, system_registrar_.handle()); // #21767
        }
        ctx.commit_transaction();
        return;
    }
    catch (const LibFred::PublicRequestsOfObjectLockGuardByObjectId::Exception& e)
    {
        if (e.is_set_object_doesnt_exist())
        {
            LOGGER.info(boost::format("contact doesn't exist (%1%)") % e.what());
            throw MojeIdImplData::ObjectDoesntExist();
        }
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const MojeIdImplData::ObjectDoesntExist&)
    {
        LOGGER.info("contact doesn't exist (ObjectDoesntExist)");
        throw;
    }
    catch (const MojeIdImplData::ValidationAlreadyProcessed&)
    {
        LOGGER.info("contact already validated (ValidationAlreadyProcessed)");
        throw;
    }
    catch (const MojeIdImplData::CreateValidationRequestValidationResult&)
    {
        LOGGER.info("request failed (CreateValidationRequestValidationResult)");
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

namespace {

typedef bool IsNotNull;

IsNotNull add_state(
        const Database::Value& _valid_from,
        LibFred::Object_State::Enum _state,
        MojeIdImplData::ContactStateInfo& _data)
{
    if (_valid_from.isnull())
    {
        return false;
    }
    const std::string db_timestamp = static_cast<std::string>(_valid_from); //2014-12-11 09:28:45.741828
    boost::posix_time::ptime valid_from = boost::posix_time::time_from_string(db_timestamp);
    switch (_state)
    {
        case LibFred::Object_State::identified_contact:
            _data.identification_date = boost::gregorian::date_from_tm(boost::posix_time::to_tm(valid_from));
            break;
        case LibFred::Object_State::validated_contact:
            _data.validation_date = boost::gregorian::date_from_tm(boost::posix_time::to_tm(valid_from));
            break;
        case LibFred::Object_State::mojeid_contact:
            _data.mojeid_activation_datetime = valid_from;
            break;
        case LibFred::Object_State::linked:
            _data.linked_date = boost::gregorian::date_from_tm(boost::posix_time::to_tm(valid_from));
            break;
        default:
            break;
    }
    return true;
}

} // namespace Fred::Backend::MojeId::{anonymous}

void MojeIdImpl::get_contacts_state_changes(
        unsigned long _last_hours,
        MojeIdImplData::ContactStateInfoList& _result) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        Database::query_param_list params(mojeid_registrar_.handle()); //$1::TEXT
        params(_last_hours); //$2::TEXT
        params(Conversion::Enums::to_db_handle(LibFred::Object_State::conditionally_identified_contact)); //$3::TEXT
        params(Conversion::Enums::to_db_handle(LibFred::Object_State::identified_contact)); //$4::TEXT
        params(Conversion::Enums::to_db_handle(LibFred::Object_State::validated_contact)); //$5::TEXT
        params(Conversion::Enums::to_db_handle(LibFred::Object_State::mojeid_contact)); //$6::TEXT
        params(Conversion::Enums::to_db_handle(LibFred::Object_State::linked)); //$7::TEXT
        const Database::Result rcontacts = ctx.get_conn().exec_params( // observe interval <now - last_hours, now)
                // clang-format off
                "WITH cic AS (SELECT id FROM enum_object_states WHERE name=$3::TEXT),"
                      "ic AS (SELECT id FROM enum_object_states WHERE name=$4::TEXT),"
                      "vc AS (SELECT id FROM enum_object_states WHERE name=$5::TEXT),"
                      "mc AS (SELECT id FROM enum_object_states WHERE name=$6::TEXT),"
                      "lc AS (SELECT id FROM enum_object_states WHERE name=$7::TEXT),"
                     "obs AS (SELECT id FROM enum_object_states WHERE name IN "//observed states
                                                                     "($3::TEXT,$4::TEXT,$5::TEXT,$6::TEXT,$7::TEXT)),"
                      "cc AS (SELECT DISTINCT c.id "//contacts whose observed states start
                             "FROM contact c "      //or stop in the course of $2 hours
                             "JOIN object_state os ON os.object_id=c.id "
                             "JOIN obs ON os.state_id=obs.id "
                             "WHERE (NOW()-($2::TEXT||'HOUR')::INTERVAL)<=os.valid_from OR (os.valid_to IS NOT NULL AND "
                                   "(NOW()-($2::TEXT||'HOUR')::INTERVAL)<=os.valid_to)) "
                "SELECT cc.id,"                                                           // [0] - contact id
                       "(SELECT valid_from FROM object_state JOIN cic ON state_id=cic.id "// [1] - cic from
                        "WHERE object_id=cc.id AND valid_to IS NULL),"
                       "(SELECT valid_from FROM object_state JOIN ic ON state_id=ic.id "  // [2] - ic from
                        "WHERE object_id=cc.id AND valid_to IS NULL),"
                       "(SELECT valid_from FROM object_state JOIN vc ON state_id=vc.id "  // [3] - vc from
                        "WHERE object_id=cc.id AND valid_to IS NULL),"
                       "(SELECT valid_from FROM object_state JOIN mc ON state_id=mc.id "  // [4] - mc from
                        "WHERE object_id=cc.id AND valid_to IS NULL), "
                       "(SELECT valid_from FROM object_state JOIN lc ON state_id=lc.id "  // [5] - lc from
                        "WHERE object_id=cc.id AND valid_to IS NULL) "
                "FROM cc "
                "JOIN object_state os ON os.object_id=cc.id AND os.valid_to IS NULL "
                "JOIN mc ON mc.id=os.state_id "
                "JOIN object o ON o.id=cc.id "
                "JOIN registrar r ON r.id=o.clid AND r.handle=$1::TEXT", params);
                // clang-format on

        _result.clear();
        _result.reserve(rcontacts.size());
        for (::size_t idx = 0; idx < rcontacts.size(); ++idx)
        {
            MojeIdImplData::ContactStateInfo data;
            data.contact_id = static_cast<ContactId>(rcontacts[idx][0]);
            if (!add_state(rcontacts[idx][1], LibFred::Object_State::conditionally_identified_contact, data))
            {
                std::ostringstream msg;
                msg << "contact " << data.contact_id << " hasn't "
                    << Conversion::Enums::to_db_handle(LibFred::Object_State::conditionally_identified_contact) << " state";
                LOGGER.error(msg.str());
                continue;
            }
            add_state(rcontacts[idx][2], LibFred::Object_State::identified_contact, data);
            add_state(rcontacts[idx][3], LibFred::Object_State::validated_contact, data);
            if (!add_state(rcontacts[idx][4], LibFred::Object_State::mojeid_contact, data))
            {
                std::ostringstream msg;
                msg << "contact " << data.contact_id << " doesn't have "
                    << Conversion::Enums::to_db_handle(LibFred::Object_State::mojeid_contact) << " state";
                throw std::runtime_error(msg.str());
            }
            add_state(rcontacts[idx][5], LibFred::Object_State::linked, data);
            _result.push_back(data);
        }
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

void MojeIdImpl::get_contact_state(
        ContactId _contact_id,
        MojeIdImplData::ContactStateInfo& _result) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        Database::query_param_list params(mojeid_registrar_.handle()); //$1::TEXT
        params(_contact_id); //$2::BIGINT
        params(Conversion::Enums::to_db_handle(LibFred::Object_State::mojeid_contact)); //$3::TEXT
        params(Conversion::Enums::to_db_handle(LibFred::Object_State::conditionally_identified_contact)); //$4::TEXT
        params(Conversion::Enums::to_db_handle(LibFred::Object_State::identified_contact)); //$5::TEXT
        params(Conversion::Enums::to_db_handle(LibFred::Object_State::validated_contact)); //$6::TEXT
        params(Conversion::Enums::to_db_handle(LibFred::Object_State::linked)); //$7::TEXT
        const Database::Result rcontact = ctx.get_conn().exec_params(
                // clang-format off
                "SELECT r.id IS NULL,"                         // 0
                       "(SELECT valid_from FROM object_state " // 1
                        "WHERE object_id=o.id AND valid_to IS NULL AND "
                              "state_id=(SELECT id FROM enum_object_states WHERE name=$3::TEXT)),"
                       "(SELECT valid_from FROM object_state " // 2
                        "WHERE object_id=o.id AND valid_to IS NULL AND "
                              "state_id=(SELECT id FROM enum_object_states WHERE name=$4::TEXT)),"
                       "(SELECT valid_from FROM object_state " // 3
                        "WHERE object_id=o.id AND valid_to IS NULL AND "
                              "state_id=(SELECT id FROM enum_object_states WHERE name=$5::TEXT)),"
                       "(SELECT valid_from FROM object_state " // 4
                        "WHERE object_id=o.id AND valid_to IS NULL AND "
                              "state_id=(SELECT id FROM enum_object_states WHERE name=$6::TEXT)),"
                       "(SELECT valid_from FROM object_state " // 5
                        "WHERE object_id=o.id AND valid_to IS NULL AND "
                              "state_id=(SELECT id FROM enum_object_states WHERE name=$7::TEXT)) "
                "FROM contact c "
                "JOIN object o ON o.id=c.id "
                "LEFT JOIN registrar r ON r.id=o.clid AND r.handle=$1::TEXT "
                "WHERE c.id=$2::BIGINT",
                // clang-format on
                params);

        if (rcontact.size() == 0)
        {
            throw MojeIdImplData::ObjectDoesntExist();
        }

        if (1 < rcontact.size())
        {
            std::ostringstream msg;
            msg << "contact " << _contact_id << " returns multiple (" << rcontact.size() << ") records";
            throw std::runtime_error(msg.str());
        }

        if (static_cast<bool>(rcontact[0][0]))
        { // contact's registrar missing
            throw MojeIdImplData::ObjectDoesntExist();
        }

        _result.contact_id = _contact_id;
        if (!add_state(rcontact[0][1], LibFred::Object_State::mojeid_contact, _result))
        {
            throw MojeIdImplData::ObjectDoesntExist();
        }
        if (!add_state(rcontact[0][2], LibFred::Object_State::conditionally_identified_contact, _result))
        {
            std::ostringstream msg;
            msg << "contact " << _contact_id << " doesn't have "
                << Conversion::Enums::to_db_handle(LibFred::Object_State::conditionally_identified_contact) << " state";
            throw std::runtime_error(msg.str());
        }
        add_state(rcontact[0][3], LibFred::Object_State::identified_contact, _result);
        add_state(rcontact[0][4], LibFred::Object_State::validated_contact, _result);
        add_state(rcontact[0][5], LibFred::Object_State::linked, _result);
    } //try
    catch (const MojeIdImplData::ObjectDoesntExist&)
    {
        LOGGER.info("ObjectDoesntExist");
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

void MojeIdImpl::cancel_account_prepare(
        ContactId _contact_id,
        const std::string& _trans_id,
        MojeIdImpl::LogRequestId _log_request_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextTwoPhaseCommitCreator ctx(_trans_id);
        const LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, _contact_id);
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(_contact_id).exec(ctx));

        if (states.absents(LibFred::Object_State::mojeid_contact))
        {
            throw MojeIdImplData::ObjectDoesntExist();
        }

        if (!(states.presents(LibFred::Object_State::validated_contact) ||
                    states.presents(LibFred::Object_State::identified_contact) ||
                    states.presents(LibFred::Object_State::conditionally_identified_contact)))
        {
            throw std::runtime_error("bad mojeID contact");
        }

        if (states.absents(LibFred::Object_State::linked))
        {
            LibFred::DeleteContactById(_contact_id).exec(ctx);
            ctx.commit_transaction();
            return;
        }

        LibFred::StatusList to_cancel;
        to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::mojeid_contact));
        if (states.presents(LibFred::Object_State::server_update_prohibited))
        {
            to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_update_prohibited));
        }
        if (states.presents(LibFred::Object_State::server_transfer_prohibited))
        {
            to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_transfer_prohibited));
        }
        if (states.presents(LibFred::Object_State::server_delete_prohibited))
        {
            to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_delete_prohibited));
        }
        if (states.presents(LibFred::Object_State::validated_contact))
        {
            to_cancel.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::validated_contact));
        }
        LibFred::CancelObjectStateRequestId(_contact_id, to_cancel).exec(ctx);

        {
            LibFred::UpdateContactById update_contact_op(_contact_id, mojeid_registrar_.handle());
            update_contact_op.unset_domain_expiration_warning_letter_enabled()
                    .reset_address<LibFred::ContactAddressType::MAILING>()
                    .reset_address<LibFred::ContactAddressType::BILLING>()
                    .reset_address<LibFred::ContactAddressType::SHIPPING>()
                    .reset_address<LibFred::ContactAddressType::SHIPPING_2>()
                    .reset_address<LibFred::ContactAddressType::SHIPPING_3>();
            if (0 < _log_request_id)
            {
                update_contact_op.set_logd_request_id(_log_request_id);
            }
            update_contact_op.exec(ctx);
        }

        LibFred::UpdatePublicRequest()
                .set_status(LibFred::PublicRequest::Status::invalidated)
                .set_reason("cancel_account_prepare call")
                .set_registrar_id(ctx, mojeid_registrar_.handle())
                .exec(locked_contact,
                      Fred::Backend::MojeId::PublicRequest::ContactValidation(),
                      get_optional_log_request_id(_log_request_id));
        prepare_transaction_storage()->store(_trans_id, _contact_id);
        ctx.commit_transaction();
        return;
    }
    catch (const LibFred::PublicRequestsOfObjectLockGuardByObjectId::Exception& e)
    {
        if (e.is_set_object_doesnt_exist())
        {
            LOGGER.info(boost::format("contact doesn't exist (%1%)") % e.what());
            throw MojeIdImplData::ObjectDoesntExist();
        }
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (const MojeIdImplData::ObjectDoesntExist&)
    {
        LOGGER.info("contact doesn't exist (ObjectDoesntExist)");
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("request failed (unknown error)");
        throw;
    }
}

void MojeIdImpl::send_new_pin3(
        ContactId _contact_id,
        MojeIdImpl::LogRequestId _log_request_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, _contact_id);
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(_contact_id).exec(ctx));
        if (states.presents(LibFred::Object_State::identified_contact))
        {
            throw MojeIdImplData::IdentificationAlreadyProcessed();
        }
        if (states.absents(LibFred::Object_State::mojeid_contact))
        {
            throw MojeIdImplData::ObjectDoesntExist();
        }

        try
        {
            const Fred::Backend::MojeId::PublicRequest::ContactIdentification type;
            LibFred::GetOpenedPublicRequest get_opened_public_request_op(type.iface());
            while (true)
            {
                const LibFred::PublicRequestId request_id = get_opened_public_request_op.exec(ctx, locked_object);
                LibFred::UpdatePublicRequest update_public_request_op;
                LibFred::PublicRequestLockGuardById locked_request(ctx, request_id);
                update_public_request_op.set_status(LibFred::PublicRequest::Status::invalidated);
                update_public_request_op.set_reason("new pin3 generated");
                update_public_request_op.set_registrar_id(ctx, mojeid_registrar_.handle());
                update_public_request_op.exec(locked_request, type.iface(), get_optional_log_request_id(_log_request_id));
            }
        }
        catch (const LibFred::GetOpenedPublicRequest::Exception& e)
        {
            if (!e.is_set_no_request_found())
            {
                throw;
            }
        }

        bool has_reidentification_request = false;
        try
        {
            const Fred::Backend::MojeId::PublicRequest::ContactReidentification type;
            LibFred::GetOpenedPublicRequest get_opened_public_request_op(type.iface());
            while (true)
            {
                const LibFred::PublicRequestId request_id = get_opened_public_request_op.exec(ctx, locked_object);
                LibFred::UpdatePublicRequest update_public_request_op;
                LibFred::PublicRequestLockGuardById locked_request(ctx, request_id);
                update_public_request_op.set_status(LibFred::PublicRequest::Status::invalidated);
                update_public_request_op.set_reason("new pin3 generated");
                update_public_request_op.set_registrar_id(ctx, mojeid_registrar_.handle());
                update_public_request_op.exec(locked_request, type.iface(), get_optional_log_request_id(_log_request_id));
                has_reidentification_request = true;
            }
        }
        catch (const LibFred::GetOpenedPublicRequest::Exception& e)
        {
            if (!e.is_set_no_request_found())
            {
                throw;
            }
        }

        const HandleMojeIdArgs* const server_conf_ptr = CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>();
        check_sent_letters_limit(ctx,
                _contact_id,
                server_conf_ptr->letter_limit_count,
                server_conf_ptr->letter_limit_interval);

        LibFred::CreatePublicRequestAuth create_public_request_op;
        create_public_request_op.set_registrar_id(mojeid_registrar_.id());
        create_public_request_op.set_reason("send_new_pin3 call");
        const LibFred::CreatePublicRequestAuth::Result result =
                create_public_request_op.exec(
                        locked_object,
                        has_reidentification_request
                                ? Fred::Backend::MojeId::PublicRequest::ContactReidentification().iface()
                                : Fred::Backend::MojeId::PublicRequest::ContactIdentification().iface(),
                        get_optional_log_request_id(_log_request_id));
        ctx.commit_transaction();
        return;
    }
    catch (const MojeIdImplData::ObjectDoesntExist& e)
    {
        LOGGER.info("ObjectDoesntExist");
        throw;
    }
    catch (const MojeIdImplData::MessageLimitExceeded& e)
    {
        LOGGER.info(e.as_string());
        throw;
    }
    catch (const LibFred::PublicRequestsOfObjectLockGuardByObjectId::Exception& e)
    {
        if (e.is_set_object_doesnt_exist())
        {
            LOGGER.info(e.what());
            throw MojeIdImplData::ObjectDoesntExist();
        }
        LOGGER.error(e.what());
        throw;
    }
    catch (const MojeIdImplData::IdentificationAlreadyProcessed&)
    {
        LOGGER.info("IdentificationAlreadyProcessed");
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown exception");
        throw;
    }
}

void MojeIdImpl::send_mojeid_card(
        ContactId _contact_id,
        MojeIdImpl::LogRequestId _log_request_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(_contact_id).exec(ctx));
        if (states.absents(LibFred::Object_State::mojeid_contact))
        {
            throw MojeIdImplData::ObjectDoesntExist();
        }
        const HandleMojeIdArgs* const server_conf_ptr = CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>();
        const LibFred::InfoContactData data = LibFred::InfoContactById(_contact_id).exec(ctx).info_contact_data;
        const LibFred::Messages::ManagerPtr manager_ptr = LibFred::Messages::create_manager();
        MojeIdImpl::send_mojeid_card(
                ctx,
                manager_ptr.get(),
                data,
                server_conf_ptr->letter_limit_count,
                server_conf_ptr->letter_limit_interval,
                _log_request_id,
                Optional<boost::posix_time::ptime>(),
                states.presents(LibFred::Object_State::validated_contact));
        ctx.commit_transaction();
    }
    catch (const MojeIdImplData::ObjectDoesntExist&)
    {
        LOGGER.info("ObjectDoesntExist");
        throw;
    }
    catch (const MojeIdImplData::MessageLimitExceeded& e)
    {
        LOGGER.info(e.as_string());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown exception");
        throw;
    }
}

void MojeIdImpl::generate_public_request_messages() const
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        Fred::Backend::MojeId::MessengerConfiguration messenger_configuration{
                CfgArgs::instance()->get_handler_ptr_by_type<HandleMessengerArgs>()->messenger_args.endpoint,
                CfgArgs::instance()->get_handler_ptr_by_type<HandleMessengerArgs>()->messenger_args.archive,
                CfgArgs::instance()->get_handler_ptr_by_type<HandleMessengerArgs>()->messenger_args.archive_rendered};

        LibFred::OperationContextCreator ctx;
        const std::string link_hostname_part = CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>()->hostname;
        Messages::Generate::for_new_requests(
                ctx,
                messenger_configuration,
                link_hostname_part);
        ctx.commit_transaction();
        return;
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown exception");
        throw;
    }
}

MojeIdImpl::MessageId MojeIdImpl::send_mojeid_card(
        LibFred::OperationContext& _ctx,
        LibFred::Messages::Manager* _msg_manager_ptr,
        const LibFred::InfoContactData& _data,
        unsigned _limit_count,
        unsigned _limit_interval,
        MojeIdImpl::LogRequestId,
        const Optional<boost::posix_time::ptime>& _letter_time,
        const Optional<bool>& _validated_contact)
{
    cancel_message_sending<MessageType::mojeid_card, CommType::letter>(_ctx, _data.id);
    check_sent_letters_limit(_ctx, _data.id, _limit_count, _limit_interval);
    std::string letter_xml("<?xml version='1.0' encoding='utf-8'?>");

    const std::string name = _data.name.get_value_or_default();
    const std::string::size_type name_delimiter_pos = name.find_last_of(' ');
    const std::string firstname = name_delimiter_pos != std::string::npos
                                          ? name.substr(0, name_delimiter_pos)
                                          : name;
    const std::string lastname = name_delimiter_pos != std::string::npos
                                         ? name.substr(name_delimiter_pos + 1)
                                         : std::string();
    static const char female_suffix[] = "á"; // utf-8 encoded
    enum
    {
        FEMALE_SUFFIX_LEN = sizeof(female_suffix) - 1,
        STR_EQUAL = 0
    };
    const std::string sex = (FEMALE_SUFFIX_LEN <= name.length()) &&
                                            (std::strcmp(name.c_str() + name.length() - FEMALE_SUFFIX_LEN, female_suffix) == STR_EQUAL)
                                    ? "female"
                                    : "male";

    const LibFred::InfoContactData::Address addr = _data.get_address<LibFred::ContactAddressType::MAILING>();
    LibFred::Messages::PostalAddress pa;
    pa.name = name;
    pa.org = _data.organization.get_value_or_default();
    pa.street1 = addr.street1;
    pa.city = addr.city;
    pa.state = addr.stateorprovince.get_value_or_default();
    pa.code = addr.postalcode;
    pa.country = addr.country;

    Database::query_param_list params(pa.country);
    std::string sql = "SELECT (SELECT country_cs FROM enum_country WHERE id=$1::TEXT OR country=$1::TEXT),"
                      "(SELECT country FROM enum_country WHERE id=$1::TEXT)";
    if (!_validated_contact.isset())
    {
        params(_data.id)(Conversion::Enums::to_db_handle(LibFred::Object_State::validated_contact));
        sql.append(",EXISTS(SELECT * FROM object_state "
                   "WHERE object_id=$2::BIGINT AND "
                   "state_id=(SELECT id FROM enum_object_states WHERE name=$3::TEXT) AND "
                   "valid_to IS NULL)");
    }
    const Database::Result dbres = _ctx.get_conn().exec_params(sql, params);
    const std::string addr_country = dbres[0][0].isnull()
                                             ? pa.country
                                             : static_cast<std::string>(dbres[0][0]);
    if (!dbres[0][1].isnull())
    {
        pa.country = static_cast<std::string>(dbres[0][1]);
    }
    const std::string contact_handle = _data.handle;
    const boost::gregorian::date letter_date = _letter_time.isset()
                                                       ? _letter_time.get_value().date()
                                                       : boost::gregorian::day_clock::local_day();
    const std::string contact_state = (_validated_contact.isset() && _validated_contact.get_value()) ||
                                                      (!_validated_contact.isset() && static_cast<bool>(dbres[0][2]))
                                              ? "validated"
                                              : "";

    // clang-format off
    ::Util::XmlTagPair("contact_auth", ::Util::vector_of<::Util::XmlCallback>
        (::Util::XmlTagPair("user", ::Util::vector_of<::Util::XmlCallback>
            (::Util::XmlTagPair("actual_date", ::Util::XmlUnparsedCData(boost::gregorian::to_iso_extended_string(letter_date))))
            (::Util::XmlTagPair("name", ::Util::XmlUnparsedCData(pa.name)))
            (::Util::XmlTagPair("organization", ::Util::XmlUnparsedCData(pa.org)))
            (::Util::XmlTagPair("street", ::Util::XmlUnparsedCData(pa.street1)))
            (::Util::XmlTagPair("city", ::Util::XmlUnparsedCData(pa.city)))
            (::Util::XmlTagPair("stateorprovince", ::Util::XmlUnparsedCData(pa.state)))
            (::Util::XmlTagPair("postal_code", ::Util::XmlUnparsedCData(pa.code)))
            (::Util::XmlTagPair("country", ::Util::XmlUnparsedCData(addr_country)))
            (::Util::XmlTagPair("account", ::Util::vector_of<::Util::XmlCallback>
                (::Util::XmlTagPair("username", ::Util::XmlUnparsedCData(boost::algorithm::to_lower_copy(contact_handle))))
                (::Util::XmlTagPair("first_name", ::Util::XmlUnparsedCData(firstname)))
                (::Util::XmlTagPair("last_name", ::Util::XmlUnparsedCData(lastname)))
                (::Util::XmlTagPair("sex", ::Util::XmlUnparsedCData(sex)))
                (::Util::XmlTagPair("email", ::Util::XmlUnparsedCData(_data.email.get_value_or_default())))
                (::Util::XmlTagPair("mobile", ::Util::XmlUnparsedCData(_data.telephone.get_value_or_default())))
                (::Util::XmlTagPair("state", ::Util::XmlUnparsedCData(contact_state)))
            ))
        ))
    )(letter_xml);
    // clang-format on

    std::stringstream xmldata;
    xmldata << letter_xml;

    const HandleRegistryArgs* const rconf =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>();
    std::unique_ptr<LibFred::Document::Manager> doc_manager_ptr =
            LibFred::Document::Manager::create(
                    rconf->docgen_path,
                    rconf->docgen_template_path,
                    rconf->fileclient_path,
                    CfgArgs::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>()->get_nameservice_host_port());
    enum
    {
        FILETYPE_MOJEID_CARD = 10
    };
    const unsigned long long file_id = doc_manager_ptr->generateDocumentAndSave(
            LibFred::Document::GT_MOJEID_CARD,
            xmldata,
            "mojeid_card-" + boost::lexical_cast<std::string>(_data.id) + "-" +
                    boost::lexical_cast<std::string>(::time(NULL)) + ".pdf",
            FILETYPE_MOJEID_CARD,
            "");

    static const char* const comm_type = "letter";
    static const char* const message_type = "mojeid_card";
    const MessageId message_id =
            _msg_manager_ptr->save_letter_to_send(
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

} // namespace Fred::Backend::MojeId
} // namespace Fred::Backend
} // namespace Fred
