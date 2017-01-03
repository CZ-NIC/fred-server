#include "src/epp/contact/contact_create_impl.h"
#include "src/epp/disclose_policy.h"

#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"

#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/registrar/info_registrar.h"

namespace Epp {

namespace {

template < ContactDisclose::Item::Enum ITEM >
bool should_item_be_disclosed(const boost::optional< ContactDisclose > &_disclose)
{
    const bool use_the_default_policy = !_disclose.is_initialized();
    if (use_the_default_policy) {
        return is_the_default_policy_to_disclose();
    }
    return _disclose->should_be_disclosed< ITEM >(is_the_default_policy_to_disclose());
}

Optional< std::string > to_db_handle(const Nullable< ContactChange::IdentType::Enum > &src)
{
    if (src.isnull()) {
        return Optional< std::string >();
    }
    switch (src.get_value())
    {
        case ContactChange::IdentType::op:       return Fred::PersonalIdUnion::get_OP("").get_type();
        case ContactChange::IdentType::pass:     return Fred::PersonalIdUnion::get_PASS("").get_type();
        case ContactChange::IdentType::ico:      return Fred::PersonalIdUnion::get_ICO("").get_type();
        case ContactChange::IdentType::mpsv:     return Fred::PersonalIdUnion::get_MPSV("").get_type();
        case ContactChange::IdentType::birthday: return Fred::PersonalIdUnion::get_BIRTHDAY("").get_type();
    }
    throw std::runtime_error("Invalid Epp::ContactChange::IdentType::Enum value.");
}

}//namespace Epp::{anonymous}

ContactCreateResult contact_create_impl(
    Fred::OperationContext &_ctx,
    const std::string &_contact_handle,
    const ContactCreateInputData &_data,
    const unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id)
{
    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    const bool handle_is_valid = Fred::Contact::get_handle_syntax_validity(_contact_handle) ==
                                 Fred::ContactHandleState::SyntaxValidity::valid;
    if (!handle_is_valid) {
        throw InvalidHandle();
    }

    {
        const Fred::ContactHandleState::Registrability::Enum contact_registrability =
            Fred::Contact::get_handle_registrability(_ctx, _contact_handle);

        const bool contact_is_registered = contact_registrability ==
                                           Fred::ContactHandleState::Registrability::registered;
        if (contact_is_registered) {
            throw ObjectExists();
        }

        AggregatedParamErrors exception;

        const bool contact_is_in_protection_period = contact_registrability ==
                                                     Fred::ContactHandleState::Registrability::in_protection_period;
        if (contact_is_in_protection_period) {
            exception.add(Error::of_scalar_parameter(Param::contact_handle, Reason::protected_period));
        }

        if (!is_country_code_valid(_ctx, _data.country_code)) {
            exception.add(Error::of_scalar_parameter(Param::contact_cc, Reason::country_notexist));
        }

        if (!exception.is_empty()) {
            throw exception;
        }
    }

    try {
        Fred::Contact::PlaceAddress place;
        switch (_data.streets.size())
        {
            case 3: place.street3 = _data.streets[2];
            case 2: place.street2 = _data.streets[1];
            case 1: place.street1 = _data.streets[0];
            case 0: break;
            default: throw std::runtime_error("Too many streets.");
        }
        place.city            = _data.city;
        place.stateorprovince = _data.state_or_province;
        place.postalcode      = _data.postal_code;
        place.country         = _data.country_code;

        if (_data.disclose.is_initialized()) {
            _data.disclose->check_validity();
        }

        const Fred::CreateContact create_contact_op(
            _contact_handle,
            Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data.handle,
            _data.authinfo ? Optional<std::string>(*_data.authinfo) : Optional<std::string>() ,
            _data.name,
            _data.organization,
            place,
            _data.telephone,
            _data.fax,
            _data.email,
            _data.notify_email,
            _data.VAT,
            to_db_handle(_data.identtype),
            _data.ident,
            // will be implemented in #13744
            Optional< Fred::ContactAddressList >(),
            should_item_be_disclosed< ContactDisclose::Item::name         >(_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::organization >(_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::address      >(_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::telephone    >(_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::fax          >(_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::email        >(_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::vat          >(_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::ident        >(_data.disclose),
            should_item_be_disclosed< ContactDisclose::Item::notify_email >(_data.disclose),
            _logd_request_id);
        const Fred::CreateContact::Result create_data = create_contact_op.exec(_ctx, "UTC");

        return ContactCreateResult(
            create_data.create_object_result.object_id,
            create_data.create_object_result.history_id,
            create_data.creation_time
        );

    } catch (const Fred::CreateContact::Exception& e) {

        /* general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority */
        if(
            e.is_set_forbidden_company_name_setting() ||
            e.is_set_unknown_registrar_handle() ||
            e.is_set_unknown_ssntype()
        ) {
            throw;
        }

        if( e.is_set_invalid_contact_handle() /* wrong exception name */ ) {
            throw ObjectExists();
        }

        if( e.is_set_unknown_country() ) {
            AggregatedParamErrors exception;
            exception.add(Error::of_scalar_parameter(Param::contact_cc, Reason::country_notexist));
            throw exception;
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}
