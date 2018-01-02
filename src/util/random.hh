#ifndef RANDOM_HH_287356A4FEE34BE9BE4B8C9B92BA9B63
#define RANDOM_HH_287356A4FEE34BE9BE4B8C9B92BA9B63

#include <string>


namespace Random {


int integer(const int &_min, const int &_max);

std::string string_from(const unsigned int &_len, const std::string &_allowed);

std::string string_lower_alpha(const unsigned int &_len);

std::string string_upper_alpha(const unsigned int &_len);

std::string string_alpha(const unsigned int &_len);

std::string string_alphanum(const unsigned int &_len);

template<typename T1, typename T2>
T2 choice(const T1 &_from)
{
    return _from[integer(0, static_cast<int>(_from.size() - 1))];
}


}

#endif /*RANDOM_UTIL_H_*/

