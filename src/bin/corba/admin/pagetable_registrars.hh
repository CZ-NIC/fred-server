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
#ifndef PAGETABLE_REGISTRARS_HH_21777BFEC188458A863AC807DD811054
#define PAGETABLE_REGISTRARS_HH_21777BFEC188458A863AC807DD811054

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Registrars_i : public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
private:
  LibFred::Registrar::RegistrarList::AutoPtr rl;
  LibFred::Zone::Manager::ZoneListPtr zl;
  LibFred::Registrar::RegistrarZoneAccess rza;

  enum cols {static_cols = 4};///number of static columns in pagetable

public:
  ccReg_Registrars_i(LibFred::Registrar::RegistrarList::AutoPtr _rl
					  , LibFred::Zone::Manager::ZoneListPtr _zl);
  ~ccReg_Registrars_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  LibFred::Registrar::Registrar* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_REGISTRARS_H_*/
