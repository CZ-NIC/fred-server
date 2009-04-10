#include "notifier_changes.h"
#include "action.h"
#include "types/stringify.h"


template<class T>
void compare_and_fill(MessageUpdateChanges::changes_map &_changes,
                      const std::string &_field, 
                      const T &_prev, 
                      const T &_act)
{
  if (_prev != _act) { 
    std::string prev = stringify(_prev);
    std::string act  = stringify(_act);

    if (prev.empty())
      prev = "<empty_value>";
    if (act.empty())
      act  = "<empty_value>";

    _changes[_field] = std::make_pair(prev, act);
  }
}


template<class T>
void compare_and_fill_admin_contacts(MessageUpdateChanges::changes_map &_changes,
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

    if (prev_admins.empty())
      prev_admins = "<empty list>";
    if (act_admins.empty())
      act_admins  = "<empty list>";

    _changes["Admin contacts"] = std::make_pair(prev_admins, act_admins);
  }
}


void compare_and_fill_temp_contacts(MessageUpdateChanges::changes_map &_changes,
                                    const Register::Domain::Domain *_prev,
                                    const Register::Domain::Domain *_act)
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

    if (prev_admins.empty())
      prev_admins = "<empty list>";
    if (act_admins.empty())
      act_admins  = "<empty list>";

    _changes["Temporary contacts"] = std::make_pair(prev_admins, act_admins);
  }
}


std::string nsset_host_list_to_simple_string(const Register::NSSet::NSSet *_nsset)
{
  std::string ret;

  for (unsigned int i = 0; i < _nsset->getHostCount(); ++i) {
    const Register::NSSet::Host *host = _nsset->getHostByIdx(i);
    unsigned int addr_size = host->getAddrCount();

    std::string tmp = host->getName();
    if (addr_size > 0)
      tmp += " ( ";
    for (unsigned int n = 0; n < addr_size; ++n) {
      tmp += host->getAddrByIdx(n) + " ";
    }
    if (addr_size > 0)
      tmp += ") ";

    ret += tmp;
  }

  return ret;
}




std::string MessageUpdateChanges::compose() const
{
  changes_map  changes;

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

  
  /* transform map to simple string for output */
  std::string message;
  changes_map::const_iterator it = changes.begin();
  for (; it != changes.end(); ++it) {
    message += it->first + ": " + it->second.first + " => " + it->second.second + "\n";
  }

  return message;
}


void MessageUpdateChanges::_collectContactChanges(changes_map &_changes) const
{
  Settings     settings;
  settings.set("filter.history", "on");

  std::auto_ptr<Register::Contact::List> data(rm_->getContactManager()->createList());

  Database::Filters::Union uf(&settings);
  Database::Filters::Contact *filter = new Database::Filters::ContactHistoryImpl();
  filter->addId().setValue(Database::ID(object_id_));
  uf.addFilter(filter);
  
  data->reload(uf, dbm_);

  if (data->size() < 2)
    throw NoChangesFound();

  unsigned int count = data->size();
  Register::Contact::Contact *prev = data->getContact(count - 2);
  Register::Contact::Contact *act  = data->getContact(count - 1);

  _diffContact(_changes, prev, act);
}


void MessageUpdateChanges::_collectDomainChanges(changes_map &_changes) const
{
  Settings     settings;
  settings.set("filter.history", "on");

  std::auto_ptr<Register::Domain::List> data(rm_->getDomainManager()->createList());

  Database::Filters::Union uf(&settings);
  Database::Filters::Domain *filter = new Database::Filters::DomainHistoryImpl();
  filter->addId().setValue(Database::ID(object_id_));
  uf.addFilter(filter);
  
  data->reload(uf, dbm_);

  if (data->size() < 2)
    throw NoChangesFound();

  unsigned int count = data->size();
  Register::Domain::Domain *prev = data->getDomain(count - 2);
  Register::Domain::Domain *act  = data->getDomain(count - 1);

  _diffDomain(_changes, prev, act);
}


