#include "nsset_diff.h"

namespace Fred {
namespace NSSet {


std::auto_ptr<const NSSet> get_nsset_by_hid(Manager *_m,
                              const unsigned long long &_hid)
{
    return get_object_by_hid<NSSet, Manager, List, Database::Filters::NSSetHistoryImpl>(_m, _hid);
}



std::string nsset_host_to_simple_string(const Fred::NSSet::Host *_host)
{
  std::string ret;
  unsigned int addr_size = _host->getAddrCount();

  std::string tmp = _host->getName();
  if (addr_size > 0)
    tmp += " ( ";
  for (unsigned int i = 0; i < addr_size; ++i) {
    tmp += _host->getAddrByIdx(i) + " ";
  }
  if (addr_size > 0)
    tmp += ") ";

  ret += tmp;

  return ret;
}



void _diff_nsset(Fred::ChangesMap &_changes,
                 const NSSet *_prev,
                 const NSSet *_act)
{
    if (_prev == _act)
        return;

    const Fred::Object *upcast_prev = dynamic_cast<const Fred::Object*>(_prev);
    const Fred::Object *upcast_act  = dynamic_cast<const Fred::Object*>(_act);

    if (upcast_prev != 0 && upcast_act != 0)
        _diff_object(_changes, upcast_prev, upcast_act);

    compare_and_fill(_changes, "nsset.check_level", _prev->getCheckLevel(), _act->getCheckLevel());
    compare_and_fill_admin_contacts(_changes, "nsset", _prev, _act);

    bool dns_changed = (_prev->getHostCount() != _act->getHostCount());
    for (unsigned int i = 0; dns_changed != true && i < _prev->getHostCount(); ++i) {
        if (*(_prev->getHostByIdx(i)) != *(_act->getHostByIdx(i))) {
            dns_changed = true;
            break;
        }
    }
    if (dns_changed) {
        for (unsigned int j = 0; j < _prev->getHostCount(); ++j) {
            std::string dns_str = nsset_host_to_simple_string(_prev->getHostByIdx(j));
            _changes["nsset.dns." + stringify(j)].first = dns_str;
        }
        for (unsigned int j = 0; j < _act->getHostCount(); ++j) {
            std::string dns_str = nsset_host_to_simple_string(_act->getHostByIdx(j));
            _changes["nsset.dns." + stringify(j)].second = dns_str;
        }
    }
}



Fred::ChangesMap diff(Manager *_m,
                      const unsigned long long &_prev_hid,
                      const unsigned long long &_act_hid)
{
    std::auto_ptr<const NSSet> prev_c = get_nsset_by_hid(_m, _prev_hid);
    std::auto_ptr<const NSSet> act_c = get_nsset_by_hid(_m, _act_hid);

    return diff(prev_c.get(), act_c.get());
}



Fred::ChangesMap diff_last_history(Manager *_m, const unsigned long long &_id)
{
    std::pair<unsigned long long, unsigned long long> history = get_last_history(_id, 2);
    return diff(_m, history.first, history.second);
}



Fred::ChangesMap diff(const NSSet *_prev,
                      const NSSet *_act)
{
    ChangesMap changes;
    _diff_nsset(changes, _prev, _act);
    return changes;
}


}
}

