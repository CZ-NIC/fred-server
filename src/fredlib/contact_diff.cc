#include "contact_diff.h"

namespace Fred {
namespace Contact {


std::auto_ptr<const Contact> get_contact_by_hid(Manager *_m,
                                  const unsigned long long &_hid)
{
    return get_object_by_hid<Contact, Manager, List, Database::Filters::ContactHistoryImpl>(_m, _hid);
}



std::string address_to_simple_string(const Fred::Contact::Contact *_contact)
{
  std::string ret;

  ret += _contact->getStreet1();
  if (!_contact->getStreet2().empty())
    ret += ", " + _contact->getStreet2();
  if (!_contact->getStreet3().empty())
    ret += ", " + _contact->getStreet3();
  if (!_contact->getProvince().empty())
    ret += ", " + _contact->getProvince();
  if (!_contact->getPostalCode().empty())
    ret += ", " + _contact->getPostalCode();
  if (!_contact->getCity().empty())
    ret += ", " + _contact->getCity();
  if (!_contact->getCountry().empty())
    ret += ", " + _contact->getCountry();

  return ret;
}



void _diff_contact(Fred::ChangesMap &_changes,
                   const Contact *_prev,
                   const Contact *_act)
{
    if (_prev == _act) {
        return;
    }

    const Fred::Object *upcast_prev = dynamic_cast<const Fred::Object*>(_prev);
    const Fred::Object *upcast_act  = dynamic_cast<const Fred::Object*>(_act);

    if (upcast_prev != 0 && upcast_act != 0) {
        _diff_object(_changes, upcast_prev, upcast_act);
    }

    compare_and_fill(_changes, "contact.name", _prev->getName(), _act->getName());
    compare_and_fill(_changes, "contact.org", _prev->getOrganization(), _act->getOrganization());
    compare_and_fill(_changes, "contact.street1", _prev->getStreet1(), _act->getStreet1());
    compare_and_fill(_changes, "contact.street2", _prev->getStreet2(), _act->getStreet2());
    compare_and_fill(_changes, "contact.street3", _prev->getStreet3(), _act->getStreet3());
    compare_and_fill(_changes, "contact.province", _prev->getProvince(), _act->getProvince());
    compare_and_fill(_changes, "contact.postal_code", _prev->getPostalCode(), _act->getPostalCode());
    compare_and_fill(_changes, "contact.city", _prev->getCity(), _act->getCity());
    compare_and_fill(_changes, "contact.country", _prev->getCountry(), _act->getCountry());
    compare_and_fill(_changes, "contact.telephone", _prev->getTelephone(), _act->getTelephone());
    compare_and_fill(_changes, "contact.fax", _prev->getFax(), _act->getFax());
    compare_and_fill(_changes, "contact.email", _prev->getEmail(), _act->getEmail());
    compare_and_fill(_changes, "contact.notify_email", _prev->getNotifyEmail(), _act->getNotifyEmail());
    compare_and_fill(_changes, "contact.ident", _prev->getSSN(), _act->getSSN());
    compare_and_fill(_changes, "contact.ident_type", _prev->getSSNType(), _act->getSSNType());
    compare_and_fill(_changes, "contact.vat", _prev->getVAT(), _act->getVAT());
    compare_and_fill(_changes, "contact.disclose.name", _prev->getDiscloseName(), _act->getDiscloseName());
    compare_and_fill(_changes, "contact.disclose.org", _prev->getDiscloseOrganization(), _act->getDiscloseOrganization());
    compare_and_fill(_changes, "contact.disclose.email", _prev->getDiscloseEmail(), _act->getDiscloseEmail());
    compare_and_fill(_changes, "contact.disclose.address", _prev->getDiscloseAddr(), _act->getDiscloseAddr());
    compare_and_fill(_changes, "contact.disclose.telephone", _prev->getDiscloseTelephone(), _act->getDiscloseTelephone());
    compare_and_fill(_changes, "contact.disclose.fax", _prev->getDiscloseFax(), _act->getDiscloseFax());
    compare_and_fill(_changes, "contact.disclose.email", _prev->getDiscloseEmail(), _act->getDiscloseEmail());
    compare_and_fill(_changes, "contact.disclose.notify_email", _prev->getDiscloseNotifyEmail(), _act->getDiscloseNotifyEmail());
    compare_and_fill(_changes, "contact.disclose.ident", _prev->getDiscloseIdent(), _act->getDiscloseIdent());
    compare_and_fill(_changes, "contact.disclose.vat", _prev->getDiscloseVat(), _act->getDiscloseVat());
    compare_and_fill(_changes, "contact.address", address_to_simple_string(_prev), address_to_simple_string(_act));
}



Fred::ChangesMap diff(Manager *_m,
                      const unsigned long long &_prev_hid,
                      const unsigned long long &_act_hid)
{
    std::auto_ptr<const Contact> prev_c = get_contact_by_hid(_m, _prev_hid);
    std::auto_ptr<const Contact> act_c = get_contact_by_hid(_m, _act_hid);

    return diff(prev_c.get(), act_c.get());
}



Fred::ChangesMap diff_last_history(Manager *_m, const unsigned long long &_id)
{
    std::pair<unsigned long long, unsigned long long> history = get_last_history(_id, 1);
    return diff(_m, history.first, history.second);
}



Fred::ChangesMap diff(const Contact *_prev,
                      const Contact *_act)
{
    ChangesMap changes;
    _diff_contact(changes, _prev, _act);
    return changes;
}


}
}

