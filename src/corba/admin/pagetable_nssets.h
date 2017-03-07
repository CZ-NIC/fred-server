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

#ifndef PAGETABLE_NSSETS_H_E7904D94854D49418F4C811593A989BD
#define PAGETABLE_NSSETS_H_E7904D94854D49418F4C811593A989BD

#include "pagetable_impl.h"

class ccReg_NSSets_i : public ccReg_PageTable_i,
                       public PortableServer::RefCountServantBase {
private:
  std::auto_ptr<Fred::Nsset::List> nl;
  
public:
  ccReg_NSSets_i(Fred::Nsset::List *nl, const Settings *_ptr);
  ~ccReg_NSSets_i();
  DECL_PAGETABLE_I;

  ccReg::Filters::Compound_ptr add();
  Fred::Nsset::Nsset* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_NSSETS_H_*/
