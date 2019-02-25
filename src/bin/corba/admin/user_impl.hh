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
#ifndef USER_IMPL_HH_7A494C20305449A9A5FE1735F88E1472
#define USER_IMPL_HH_7A494C20305449A9A5FE1735F88E1472

#include <string>

class ccReg_User_i : public POA_Registry::User,
                     public PortableServer::RefCountServantBase {
  ccReg::TID m_id;
  std::string m_username;
  std::string m_firstname;
  std::string m_surname;
 public:
  ccReg_User_i(ccReg::TID _id, const std::string& _username);
  ccReg_User_i(ccReg::TID _id, const std::string& _username,
  		const std::string& _firstname, const std::string& _surname);
  ~ccReg_User_i() { }
  ccReg::TID id() { return m_id; }
  void id(ccReg::TID _id) { m_id = _id; }
  char* username() { return CORBA::string_dup(m_username.c_str()); }
  void username(const char* _username) { m_username = _username; }
  char* firstname() { return CORBA::string_dup(m_firstname.c_str()); }
  void firstname(const char* _firstname) { m_firstname = _firstname; }
  char* surname() { return CORBA::string_dup(m_surname.c_str()); }
  void surname(const char* _surname) { m_surname = _surname; }
  CORBA::Boolean hasNPermission(const char* _np);
};

#endif /*USER_IMPL_H_*/
