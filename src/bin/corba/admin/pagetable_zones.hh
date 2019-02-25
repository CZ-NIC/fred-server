/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#ifndef PAGETABLE_ZONES_HH_B496FF4EB63A448F8BFF649F3603EAB6
#define PAGETABLE_ZONES_HH_B496FF4EB63A448F8BFF649F3603EAB6

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Zones_i:
    public ccReg_PageTable_i,
    public PortableServer::RefCountServantBase {
private:    
    LibFred::Zone::Manager::ZoneListPtr m_zoneList;
public:
    ccReg_Zones_i(LibFred::Zone::Manager::ZoneListPtr zoneList);
    ~ccReg_Zones_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    LibFred::Zone::Zone *findId(ccReg::TID id);
}; // class ccReg_Zones_i

#endif