void MessageUpdateChanges::_collectNSSetChanges(changes_map &_changes) const
{
  Settings     settings;
  settings.set("filter.history", "on");

  std::auto_ptr<Register::NSSet::List> data(rm_->getNSSetManager()->createList());

  Database::Filters::Union uf(&settings);
  Database::Filters::NSSet *filter = new Database::Filters::NSSetHistoryImpl();
  filter->addId().setValue(Database::ID(object_id_));
  uf.addFilter(filter);
  
  data->reload(uf, dbm_);

  if (data->size() < 2)
    throw NoChangesFound();

  unsigned int count = data->size();
  Register::NSSet::NSSet *prev = data->getNSSet(count - 2);
  Register::NSSet::NSSet *act  = data->getNSSet(count - 1);

  _diffNSSet(_changes, prev, act);
}


void MessageUpdateChanges::_collectKeySetChanges(changes_map &_changes) const
{
  Settings     settings;
  settings.set("filter.history", "on");

  std::auto_ptr<Register::KeySet::List> data(rm_->getKeySetManager()->createList());

  Database::Filters::Union uf(&settings);
  Database::Filters::KeySet *filter = new Database::Filters::KeySetHistoryImpl();
  filter->addId().setValue(Database::ID(object_id_));
  uf.addFilter(filter);
  
  data->reload(uf, dbm_);

  if (data->size() < 2)
    throw NoChangesFound();

  unsigned int count = data->size();
  Register::KeySet::KeySet *prev = data->getKeySet(count - 2);
  Register::KeySet::KeySet *act  = data->getKeySet(count - 1);

  _diffKeySet(_changes, prev, act);
}


void MessageUpdateChanges::_diffObject(changes_map &_changes,
                                       const Register::Object::Object *_prev,
                                       const Register::Object::Object *_act) const
{
  compare_and_fill(_changes, "Client registrar", _prev->getRegistrarHandle(), _act->getRegistrarHandle());
  compare_and_fill(_changes, "Authinfo password", _prev->getAuthPw(), _act->getAuthPw());
}


void MessageUpdateChanges::_diffContact(changes_map &_changes, 
                                        const Register::Contact::Contact *_prev,
                                        const Register::Contact::Contact *_act) const 
{
  if (_prev == _act)
    return;

  const Register::Object::Object *upcast_prev = dynamic_cast<const Register::Object::Object*>(_prev); 
  const Register::Object::Object *upcast_act  = dynamic_cast<const Register::Object::Object*>(_act);

  if (upcast_prev != 0 && upcast_act != 0)
    _diffObject(_changes, upcast_prev, upcast_act);

  compare_and_fill(_changes, "Name", _prev->getName(), _act->getName());
  compare_and_fill(_changes, "Organization", _prev->getOrganization(), _act->getOrganization());
  compare_and_fill(_changes, "Street1", _prev->getStreet1(), _act->getStreet1());
  compare_and_fill(_changes, "Street2", _prev->getStreet2(), _act->getStreet2());
  compare_and_fill(_changes, "Street3", _prev->getStreet3(), _act->getStreet3());
  compare_and_fill(_changes, "Province", _prev->getProvince(), _act->getProvince());
  compare_and_fill(_changes, "Postal code", _prev->getPostalCode(), _act->getPostalCode());
  compare_and_fill(_changes, "City", _prev->getCity(), _act->getCity());
  compare_and_fill(_changes, "Country", _prev->getCountry(), _act->getCountry());
  compare_and_fill(_changes, "Telephone", _prev->getTelephone(), _act->getTelephone());
  compare_and_fill(_changes, "Fax", _prev->getFax(), _act->getFax());
  compare_and_fill(_changes, "Email", _prev->getEmail(), _act->getEmail());
  compare_and_fill(_changes, "Notify email", _prev->getNotifyEmail(), _act->getNotifyEmail());
  compare_and_fill(_changes, "Ident", _prev->getSSN(), _act->getSSN());
  compare_and_fill(_changes, "Ident type", _prev->getSSNType(), _act->getSSNType());
  compare_and_fill(_changes, "Vat", _prev->getVAT(), _act->getVAT());
  compare_and_fill(_changes, "Disclose Name", _prev->getDiscloseName(), _act->getDiscloseName());
  compare_and_fill(_changes, "Disclose Organization", _prev->getDiscloseOrganization(), _act->getDiscloseOrganization());
  compare_and_fill(_changes, "Disclose Email", _prev->getDiscloseEmail(), _act->getDiscloseEmail());
  compare_and_fill(_changes, "Disclose Address", _prev->getDiscloseAddr(), _act->getDiscloseAddr());
  compare_and_fill(_changes, "Disclose Telephone", _prev->getDiscloseTelephone(), _act->getDiscloseTelephone());
  compare_and_fill(_changes, "Disclose Fax", _prev->getDiscloseFax(), _act->getDiscloseFax());
  compare_and_fill(_changes, "Disclose Email", _prev->getDiscloseEmail(), _act->getDiscloseEmail());
  compare_and_fill(_changes, "Disclose Notify email", _prev->getDiscloseNotifyEmail(), _act->getDiscloseNotifyEmail());
  compare_and_fill(_changes, "Disclose Ident", _prev->getDiscloseIdent(), _act->getDiscloseIdent());
  compare_and_fill(_changes, "Disclose Vat", _prev->getDiscloseVat(), _act->getDiscloseVat());
}


