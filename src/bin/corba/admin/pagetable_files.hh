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
#ifndef PAGETABLE_FILES_HH_FBEA4C0A6EAB48FAB91EE21A7845E074
#define PAGETABLE_FILES_HH_FBEA4C0A6EAB48FAB91EE21A7845E074

#include "src/bin/corba/admin/pagetable_impl.hh"
#include "src/deprecated/libfred/file.hh"

class ccReg_Files_i : public ccReg_PageTable_i,
                      public PortableServer::RefCountServantBase {
private:
  std::unique_ptr<LibFred::File::List> file_list_;

public:
  ccReg_Files_i(LibFred::File::List *_list);
  ~ccReg_Files_i();
  DECL_PAGETABLE_I;

  ccReg::Filters::Compound_ptr add();
  LibFred::File::File* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_FILES_H_*/
