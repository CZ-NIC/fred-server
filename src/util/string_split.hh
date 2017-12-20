#include <string>
#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>


template<class T>
std::vector<T> split(const std::string &_str, const char *_separators)
{
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(_separators);

    std::vector<T> output;
    tokenizer t(_str, sep);
    for (tokenizer::iterator it = t.begin(); it != t.end(); ++it)
    {
        output.push_back(boost::lexical_cast<T>(*it));
    }
    return output;
}

