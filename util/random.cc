#include "random.h"
#include <iostream>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <string>
#include <vector>
#include <time.h>
#include <sys/time.h>


namespace Random {


unsigned long msseed()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec + tv.tv_sec;
}

boost::mt19937 rng(msseed());


int integer(const int &_min, const int &_max)
{
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

