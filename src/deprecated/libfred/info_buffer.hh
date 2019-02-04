/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INFO_BUFFER_HH_C831A2B1446141E0B8B832E81B3C18E7
#define INFO_BUFFER_HH_C831A2B1446141E0B8B832E81B3C18E7

#include <string>
#include <stdexcept>
#include "libfred/types.hh"
#include "src/deprecated/libfred/registrable_object/domain.hh"
#include "src/deprecated/libfred/registrable_object/contact.hh"
#include "src/deprecated/libfred/registrable_object/nsset.hh"
#include "src/deprecated/libfred/registrable_object/keyset.hh"
#include "src/deprecated/libfred/exceptions.hh"

namespace LibFred
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
        Nsset::Manager *nm,
        Contact::Manager *cm,
        Keyset::Manager *km
      );
      virtual unsigned long info(
        const std::string &registrar, Type infotype, const std::string& request
      ) = 0;
      /// fill buffer with result of info command and reset pointer to start
      virtual unsigned long info(
        TID registrar, Type infotype, const std::string& request
      ) = 0;
      /// get chunk of result of specified size and update pointer
      virtual Chunk* getChunk(TID registrar, unsigned size)
        = 0;
      virtual Chunk* getChunk(const std::string &registrar, unsigned size)
        = 0;
    };
  };
};

#endif
