/*  
 * Copyright (C) 2009  CZ.NIC, z.s.p.o.
 * 
 * This file is part of FRED.
 * 
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 * 
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file notifier_changes.h
 *
 *  Helper class for gather changes between last two objects
 *  (domain, contact, nsset, keyset) in history for notification
 *  of EPP update command.
 */


#ifndef NOTIFIER_CHANGES_H_
#define NOTIFIER_CHANGES_H_

#include <map>
#include "fredlib/registry.h"

class MessageUpdateChanges {
public:
  class NoChangesFound { };


  typedef std::map<std::string, std::pair<std::string, std::string> >  ChangesMap;


  MessageUpdateChanges(Fred::Manager *_rm,
                       const unsigned long long &_object_id,
                       const short &_enum_action)
                     : rm_(_rm),
                       object_id_(_object_id),
                       enum_action_(_enum_action)
  {
  }


  ~MessageUpdateChanges()
  {
  }


  MessageUpdateChanges::ChangesMap compose() const;


private:
  Fred::Manager   *rm_;
  unsigned long long   object_id_;
  short                enum_action_;

 
  
  void _collectContactChanges(ChangesMap &_changes) const;


  void _collectDomainChanges(ChangesMap &_changes) const;


  void _collectNSSetChanges(ChangesMap &_changes) const;


  void _collectKeySetChanges(ChangesMap &_changes) const;


  void _diffObject(ChangesMap &_changes, 
                   const Fred::Object *_prev,
                   const Fred::Object *_act) const;


  void _diffContact(ChangesMap &_changes, 
                    const Fred::Contact::Contact *_prev,
                    const Fred::Contact::Contact *_act) const;
  

  void _diffDomain(ChangesMap &_changes, 
                   const Fred::Domain::Domain *_prev,
                   const Fred::Domain::Domain *_act) const;


  void _diffNSSet(ChangesMap &_changes, 
                  const Fred::NSSet::NSSet *_prev,
                  const Fred::NSSet::NSSet *_act) const;


  void _diffKeySet(ChangesMap &_changes, 
                   const Fred::KeySet::KeySet *_prev,
                   const Fred::KeySet::KeySet *_act) const;



};


#endif /*NOTIFIER_CHANGES_H_*/

