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
#ifndef PAGETABLE_FILTERS_HH_74B4C35E5F934C618BBCC1967E38525D
#define PAGETABLE_FILTERS_HH_74B4C35E5F934C618BBCC1967E38525D

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Filters_i : public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase {
private:
  LibFred::Filter::List& m_filter_list;
 
public:
  ccReg_Filters_i(LibFred::Filter::List& _filter_list);
  ~ccReg_Filters_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
};

#endif /*PAGETABLE_FILTERS_H_*/
