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
    HT_ENUM_BLOCK, ///< Handle is block number
    HT_ENUM_NUMBER, ///< Handle is number
    HT_ENUM_DOMAIN_BLOCK, ///< Handle is block enum domain
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
    CH_UNREGISTRABLE, ///< is outside of register
    CH_REGISTRED, ///<  is registred
    CH_REGISTRED_UNDER, ///< is registred under super domain
    CH_REGISTRED_PART, /// < has registred part
    CH_PART, ///< is part of potentialy registrable handle
    CH_LONG, ///< Handle is part of potentialy registrable handle 
    CH_FREE ///< Handle is free for registration or has unknown stattus
  };
  /// return type for checkHandle
  struct CheckHandle 
  {
    std::string newHandle; ///< transformed handle if appropriate
    CheckHandleClass handleClass; ///< result of handle classification
    HandleType type; ///< type of handle
  };
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
      const std::string& handle, CheckHandle& ch
    ) const = 0;
    /// return domain manager
    virtual Domain::Manager *getDomainManager() = 0;
    /// return registrar manager
    virtual Registrar::Manager *getRegistrarManager() = 0;
    /// return contact manager
    virtual Contact::Manager *getContactManager() = 0;
    /// return nsset manager
    virtual NSSet::Manager *getNSSetManager() = 0;
    /// return size of country description list
    virtual unsigned getCountryDescSize() const = 0;
    /// return country description by index in list
    virtual const CountryDesc& getCountryDescByIdx(unsigned idx) const 
     throw (NOT_FOUND) = 0;
    /// factory method
    static Manager *create(DB *db);    
  };
};

#endif /*REGISTER_H_*/
