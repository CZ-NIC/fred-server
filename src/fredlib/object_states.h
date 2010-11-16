#ifndef OBJECT_STATES_H_
#define OBJECT_STATES_H_

#include "fredlib/db_settings.h"

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

};

#endif /*OBJECT_STATES_H_*/

