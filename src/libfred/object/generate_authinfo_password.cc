#include "src/libfred/object/generate_authinfo_password.hh"

#include <stdexcept>
#include <string>

#include <boost/random/uniform_int.hpp>
#include <boost/nondet_random.hpp>


#include <sys/time.h>

namespace LibFred
{
    GeneratedAuthInfoPassword generate_authinfo_pw() {
        const std::string alphabet = get_chars_allowed_in_generated_authinfopw();
        boost::random_device rng;
        boost::uniform_int<> index_dist(0, alphabet.size() - 1);
        std::string result;
        const int authinfo_pw_length = 8;
        for(int i = 0; i < authinfo_pw_length; ++i) {
            result += alphabet.at(index_dist(rng));
        }
        return GeneratedAuthInfoPassword(result);
    }
}
