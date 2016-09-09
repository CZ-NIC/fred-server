#include "src/fredlib/object/generate_authinfo_password.h"

#include <stdexcept>
#include <string>

#include <boost/random/uniform_int.hpp>
#include <boost/nondet_random.hpp>


#include <sys/time.h>

namespace Fred
{
    GeneratedAuthInfoPassword generate_authinfo_pw() {
        const std::string alphabet = get_chars_allowed_in_generated_authinfopw();
        boost::random_device rng;
        boost::uniform_int<> index_dist(0, alphabet.size() - 1);
        std::string result;
        for(int i = 0; i < 8; ++i) {
            result += alphabet.at(index_dist(rng));
        }
        return GeneratedAuthInfoPassword(result);
    }
}
