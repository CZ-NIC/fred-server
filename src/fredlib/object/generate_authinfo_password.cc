#include "src/fredlib/object/generate_authinfo_password.h"

#include <stdexcept>
#include <string>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#include <sys/time.h>

namespace Fred
{
    static unsigned long msseed() {
        struct timeval tv;
        if( gettimeofday(&tv, NULL) != 0 ) {
            throw std::runtime_error("gettimeofday() failed");
        }

        return tv.tv_usec + tv.tv_sec;
    }

    GeneratedAuthInfoPassword generate_authinfo_pw() {
        static const std::string alphabet("ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789");

        static boost::variate_generator<boost::mt19937, boost::uniform_int<> > random_alphabet_idx_generator(
            boost::mt19937(msseed()),
            boost::uniform_int<>(0, alphabet.size() - 1)
        );

        std::string result;

        for(unsigned i = 0; i < 8; ++i) {
            result += alphabet.at(random_alphabet_idx_generator());
        }

        return GeneratedAuthInfoPassword(result);
    }
}
