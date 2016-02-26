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

    struct ExceptionUnknownRegistrar : Exception {
        const char* what() const throw() { return "unknown registrar"; }
    };

    struct ExceptionUnknownObjectId : Exception {
        const char* what() const throw() { return "unknown object id"; }
    };

    struct UnknownContactId : ExceptionUnknownObjectId {
        const char* what() const throw() { return "unknown contact id"; }
    };
}

#endif
