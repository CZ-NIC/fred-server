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
#ifndef PAGETABLE_LOGSESSION_HH_6FA7C69F550246F1A3BFE920EDB1C534
#define PAGETABLE_LOGSESSION_HH_6FA7C69F550246F1A3BFE920EDB1C534

#include "src/deprecated/libfred/requests/session.hh"
#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_LogSession_i : public ccReg_PageTable_i,
	public PortableServer::RefCountServantBase {
private: 
	std::unique_ptr<LibFred::Session::List> m_lel;

public:
	//ccReg_Session_i(LibFred::Request::List *list, const Settings *_ptr);
	ccReg_LogSession_i(LibFred::Session::List *list);
	~ccReg_LogSession_i();
	DECL_PAGETABLE_I;

	ccReg::Filters::Compound_ptr add();
	LibFred::Session::Session *findId(ccReg::TID _id);

public: 
	const static int NUM_COLUMNS;
};

#endif
