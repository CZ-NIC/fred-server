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
#ifndef OBJECT_STATES_HH_6A367F3320C141AAB5F9571FE05FE382
#define OBJECT_STATES_HH_6A367F3320C141AAB5F9571FE05FE382

#include "libfred/db_settings.hh"
#include "src/util/types/optional.hh"
#include "src/deprecated/libfred/object_state/object_state_name.hh"

#include <string>
#include <exception>

namespace LibFred {

bool object_has_state(
        const unsigned long long &_object_id,
        const std::string &_state_name);

bool object_has_one_of_states(
        const unsigned long long &_object_id,
        const std::vector<std::string> & _state_names);

bool object_has_all_of_states(
        const unsigned long long &_object_id,
        const std::vector<std::string> & _state_names);

unsigned long long insert_object_state(
        const unsigned long long &_object_id,
        const std::string &_state_name);

struct ExceptionObjectStateRequestNotFound : public std::runtime_error {
    explicit ExceptionObjectStateRequestNotFound(const std::string& _msg)
    : std::runtime_error(_msg)
    { }
};

bool cancel_object_state(
        const unsigned long long &_object_id,
        const std::string &_state_name);

/**
 * try to cancel object state request
 * don't throw exception if request not found
 * don't cancel related object state
 */
void cancel_object_state_request(
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
        , const optional_string& valid_from
        , const optional_string& valid_to
        , bool update_object_state);

void lock_object_state_request_lock(unsigned long long object_id);

};

#endif /*OBJECT_STATES_H_*/

