#ifndef GENERATE_AUTHINFO_PASSWORD_9134743434
#define GENERATE_AUTHINFO_PASSWORD_9134743434

#include "src/fredlib/object/generated_authinfo_password.h"

namespace Fred
{
    /**
    * Pseudo-random transfer password generator.
    * @return new generated password of length 8 characters
    * @note uses alphabet "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789" with visually ambiguous characters excluded
    */
    GeneratedAuthInfoPassword generate_authinfo_pw();
}

#endif
