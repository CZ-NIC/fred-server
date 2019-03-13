/*
 * Copyright (C) 2009  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/libfred/requests/model_session.hh"

std::string ModelSession::table_name = "session";

DEFINE_PRIMARY_KEY(ModelSession, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelSession, std::string, userName, m_userName, table_name, "user_name", .setNotNull())
DEFINE_BASIC_FIELD(ModelSession, unsigned long long, userId, m_userId, table_name, "user_id", )
DEFINE_BASIC_FIELD(ModelSession, Database::DateTime, loginDate, m_loginDate, table_name, "login_date", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelSession, Database::DateTime, logoutDate, m_logoutDate, table_name, "logout_date", )


ModelSession::field_list ModelSession::fields = list_of<ModelSession::field_list::value_type>
    (&ModelSession::id)
    (&ModelSession::userName)
    (&ModelSession::userId)
    (&ModelSession::loginDate)
    (&ModelSession::logoutDate)
;

