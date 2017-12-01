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

#ifndef PAGETABLE_KEYSETS_H_452105AE1547420DA08510AA98FAE347
#define PAGETABLE_KEYSETS_H_452105AE1547420DA08510AA98FAE347

#include "pagetable_impl.h"

class ccReg_KeySets_i : public ccReg_PageTable_i,
    public PortableServer::RefCountServantBase {
private:
    std::unique_ptr<Fred::Keyset::List> m_kl;

public:
    ccReg_KeySets_i(Fred::Keyset::List *kl, const Settings *_ptr);
    ~ccReg_KeySets_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    Fred::Keyset::Keyset *findId(ccReg::TID _id);
};

#endif // PAGETABLE_KEYSETS_H_

