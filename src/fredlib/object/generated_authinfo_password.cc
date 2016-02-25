#include "src/fredlib/object/generated_authinfo_password.h"

#include <string>
#include <set>

#include <boost/foreach.hpp>

#include "src/fredlib/object/generated_authinfo_password_exception.h"

namespace Fred
{
    static inline std::string::size_type min_length() { return 8; }
    static inline std::string::size_type max_length() { return 300; }
    /* This is not the most efficient solution. Expecting to be optimized in case it is an issue (but avoiding less clear and potentially unnecessary premature optimization for now). */
    static inline std::set<char> get_allowed_chars() {
        const std::string allowed_chars_as_string("ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789");

        return std::set<char>(
            allowed_chars_as_string.begin(),
            allowed_chars_as_string.end()
        );
    }


    GeneratedAuthInfoPassword::GeneratedAuthInfoPassword(const std::string& _password)
    :   password_(_password)
    {
        if(password_.length() < min_length() || password_.length() > max_length()) { throw InvalidGeneratedAuthInfoPassword(); }

        const std::set<char> allowed_chars = get_allowed_chars();

        BOOST_FOREACH(const char& c, password_) {
            if( allowed_chars.find(c) == allowed_chars.end() ) {
                throw InvalidGeneratedAuthInfoPassword();
            }
        }
    }
}
