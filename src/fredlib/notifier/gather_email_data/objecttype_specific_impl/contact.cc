#include "src/fredlib/notifier/gather_email_data/objecttype_specific_impl/contact.h"

#include "src/fredlib/notifier/util/add_old_new_suffix_pair.h"
#include "src/fredlib/notifier/util/get_previous_object_historyid.h"
#include "src/fredlib/notifier/util/bool_to_string.h"
#include "src/fredlib/notifier/exception.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/info_contact_diff.h"

#include <boost/foreach.hpp>
#include <boost/algorithm/string/join.hpp>

namespace Notification {

std::string to_template_handle(Fred::ContactAddressType::Value _type) {
    switch (_type) {
        case Fred::ContactAddressType::MAILING      : return "mailing";
        case Fred::ContactAddressType::BILLING      : return "billing";
        case Fred::ContactAddressType::SHIPPING     : return "shipping";
        case Fred::ContactAddressType::SHIPPING_2   : return "shipping_2";
        case Fred::ContactAddressType::SHIPPING_3   : return "shipping_3";
    };

    throw ExceptionAddressTypeNotImplemented();
}

static std::map<std::string, std::string> gather_contact_update_data_change(
    const Fred::InfoContactData& _before,
    const Fred::InfoContactData& _after
) {
    std::map<std::string, std::string> result;

    const Fred::InfoContactDiff diff = diff_contact_data(_before, _after);

    if(diff.authinfopw.isset()) {
        add_old_new_changes_pair_if_different(
            result, "object.authinfo",
            diff.authinfopw.get_value().first,
            diff.authinfopw.get_value().second
        );
    }

    if(diff.name.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.name",
            diff.name.get_value().first.get_value_or(""),
            diff.name.get_value().second.get_value_or("")
        );
    }

    if(diff.organization.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.org",
            diff.organization.get_value().first.get_value_or(""),
            diff.organization.get_value().second.get_value_or("")
        );
    }

    struct convert {
        private:
            static void aggregate_nonempty(std::vector<std::string>& _target, const std::string& _raw_input) {
                if( !_raw_input.empty() ) {
                    _target.push_back(_raw_input);
                }
            }
        public:
            static std::string to_string(const Fred::Contact::PlaceAddress& _address) {
                std::vector<std::string> non_empty_parts;

                aggregate_nonempty(non_empty_parts, _address.street1);
                aggregate_nonempty(non_empty_parts, _address.street2.get_value_or(""));
                aggregate_nonempty(non_empty_parts, _address.street3.get_value_or(""));
                aggregate_nonempty(non_empty_parts, _address.stateorprovince.get_value_or(""));
                aggregate_nonempty(non_empty_parts, _address.postalcode);
                aggregate_nonempty(non_empty_parts, _address.city);
                aggregate_nonempty(non_empty_parts, _address.country);

                return boost::join(non_empty_parts, ", ");
            }

            static std::string to_string(const Fred::ContactAddress& _address) {
                std::vector<std::string> non_empty_parts;

                aggregate_nonempty(non_empty_parts, _address.company_name.get_value_or(""));
                aggregate_nonempty(
                    non_empty_parts,
                    to_string(
                        static_cast<const Fred::Contact::PlaceAddress&>(_address)
                    )
                );

                return boost::join(non_empty_parts, ", ");
            }
    };

    if(diff.place.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.address.permanent",
            convert::to_string( diff.place.get_value().first.get_value_or( Fred::Contact::PlaceAddress() ) ),
            convert::to_string( diff.place.get_value().second.get_value_or( Fred::Contact::PlaceAddress() ) )
        );
    }

    if(diff.addresses.isset()) {
        const std::map<Fred::ContactAddressType, Fred::ContactAddress> old_addresses = diff.addresses.get_value().first;
        const std::map<Fred::ContactAddressType, Fred::ContactAddress> new_addresses = diff.addresses.get_value().second;

        BOOST_FOREACH( Fred::ContactAddressType::Value type, Fred::ContactAddressType::get_all() ) {
            const std::map<Fred::ContactAddressType, Fred::ContactAddress>::const_iterator old_it = old_addresses.find(type);
            const std::map<Fred::ContactAddressType, Fred::ContactAddress>::const_iterator new_it = new_addresses.find(type);

            add_old_new_changes_pair_if_different(
                result, "contact.address." + to_template_handle(type),
                old_it != old_addresses.end() ? convert::to_string( old_it->second ) : "",
                new_it != new_addresses.end() ? convert::to_string( new_it->second ) : ""
            );
        }
    }

    if(diff.telephone.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.telephone",
            diff.telephone.get_value().first.get_value_or(""),
            diff.telephone.get_value().second.get_value_or("")
        );
    }

    if(diff.fax.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.fax",
            diff.fax.get_value().first.get_value_or(""),
            diff.fax.get_value().second.get_value_or("")
        );
    }

    if(diff.email.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.email",
            diff.email.get_value().first.get_value_or(""),
            diff.email.get_value().second.get_value_or("")
        );
    }

    if(diff.notifyemail.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.notify_email",
            diff.notifyemail.get_value().first.get_value_or(""),
            diff.notifyemail.get_value().second.get_value_or("")
        );
    }

    /* Yes we are using database "enum" values as e-mail template parameters. It's flexible. And it works! A vubec! */
    struct translate_ssntypes
    {
        static std::string exec(const Nullable< Fred::PersonalIdUnion > &_nullable_personal_id)
        {
            if (_nullable_personal_id.isnull() ||
                _nullable_personal_id.get_value().get_type().empty()) { return ""; }

            const std::string type = _nullable_personal_id.get_value().get_type();

            if (type == "PASS") { return "PASSPORT"; }

            if (type == "RC"   ||
                type == "OP"   ||
                type == "ICO"  ||
                type == "MPSV" ||
                type == "BIRTHDAY") { return type; }

            throw ExceptionUnknownSSNType();
        }
    };

    if (diff.personal_id.isset()) {
        const Nullable< Fred::PersonalIdUnion > nullable_personal_id_a = diff.personal_id.get_value().first;
        const Nullable< Fred::PersonalIdUnion > nullable_personal_id_b = diff.personal_id.get_value().second;
        add_old_new_changes_pair_if_different(
            result, "contact.ident_type",
            translate_ssntypes::exec(nullable_personal_id_a),
            translate_ssntypes::exec(nullable_personal_id_b));
        add_old_new_changes_pair_if_different(
            result, "contact.ident",
            nullable_personal_id_a.get_value_or_default().get(),
            nullable_personal_id_b.get_value_or_default().get());
    }

    if(diff.vat.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.vat",
            diff.vat.get_value().first.get_value_or(""),
            diff.vat.get_value().second.get_value_or("")
        );
    }

    if(diff.disclosename.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.disclose.name",
            to_string( diff.disclosename.get_value().first ),
            to_string( diff.disclosename.get_value().second )
        );
    }

    if(diff.discloseorganization.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.disclose.org",
            to_string( diff.discloseorganization.get_value().first ),
            to_string( diff.discloseorganization.get_value().second )
        );
    }

    if(diff.discloseemail.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.disclose.email",
            to_string( diff.discloseemail.get_value().first ),
            to_string( diff.discloseemail.get_value().second )
        );
    }

    if(diff.discloseaddress.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.disclose.address",
            to_string( diff.discloseaddress.get_value().first ),
            to_string( diff.discloseaddress.get_value().second )
        );
    }

    if(diff.disclosenotifyemail.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.disclose.notify_email",
            to_string( diff.disclosenotifyemail.get_value().first ),
            to_string( diff.disclosenotifyemail.get_value().second )
        );
    }

    if(diff.discloseident.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.disclose.ident",
            to_string( diff.discloseident.get_value().first ),
            to_string( diff.discloseident.get_value().second )
        );
    }

    if(diff.disclosevat.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.disclose.vat",
            to_string( diff.disclosevat.get_value().first ),
            to_string( diff.disclosevat.get_value().second )
        );
    }

    if(diff.disclosetelephone.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.disclose.telephone",
            to_string( diff.disclosetelephone.get_value().first ),
            to_string( diff.disclosetelephone.get_value().second )
        );
    }

    if(diff.disclosefax.isset()) {
        add_old_new_changes_pair_if_different(
            result, "contact.disclose.fax",
            to_string( diff.disclosefax.get_value().first ),
            to_string( diff.disclosefax.get_value().second )
        );
    }

    result["changes"] = result.empty() ? "0" : "1";

    return result;
}

