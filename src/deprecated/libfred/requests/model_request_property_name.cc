/*
 * Copyright (C) 2010  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/libfred/requests/model_request_property_name.hh"

std::string ModelRequestPropertyName::table_name = "request_property_name";

DEFINE_PRIMARY_KEY(ModelRequestPropertyName, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRequestPropertyName, std::string, name, m_name, table_name, "name", .setNotNull())


ModelRequestPropertyName::field_list ModelRequestPropertyName::fields = list_of<ModelRequestPropertyName::field_list::value_type>
    (&ModelRequestPropertyName::id)
    (&ModelRequestPropertyName::name)
;

