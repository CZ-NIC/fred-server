#ifndef REGISTER_H_
#define REGISTER_H_

#include <string>
#include "domain.h"
#include "registrar.h"
#include "contact.h"
#include "nsset.h"
#include "auth_info.h"

/// forward declared parameter type
class DB;

namespace Register
{
  /// Deducted type of checked handle
  enum HandleType 
  {
    HT_ENUM_NUMBER, ///< Handle is number
    HT_ENUM_DOMAIN, ///< Handle is enum domain
    HT_DOMAIN, ///< Handle is non enum domain
    HT_CONTACT, ///< Handle is contact
    HT_NSSET, ///< Handle is nsset
    HT_REGISTRAR, ///< Handle is registrar
    HT_OTHER ///< Invalid handle    
  };
  /// classification for register object handle 
  enum CheckHandleClass 
  {
    CH_UNREGISTRABLE, ///< Handle is outside of register
    CH_UNREGISTRABLE_LONG, ///< Handle is too long
    CH_REGISTRED, ///< Handle is registred
    CH_REGISTRED_PARENT, ///< Handle is registred under super domain
    CH_REGISTRED_CHILD, /// < Handle has registred subdomain
    CH_PROTECTED, //< Handle is in protected period or on blacklist
    CH_FREE ///< Handle is free for registration or has unknown stattus
  };
  /// one classification record 
  struct CheckHandle 
  {
    std::string newHandle; ///< transformed handle if appropriate
    std::string conflictHandle; ///< handle that is in conflict
    CheckHandleClass handleClass; ///< result of handle classification
    HandleType type; ///< type of handle
  };
  /// return type for checkHandle, list of classification records
  typedef std::vector<CheckHandle> CheckHandleList;
  /// structure with country description
  struct CountryDesc
  {
    std::string cc; ///< country code
    std::string name; ///< country name
  };
  /// main register entry class
  class Manager
  {
   public:
    /// destructor
    virtual ~Manager() {}
    /// classify input handle according register rules 
    virtual void checkHandle(
      const std::string& handle, CheckHandleList& ch
    ) const = 0;
    /// return domain manager
    virtual Domain::Manager *getDomainManager() = 0;
    /// return registrar manager
    virtual Registrar::Manager *getRegistrarManager() const = 0;
    /// return contact manager
    virtual Contact::Manager *getContactManager() const = 0;
    /// return nsset manager
    virtual NSSet::Manager *getNSSetManager() const = 0;
    /// return size of country description list
    virtual unsigned getCountryDescSize() const = 0;
    /// return country description by index in list
    virtual const CountryDesc& getCountryDescByIdx(unsigned idx) const 
     throw (NOT_FOUND) = 0;
    /// factory method
    static Manager *create(DB *db, bool restrictedHandles);    
  };
};

#endif /*REGISTER_H_*/
