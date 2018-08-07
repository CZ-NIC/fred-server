#ifndef RANDOM_HH_287356A4FEE34BE9BE4B8C9B92BA9B63
#define RANDOM_HH_287356A4FEE34BE9BE4B8C9B92BA9B63

#include <string>

namespace Random {

int integer(int _min, int _max);

std::string string_from(unsigned _len, const std::string& _allowed);

std::string string_lower_alpha(unsigned _len);

std::string string_upper_alpha(unsigned _len);

std::string string_alpha(unsigned _len);

std::string string_alphanum(unsigned _len);

} // namespace Random

#endif /*RANDOM_UTIL_H_*/
