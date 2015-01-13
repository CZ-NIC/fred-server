#ifndef CONTACT_VERIFICATION_CHECKERS_H_454838214
#define CONTACT_VERIFICATION_CHECKERS_H_454838214

#include "src/fredlib/contact_verification/contact_validator.h"
#include "util/optional_value.h"
#include <boost/noncopyable.hpp>
#include <boost/regex.hpp>
#include <string>
#include <algorithm>

#include <idna.h>

namespace Fred {
namespace Contact {
namespace Verification {

const boost::regex PHONE_PATTERN("^\\+[0-9]{1,3}\\.[0-9]{1,14}$");
const boost::regex PHONE_CZ_SK_PATTERN("^\\+42(0\\.(60[1-9]|7\\d\\d|91\\d)|1\\.9(0[1-9]|[145]\\d))[0-9]{6}$");
const boost::regex EMAIL_PATTERN("^[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+(\\.[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+)*"
                                 "@(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,6}\\.?$");
const boost::regex POSTALCODE_CZ_PATTERN("^[0-9]{3} ?[0-9]{2}$");
const std::string EMAIL_PHONE_PROTECTION_PERIOD = "1 month";

const std::string field_phone        = "phone.number";
const std::string field_fax          = "phone.fax";
const std::string field_first_name   = "contact.first_name";
const std::string field_last_name    = "contact.last_name";
const std::string field_street1      = "address.street1";
const std::string field_country      = "address.country";
const std::string field_city         = "address.city";
const std::string field_postal_code  = "address.postal_code";
const std::string field_email        = "email.email";
const std::string field_notify_email = "email.notify_email";
const std::string field_auth_info    = "contact.auth_info";
const std::string field_status       = "contact.status";

/**
 * should work the same as django validator viz https://github.com/django/django/blob/1.6.9/django/core/validators.py#L80-L124
 */
class ContactEmailFormat
{
    const std::vector<std::string> domain_whitelist_;
    const boost::regex user_regex;
    const boost::regex domain_regex;

    class UTF8ToPunnycode : boost::noncopyable
    {
        char *out_p;
    public:
        UTF8ToPunnycode(const std::string& utf8_str)
        : out_p(0)
        {
            if(idna_to_ascii_8z(utf8_str.c_str(), &out_p, 0) != IDNA_SUCCESS) out_p = 0;
        }

        ~UTF8ToPunnycode()
        {
            if(out_p != 0) free(out_p);
        }

        Optional<std::string> get()
        {
            if(out_p != 0)
                return Optional<std::string>(std::string(out_p));
            else
                return Optional<std::string>();
        }
    };


public:
    ContactEmailFormat(const std::vector<std::string>& domain_whitelist = std::vector<std::string>(1,"localhost"))
    : domain_whitelist_(domain_whitelist)
    , user_regex("(^[-!#$%&'*+/=?^_`{}|~0-9A-Z]+(\\.[-!#$%&'*+/=?^_`{}|~0-9A-Z]+)*$" //dot-atom
        "|^\"([\\0001-\\0010\\0013\\0014\\0016-\\0037!#-\\[\\]-\\0177]|\\\\[\\0001-\\0011\\0013\\0014\\0016-\\0177])*\"$)" //quoted-string
        , boost::regex::icase)
    , domain_regex("(?:[A-Z0-9](?:[A-Z0-9-]{0,61}[A-Z0-9])?\\.)+(?:[A-Z]{2,6}|[A-Z0-9-]{2,}(?<!-))$"
        "|^\\[(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\.(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3}\\]$"// literal form, ipv4 address (SMTP 4.1.3)
        , boost::regex::icase)
    {}



    bool check(const std::string& email )
    {
        if(email.empty()) return false;
        if(email.length() > 200) return false;//mojeid max email length
        std::string::size_type email_at = email.rfind('@');
        if (email_at == std::string::npos) return false;

        try
        {
            std::string user_part = email.substr(0,email_at);
            if(user_part.empty()) return false;

            std::string domain_part = email.substr(email_at+1);
            if(domain_part.empty()) return false;

            if (!boost::regex_match(user_part,user_regex)) return false;

            if(std::find(domain_whitelist_.begin(), domain_whitelist_.end()
                , domain_part)!=domain_whitelist_.end())
            {
                return true;
            }

            if (!boost::regex_match(domain_part,domain_regex))
            {
                //domain part idn conversion
                std::string converted_domain_part = UTF8ToPunnycode(domain_part).get().get_value_or_default();

                if(converted_domain_part.empty()) return false;

                if(!boost::regex_match(converted_domain_part,domain_regex)) return false;
            }
        }
        catch(const std::exception& ex)
        {
            return false;
        }
        return true;
    };

};


bool contact_checker_name(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_phone_cz_sk_format(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_phone_format(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_fax_format(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_auth_info(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_phone_required(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_phone_unique(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_email_format(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_email_required(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_email_unique(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_notify_email_format(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_address_required(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_address_country(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_address_postalcode_format_cz(const Contact &_data, FieldErrorMap &_errors);



}
}
}

#endif /*CONTACT_VERIFICATION_CHECKERS_H__*/

