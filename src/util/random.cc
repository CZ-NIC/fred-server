#include "src/util/random.hh"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#include <atomic>

#include <sys/time.h>
#include <time.h>


namespace Random {

namespace {

boost::mt19937& get_engine()
{
    class RandomSequenceNumber
    {
    public:
        static RandomSequenceNumber& get_singleton()
        {
            static RandomSequenceNumber singleton;
            return singleton;
        }
        unsigned long next()
        {
            return static_cast<unsigned long>(++id_);
        }

    private:
        RandomSequenceNumber()
            : id_(pseudorandom_value())
        {
        }
        static unsigned long pseudorandom_value()
        {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            return 1000 * 1000 * static_cast<unsigned long>(tv.tv_sec) + tv.tv_usec;
        }
        std::atomic<unsigned long> id_;
    };

    thread_local boost::mt19937 engine(RandomSequenceNumber::get_singleton().next());
    return engine;
}

} // namespace Random::{anonymous}

int integer(int _min, int _max)
{
    boost::uniform_int<> range(_min, _max);
    boost::variate_generator<decltype(get_engine()), boost::uniform_int<>> gen(get_engine(), range);
    return gen();
}

std::string string_from(unsigned _len, const std::string& _allowed)
{
    std::string out;
    out.reserve(_len);
    for (unsigned int i = 0; i < _len; ++i)
    {
        out += _allowed[integer(0, static_cast<int>(_allowed.size() - 1))];
    }
    return out;
}

std::string string_lower_alpha(unsigned _len)
{
    return string_from(_len, "abcdefghijklmnopqrstuvwxyz");
}

std::string string_upper_alpha(unsigned _len)
{
    return string_from(_len, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

std::string string_alpha(unsigned _len)
{
    return string_from(_len, "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

std::string string_alphanum(unsigned _len)
{
    return string_from(_len, "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "1234567890");
}

} // namespace Random
