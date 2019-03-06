/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include "test/backend/epp/domain/fixture.hh"
#include "libfred/object/object_id_handle_pair.hh"

#include <string>
#include <vector>

namespace Test {
namespace Backend {
namespace Epp {
namespace Domain {

std::vector<std::string> vector_of_Fred_ObjectIdHandlePair_to_vector_of_string(const std::vector<::LibFred::ObjectIdHandlePair>& admin_contacts) {
    std::vector<std::string> admin;
    for (
        std::vector<::LibFred::ObjectIdHandlePair>::const_iterator object_id_handle_pair = admin_contacts.begin();
        object_id_handle_pair != admin_contacts.end();
        ++object_id_handle_pair
    ) {
        admin.push_back(object_id_handle_pair->handle);
    }
    return admin;
}

} // namespace Test::Backend::Epp::Domain
} // namespace Test::Backend::Epp
} // namespace Test::Backend
} // namespace Test
