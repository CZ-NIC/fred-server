#ifndef GENERATED_AUTHINFO_PASSWORD_EXCEPTION_35094544620
#define GENERATED_AUTHINFO_PASSWORD_EXCEPTION_35094544620

#include "src/fredlib/exception.h"

namespace Fred
{
    struct InvalidGeneratedAuthInfoPassword : Exception {
        const char* const what() const { return "invalid generated auth info password"; }
    };
}

#endif
