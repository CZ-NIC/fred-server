#include <boost/algorithm/string/trim.hpp>
#include <vector>

#include "src/util/db/value.hh"
#include "src/util/string_split.hh"


namespace Database {


std::vector<std::string> array_to_vector(std::string _dbarr)
{
    boost::algorithm::trim(_dbarr);
    if ((*(_dbarr.begin()) == '{') && (*(_dbarr.end() - 1) == '}'))
    {
        _dbarr.erase(_dbarr.begin());
        _dbarr.erase(_dbarr.end() -1);
    }
    else
    {
        throw std::runtime_error("not a database array value");
    }

    return split<std::string>(_dbarr, ",");
}


}

