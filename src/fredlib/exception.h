#ifndef EXCEPTION_563065613107
#define EXCEPTION_563065613107

namespace Fred
{
    struct Exception {
        virtual const char* const what() const = 0;
        virtual ~Exception() { }
    };

    struct IncorrectAuthInfoPw : Exception {
        const char* const what() const { return "incorrect AuthInfoPw"; }
    };

    struct ExceptionUnknownRegistrar : Exception {
        const char* const what() const { return "unknown registrar"; }
    };

    struct ExceptionUnknownObjectId : Exception {
        const char* const what() const { return "unknown object id"; }
    };

    struct UnknownContactId : ExceptionUnknownObjectId {
        const char* const what() const { return "unknown contact id"; }
    };
}

#endif
