#ifndef TRANSFER_OBJECT_EXCEPTION_978831431006
#define TRANSFER_OBJECT_EXCEPTION_978831431006

#include "src/fredlib/exception.h"

namespace Fred
{
    struct NewRegistrarIsAlreadySponsoring : Exception {
        const char* what() const noexcept { return "new registrar is already sponsoring"; }
    };
}

#endif
