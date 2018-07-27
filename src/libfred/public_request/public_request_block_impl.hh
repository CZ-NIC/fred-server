#ifndef PUBLIC_REQUEST_BLOCK_IMPL_HH_6737A5987F404D159FEE30FE07322386
#define PUBLIC_REQUEST_BLOCK_IMPL_HH_6737A5987F404D159FEE30FE07322386

#include "src/libfred/public_request/public_request.hh"
#include "src/util/factory.hh"


namespace LibFred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(block)


extern const Type PRT_BLOCK_TRANSFER_EMAIL_PIF;
extern const Type PRT_BLOCK_CHANGES_EMAIL_PIF;
extern const Type PRT_UNBLOCK_TRANSFER_EMAIL_PIF;
extern const Type PRT_UNBLOCK_CHANGES_EMAIL_PIF;
extern const Type PRT_BLOCK_TRANSFER_POST_PIF;
extern const Type PRT_BLOCK_CHANGES_POST_PIF;
extern const Type PRT_UNBLOCK_TRANSFER_POST_PIF;
extern const Type PRT_UNBLOCK_CHANGES_POST_PIF;


} // namespace LibFred::PublicRequest
} // namespace LibFred

#endif
