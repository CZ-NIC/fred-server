#ifndef TRANSFER_945654403401
#define TRANSFER_945654403401

#include <string>

#include "src/libfred/object/generated_authinfo_password.hh"
#include "src/libfred/opcontext.hh"

namespace LibFred
{
    /**
     * @returns historyid of transferred object
     * @throws UnknownRegistrar
     * @throws UnknownObjectId
     * @throws NewRegistrarIsAlreadySponsoring
     */
    unsigned long long transfer_object(
        LibFred::OperationContext& _ctx,
        const unsigned long long _object_id,
        const std::string& _new_registrar_handle,
        const GeneratedAuthInfoPassword& _new_authinfopw,
        const Nullable<unsigned long long>& _logd_request_id = Nullable<unsigned long long>()
    );
}

#endif
