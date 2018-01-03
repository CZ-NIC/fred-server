#ifndef TRANSFER_OBJECT_EXCEPTION_HH_82F203BD38904261926C0EAFB443A66B
#define TRANSFER_OBJECT_EXCEPTION_HH_82F203BD38904261926C0EAFB443A66B

#include "src/libfred/exception.hh"

namespace LibFred
{
    struct NewRegistrarIsAlreadySponsoring : Exception {
        const char* what() const noexcept { return "new registrar is already sponsoring"; }
    };
}

#endif
