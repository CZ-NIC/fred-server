#ifndef TRANSFER_945654403401
#define TRANSFER_945654403401

#include <string>

#include "src/fredlib/object/generated_authinfo_password.h"
#include "src/fredlib/opcontext.h"

namespace Fred
{
    /**
     * @throws UnknownRegistrar
     * @throws UnknownObjectId
     * @throws NewRegistrarIsAlreadySponsoring
     */
    void transfer_object(
        Fred::OperationContext& _ctx,
        const unsigned long long _object_id,
        const std::string& _new_registrar_handle,
        const GeneratedAuthInfoPassword& _new_authinfopw
    );
}

#endif
