#include "keyset_diff.h"

namespace Fred {
namespace KeySet {


std::auto_ptr<const KeySet> get_keyset_by_hid(Manager *_m,
                                const unsigned long long &_hid)
{
    return get_object_by_hid<KeySet, Manager, List, Database::Filters::KeySetHistoryImpl>(_m, _hid);
}



std::string dsrecord_to_simple_string(const Fred::KeySet::DSRecord *_ds)
{
  std::string ret;

  ret += "(keytag: "       + stringify(_ds->getKeyTag())
      + " algorithm: "     + stringify(_ds->getAlg())
      + " digest type: "   + stringify(_ds->getDigestType())
      + " digest: "        + stringify(_ds->getDigest())
      + " max sig. life: " + stringify(_ds->getMaxSigLife())
      + ")";

  return ret;
}



std::string dnskey_to_simple_string(const Fred::KeySet::DNSKey *_dns)
{
  std::string ret;

  ret += "(flags: "     + stringify(_dns->getFlags())
      + " protocol: "   + stringify(_dns->getProtocol())
      + " algorithm: "  + stringify(_dns->getAlg())
      + " key: "        + stringify(_dns->getKey())
      + ")";

  return ret;
}



void _diff_keyset(Fred::ChangesMap &_changes,
                  const KeySet *_prev,
                  const KeySet *_act)
{
    if (_prev == _act)
        return;

    const Fred::Object *upcast_prev = dynamic_cast<const Fred::Object*>(_prev);
    const Fred::Object *upcast_act  = dynamic_cast<const Fred::Object*>(_act);

    if (upcast_prev != 0 && upcast_act != 0)
        _diff_object(_changes, upcast_prev, upcast_act);

    compare_and_fill_admin_contacts(_changes, "keyset", _prev, _act);

    bool ds_changed = (_prev->getDSRecordCount() != _act->getDSRecordCount());
    for (unsigned int i = 0; ds_changed != true && i < _prev->getDSRecordCount(); ++i) {
        if (*(_prev->getDSRecordByIdx(i)) != *(_act->getDSRecordByIdx(i))) {
            ds_changed = true;
            break;
        }
    }
    if (ds_changed) {
        for (unsigned int j = 0; j < _prev->getDSRecordCount(); ++j) {
            std::string ds_str = dsrecord_to_simple_string(_prev->getDSRecordByIdx(j));
            _changes["keyset.ds." + stringify(j)].first = ds_str;
        }
        for (unsigned int j = 0; j < _act->getDSRecordCount(); ++j) {
            std::string ds_str = dsrecord_to_simple_string(_act->getDSRecordByIdx(j));
            _changes["keyset.ds." + stringify(j)].second = ds_str;
        }
    }

    bool key_changed = (_prev->getDNSKeyCount() != _act->getDNSKeyCount());
    for (unsigned int i = 0; key_changed != true && i < _prev->getDNSKeyCount(); ++i) {
        if (*(_prev->getDNSKeyByIdx(i)) != *(_act->getDNSKeyByIdx(i))) {
            key_changed = true;
            break;
        }
    }
    if (key_changed) {
        for (unsigned int j = 0; j < _prev->getDNSKeyCount(); ++j) {
            std::string dns_str = dnskey_to_simple_string(_prev->getDNSKeyByIdx(j));
            _changes["keyset.dnskey." + stringify(j)].first = dns_str;
        }
        for (unsigned int j = 0; j < _act->getDNSKeyCount(); ++j) {
            std::string dns_str = dnskey_to_simple_string(_act->getDNSKeyByIdx(j));
            _changes["keyset.dnskey." + stringify(j)].second = dns_str;
        }
    }
}


Fred::ChangesMap diff(Manager *_m,
                      const unsigned long long &_prev_hid,
                      const unsigned long long &_act_hid)
{
    std::auto_ptr<const KeySet> prev_c = get_keyset_by_hid(_m, _prev_hid);
    std::auto_ptr<const KeySet> act_c = get_keyset_by_hid(_m, _act_hid);

    return diff(prev_c.get(), act_c.get());
}



Fred::ChangesMap diff_last_history(Manager *_m, const unsigned long long &_id)
{
    std::pair<unsigned long long, unsigned long long> history = get_last_history(_id, 4);
    return diff(_m, history.first, history.second);
}



Fred::ChangesMap diff(const KeySet *_prev,
                      const KeySet *_act)
{
    ChangesMap changes;
    _diff_keyset(changes, _prev, _act);
    return changes;
}


}
}


