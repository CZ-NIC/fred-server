#include "src/epp/contact/util.h"

#include "util/db/nullable.h"

#include <boost/algorithm/string/trim.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Contact {

boost::optional<std::string> trim(const boost::optional<std::string>& src)
{
    return src ? boost::optional<std::string>(boost::trim_copy(*src))
               : boost::optional<std::string>();
}


boost::optional<Nullable<std::string> > trim(const boost::optional<Nullable<std::string> >& src)
{
    return src && !src->isnull() ? boost::optional<Nullable<std::string> >(boost::trim_copy(src->get_value()))
                                 : src;
}


std::vector<boost::optional<Nullable<std::string> > > trim(
        const std::vector<boost::optional<Nullable<std::string> > >& src)
{
    std::vector<boost::optional<Nullable<std::string> > > result;
    result.reserve(src.size());
    for (std::vector<boost::optional<Nullable<std::string> > >::const_iterator data_ptr = src.begin();
         data_ptr != src.end();
         ++data_ptr)
    {
        result.push_back(trim(*data_ptr));
    }
    return result;
}


} // namespace Epp::Contact
} // namespace Epp
