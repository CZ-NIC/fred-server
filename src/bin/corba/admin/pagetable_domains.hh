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
#ifndef PAGETABLE_DOMAINS_HH_4C3AC2808BAE4DC68423C4FA72C0997B
#define PAGETABLE_DOMAINS_HH_4C3AC2808BAE4DC68423C4FA72C0997B

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Domains_i : public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase {
private:
  std::unique_ptr<LibFred::Domain::List> dl;

public:
  ccReg_Domains_i(LibFred::Domain::List *dl, const Settings *_ptr);
  ~ccReg_Domains_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  LibFred::Domain::Domain* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_DOMAINS_H_*/
