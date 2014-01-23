#ifndef OBJECT_STATES_H_
#define OBJECT_STATES_H_

#include "src/fredlib/db_settings.h"
#include "util/types/optional.h"



#include <string>


namespace Fred {

namespace ObjectState
{//object state names from db table enum_object_states
    const std::string SERVER_DELETE_PROHIBITED = "serverDeleteProhibited";//id 1
    const std::string SERVER_RENEW_PROHIBITED = "serverRenewProhibited";//id 2
    const std::string SERVER_TRANSFER_PROHIBITED = "serverTransferProhibited";//id 3
    const std::string SERVER_UPDATE_PROHIBITED = "serverUpdateProhibited";//id 4
    const std::string SERVER_OUTZONE_MANUAL = "serverOutzoneManual";//id 5
    const std::string SERVER_INZONE_MANUAL = "serverInzoneManual";//id 6
    const std::string SERVER_BLOCKED = "serverBlocked";//id 7
    const std::string EXPIRATION_WARNING = "expirationWarning";//id 8
    const std::string EXPIRED = "expired";//id 9
    const std::string UNGUARDED = "unguarded";//id 10
    const std::string VALIDATION_WARNING1 = "validationWarning1";//id 11
    const std::string VALIDATION_WARNING2 = "validationWarning2";//id 12
    const std::string NOT_VALIDATED = "notValidated";//id 13
    const std::string NSSET_MISSING = "nssetMissing";//id 14
    const std::string OUTZONE = "outzone";//id 15
    const std::string LINKED = "linked";//id 16
    const std::string DELETE_CANDIDATE = "deleteCandidate";//id 17
    const std::string SERVER_REGISTRANT_CHANGE_PROHIBITED = "serverRegistrantChangeProhibited";//id 18
    const std::string DELETE_WARNING = "deleteWarning";//id 19
    const std::string OUTZONE_UNGUARDED = "outzoneUnguarded";//id 20
    const std::string CONDITIONALLY_IDENTIFIED_CONTACT = "conditionallyIdentifiedContact";//id 21
    const std::string IDENTIFIED_CONTACT = "identifiedContact";//id 22
    const std::string VALIDATED_CONTACT = "validatedContact";//id 23
    //const std::string MOJEID_CONTACT = "mojeidContact";//id 24
};

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

bool cancel_object_state(
        const unsigned long long &_object_id,
        const std::string &_state_name);

void cancel_multiple_object_states(
        const unsigned long long &_object_id,
        const std::vector<std::string> &_states_names);

void lock_multiple_object_states(
    const unsigned long long _object_id
    , const std::vector<std::string> &_states_names);

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
        , bool update_object_state);

void lock_object_state_request_lock(
        unsigned long long state_id
        , unsigned long long object_id);

void lock_object_state_request_lock(
        const std::string& state_name
        , unsigned long long object_id);
};

#endif /*OBJECT_STATES_H_*/

