#ifndef PUBLIC_REQUEST_FIXTURE_COMMON_H_
#define PUBLIC_REQUEST_FIXTURE_COMMON_H_

#include "src/libfred/opcontext.hh"

Database::Result get_db_public_request(
    const ::LibFred::OperationContext& ctx,
    const unsigned long long id,
    const unsigned type,
    const unsigned status);

Database::Result get_db_public_request(
    const ::LibFred::OperationContext& ctx,
    const unsigned long long id,
    const unsigned type,
    const unsigned status,
    const std::string& email_to_answer);

#endif //PUBLIC_REQUEST_FIXTURE_COMMON_H_
