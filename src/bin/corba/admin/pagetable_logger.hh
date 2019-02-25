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
#ifndef PAGETABLE_LOGGER_HH_67E6A46D22DA4C55B3A4E36505728A45
#define PAGETABLE_LOGGER_HH_67E6A46D22DA4C55B3A4E36505728A45

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Logger_i : public ccReg_PageTable_i,
	public PortableServer::RefCountServantBase {
private: 
	std::unique_ptr<LibFred::Logger::List> m_lel;

public:
	//ccReg_Logger_i(LibFred::Request::List *list, const Settings *_ptr);
	ccReg_Logger_i(LibFred::Logger::List *list);
	~ccReg_Logger_i();
	DECL_PAGETABLE_I;

        void setOffset(CORBA::Long _offset);
	ccReg::Filters::Compound_ptr add();
	LibFred::Logger::Request *findId(ccReg::TID _id);

public: 
	const static int NUM_COLUMNS;
};

#endif
