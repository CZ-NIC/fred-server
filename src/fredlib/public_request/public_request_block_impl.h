#ifndef PUBLIC_REQUEST_BLOCK_IMPL_H_
#define PUBLIC_REQUEST_BLOCK_IMPL_H_

#include "public_request.h"
#include "factory.h"

namespace Fred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(block)


const Type PRT_BLOCK_TRANSFER_EMAIL_PIF = "block_transfer_email_pif";
const Type PRT_BLOCK_CHANGES_EMAIL_PIF = "block_changes_email_pif";
const Type PRT_UNBLOCK_TRANSFER_EMAIL_PIF = "unblock_transfer_email_pif";
const Type PRT_UNBLOCK_CHANGES_EMAIL_PIF = "unblock_changes_email_pif";
const Type PRT_BLOCK_TRANSFER_POST_PIF = "block_transfer_post_pif";
const Type PRT_BLOCK_CHANGES_POST_PIF = "block_changes_post_pif";
const Type PRT_UNBLOCK_TRANSFER_POST_PIF = "unblock_transfer_post_pif";
const Type PRT_UNBLOCK_CHANGES_POST_PIF = "unblock_changes_post_pif";


}
}

#endif /* PUBLIC_REQUEST_BLOCK_IMPL_H_ */

