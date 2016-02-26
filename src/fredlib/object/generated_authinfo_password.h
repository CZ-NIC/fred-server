#ifndef GENERATED_AUTHINFO_PASSWORD_6453145310
#define GENERATED_AUTHINFO_PASSWORD_6453145310

#include <string>

namespace Fred
{
    /**
     * Represents newly generated authinfo that is valid according to our rules.
     */
    struct GeneratedAuthInfoPassword {
        /** @throws InvalidGeneratedAuthInfoPassword */
        explicit GeneratedAuthInfoPassword(const std::string& _password);
        const std::string password_;
    };
}

#endif
