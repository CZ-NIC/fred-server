#ifndef _INFO_BUFFER_H_
#define _INFO_BUFFER_H_

#include <string>
#include <stdexcept>
#include "types.h"
#include "domain.h"
#include "contact.h"
#include "nsset.h"
#include "keyset.h"
#include "exceptions.h" 

namespace Fred
{
  namespace InfoBuffer
  {
    struct INVALID_REGISTRAR : public std::runtime_error
    {
        INVALID_REGISTRAR()
                : std::runtime_error("InfoBuffer INVALID_REGISTRAR")
        {}
    };
    /// type of info query
    enum Type
    {
      T_LIST_CONTACTS, ///< list contact that belongs to particular registrar
      T_LIST_DOMAINS, ///< list domains that belongs to particular registrar
      T_LIST_NSSETS, ///< list nssets that belongs to particular registrar
      T_LIST_KEYSETS, ///< list keysets that belongs to particular registrar
      T_DOMAINS_BY_NSSET, ///< list domains that are using specified nsset
      T_DOMAINS_BY_CONTACT, ///< list domains somewhat connected to contact
      T_DOMAINS_BY_KEYSET, ///< list domains which are using specified keyset
      T_NSSETS_BY_CONTACT, ///< list nssets with specified admin contact
      T_NSSETS_BY_NS, ///< list nssets containing specified host as nameserver
      T_KEYSETS_BY_CONTACT, ///< list keysets with specifies admin contact
    };
    /// one chunk of data returned by one call of retrieving function
    class Chunk
    {
     public:
      /// destructor 
      virtual ~Chunk() {}
      /// returns number of records in chunk
      virtual unsigned long getCount() const = 0;
      /// returns actual record in chunk and move ahead, at the end return ""
      virtual const std::string& getNext() = 0;
    };
    /// management class for info buffer manipulation
    class Manager
    {
     public:
      /// destructor 
      virtual ~Manager() {}
      /// factory method
      static Manager *create(
        DBSharedPtr db, Domain::Manager *dm,
        NSSet::Manager *nm, 
        Contact::Manager *cm, 
        KeySet::Manager *km
      );
      virtual unsigned long info(
        const std::string &registrar, Type infotype, const std::string& request
      ) throw (SQL_ERROR, INVALID_REGISTRAR) = 0;
      /// fill buffer with result of info command and reset pointer to start
      virtual unsigned long info(
        TID registrar, Type infotype, const std::string& request
      ) throw (SQL_ERROR, INVALID_REGISTRAR) = 0;
      /// get chunk of result of specified size and update pointer 
      virtual Chunk* getChunk(TID registrar, unsigned size) 
        throw (SQL_ERROR, INVALID_REGISTRAR) = 0;
      virtual Chunk* getChunk(const std::string &registrar, unsigned size) 
        throw (SQL_ERROR, INVALID_REGISTRAR) = 0;
    }; 
  };
};

#endif
