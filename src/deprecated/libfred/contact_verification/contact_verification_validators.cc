#include "src/deprecated/libfred/contact_verification/contact_verification_validators.hh"

namespace LibFred {
namespace Contact {
namespace Verification {


ContactValidator create_default_contact_validator()
{
    ContactValidator tmp;
    tmp.add_checker(contact_checker_name);
    tmp.add_checker(contact_checker_address_required);
    tmp.add_checker(contact_checker_email_format);
    tmp.add_checker(contact_checker_email_required);
    tmp.add_checker(contact_checker_phone_format);
    tmp.add_checker(contact_checker_notify_email_format);
    tmp.add_checker(contact_checker_fax_format);
    return tmp;
}


ContactValidator create_conditional_identification_validator()
{
    ContactValidator tmp = create_default_contact_validator();
    tmp.add_checker(contact_checker_email_unique);
    tmp.add_checker(contact_checker_phone_required);
    tmp.add_checker(contact_checker_phone_unique);
    return tmp;
}


ContactValidator create_identification_validator()
{
    ContactValidator tmp = create_default_contact_validator();
    tmp.add_checker(contact_checker_email_unique);
    return tmp;
}


ContactValidator create_finish_identification_validator()
{
    ContactValidator tmp = create_default_contact_validator();
    return tmp;
}


}
}
}

