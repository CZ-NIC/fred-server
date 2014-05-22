#include "src/fredlib/contact_verification/contact_verification_state.h"
#include "src/fredlib/object_states.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/db_settings.h"
#include <map>
#include <stdexcept>
#include <sstream>

namespace Fred {
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
    throw std::invalid_argument("unknown contact verification state name: " + _state);
}

const std::string State::STR_C = "conditionallyIdentifiedContact";
const std::string State::STR_I = "identifiedContact";
const std::string State::STR_V = "validatedContact";
const std::string State::STR_M = "mojeidContact";

void lock_contact_verification_states(::uint64_t _contact_id)
{
    enum { VERIFICATION_STATE_COUNT = 4 };
    static const std::string array_of_states[VERIFICATION_STATE_COUNT] = {
        State::STR_C,
        State::STR_I,
        State::STR_V,
        State::STR_M
    };
    static const std::vector< std::string > verification_states(
        array_of_states, array_of_states + VERIFICATION_STATE_COUNT);
    Fred::lock_multiple_object_states(_contact_id, verification_states);
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
        throw std::runtime_error("contact handle '" + _contact_handle + "' not found");
    }
    const ::uint64_t contact_id = static_cast< ::uint64_t >(res[0][0]);
    lock_contact_verification_states(contact_id);
    return contact_id;
}

State get_contact_verification_state(::uint64_t _contact_id)
{
    lock_contact_verification_states(_contact_id);
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
        throw std::runtime_error(msg.str());
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

State get_contact_verification_state(const std::string &_contact_handle)
{
    const ::uint64_t contact_id = lock_contact_verification_states(_contact_handle);
    return get_contact_verification_state(contact_id);
}

} // namespace Verification
} // namespace Contact
} // namespace Fred