std::map<std::string, std::string> gather_contact_data_change(
    Fred::OperationContext& _ctx,
    const notified_event& _event,
    unsigned long long _history_id_post_change
) {

    if( _event != updated ) {

        return std::map<std::string, std::string>();

    } else {

        return gather_contact_update_data_change(
            Fred::InfoContactHistoryByHistoryid(
                Fred::get_previous_object_historyid(_ctx, _history_id_post_change)
                    .get_value_or_throw<ExceptionInvalidUpdateEvent>()
            ).exec(_ctx).info_contact_data,
            Fred::InfoContactHistoryByHistoryid(_history_id_post_change).exec(_ctx).info_contact_data
        );

    }

}

std::set<std::string> get_emails_to_notify_contact_event(
    Fred::OperationContext& _ctx,
    notified_event _event,
    unsigned long long _history_id_after_change
) {
    std::set<std::string> emails_to_notify;

    // always notify new value of notify_email if present
    {
        Nullable<std::string> notify_email = Fred::InfoContactHistoryByHistoryid(_history_id_after_change).exec(_ctx).info_contact_data.notifyemail;
        if( !notify_email.get_value_or("").empty() ) {
            emails_to_notify.insert( notify_email.get_value() );
        }
    }

    // if there were possibly other old values notify those as well
    if( _event == updated ) {

        Nullable<std::string> notify_email =
            Fred::InfoContactHistoryByHistoryid(
                Fred::get_previous_object_historyid(_ctx, _history_id_after_change)
                    .get_value_or_throw<ExceptionInvalidUpdateEvent>()
            )
            .exec(_ctx)
            .info_contact_data.notifyemail;

        if( !notify_email.get_value_or("").empty() ) {
            emails_to_notify.insert( notify_email.get_value() );
        }
    }

    return emails_to_notify;
}

}