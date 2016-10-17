#ifndef PUBLIC_REQUEST_FIXTURE_COMMON_H_
#define PUBLIC_REQUEST_FIXTURE_COMMON_H_

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"

Database::Result get_db_public_request(
    const Fred::OperationContext& ctx,
    const unsigned long long id,
    const unsigned type,
    const unsigned status);

Database::Result get_db_public_request(
    const Fred::OperationContext& ctx,
    const unsigned long long id,
    const unsigned type,
    const unsigned status,
    const std::string& reason);

Database::Result get_db_public_request(
    const Fred::OperationContext& ctx,
    const unsigned long long id,
    const unsigned type,
    const unsigned status,
    const std::string& reason,
    const std::string& email_to_answer);

#endif //PUBLIC_REQUEST_FIXTURE_COMMON_H_
