#ifndef OBJECT_STATES_H_
#define OBJECT_STATES_H_

#include "fredlib/db_settings.h"
#include "util/types/optional.h"
#include "fredlib/registry.h"


#include <string>


namespace Fred {



bool object_has_state(
        const unsigned long long &_object_id,
        const std::string &_state_name);

void insert_object_state(
        const unsigned long long &_object_id,
        const std::string &_state_name);

bool cancel_object_state(
        const unsigned long long &_object_id,
        const std::string &_state_name);

void cancel_multiple_object_states(
        const unsigned long long &_object_id,
        const std::vector<std::string> &_states_names);

void update_object_states(
        const unsigned long long &_object_id);

std::vector<std::string> states_conversion(
        const std::vector<int> state_codes); 

void createObjectStateRequestName(
        const std::string & object_name
        , unsigned long object_type
        , std::vector< std::string > object_state_name
        , const std::string& valid_from
        , const optional_string& valid_to
        , DBSharedPtr _m_db
        , bool _restricted_handles
        , bool update_object_state);


};

#endif /*OBJECT_STATES_H_*/

