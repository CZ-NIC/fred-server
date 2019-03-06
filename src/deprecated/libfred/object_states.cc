/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/libfred/object_states.hh"
#include "src/deprecated/libfred/db_settings.hh"

#include <memory>

namespace LibFred {

bool object_has_state(
        const unsigned long long &_object_id,
        const std::string &_state_name)
{
    Database::Connection conn = Database::Manager::acquire();
    lock_object_state_request_lock(_object_id);
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

bool object_has_one_of_states(
        const unsigned long long &_object_id,
        const std::vector<std::string> & _state_names)
{
    for (std::vector<std::string>::const_iterator it = _state_names.begin()
            ; it != _state_names.end(); ++it)
    {
        if(object_has_state(_object_id, *it)) return true;
    }
    return false;
}


bool object_has_all_of_states(
        const unsigned long long &_object_id,
        const std::vector<std::string> & _state_names)
{
    for (std::vector<std::string>::const_iterator it = _state_names.begin()
            ; it != _state_names.end(); ++it)
    {
        if(!object_has_state(_object_id, *it)) return false;
    }
    return true;
}


unsigned long long insert_object_state(
        const unsigned long long &_object_id,
        const std::string &_state_name)
{
    Database::Connection conn = Database::Manager::acquire();
    lock_object_state_request_lock(_object_id);
    Database::Result reqid =conn.exec_params(
            "INSERT INTO object_state_request (object_id, state_id)"
            " VALUES ($1::integer, (SELECT id FROM enum_object_states"
            " WHERE name = $2::text)) RETURNING id",
            Database::query_param_list
                (_object_id)
                (_state_name));
    if(reqid.size() == 0)
        return 0;

    return static_cast<unsigned long long>(reqid[0][0]);
}

/**
 * Convert list of state codes to list of state codes names
 * The function doesn't keep the order of elements
 */
std::vector<std::string> states_conversion(const std::vector<int> state_codes) {
    std::vector<std::string> ret;
    
    unsigned input_size = state_codes.size();
    if(input_size == 0) {
        return ret;
    }
    ret.reserve(input_size);

    std::stringstream ostr;
    std::vector<int>::const_iterator it = state_codes.begin();
    ostr << "{" << *it;
    it++;

    for(;
        it != state_codes.end();
        it++) {
        ostr << ", ";
        ostr << *it;     
    }
    ostr << "}";

    Database::Connection conn = Database::Manager::acquire();

    Database::Result res = conn.exec_params("SELECT name FROM enum_object_states WHERE id = ANY($1::integer[])", 
            Database::query_param_list(ostr.str()));

    if(res.size() != input_size) {
        boost::format errfmt = boost::format(
            "Failed to convert state codes using table enum_object_states, codes: %1%. ") % ostr.str();
        throw std::runtime_error(errfmt.str());
    }

    for(unsigned i=0;i<res.size();i++) {
        ret.push_back(res[i][0]);
    }

    return ret;
}

bool cancel_object_state(
        const unsigned long long &_object_id,
        const std::string &_state_name)
{
    Database::Connection conn = Database::Manager::acquire();
    /* check if state is active on object */
    if (object_has_state(_object_id, _state_name) == true) {
        Database::Transaction tx(conn);

        lock_object_state_request_lock(_object_id);

        Database::Result rid_result = conn.exec_params(
                "UPDATE object_state_request SET canceled = CURRENT_TIMESTAMP WHERE id IN ("
                " SELECT osr.id FROM object_state_request osr"
                " JOIN enum_object_states eos ON eos.id = osr.state_id"
                " WHERE eos.name = $1::text AND (osr.valid_to is NULL OR osr.valid_to > CURRENT_TIMESTAMP)"
                " AND osr.valid_from <= CURRENT_TIMESTAMP "
                " AND osr.canceled is NULL AND osr.object_id = $2::integer ) RETURNING id",
                Database::query_param_list
                    (_state_name)
                    (_object_id));

        std::string rid;
        if (rid_result.size() == 0) {
            throw ExceptionObjectStateRequestNotFound("cancel_object_state: object state request not found for object state");
        }
        else {
            rid = "cancel_object_state: canceled request id:";
            for (unsigned i = 0; i < rid_result.size(); ++i) {
                rid += " " +std::string(rid_result[i][0]);
            }
        }

        tx.commit();

        Logging::Manager::instance_ref().debug(rid);
        return true;
    }

    return false;
}

void cancel_object_state_request(
    const unsigned long long &_object_id,
    const std::string &_state_name
) {
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);

    lock_object_state_request_lock(_object_id);

    Database::Result rid_result = conn.exec_params(
        "UPDATE object_state_request SET canceled = CURRENT_TIMESTAMP WHERE id IN ("
        " SELECT osr.id FROM object_state_request osr"
        " JOIN enum_object_states eos ON eos.id = osr.state_id"
        " WHERE eos.name = $1::text AND (osr.valid_to is NULL OR osr.valid_to > CURRENT_TIMESTAMP)"
        " AND osr.valid_from <= CURRENT_TIMESTAMP "
        " AND osr.canceled is NULL AND osr.object_id = $2::integer ) RETURNING id",
        Database::query_param_list
            (_state_name)
            (_object_id)
    );

    tx.commit();
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

    conn.exec_params("UPDATE object_state_request osr SET canceled = CURRENT_TIMESTAMP FROM enum_object_states eos  WHERE eos.id = osr.state_id AND eos.name = ANY ($1::text[]) AND (osr.valid_to is NULL OR osr.valid_to > CURRENT_TIMESTAMP)  AND osr.canceled is NULL AND osr.object_id = $2::integer", 
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

void createObjectStateRequestName(
        const std::string & object_name
        , unsigned long object_type
        , std::vector< std::string > _object_state_name
        , const optional_string& valid_from
        , const optional_string& valid_to
        , bool update_object_state
        )
{
    std::string object_state_names;

    for (std::vector<std::string>::iterator i= _object_state_name.begin()
            ; i != _object_state_name.end()
            ; ++i) object_state_names += (*i) + " ";

    Logging::Manager::instance_ref().debug(std::string(
        "createObjectStateRequestName object name: ") + object_name
        + " object type: " + boost::lexical_cast<std::string>(object_type)
        + " object state name: " + object_state_names
        + " valid from: " + valid_from.get_value()
        + " valid to: " + valid_to.get_value());

    Database::Connection conn = Database::Manager::acquire();

    Database::Transaction tx(conn);

    //get object
    Database::Result obj_id_res = conn.exec_params(
            "SELECT id FROM object_registry "
            " WHERE type=$1::integer AND name=$2::text AND erdate IS NULL"
            , Database::query_param_list
                (object_type)(object_name));

    if(obj_id_res.size() != 1)
        throw std::runtime_error("object not found");

    unsigned long long object_id = obj_id_res[0][0];
    lock_object_state_request_lock(object_id);

    for (std::vector<std::string>::iterator i= _object_state_name.begin()
            ; i != _object_state_name.end()
            ; ++i)
    {

        std::string object_state_name(*i);

        //get object state
        Database::Result obj_state_res = conn.exec_params(
                    "SELECT id FROM enum_object_states "
                    " WHERE name=$1::text"
                    , Database::query_param_list
                        (object_state_name));

        if(obj_state_res.size() !=1)
            throw std::runtime_error("object state not found");

        unsigned long long object_state_id = obj_state_res[0][0];

        //get existing state requests for object and state
        //assuming requests for different states of the same object may overlay
        Database::Result requests_result = conn.exec_params(
            "SELECT valid_from, valid_to, canceled FROM object_state_request "
            " WHERE object_id=$1::bigint AND state_id=$2::bigint "
            , Database::query_param_list(object_id)(object_state_id));

        //check time
        std::string tmp_time_from ( valid_from.get_value());
        if (!tmp_time_from.empty()) {
            size_t idx_from = tmp_time_from.find('T');
            if(idx_from == std::string::npos) {
                throw std::runtime_error("Wrong date format in valid_from");
            }
            tmp_time_from[idx_from] = ' ';
        }

        boost::posix_time::ptime new_valid_from;
        if (!tmp_time_from.empty())
        {
            new_valid_from = boost::posix_time::time_from_string(tmp_time_from);
        }
        else
        {
            new_valid_from = time_from_string(static_cast<std::string>(conn.exec("SELECT now()")[0][0]));
        }

        std::string tmp_time_to ( valid_to.get_value());
        if(!tmp_time_to.empty()) {
            size_t idx_to = tmp_time_to.find('T');
            if(idx_to == std::string::npos) {
                throw std::runtime_error("Wrong date format in valid_to");
            }
            tmp_time_to[idx_to] = ' ';
        }

        boost::posix_time::ptime new_valid_to
            = tmp_time_to.empty() ? boost::posix_time::pos_infin
                : boost::posix_time::time_from_string(tmp_time_to);

        if(new_valid_from > new_valid_to )
            throw std::runtime_error("new_valid_from > new_valid_to");

        for(std::size_t i = 0 ; i < requests_result.size(); ++i)
        {
            boost::posix_time::ptime obj_valid_from = requests_result[i][0];

            boost::posix_time::ptime obj_valid_to = requests_result[i][1];

            //if obj_canceled is not null
            if(requests_result[i][2].isnull() == false)
            {
                boost::posix_time::ptime obj_canceled = requests_result[i][2];

                if (obj_canceled < obj_valid_to) obj_valid_to = obj_canceled;
            }//if obj_canceled is not null

            if(obj_valid_from > obj_valid_to )
                throw std::runtime_error("obj_valid_from > obj_valid_to");

            if(obj_valid_to.is_special())
                obj_valid_to = boost::posix_time::pos_infin;

            Logging::Manager::instance_ref().debug(std::string(
                "createObjectStateRequestName new_valid_from: ")
                + boost::posix_time::to_iso_extended_string(new_valid_from)
                + " new_valid_to: " + boost::posix_time::to_iso_extended_string(new_valid_to)
                + " obj_valid_from: " + boost::posix_time::to_iso_extended_string(obj_valid_from)
                + " obj_valid_to: " + boost::posix_time::to_iso_extended_string(obj_valid_to)
            );


            //check overlay
            if(((new_valid_from >= obj_valid_from) && (new_valid_from < obj_valid_to))
              || ((new_valid_to > obj_valid_from) && (new_valid_to <= obj_valid_to)))
                throw std::runtime_error("overlayed validity time intervals");
        }//for check with existing object state requests

        conn.exec_params(
            "INSERT INTO object_state_request "
            "(object_id,state_id,crdate, valid_from,valid_to) VALUES "
            "( $1::bigint , $2::bigint "
            ",CURRENT_TIMESTAMP, $3::timestamp, "
            "$4::timestamp )"
            , Database::query_param_list
                (object_id)(object_state_id)
                (new_valid_from)(new_valid_to.is_special()
                        ? Database::QPNull
                                : Database::QueryParam(new_valid_to) )
            );

    }//for object_state

    tx.commit();

    if (update_object_state)
    {
        update_object_states(object_id);
    }//if (update_object_state)

    return;
}//createObjectStateRequestName

//select for update by object_id from object_registry.id
void lock_object_state_request_lock(unsigned long long object_id)
{
    Database::Connection conn = Database::Manager::acquire();
    conn.exec_params("SELECT lock_object_state_request_lock($1::bigint)"
        , Database::query_param_list(object_id));
}

};
