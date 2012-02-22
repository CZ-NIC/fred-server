#include "domain_diff.h"

namespace Fred {
namespace Domain {


std::auto_ptr<const Domain> get_domain_by_hid(Manager *_m,
                                const unsigned long long &_hid)
{
    return get_object_by_hid<Domain, Manager, List, Database::Filters::DomainHistoryImpl>(_m, _hid);
}



void compare_and_fill_temp_contacts(Fred::ChangesMap &_changes,
                                    const std::string &_field_prefix,
                                    const Domain *_prev,
                                    const Domain *_act)
{
    bool changed = (_prev->getAdminCount(2) != _act->getAdminCount(2));
    for (unsigned int i = 0; changed != true && i < _prev->getAdminCount(2); ++i) {
        if (_prev->getAdminIdByIdx(i, 2) != _act->getAdminIdByIdx(i, 2)) {
            changed = true;
            break;
        }
    }
    if (changed) {
        std::string prev_admins, act_admins;

        for (unsigned int i = 0; i < _prev->getAdminCount(2); ++i)
            prev_admins += _prev->getAdminHandleByIdx(i, 2) + " ";

        for (unsigned int i = 0; i < _act->getAdminCount(2); ++i)
            act_admins  += _act->getAdminHandleByIdx(i, 2) + " ";

        _changes[_field_prefix + ".temp_c"] = std::make_pair(prev_admins, act_admins);
    }
}



void _diff_domain(Fred::ChangesMap &_changes,
                  const Domain *_prev,
                  const Domain *_act)
{
    if (_prev == _act)
        return;

    const Fred::Object *upcast_prev = dynamic_cast<const Fred::Object*>(_prev);
    const Fred::Object *upcast_act  = dynamic_cast<const Fred::Object*>(_act);

    if (upcast_prev != 0 && upcast_act != 0)
        _diff_object(_changes, upcast_prev, upcast_act);

    compare_and_fill(_changes, "domain.publish", _prev->getPublish(), _act->getPublish());
    compare_and_fill(_changes, "domain.val_ex_date", _prev->getValExDate(), _act->getValExDate());
    compare_and_fill(_changes, "domain.registrant", _prev->getRegistrantHandle(), _act->getRegistrantHandle());
    compare_and_fill(_changes, "domain.nsset", _prev->getNSSetHandle(), _act->getNSSetHandle());
    compare_and_fill(_changes, "domain.keyset", _prev->getKeySetHandle(), _act->getKeySetHandle());
    compare_and_fill_admin_contacts(_changes, "domain", _prev, _act);
    compare_and_fill_temp_contacts(_changes, "domain", _prev, _act);
}



Fred::ChangesMap diff(Manager *_m,
                      const unsigned long long &_prev_hid,
                      const unsigned long long &_act_hid)
{
    std::auto_ptr<const Domain> prev_d = get_domain_by_hid(_m, _prev_hid);
    std::auto_ptr<const Domain> act_d = get_domain_by_hid(_m, _act_hid);

    return diff(prev_d.get(), act_d.get());
}



Fred::ChangesMap diff_last_history(Manager *_m, const unsigned long long &_id)
{
    std::pair<unsigned long long, unsigned long long> history = get_last_history(_id, 3);
    return diff(_m, history.first, history.second);
}



Fred::ChangesMap diff(const Domain *_prev,
                      const Domain *_act)
{
    ChangesMap changes;
    _diff_domain(changes, _prev, _act);
    return changes;
}


}
}

