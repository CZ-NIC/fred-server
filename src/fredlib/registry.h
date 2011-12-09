#ifndef REGISTER_H_
#define REGISTER_H_

#include <string>
#include "domain.h"
#include "registrar.h"
#include "contact.h"
#include "nsset.h"
#include "keyset.h"
#include "public_request/public_request.h"
#include "invoicing/invoice.h"
#include "bank_manager.h"
#include "mail.h"
#include "file.h"
#include "filter.h"
#include "messages/messages_impl.h"

/// forward declared parameter type
class DB;

namespace Fred {
/// Deducted type of checked handle
enum HandleType
{
  HT_ENUM_NUMBER, ///< Handle is number
  HT_ENUM_DOMAIN, ///< Handle is enum domain
  HT_DOMAIN, ///< Handle is non enum domain
  HT_CONTACT, ///< Handle is contact
  HT_NSSET, ///< Handle is nsset
  HT_KEYSET, ///< Handle is keyset
  HT_REGISTRAR, ///< Handle is registrar
  HT_OTHER ///< Invalid handle    
};
/// classification for registry object handle
enum CheckHandleClass
{
  CH_UNREGISTRABLE, ///< Handle is outside of registry
  CH_UNREGISTRABLE_LONG, ///< Handle is too long
  CH_REGISTRED, ///< Handle is registred
  CH_REGISTRED_PARENT, ///< Handle is registred under super domain
  CH_REGISTRED_CHILD, /// < Handle has registred subdomain
  CH_PROTECTED, //< Handle is in protected period or on blacklist
  CH_FREE ///< Handle is free for registration or has unknown stattus
};
/// one classification record 
struct CheckHandle {
  std::string newHandle; ///< transformed handle if appropriate
  std::string conflictHandle; ///< handle that is in conflict
  CheckHandleClass handleClass; ///< result of handle classification
  HandleType type; ///< type of handle
};
/// return type for checkHandle, list of classification records
typedef std::vector<CheckHandle> CheckHandleList;
/// structure with country description
struct CountryDesc {
  std::string cc; ///< country code
  std::string name; ///< country name
};
/// main registry entry class
class Manager {
public:
  /// destructor
  virtual ~Manager() {
  }
  /// classify input handle according registry rules
  virtual void checkHandle(const std::string& handle, 
  											   CheckHandleList& ch, 
  											   bool allowIDN) const = 0;
  /// return message manager
  virtual Messages::ManagerPtr getMessageManager() const = 0;
  /// return zone manager
  virtual Zone::Manager *getZoneManager() const = 0;
  /// return domain manager
  virtual Domain::Manager *getDomainManager() const = 0;
  /// return registrar manager
  virtual Registrar::Manager *getRegistrarManager() const = 0;
  /// return contact manager
  virtual Contact::Manager *getContactManager() const = 0;
  /// return nsset manager
  virtual NSSet::Manager *getNSSetManager() const = 0;
  /// return KeySet manager
  virtual KeySet::Manager *getKeySetManager() const = 0;
  /// return filter manager
  virtual Filter::Manager* getFilterManager() const = 0;
  /// loads country codes description from database
  virtual void loadCountryDesc() = 0;
  /// return size of country description list
  virtual unsigned getCountryDescSize() const = 0;
  /// return country description by index in list
  virtual const CountryDesc& getCountryDescByIdx(unsigned idx) const
      throw (NOT_FOUND) = 0;
  /// initialize list of states
  virtual void initStates() = 0;
  /// return status description
  virtual const StatusDesc* getStatusDesc(TID status) const = 0;
  virtual const StatusDesc* getStatusDesc(const std::string &_name) const = 0;
  /// return status list count 
  virtual unsigned getStatusDescCount() const = 0;
  /// return status desctription by index
  virtual const StatusDesc* getStatusDescByIdx(unsigned idx) const = 0;
  /// globaly update all states of all objects
  virtual void updateObjectStates(unsigned long long _id = 0) //const throw (SQL_ERROR)
          = 0;
  /// temporary for new database manager init
  virtual void dbManagerInit() = 0;
  /// factory method
  static Manager *create(DBSharedPtr db, bool _restrictedHandles);
  /// factory method
  static Manager *create(bool _restricted_handles);
};
}
;

#endif /*REGISTER_H_*/
