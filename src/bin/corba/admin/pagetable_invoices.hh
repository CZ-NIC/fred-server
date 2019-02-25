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
#ifndef PAGETABLE_INVOICES_HH_41AB4BFB090C42228DC2EBACAE626194
#define PAGETABLE_INVOICES_HH_41AB4BFB090C42228DC2EBACAE626194

#include "src/bin/corba/admin/pagetable_impl.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"

class ccReg_Invoices_i : public ccReg_PageTable_i,
                         public PortableServer::RefCountServantBase {
private:
  std::unique_ptr<LibFred::Invoicing::List> invoice_list_;

public:
  ccReg_Invoices_i(LibFred::Invoicing::List* _invoice_list);
  ~ccReg_Invoices_i();
  DECL_PAGETABLE_I;

  ccReg::Filters::Compound_ptr add();
  LibFred::Invoicing::Invoice* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_INVOICES_H_*/
