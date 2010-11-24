#include "notifier_changes.h"
#include "action.h"
#include "types/stringify.h"


template<class T>
void compare_and_fill(MessageUpdateChanges::ChangesMap &_changes,
                      const std::string &_field, 
                      const T &_prev, 
                      const T &_act)
{
  if (_prev != _act) { 
    std::string prev = stringify(_prev);
    std::string act  = stringify(_act);

    _changes[_field] = std::make_pair(prev, act);
  }
}


template<class T>
void compare_and_fill_admin_contacts(MessageUpdateChanges::ChangesMap &_changes,
                                     const std::string &_field_prefix,
                                     const T *_prev,
                                     const T *_act)
{
  bool changed = (_prev->getAdminCount() != _act->getAdminCount());
  for (unsigned int i = 0; changed != true && i < _prev->getAdminCount(); ++i) {
    if (_prev->getAdminIdByIdx(i) != _act->getAdminIdByIdx(i)) {
      changed = true;
      break;
    }
  }
  if (changed) {
    std::string prev_admins, act_admins;

    for (unsigned int i = 0; i < _prev->getAdminCount(); ++i) 
      prev_admins += _prev->getAdminHandleByIdx(i) + " ";

    for (unsigned int i = 0; i < _act->getAdminCount(); ++i)
      act_admins  += _act->getAdminHandleByIdx(i) + " ";

    _changes[_field_prefix + ".admin_c"] = std::make_pair(prev_admins, act_admins);
  }
}


void compare_and_fill_temp_contacts(MessageUpdateChanges::ChangesMap &_changes,
                                    const std::string &_field_prefix,
                                    const Fred::Domain::Domain *_prev,
                                    const Fred::Domain::Domain *_act)
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




MessageUpdateChanges::ChangesMap MessageUpdateChanges::compose() const
{
  ChangesMap  changes;

  switch (enum_action_) {
    case EPP_ContactUpdate:
      _collectContactChanges(changes);
    break;
    
    case EPP_DomainUpdate:
      _collectDomainChanges(changes);
    break;

    case EPP_NSsetUpdate:
      _collectNSSetChanges(changes);
    break;

    case EPP_KeySetUpdate:
      _collectKeySetChanges(changes);
    break;
  }

  return changes;
}


void MessageUpdateChanges::_collectContactChanges(ChangesMap &_changes) const
{
  Settings     settings;
  settings.set("filter.history", "on");

  std::auto_ptr<Fred::Contact::List> data(rm_->getContactManager()->createList());

  Database::Filters::Union uf(&settings);
  Database::Filters::Contact *filter = new Database::Filters::ContactHistoryImpl();
  filter->addId().setValue(Database::ID(object_id_));
  uf.addFilter(filter);
  
  data->reload(uf);

  if (data->size() < 2)
    throw NoChangesFound();

  unsigned int count = data->size();
  Fred::Contact::Contact *prev = data->getContact(count - 2);
  Fred::Contact::Contact *act  = data->getContact(count - 1);

  _diffContact(_changes, prev, act);
}


void MessageUpdateChanges::_collectDomainChanges(ChangesMap &_changes) const
{
  Settings     settings;
  settings.set("filter.history", "on");

  std::auto_ptr<Fred::Domain::List> data(rm_->getDomainManager()->createList());

  Database::Filters::Union uf(&settings);
  Database::Filters::Domain *filter = new Database::Filters::DomainHistoryImpl();
  filter->addId().setValue(Database::ID(object_id_));
  uf.addFilter(filter);
  
  data->reload(uf);

  if (data->size() < 2)
    throw NoChangesFound();

  unsigned int count = data->size();
  Fred::Domain::Domain *prev = data->getDomain(count - 2);
  Fred::Domain::Domain *act  = data->getDomain(count - 1);

  _diffDomain(_changes, prev, act);
}


void MessageUpdateChanges::_collectNSSetChanges(ChangesMap &_changes) const
{
  Settings     settings;
  settings.set("filter.history", "on");

  std::auto_ptr<Fred::NSSet::List> data(rm_->getNSSetManager()->createList());

  Database::Filters::Union uf(&settings);
  Database::Filters::NSSet *filter = new Database::Filters::NSSetHistoryImpl();
  filter->addId().setValue(Database::ID(object_id_));
  uf.addFilter(filter);
  
  data->reload(uf);

  if (data->size() < 2)
    throw NoChangesFound();

  unsigned int count = data->size();
  Fred::NSSet::NSSet *prev = data->getNSSet(count - 2);
  Fred::NSSet::NSSet *act  = data->getNSSet(count - 1);

  _diffNSSet(_changes, prev, act);
}


