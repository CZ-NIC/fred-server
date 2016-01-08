#include "src/fredlib/notifier2/gather_email_data/objecttype_specific_impl/keyset.h"

#include "src/fredlib/notifier2/util/add_old_new_suffix_pair.h"
#include "src/fredlib/notifier2/util/get_previous_object_historyid.h"
#include "src/fredlib/notifier2/util/string_list_utils.h"
#include "src/fredlib/notifier2/exception.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/keyset/info_keyset_diff.h"

#include <boost/foreach.hpp>
#include <boost/algorithm/string/join.hpp>

namespace Notification {

static bool sort_by_key(const Fred::DnsKey& a, const Fred::DnsKey& b) {
    return a.get_key() < b.get_key();
}

static bool equal(const Fred::DnsKey& a, const Fred::DnsKey& b) {
    return a.get_alg()      == b.get_alg()
        && a.get_protocol() == b.get_protocol()
        && a.get_alg()      == b.get_alg()
        && a.get_key()      == b.get_key();
}
static bool equal(const std::vector<Fred::DnsKey>& a, const std::vector<Fred::DnsKey>& b) {
    if( a.size() != b.size() ) {
        return false;
    }
    for(std::vector<Fred::DnsKey>::size_type i = 0; i < a.size(); ++i) {
        if( ! equal(a.at(i), b.at(i)) ) { return false; }
    }
    return true;
}

static std::string dns_key_to_string(const Fred::DnsKey& key) {
    return
        "("
        "flags: "       + boost::lexical_cast<std::string>( key.get_flags()    ) + " "
        "protocol: "    + boost::lexical_cast<std::string>( key.get_protocol() ) + " "
        "algorithm: "   + boost::lexical_cast<std::string>( key.get_alg()      ) + " "
        "key: "         + key.get_key() +
        ")";
}

static std::map<std::string, std::string> gather_keyset_update_data_change(
    const Fred::InfoKeysetData& _before,
    const Fred::InfoKeysetData& _after
) {
    std::map<std::string, std::string> result;

    const Fred::InfoKeysetDiff diff = diff_keyset_data(_before, _after);


    if(diff.authinfopw.isset()) {
        add_old_new_suffix_pair_if_different(
            result, "object.authinfo",
            diff.authinfopw.get_value().first,
            diff.authinfopw.get_value().second
        );
    }

    if(diff.tech_contacts.isset()) {
        add_old_new_suffix_pair_if_different(
            result, "keyset.tech_c",
            boost::algorithm::join( sort( get_handles( diff.tech_contacts.get_value().first  ) ), " " ),
            boost::algorithm::join( sort( get_handles( diff.tech_contacts.get_value().second ) ), " " )
        );
    }

    if(diff.dns_keys.isset()) {
        std::vector<Fred::DnsKey> sorted_keys_old = diff.dns_keys.get_value().first;
        std::sort(sorted_keys_old.begin(), sorted_keys_old.end(), sort_by_key);

        std::vector<Fred::DnsKey> sorted_keys_new = diff.dns_keys.get_value().second;
        std::sort(sorted_keys_new.begin(), sorted_keys_new.end(), sort_by_key);

        if( ! equal(sorted_keys_old, sorted_keys_new) ) {
            result["keyset.dnskey"] = "1";

            for(std::vector<Fred::DnsKey>::size_type i = 0; i < sorted_keys_old.size(); ++i) {
                result["keyset.dnskey.old." + boost::lexical_cast<std::string>(i)] = dns_key_to_string( sorted_keys_old.at(i) );
            }

            for(std::vector<Fred::DnsKey>::size_type i = 0; i < sorted_keys_new.size(); ++i) {
                result["keyset.dnskey.new." + boost::lexical_cast<std::string>(i)] = dns_key_to_string( sorted_keys_new.at(i) );
            }
        }
    }

    result["changes"] = result.empty() ? "0" : "1";

    return result;
}

std::map<std::string, std::string> gather_keyset_data_change(
    Fred::OperationContext& _ctx,
    const notified_event& _event,
    unsigned long long _history_id_post_change
) {

    if( _event != updated ) {

        return std::map<std::string, std::string>();

    } else {

        return gather_keyset_update_data_change(
            Fred::InfoKeysetHistoryByHistoryid(
                null_filter<ExceptionInvalidUpdateEvent>(
                    Fred::get_previous_object_historyid(_ctx, _history_id_post_change)
                )
            ).exec(_ctx).info_keyset_data,
            Fred::InfoKeysetHistoryByHistoryid(_history_id_post_change).exec(_ctx).info_keyset_data
        );
    }

}




static std::set<unsigned long long> get_ids_of_keysets_accepting_notifications(const Fred::InfoKeysetData& _data) {
    std::set<unsigned long long> result;
    BOOST_FOREACH(const Fred::ObjectIdHandlePair& tech_c, _data.tech_contacts) {
        result.insert(tech_c.id);
    }

    return result;
}

std::set<unsigned long long> gather_contact_ids_to_notify_keyset_event(
    Fred::OperationContext& _ctx,
    notified_event _event,
    unsigned long long _history_id_after_change
) {
    std::set<unsigned long long> keyset_ids;

    // always notify new values of notifiable contacts
    {
        const std::set<unsigned long long> keysets_accepting_notifications_after_change = get_ids_of_keysets_accepting_notifications(
            Fred::InfoKeysetHistoryByHistoryid(_history_id_after_change).exec(_ctx).info_keyset_data
        );
        keyset_ids.insert(keysets_accepting_notifications_after_change.begin(), keysets_accepting_notifications_after_change.end());
    }

    // if there were possibly other old values notify those as well
    if( _event == updated ) {
        const std::set<unsigned long long> keysets_accepting_notifications_before_change = get_ids_of_keysets_accepting_notifications(
            Fred::InfoKeysetHistoryByHistoryid(
                null_filter<ExceptionInvalidUpdateEvent>(
                    Fred::get_previous_object_historyid(_ctx, _history_id_after_change)
                )
            ).exec(_ctx).info_keyset_data
        );

        keyset_ids.insert( keysets_accepting_notifications_before_change.begin(), keysets_accepting_notifications_before_change.end() );
    }

    return keyset_ids;
}

}