void MessageUpdateChanges::_diffDomain(changes_map &_changes, 
                                       const Register::Domain::Domain *_prev,
                                       const Register::Domain::Domain *_act) const 
{
  if (_prev == _act)
    return;

  const Register::Object::Object *upcast_prev = dynamic_cast<const Register::Object::Object*>(_prev); 
  const Register::Object::Object *upcast_act  = dynamic_cast<const Register::Object::Object*>(_act);

  if (upcast_prev != 0 && upcast_act != 0)
    _diffObject(_changes, upcast_prev, upcast_act);

  compare_and_fill(_changes, "Registrant", _prev->getRegistrantHandle(), _act->getRegistrantHandle());
  compare_and_fill(_changes, "Expiration date", _prev->getExpirationDate(), _act->getExpirationDate());
  compare_and_fill(_changes, "Validation expiration date", _prev->getValExDate(), _act->getValExDate());
  compare_and_fill(_changes, "NSSet", _prev->getNSSetHandle(), _act->getNSSetHandle());
  compare_and_fill(_changes, "KeySet ID", _prev->getKeySetId(), _act->getKeySetId());
  compare_and_fill_admin_contacts(_changes, _prev, _act);
  compare_and_fill_temp_contacts(_changes, _prev, _act);
}


void MessageUpdateChanges::_diffNSSet(changes_map &_changes, 
                                      const Register::NSSet::NSSet *_prev,
                                      const Register::NSSet::NSSet *_act) const 
{
  if (_prev == _act)
    return;

  const Register::Object::Object *upcast_prev = dynamic_cast<const Register::Object::Object*>(_prev); 
  const Register::Object::Object *upcast_act  = dynamic_cast<const Register::Object::Object*>(_act);

  if (upcast_prev != 0 && upcast_act != 0)
    _diffObject(_changes, upcast_prev, upcast_act);

  compare_and_fill(_changes, "Check level", _prev->getCheckLevel(), _act->getCheckLevel());
  compare_and_fill_admin_contacts(_changes, _prev, _act);

  try {
    bool dns_changed = (_prev->getHostCount() != _act->getHostCount());
    for (unsigned int i = 0; dns_changed != true && i < _prev->getHostCount(); ++i) {
      if (*(_prev->getHostByIdx(i)) != *(_act->getHostByIdx(i))) {
        dns_changed = true;
        break;
      }
    }
    if (dns_changed) {
      std::string prev_dns = nsset_host_list_to_simple_string(_prev);
      std::string act_dns  = nsset_host_list_to_simple_string(_act);
  
      _changes["DNS host list"] = make_pair(prev_dns, act_dns);
    }
  }
  catch (Register::NOT_FOUND &_ex) {
    // report code error
  }
}


void MessageUpdateChanges::_diffKeySet(changes_map &_changes, 
                                       const Register::KeySet::KeySet *_prev,
                                       const Register::KeySet::KeySet *_act) const 
{
  if (_prev == _act)
    return;

  const Register::Object::Object *upcast_prev = dynamic_cast<const Register::Object::Object*>(_prev); 
  const Register::Object::Object *upcast_act  = dynamic_cast<const Register::Object::Object*>(_act);

  if (upcast_prev != 0 && upcast_act != 0)
    _diffObject(_changes, upcast_prev, upcast_act);

  compare_and_fill_admin_contacts(_changes, _prev, _act);
}

