#ifndef GENERATED_AUTHINFO_PASSWORD_HH_6D0E2560F0FD407DBCBA5E6F072AB663
#define GENERATED_AUTHINFO_PASSWORD_HH_6D0E2560F0FD407DBCBA5E6F072AB663

#include <string>

namespace LibFred
{
    /**
     * @returns character allowed to be used in GeneratedAuthInfoPassword
     *
     * Characters should be visually distinct to prevent confusion (e. g. 'l', 'I','1' are not allowed).
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
