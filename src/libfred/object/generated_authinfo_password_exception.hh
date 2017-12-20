#ifndef GENERATED_AUTHINFO_PASSWORD_EXCEPTION_35094544620
#define GENERATED_AUTHINFO_PASSWORD_EXCEPTION_35094544620

#include "src/libfred/exception.hh"

namespace LibFred
{
    struct InvalidGeneratedAuthInfoPassword : Exception {
        const char* what() const noexcept { return "invalid generated auth info password"; }
    };
}

#endif
