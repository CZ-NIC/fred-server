#ifndef GENERATE_AUTHINFO_PASSWORD_HH_8124C724E70D40E0A3D1BC4D95AF19D0
#define GENERATE_AUTHINFO_PASSWORD_HH_8124C724E70D40E0A3D1BC4D95AF19D0

#include "src/libfred/object/generated_authinfo_password.hh"

namespace LibFred
{
    /**
    * Pseudo-random transfer password generator.
    * @return new generated password of length 8 characters
    * @note uses alphabet "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789" with visually ambiguous characters excluded
    */
    GeneratedAuthInfoPassword generate_authinfo_pw();
}

#endif
