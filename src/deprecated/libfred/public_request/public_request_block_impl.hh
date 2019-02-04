#ifndef PUBLIC_REQUEST_BLOCK_IMPL_HH_6737A5987F404D159FEE30FE07322386
#define PUBLIC_REQUEST_BLOCK_IMPL_HH_6737A5987F404D159FEE30FE07322386

#include "src/deprecated/libfred/public_request/public_request.hh"
#include "util/factory.hh"


namespace LibFred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(block)


static const Type PRT_BLOCK_TRANSFER_EMAIL_PIF = "block_transfer_email_pif";
static const Type PRT_BLOCK_CHANGES_EMAIL_PIF = "block_changes_email_pif";
static const Type PRT_UNBLOCK_TRANSFER_EMAIL_PIF = "unblock_transfer_email_pif";
static const Type PRT_UNBLOCK_CHANGES_EMAIL_PIF = "unblock_changes_email_pif";
static const Type PRT_BLOCK_TRANSFER_POST_PIF = "block_transfer_post_pif";
static const Type PRT_BLOCK_CHANGES_POST_PIF = "block_changes_post_pif";
static const Type PRT_UNBLOCK_TRANSFER_POST_PIF = "unblock_transfer_post_pif";
static const Type PRT_UNBLOCK_CHANGES_POST_PIF = "unblock_changes_post_pif";
static const Type PRT_BLOCK_TRANSFER_GOVERNMENT_PIF = "block_transfer_government_pif";
static const Type PRT_BLOCK_CHANGES_GOVERNMENT_PIF = "block_changes_government_pif";
static const Type PRT_UNBLOCK_TRANSFER_GOVERNMENT_PIF = "unblock_transfer_government_pif";
static const Type PRT_UNBLOCK_CHANGES_GOVERNMENT_PIF = "unblock_changes_government_pif";

} // namespace LibFred::PublicRequest
} // namespace LibFred

#endif
