#ifndef OBJECT_STATES_H_
#define OBJECT_STATES_H_

#include "fredlib/db_settings.h"
#include "util/types/optional.h"
#include "fredlib/registry.h"


#include <string>


namespace Fred {

namespace ObjectState
{//object state names from db table enum_object_states
    const std::string SERVER_DELETE_PROHIBITED = "serverDeleteProhibited";
    const std::string SERVER_RENEW_PROHIBITED = "serverRenewProhibited";
    const std::string SERVER_TRANSFER_PROHIBITED = "serverTransferProhibited";
    const std::string SERVER_UPDATE_PROHIBITED = "serverUpdateProhibited";
    const std::string SERVER_OUTZONE_MANUAL = "serverOutzoneManual";
    const std::string SERVER_INZONE_MANUAL = "serverInzoneManual";
    const std::string SERVER_BLOCKED = "serverBlocked";
    const std::string EXPIRATION_WARNING = "expirationWarning";
    const std::string EXPIRED = "expired";
    const std::string UNGUARDED = "unguarded";
    const std::string VALIDATION_WARNING1 = "validationWarning1";
    const std::string VALIDATION_WARNING2 = "validationWarning2";
    const std::string NOT_VALIDATED = "notValidated";
    const std::string NSSET_MISSING = "nssetMissing";
    const std::string OUTZONE = "outzone";
    const std::string LINKED = "linked";
    const std::string DELETE_CANDIDATE = "deleteCandidate";
    const std::string SERVER_REGISTRANT_CHANGE_PROHIBITED = "serverRegistrantChangeProhibited";
    const std::string DELETE_WARNING = "deleteWarning";
    const std::string OUTZONE_UNGUARDED = "outzoneUnguarded";
    const std::string CONDITIONALLY_IDENTIFIED_CONTACT = "conditionallyIdentifiedContact";
    const std::string IDENTIFIED_CONTACT = "identifiedContact";
    const std::string VALIDATED_CONTACT = "validatedContact";
    //const std::string MOJEID_CONTACT = "mojeidContact";
};

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

