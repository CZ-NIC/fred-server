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

#ifndef NOTIFY_HH_CC3CE4F4CE76408D87452BF176947323
#define NOTIFY_HH_CC3CE4F4CE76408D87452BF176947323

#include "src/libfred/mailer.hh"
#include "src/libfred/registrable_object/contact.hh"
#include "src/libfred/registrable_object/nsset.hh"
#include "src/libfred/registrable_object/domain.hh"
#include "src/libfred/registrable_object/keyset.hh"
#include "src/libfred/documents.hh"
#include "src/libfred/exceptions.hh"
#include "src/libfred/registrar.hh"
#include "src/libfred/file_transferer.hh"
#include "src/libfred/messages/messages_impl.hh"
#include "src/deprecated/util/dbsql.hh"


namespace LibFred
{
  namespace Notify
  {
    class Manager
    {
    public:
      virtual ~Manager() {}
      /// notify contacts about state changes
      virtual void notifyStateChanges(
        const std::string& exceptList,
        unsigned limit,
        std::ostream* debugOutput
      ) = 0;
      virtual void generateLetters(unsigned item_count_limit) = 0;
      /// factory method
      static Manager *create(
        DBSharedPtr db,
        Mailer::Manager *mm,
        Contact::Manager *cm,
        Nsset::Manager *nm,
        Keyset::Manager *km,
        Domain::Manager *dm,
        Document::Manager *docm,
        Registrar::Manager *rm,
        Messages::ManagerPtr msgm
      );
    };
  }
}

#endif