#include "src/fredlib/notifier/gather_email_data/objecttype_specific_impl/domain.h"

#include "src/fredlib/notifier/gather_email_data/objecttype_specific_impl/util.h"
#include "src/fredlib/notifier/util/add_old_new_suffix_pair.h"
#include "src/fredlib/notifier/util/get_previous_object_historyid.h"
#include "src/fredlib/notifier/util/string_list_utils.h"
#include "src/fredlib/notifier/util/bool_to_string.h"
#include "src/fredlib/notifier/util/boost_date_to_cz_string.h"
#include "src/fredlib/notifier/exception.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/domain/info_domain_diff.h"

#include <boost/foreach.hpp>
#include <boost/algorithm/string/join.hpp>

namespace Notification {

static std::map<std::string, std::string> gather_domain_update_data_change(
    const Fred::InfoDomainData& _before,
    const Fred::InfoDomainData& _after
) {

    std::map<std::string, std::string> result;

    const Fred::InfoDomainDiff diff = diff_domain_data(_before, _after);

    if(diff.authinfopw.isset()) {
        add_old_new_changes_pair_if_different(
            result, "object.authinfo",
            diff.authinfopw.get_value().first,
            diff.authinfopw.get_value().second
        );
    }

    if(diff.registrant.isset()) {
        add_old_new_changes_pair_if_different(
            result, "domain.registrant",
            diff.registrant.get_value().first.handle,
            diff.registrant.get_value().second.handle
        );
    }

    if( diff.nsset.isset() ) {
        add_old_new_changes_pair_if_different(
            result, "domain.nsset",
            diff.nsset.get_value().first.isnull()  ? "" : diff.nsset.get_value().first.get_value().handle,
            diff.nsset.get_value().second.isnull() ? "" : diff.nsset.get_value().second.get_value().handle
        );
    }

    if(diff.keyset.isset()) {
        add_old_new_changes_pair_if_different(
            result, "domain.keyset",
            diff.keyset.get_value().first.isnull()  ? "" : diff.keyset.get_value().first.get_value().handle,
            diff.keyset.get_value().second.isnull() ? "" : diff.keyset.get_value().second.get_value().handle
        );
    }

    if(diff.admin_contacts.isset()) {
        add_old_new_changes_pair_if_different(
            result, "domain.admin_c",
            boost::algorithm::join( sort( get_handles( diff.admin_contacts.get_value().first  ) ), " " ),
            boost::algorithm::join( sort( get_handles( diff.admin_contacts.get_value().second ) ), " " )
        );
    }

    if( diff.enum_domain_validation.isset() ) {

        add_old_new_changes_pair_if_different(
            result, "domain.val_ex_date",
            to_cz_format(
                diff.enum_domain_validation.get_value().first.get_value_or(
                    Fred::ENUMValidationExtension( boost::gregorian::date(boost::gregorian::not_a_date_time), false )
                ).validation_expiration
            ),
            to_cz_format(
                diff.enum_domain_validation.get_value().second.get_value_or(
                    Fred::ENUMValidationExtension( boost::gregorian::date(boost::gregorian::not_a_date_time), false )
                ).validation_expiration
            )
        );
    }

    if( diff.enum_domain_validation.isset() ) {
        add_old_new_changes_pair_if_different(
            result, "domain.publish",
            diff.enum_domain_validation.get_value().first.isnull()  ? "" : to_string( diff.enum_domain_validation.get_value().first.get_value().publish ),
            diff.enum_domain_validation.get_value().second.isnull() ? "" : to_string( diff.enum_domain_validation.get_value().second.get_value().publish )
        );
    }

    result["changes"] = result.empty() ? "0" : "1";

    return result;
}

std::map<std::string, std::string> gather_domain_data_change(
    Fred::OperationContext& _ctx,
    const notified_event& _event,
    unsigned long long _history_id_post_change
) {

    if( _event != updated ) {

        return std::map<std::string, std::string>();

    } else {

        return gather_domain_update_data_change(
            Fred::InfoDomainHistoryByHistoryid(
                Fred::get_previous_object_historyid(_ctx, _history_id_post_change)
                    .get_value_or_throw<ExceptionInvalidUpdateEvent>()
            ).exec(_ctx).info_domain_data,
            Fred::InfoDomainHistoryByHistoryid(_history_id_post_change).exec(_ctx).info_domain_data
        );
    }

}



static std::set<unsigned long long> get_ids_of_contacts_accepting_notifications(const Fred::InfoDomainData& _data) {
    std::set<unsigned long long> result;
    result.insert(_data.registrant.id);
    BOOST_FOREACH(const Fred::ObjectIdHandlePair& admin_c, _data.admin_contacts) {
        result.insert(admin_c.id);
    }

    return result;
}

std::set<unsigned long long> gather_contact_ids_to_notify_domain_event(
    Fred::OperationContext& _ctx,
    notified_event _event,
    unsigned long long _history_id_after_change
) {

    std::set<unsigned long long> contact_ids;

    // always notify new values of notifiable contacts
    {
        const std::set<unsigned long long> contacts_accepting_notifications_after_change = get_ids_of_contacts_accepting_notifications(
            Fred::InfoDomainHistoryByHistoryid(_history_id_after_change).exec(_ctx).info_domain_data
        );
        contact_ids.insert(contacts_accepting_notifications_after_change.begin(), contacts_accepting_notifications_after_change.end());
    }

    // if there were possibly other old values notify those as well
    if( _event == updated ) {
        const unsigned long long history_id_before_change =
            Fred::get_previous_object_historyid(_ctx, _history_id_after_change)
                .get_value_or_throw<ExceptionInvalidUpdateEvent>();
        const std::set<unsigned long long> contacts_accepting_notifications_before_change = get_ids_of_contacts_accepting_notifications(
            Fred::InfoDomainHistoryByHistoryid( history_id_before_change).exec(_ctx).info_domain_data
        );
        contact_ids.insert( contacts_accepting_notifications_before_change.begin(), contacts_accepting_notifications_before_change.end() );

        const Fred::InfoDomainDiff diff = diff_domain_data(
            Fred::InfoDomainHistoryByHistoryid( history_id_before_change ).exec(_ctx).info_domain_data,
            Fred::InfoDomainHistoryByHistoryid( _history_id_after_change ).exec(_ctx).info_domain_data
        );

        if(diff.nsset.isset()) {

            std::set<unsigned long long> nssets;
            if( !diff.nsset.get_value().first.isnull() ) {
                nssets.insert( diff.nsset.get_value().first.get_value().id );
            }
            if( !diff.nsset.get_value().second.isnull() ) {
                nssets.insert( diff.nsset.get_value().second.get_value().id );
            }

            const boost::posix_time::ptime time_of_change = Notification::get_utc_time_of_event(_ctx, _event, _history_id_after_change);

            BOOST_FOREACH(unsigned long long nsset_id, nssets ) {

                bool corresponding_nsset_history_state_found = false;

                BOOST_FOREACH(
                    const Fred::InfoNssetOutput& nsset_history_state,
                    Fred::InfoNssetHistoryById(nsset_id).exec(_ctx, "UTC")
                ) {
                    if(
                        nsset_history_state.history_valid_from <= time_of_change
                        &&
                        time_of_change <= nsset_history_state.history_valid_to.get_value_or(boost::posix_time::pos_infin)
                    ) {
                        corresponding_nsset_history_state_found = true;
                        BOOST_FOREACH(
                            const Fred::ObjectIdHandlePair& tech_c,
                            nsset_history_state.info_nsset_data.tech_contacts
                        ) {
                            contact_ids.insert(tech_c.id);
                        }
                        /* continuing search through other history versions - chances are there might be two history states bordering the domain event */
                    }
                }

                if( !corresponding_nsset_history_state_found ) {
                    throw std::runtime_error("inconsistent data - Nsset that was associated before or after event to domain should exist at that time");
                }
            }
        }

        if(diff.keyset.isset()) {

            std::set<unsigned long long> keysets;
            if( !diff.keyset.get_value().first.isnull() ) {
                keysets.insert( diff.keyset.get_value().first.get_value().id );
            }
            if( !diff.keyset.get_value().second.isnull() ) {
                keysets.insert( diff.keyset.get_value().second.get_value().id );
            }

            const boost::posix_time::ptime time_of_change = Notification::get_utc_time_of_event(_ctx, _event, _history_id_after_change);

            BOOST_FOREACH(unsigned long long keyset_id, keysets ) {

                bool corresponding_keyset_history_state_found = false;

                BOOST_FOREACH(
                    const Fred::InfoKeysetOutput& keyset_history_state,
                    Fred::InfoKeysetHistoryById(keyset_id).exec(_ctx, "UTC")
                ) {
                    if(
                        keyset_history_state.history_valid_from <= time_of_change
                        &&
                        time_of_change <= keyset_history_state.history_valid_to.get_value_or(boost::posix_time::pos_infin)
                    ) {
                        corresponding_keyset_history_state_found = true;
                        BOOST_FOREACH(
                            const Fred::ObjectIdHandlePair& tech_c,
                            keyset_history_state.info_keyset_data.tech_contacts
                        ) {
                            contact_ids.insert(tech_c.id);
                        }
                        /* continuing search through other history versions - chances are there might be two history states bordering the domain event */
                    }
                }

                if( !corresponding_keyset_history_state_found ) {
                    throw std::runtime_error("inconsistent data - Keyset that was associated before or after event to domain should exist at that time");
                }
            }
        }
    }

    return contact_ids;
}

}
