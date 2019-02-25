/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/libfred/contact_verification/contact_verification_state.hh"
#include "src/deprecated/libfred/object_states.hh"
#include "src/deprecated/libfred/db_settings.hh"
#include "src/deprecated/libfred/object_state/object_state_name.hh"
#include "libfred/db_settings.hh"
#include <map>
#include <stdexcept>
#include <sstream>

namespace LibFred {
namespace Contact {
namespace Verification {

namespace
{
typedef std::map< std::string, enum State::Value > Str2Value;
}

State& State::add(const std::string &_state)
{
    return *this = value_ | State(_state).get();
}

enum State::Value State::str2value(const std::string &_state)
{
    static Str2Value str2value;
    if (str2value.empty()) {
        str2value.insert(std::make_pair(STR_C, C));
        str2value.insert(std::make_pair(STR_I, I));
        str2value.insert(std::make_pair(STR_V, V));
        str2value.insert(std::make_pair(STR_M, M));
    }
    Str2Value::const_iterator value_ptr = str2value.find(_state);
    if (value_ptr != str2value.end()) {
        return value_ptr->second;
    }
    throw ConversionFailure("unknown contact verification state name: " + _state);
}

const std::string State::STR_C = "conditionallyIdentifiedContact";
const std::string State::STR_I = "identifiedContact";
const std::string State::STR_V = "validatedContact";
const std::string State::STR_M = "mojeidContact";

void lock_contact_verification_states(::uint64_t _contact_id)
{
    LibFred::lock_object_state_request_lock(_contact_id);
}

::uint64_t lock_contact_verification_states(const std::string &_contact_handle)
{
    Database::Connection conn = Database::Manager::acquire();
    static const std::string sql =
        "SELECT c.id "
        "FROM object_registry obr "
        "JOIN object o ON o.id=obr.id "
        "JOIN contact c ON c.id=o.id "
        "WHERE obr.name=$1::text AND obr.erdate IS NULL";
    Database::Result res = conn.exec_params(sql, Database::query_param_list(_contact_handle));
    if (res.size() <= 0) {
        throw ContactNotFound("contact handle '" + _contact_handle + "' not found");
    }
    const ::uint64_t contact_id = static_cast< ::uint64_t >(res[0][0]);
    lock_contact_verification_states(contact_id);
    return contact_id;
}

namespace
{

State get_contact_verification_state_without_lock(::uint64_t _contact_id)
{
    Database::Connection conn = Database::Manager::acquire();
    static const std::string sql =
        "SELECT COALESCE(BOOL_OR(eos.name='" + State::STR_C + "'),False),"
               "COALESCE(BOOL_OR(eos.name='" + State::STR_I + "'),False),"
               "COALESCE(BOOL_OR(eos.name='" + State::STR_V + "'),False),"
               "COALESCE(BOOL_OR(eos.name='" + State::STR_M + "'),False) "
        "FROM object_registry obr "
        "JOIN object o ON o.id=obr.id "
        "JOIN contact c ON c.id=o.id "
        "LEFT JOIN object_state os ON (os.object_id=obr.id AND os.valid_to IS NULL) "
        "LEFT JOIN enum_object_states eos ON eos.id=os.state_id "
        "WHERE obr.id=$1::bigint "
        "GROUP BY obr.id";
    Database::Result res = conn.exec_params(sql, Database::query_param_list(_contact_id));
    if (res.size() <= 0) {
        std::ostringstream msg;
        msg << "contact id " << _contact_id << " not found";
        throw ContactNotFound(msg.str());
    }
    return State((static_cast< std::string >(res[0][0]) == "f" ? State::c :
                                                                 State::C) |
                 (static_cast< std::string >(res[0][1]) == "f" ? State::i :
                                                                 State::I) |
                 (static_cast< std::string >(res[0][2]) == "f" ? State::v :
                                                                 State::V) |
                 (static_cast< std::string >(res[0][3]) == "f" ? State::m :
                                                                 State::M));
}

}

State get_contact_verification_state(::uint64_t _contact_id)
{
    lock_contact_verification_states(_contact_id);
    return get_contact_verification_state_without_lock(_contact_id);
}

State get_contact_verification_state(const std::string &_contact_handle)
{
    const ::uint64_t contact_id = lock_contact_verification_states(_contact_handle);
    return get_contact_verification_state_without_lock(contact_id);
}

} // namespace Verification
} // namespace Contact
} // namespace LibFred
