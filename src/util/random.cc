#include "src/util/random.hh"
#include <iostream>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <string>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <atomic>


namespace Random {

namespace
{

class RandomSequenceNumber
{
public:
    RandomSequenceNumber()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        id_.store(1000 * 1000 * static_cast<unsigned long>(tv.tv_sec) + tv.tv_usec);
    }

    unsigned long next()
    {
        return static_cast<unsigned long>(++id_);
    }

private:
    std::atomic<unsigned long> id_;
};

RandomSequenceNumber& get_random_sequence_number()
{
    static RandomSequenceNumber global_seed;
    return global_seed;
}

} // namespace Random::{anonymous}


int integer(const int &_min, const int &_max)
{
    thread_local boost::mt19937 rng(get_random_sequence_number().next());
    boost::uniform_int<> range(_min, _max);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<> > gen(rng, range);
    return gen();
}


std::string string_from(const unsigned int &_len, const std::string &_allowed)
{
    std::string out;
    out.reserve(_len);
    for (unsigned int i = 0; i < _len; i++) {
        out += choice<std::string, char>(_allowed);
    }
    return out;
}


std::string string_lower_alpha(const unsigned int &_len)
{
    return string_from(_len, "abcdefghijklmnopqrstuvwxyz");
}


std::string string_upper_alpha(const unsigned int &_len)
{
    return string_from(_len, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}


std::string string_alpha(const unsigned int &_len)
{
    return string_from(_len, "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}


std::string string_alphanum(const unsigned int &_len)
{
    return string_from(_len, "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
}


}

