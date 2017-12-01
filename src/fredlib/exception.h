#ifndef EXCEPTION_563065613107
#define EXCEPTION_563065613107

#include <stdexcept>

namespace Fred
{
    struct Exception : std::exception {
        virtual const char* what() const noexcept = 0;
        virtual ~Exception() { }
    };

    struct IncorrectAuthInfoPw : Exception {
        const char* what() const noexcept { return "incorrect AuthInfoPw"; }
    };

    struct UnknownRegistrar : Exception {
        const char* what() const noexcept { return "unknown registrar"; }
    };

    struct UnknownObjectId : Exception {
        const char* what() const noexcept { return "unknown object id"; }
    };

    struct UnknownContactId : UnknownObjectId {
        const char* what() const noexcept { return "unknown contact id"; }
    };

    struct UnknownDomainId : UnknownObjectId {
        const char* what() const noexcept { return "unknown domain id"; }
    };

    struct UnknownKeysetId : UnknownObjectId {
        const char* what() const noexcept { return "unknown keyset id"; }
    };

    struct UnknownNssetId : UnknownObjectId {
        const char* what() const noexcept { return "unknown nsset id"; }
    };
}

#endif
