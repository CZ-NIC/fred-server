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
#include "src/bin/corba/Admin.hh"
#include "src/bin/corba/admin/user_impl.hh"

ccReg_User_i::ccReg_User_i(ccReg::TID _id, const std::string& _username) :
	m_id(_id), m_username(_username), m_firstname(), m_surname() {
}

ccReg_User_i::ccReg_User_i(ccReg::TID _id, const std::string& _username,
		const std::string& _firstname, const std::string& _surname) :
	m_id(_id), m_username(_username), m_firstname(_firstname),
			m_surname(_surname) {
}

CORBA::Boolean ccReg_User_i::hasNPermission(const char* _np [[gnu::unused]]) {
	return false;
}
