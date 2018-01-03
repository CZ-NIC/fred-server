#ifndef GENERATED_AUTHINFO_PASSWORD_EXCEPTION_HH_6F02572E96FB45F98144165EA2284D88
#define GENERATED_AUTHINFO_PASSWORD_EXCEPTION_HH_6F02572E96FB45F98144165EA2284D88

#include "src/libfred/exception.hh"

namespace LibFred
{
    struct InvalidGeneratedAuthInfoPassword : Exception {
        const char* what() const noexcept { return "invalid generated auth info password"; }
    };
}

#endif
