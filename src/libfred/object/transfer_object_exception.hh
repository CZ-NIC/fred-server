#ifndef TRANSFER_OBJECT_EXCEPTION_978831431006
#define TRANSFER_OBJECT_EXCEPTION_978831431006

#include "src/libfred/exception.hh"

namespace LibFred
{
    struct NewRegistrarIsAlreadySponsoring : Exception {
        const char* what() const noexcept { return "new registrar is already sponsoring"; }
    };
}

#endif
