#ifndef CONTACT_VALIDATOR_HH_082D41229FAB4477BC5BBB3C88922C44
#define CONTACT_VALIDATOR_HH_082D41229FAB4477BC5BBB3C88922C44

#include "src/deprecated/libfred/contact_verification/contact.hh"
#include "util/log/logger.hh"

#include <boost/function.hpp>
#include <map>

namespace LibFred {
namespace Contact {
namespace Verification {


enum ValidationError
{
    NOT_AVAILABLE,
    INVALID,
    REQUIRED
};

typedef std::map<std::string, ValidationError> FieldErrorMap;

struct DataValidationError : public std::runtime_error
{
    DataValidationError(const FieldErrorMap &_e) :
        std::runtime_error("data validation error"),
        errors(_e)
    {
    }
    ~DataValidationError() {}
    FieldErrorMap errors;
};


class ContactValidator
{
public:
    typedef boost::function<bool (const Contact &_data, FieldErrorMap &_errors)> Checker;

    void add_checker(Checker _func)
    {
        checkers_.push_back(_func);
    }

    void check(const Contact &_data) const
    {
        FieldErrorMap errors;

        std::vector<Checker>::const_iterator check = checkers_.begin();
        for (; check != checkers_.end(); ++check) {
            (*check)(_data, errors);
        }

        LOGGER.debug(boost::format("data validation ran %1% check(s)"
                    " -- found %2% error(s)") % checkers_.size() % errors.size());
        if (!errors.empty()) {
            throw DataValidationError(errors);
        }
    }


private:
    std::vector<Checker> checkers_;
};


}
}
}

#endif

