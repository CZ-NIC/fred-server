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
#ifndef PAGETABLE_PUBLICREQUESTS_HH_627D33FAE5A24136B6772CD7AC5A2EB7
#define PAGETABLE_PUBLICREQUESTS_HH_627D33FAE5A24136B6772CD7AC5A2EB7

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_PublicRequests_i : public ccReg_PageTable_i,
                               public PortableServer::RefCountServantBase {
private:
  std::unique_ptr<LibFred::PublicRequest::List> request_list_;

public:
  ccReg_PublicRequests_i(LibFred::PublicRequest::List *_list);
  ~ccReg_PublicRequests_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  LibFred::PublicRequest::PublicRequest* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_PUBLICREQUESTS_H_*/
