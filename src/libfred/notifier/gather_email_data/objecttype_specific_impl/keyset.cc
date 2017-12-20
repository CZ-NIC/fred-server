#include "src/libfred/notifier/gather_email_data/objecttype_specific_impl/keyset.hh"

#include "src/libfred/notifier/util/add_old_new_suffix_pair.hh"
#include "src/libfred/notifier/util/get_previous_object_historyid.hh"
#include "src/libfred/notifier/util/string_list_utils.hh"
#include "src/libfred/notifier/exception.hh"
#include "src/libfred/registrable_object/keyset/info_keyset.hh"
#include "src/libfred/registrable_object/keyset/info_keyset_diff.hh"

#include <boost/foreach.hpp>
#include <boost/algorithm/string/join.hpp>

namespace Notification {

static bool sort_by_key(const LibFred::DnsKey& a, const LibFred::DnsKey& b) {
    return a.get_key() < b.get_key();
}

static bool equal(const LibFred::DnsKey& a, const LibFred::DnsKey& b) {
    return a.get_alg()      == b.get_alg()
        && a.get_protocol() == b.get_protocol()
        && a.get_alg()      == b.get_alg()
        && a.get_key()      == b.get_key();
}
static bool equal(const std::vector<LibFred::DnsKey>& a, const std::vector<LibFred::DnsKey>& b) {
    if( a.size() != b.size() ) {
        return false;
    }
    for(std::vector<LibFred::DnsKey>::size_type i = 0; i < a.size(); ++i) {
        if( ! equal(a.at(i), b.at(i)) ) { return false; }
    }
    return true;
}

static std::string dns_key_to_string(const LibFred::DnsKey& key) {
    return
        "("
        "flags: "       + boost::lexical_cast<std::string>( key.get_flags()    ) + " "
        "protocol: "    + boost::lexical_cast<std::string>( key.get_protocol() ) + " "
        "algorithm: "   + boost::lexical_cast<std::string>( key.get_alg()      ) + " "
        "key: "         + key.get_key() +
        ")";
}

static std::map<std::string, std::string> gather_keyset_update_data_change(
    const LibFred::InfoKeysetData& _before,
    const LibFred::InfoKeysetData& _after
) {
    std::map<std::string, std::string> result;

    const LibFred::InfoKeysetDiff diff = diff_keyset_data(_before, _after);


    if(diff.authinfopw.isset()) {
        add_old_new_changes_pair_if_different(
            result, "object.authinfo",
            diff.authinfopw.get_value().first,
            diff.authinfopw.get_value().second
        );
    }

    if(diff.tech_contacts.isset()) {
        add_old_new_changes_pair_if_different(
            result, "keyset.tech_c",
            boost::algorithm::join( sort( get_handles( diff.tech_contacts.get_value().first  ) ), " " ),
            boost::algorithm::join( sort( get_handles( diff.tech_contacts.get_value().second ) ), " " )
        );
    }

    if(diff.dns_keys.isset()) {
        std::vector<LibFred::DnsKey> sorted_keys_old = diff.dns_keys.get_value().first;
        std::sort(sorted_keys_old.begin(), sorted_keys_old.end(), sort_by_key);

        std::vector<LibFred::DnsKey> sorted_keys_new = diff.dns_keys.get_value().second;
        std::sort(sorted_keys_new.begin(), sorted_keys_new.end(), sort_by_key);

        if( ! equal(sorted_keys_old, sorted_keys_new) ) {
            result["changes.keyset.dnskey"] = "1";

            for(std::vector<LibFred::DnsKey>::size_type i = 0; i < sorted_keys_old.size(); ++i) {
                result["changes.keyset.dnskey.old." + boost::lexical_cast<std::string>(i)] = dns_key_to_string( sorted_keys_old.at(i) );
            }

            for(std::vector<LibFred::DnsKey>::size_type i = 0; i < sorted_keys_new.size(); ++i) {
                result["changes.keyset.dnskey.new." + boost::lexical_cast<std::string>(i)] = dns_key_to_string( sorted_keys_new.at(i) );
            }
        }
    }

    result["changes"] = result.empty() ? "0" : "1";

    return result;
}

std::map<std::string, std::string> gather_keyset_data_change(
    LibFred::OperationContext& _ctx,
    const notified_event& _event,
    unsigned long long _history_id_post_change
) {

    if( _event != updated ) {

        return std::map<std::string, std::string>();

    } else {

        return gather_keyset_update_data_change(
            LibFred::InfoKeysetHistoryByHistoryid(
                LibFred::get_previous_object_historyid(_ctx, _history_id_post_change)
                    .get_value_or_throw<ExceptionInvalidUpdateEvent>()
            ).exec(_ctx).info_keyset_data,
            LibFred::InfoKeysetHistoryByHistoryid(_history_id_post_change).exec(_ctx).info_keyset_data
        );
    }

}




static std::set<unsigned long long> get_ids_of_keysets_accepting_notifications(const LibFred::InfoKeysetData& _data) {
    std::set<unsigned long long> result;
    BOOST_FOREACH(const LibFred::ObjectIdHandlePair& tech_c, _data.tech_contacts) {
        result.insert(tech_c.id);
    }

    return result;
}

std::set<unsigned long long> gather_contact_ids_to_notify_keyset_event(
    LibFred::OperationContext& _ctx,
    notified_event _event,
    unsigned long long _history_id_after_change
) {
    std::set<unsigned long long> keyset_ids;

    // always notify new values of notifiable contacts
    {
        const std::set<unsigned long long> keysets_accepting_notifications_after_change = get_ids_of_keysets_accepting_notifications(
            LibFred::InfoKeysetHistoryByHistoryid(_history_id_after_change).exec(_ctx).info_keyset_data
        );
        keyset_ids.insert(keysets_accepting_notifications_after_change.begin(), keysets_accepting_notifications_after_change.end());
    }

    // if there were possibly other old values notify those as well
    if( _event == updated ) {
        const std::set<unsigned long long> keysets_accepting_notifications_before_change = get_ids_of_keysets_accepting_notifications(
            LibFred::InfoKeysetHistoryByHistoryid(
                LibFred::get_previous_object_historyid(_ctx, _history_id_after_change)
                    .get_value_or_throw<ExceptionInvalidUpdateEvent>()
            ).exec(_ctx).info_keyset_data
        );

        keyset_ids.insert( keysets_accepting_notifications_before_change.begin(), keysets_accepting_notifications_before_change.end() );
    }

    return keyset_ids;
}

}
