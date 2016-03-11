#ifndef EXCEPTION_563065613107
#define EXCEPTION_563065613107

#include <stdexcept>

namespace Fred
{
    struct Exception : std::exception {
        virtual const char* what() const throw() = 0;
        virtual ~Exception() throw() { }
    };

    struct IncorrectAuthInfoPw : Exception {
        const char* what() const throw() { return "incorrect AuthInfoPw"; }
    };

    struct UnknownRegistrar : Exception {
        const char* what() const throw() { return "unknown registrar"; }
    };

    struct UnknownObjectId : Exception {
        const char* what() const throw() { return "unknown object id"; }
    };

    struct UnknownContactId : UnknownObjectId {
        const char* what() const throw() { return "unknown contact id"; }
    };

    struct UnknownDomainId : UnknownObjectId {
        const char* what() const throw() { return "unknown contact id"; }
    };

    struct UnknownKeysetId : UnknownObjectId {
        const char* what() const throw() { return "unknown contact id"; }
    };

    struct UnknownNssetId : UnknownObjectId {
        const char* what() const throw() { return "unknown contact id"; }
    };
}

#endif
