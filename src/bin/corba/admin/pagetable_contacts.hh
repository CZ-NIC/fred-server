/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef PAGETABLE_CONTACTS_HH_1DBBB8837AF745B2837E47F11CEB324F
#define PAGETABLE_CONTACTS_HH_1DBBB8837AF745B2837E47F11CEB324F

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Contacts_i: public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase {
private:
  std::unique_ptr<LibFred::Contact::List> cl;

public:
  ccReg_Contacts_i(LibFred::Contact::List *cl, const Settings *_ptr);
  ~ccReg_Contacts_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  LibFred::Contact::Contact* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_CONTACTS_H_*/
