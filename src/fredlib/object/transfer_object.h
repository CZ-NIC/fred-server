#ifndef TRANSFER_945654403401
#define TRANSFER_945654403401

#include <string>

#include "src/fredlib/object/generated_authinfo_password.h"
#include "src/fredlib/opcontext.h"

namespace Fred
{
    /**
     * @returns historyid of transferred object
     * @throws UnknownRegistrar
     * @throws UnknownObjectId
     * @throws NewRegistrarIsAlreadySponsoring
     */
    unsigned long long transfer_object(
        Fred::OperationContext& _ctx,
        const unsigned long long _object_id,
        const std::string& _new_registrar_handle,
        const GeneratedAuthInfoPassword& _new_authinfopw,
        const Nullable<unsigned long long>& _logd_request_id = Nullable<unsigned long long>()
    );
}

#endif
