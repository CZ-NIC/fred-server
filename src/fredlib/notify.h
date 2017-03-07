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

#ifndef NOTIFY_H_D320FB5E3AEC489188346088EA8FD72E
#define NOTIFY_H_D320FB5E3AEC489188346088EA8FD72E

#include "src/fredlib/mailer.h"
#include "src/fredlib/contact.h"
#include "src/fredlib/nsset.h"
#include "src/fredlib/domain.h"
#include "src/fredlib/keyset.h"
#include "src/fredlib/documents.h"
#include "src/fredlib/exceptions.h"
#include "src/fredlib/registrar.h"
#include "src/fredlib/file_transferer.h"
#include "src/fredlib/messages/messages_impl.h"
#include "src/old_utils/dbsql.h"


namespace Fred
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
        std::ostream *debugOutput,
        bool useHistory
      ) throw (SQL_ERROR) = 0;
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
