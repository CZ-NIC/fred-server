#ifndef GENERATED_AUTHINFO_PASSWORD_6453145310
#define GENERATED_AUTHINFO_PASSWORD_6453145310

#include <string>

namespace Fred
{
    /**
     * @returns character allowed to be used in GeneratedAuthInfoPassword
     *
     * Characters should be visually distinct to prevent confustion (e. g. 'l', 'I','1' are not allowed).
     */
    inline std::string get_chars_allowed_in_generated_authinfopw() {
        return "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789";
    }

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