void MessageUpdateChanges::_collectKeySetChanges(ChangesMap &_changes) const
{
  Settings     settings;
  settings.set("filter.history", "on");

  std::auto_ptr<Fred::KeySet::List> data(rm_->getKeySetManager()->createList());

  Database::Filters::Union uf(&settings);
  Database::Filters::KeySet *filter = new Database::Filters::KeySetHistoryImpl();
  filter->addId().setValue(Database::ID(object_id_));
  uf.addFilter(filter);
  
  data->reload(uf);

  if (data->size() < 2)
    throw NoChangesFound();

  unsigned int count = data->size();
  Fred::KeySet::KeySet *prev = data->getKeySet(count - 2);
  Fred::KeySet::KeySet *act  = data->getKeySet(count - 1);

  _diffKeySet(_changes, prev, act);
}


void MessageUpdateChanges::_diffObject(ChangesMap &_changes,
                                       const Fred::Object *_prev,
                                       const Fred::Object *_act) const
{
  compare_and_fill(_changes, "object.authinfo", _prev->getAuthPw(), _act->getAuthPw());
}


void MessageUpdateChanges::_diffContact(ChangesMap &_changes, 
                                        const Fred::Contact::Contact *_prev,
                                        const Fred::Contact::Contact *_act) const
{
  if (_prev == _act)
    return;

  const Fred::Object *upcast_prev = dynamic_cast<const Fred::Object*>(_prev);
  const Fred::Object *upcast_act  = dynamic_cast<const Fred::Object*>(_act);

  if (upcast_prev != 0 && upcast_act != 0)
    _diffObject(_changes, upcast_prev, upcast_act);

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


void MessageUpdateChanges::_diffDomain(ChangesMap &_changes, 
                                       const Fred::Domain::Domain *_prev,
                                       const Fred::Domain::Domain *_act) const
{
  if (_prev == _act)
    return;

  const Fred::Object *upcast_prev = dynamic_cast<const Fred::Object*>(_prev);
  const Fred::Object *upcast_act  = dynamic_cast<const Fred::Object*>(_act);

  if (upcast_prev != 0 && upcast_act != 0)
    _diffObject(_changes, upcast_prev, upcast_act);

  compare_and_fill(_changes, "domain.publish", _prev->getPublish(), _act->getPublish());
  compare_and_fill(_changes, "domain.val_ex_date", _prev->getValExDate(), _act->getValExDate());
  compare_and_fill(_changes, "domain.registrant", _prev->getRegistrantHandle(), _act->getRegistrantHandle());
  compare_and_fill(_changes, "domain.nsset", _prev->getNSSetHandle(), _act->getNSSetHandle());
  compare_and_fill(_changes, "domain.keyset", _prev->getKeySetHandle(), _act->getKeySetHandle());
  compare_and_fill_admin_contacts(_changes, "domain", _prev, _act);
  compare_and_fill_temp_contacts(_changes, "domain", _prev, _act);
}


void MessageUpdateChanges::_diffNSSet(ChangesMap &_changes, 
                                      const Fred::NSSet::NSSet *_prev,
                                      const Fred::NSSet::NSSet *_act) const
{
  if (_prev == _act)
    return;

  const Fred::Object *upcast_prev = dynamic_cast<const Fred::Object*>(_prev);
  const Fred::Object *upcast_act  = dynamic_cast<const Fred::Object*>(_act);

  if (upcast_prev != 0 && upcast_act != 0)
    _diffObject(_changes, upcast_prev, upcast_act);

  compare_and_fill(_changes, "nsset.check_level", _prev->getCheckLevel(), _act->getCheckLevel());
  compare_and_fill_admin_contacts(_changes, "nsset", _prev, _act);

  try {
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
  catch (Fred::NOT_FOUND &_ex) {
    // report code error
  }
}


void MessageUpdateChanges::_diffKeySet(ChangesMap &_changes, 
                                       const Fred::KeySet::KeySet *_prev,
                                       const Fred::KeySet::KeySet *_act) const
{
  if (_prev == _act)
    return;

  const Fred::Object *upcast_prev = dynamic_cast<const Fred::Object*>(_prev);
  const Fred::Object *upcast_act  = dynamic_cast<const Fred::Object*>(_act);

  if (upcast_prev != 0 && upcast_act != 0)
    _diffObject(_changes, upcast_prev, upcast_act);

  compare_and_fill_admin_contacts(_changes, "keyset", _prev, _act);

  try {
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
  }
  catch (Fred::NOT_FOUND &_ex) {
    // report code error
  }

  try {
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
  catch (Fred::NOT_FOUND &_ex) {
    // report code error
  }


}

