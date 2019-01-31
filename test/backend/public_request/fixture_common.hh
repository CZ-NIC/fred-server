#ifndef FIXTURE_COMMON_HH_44EF6841264E4440A7EC39975EEC188E
#define FIXTURE_COMMON_HH_44EF6841264E4440A7EC39975EEC188E

#include "libfred/opcontext.hh"

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

#endif
