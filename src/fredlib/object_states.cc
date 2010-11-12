#include "object_states.h"


namespace Fred {

bool object_has_state(
        const unsigned long long &_object_id,
        const std::string &_state_name)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result rcheck = conn.exec_params(
            "SELECT count(*) FROM object_state os"
            " JOIN enum_object_states eos ON eos.id = os.state_id"
            " WHERE os.object_id = $1::integer AND eos.name = $2::text"
            " AND valid_to IS NULL",
            Database::query_param_list
                (_object_id)
                (_state_name));
    return static_cast<int>(rcheck[0][0]);
}



void insert_object_state(
        const unsigned long long &_object_id,
        const std::string &_state_name)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);
    conn.exec_params(
            "INSERT INTO object_state_request (object_id, state_id)"
            " VALUES ($1::integer, (SELECT id FROM enum_object_states"
            " WHERE name = $2::text))",
            Database::query_param_list
                (_object_id)
                (_state_name));
    // conn.exec_params("SELECT update_object_states($1::integer)",
    //         Database::query_param_list(_object_id));
    tx.commit();
}


bool cancel_object_state(
        const unsigned long long &_object_id,
        const std::string &_state_name)
{
    Database::Connection conn = Database::Manager::acquire();
    /* check if state is active on object */
    if (object_has_state(_object_id, _state_name) == true) {
        Database::Transaction tx(conn);
        Database::Result rid_result = conn.exec_params(
                "SELECT osr.id FROM object_state_request osr"
                " JOIN enum_object_states eos ON eos.id = osr.state_id"
                " WHERE eos.name = $1::text AND osr.valid_to is NULL"
                " AND osr.canceled is NULL AND osr.object_id = $2::integer",
                Database::query_param_list
                    (_state_name)
                    (_object_id));
        /* cancel this status TODO: or cancel all and dont throw??? */
        if (rid_result.size() == 1) {
            conn.exec_params("UPDATE object_state_request"
                    " SET canceled = CURRENT_TIMESTAMP WHERE id = $1::integer",
                    Database::query_param_list(
                            static_cast<unsigned long long>(rid_result[0][0])));
            // conn.exec_params("SELECT update_object_states($1::integer)",
            //         Database::query_param_list(_object_id));
            tx.commit();
            return true;
        }
        throw std::runtime_error(str(boost::format(
                        "too many opened states (object_id=%1%, state=%2%")
                        % _object_id % _state_name));
    }
    else {
        return false;
    }
}

void cancel_multiple_object_states(
        const unsigned long long &_object_id,
        const std::vector<std::string> &_states_names)
{
    if(_states_names.size() == 0) return;

    // generate string representing an array with state names
    std::stringstream states;
    states << "{";
    
    std::vector<std::string>::const_iterator it = _states_names.begin();
    states << *it;
    it++;
    for (; 
        it != _states_names.end();
        it++) {

        states << ", " << *it;
    }

    states << "}";

    Database::Connection conn = Database::Manager::acquire();

    conn.exec_params("UPDATE object_state_request osr SET canceled = CURRENT_TIMESTAMP FROM enum_object_states eos  WHERE eos.id = osr.state_id AND eos.name = ANY ($1::text[]) AND osr.valid_to is NULL AND osr.canceled is NULL AND osr.object_id = $2::integer", 
            Database::query_param_list
            (states.str())
            (_object_id));

}

void update_object_states(
        const unsigned long long &_object_id)
{
    Database::Connection conn = Database::Manager::acquire();
    conn.exec_params("SELECT update_object_states($1::integer)",
            Database::query_param_list(_object_id));
}


};
