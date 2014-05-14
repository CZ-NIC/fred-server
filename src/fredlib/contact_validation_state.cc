#include "src/fredlib/contact_validation_state.h"
#include "src/fredlib/object_states.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/db_settings.h"
#include <map>
#include <stdexcept>
#include <sstream>

namespace Fred {
namespace Contact {

namespace
{
typedef std::map< std::string, enum ValidationState::Value > Str2Value;
}

ValidationState& ValidationState::add(const std::string &_state)
{
    return *this = value_ | ValidationState(_state).get();
}

enum ValidationState::Value ValidationState::str2value(const std::string &_state)
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
    throw std::invalid_argument("unknown contact validation state name: " + _state);
}

const std::string ValidationState::STR_C = ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT;
const std::string ValidationState::STR_I = ObjectState::IDENTIFIED_CONTACT;
const std::string ValidationState::STR_V = ObjectState::VALIDATED_CONTACT;
const std::string ValidationState::STR_M = ObjectState::MOJEID_CONTACT;

void lock_contact_validation_states(::uint64_t _contact_id)
{
    enum { VALIDATION_STATE_COUNT = 4 };
    static const std::string array_of_states[VALIDATION_STATE_COUNT] = {
        ValidationState::STR_C,
        ValidationState::STR_I,
        ValidationState::STR_V,
        ValidationState::STR_M
    };
    static const std::vector< std::string > validation_states(array_of_states,
                                                              array_of_states + VALIDATION_STATE_COUNT);
    Fred::lock_multiple_object_states(_contact_id, validation_states);
}

void lock_contact_validation_states(const std::string &_contact_handle)
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
    lock_contact_validation_states(static_cast< ::uint64_t >(res[0][0]));
}

ValidationState get_contact_validation_state(::uint64_t _contact_id)
{
    Database::Connection conn = Database::Manager::acquire();
    static const std::string sql =
        "SELECT COALESCE(BOOL_OR(eos.name='" + ValidationState::STR_C + "'),False),"
               "COALESCE(BOOL_OR(eos.name='" + ValidationState::STR_I + "'),False),"
               "COALESCE(BOOL_OR(eos.name='" + ValidationState::STR_V + "'),False),"
               "COALESCE(BOOL_OR(eos.name='" + ValidationState::STR_M + "'),False) "
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
    return ValidationState((static_cast< std::string >(res[0][0]) == "f" ? ValidationState::c :
                                                                           ValidationState::C) |
                           (static_cast< std::string >(res[0][1]) == "f" ? ValidationState::i :
                                                                           ValidationState::I) |
                           (static_cast< std::string >(res[0][2]) == "f" ? ValidationState::v :
                                                                           ValidationState::V) |
                           (static_cast< std::string >(res[0][3]) == "f" ? ValidationState::m :
                                                                           ValidationState::M));
}

ValidationState get_contact_validation_state(const std::string &_contact_handle)
{
    Database::Connection conn = Database::Manager::acquire();
    static const std::string sql =
        "SELECT COALESCE(BOOL_OR(eos.name='" + ValidationState::STR_C + "'),False),"
               "COALESCE(BOOL_OR(eos.name='" + ValidationState::STR_I + "'),False),"
               "COALESCE(BOOL_OR(eos.name='" + ValidationState::STR_V + "'),False),"
               "COALESCE(BOOL_OR(eos.name='" + ValidationState::STR_M + "'),False) "
        "FROM object_registry obr "
        "JOIN object o ON o.id=obr.id "
        "JOIN contact c ON c.id=o.id "
        "LEFT JOIN object_state os ON (os.object_id=obr.id AND os.valid_to IS NULL) "
        "LEFT JOIN enum_object_states eos ON eos.id=os.state_id "
        "WHERE obr.name=$1::text AND obr.erdate IS NULL "
        "GROUP BY obr.id";
    Database::Result res = conn.exec_params(sql, Database::query_param_list(_contact_handle));
    if (res.size() <= 0) {
        throw std::runtime_error("contact handle '" + _contact_handle + "' not found");
    }
    return ValidationState((static_cast< std::string >(res[0][0]) == "f" ? ValidationState::c :
                                                                           ValidationState::C) |
                           (static_cast< std::string >(res[0][1]) == "f" ? ValidationState::i :
                                                                           ValidationState::I) |
                           (static_cast< std::string >(res[0][2]) == "f" ? ValidationState::v :
                                                                           ValidationState::V) |
                           (static_cast< std::string >(res[0][3]) == "f" ? ValidationState::m :
                                                                           ValidationState::M));
}

} // namespace Contact
} // namespace Fred
