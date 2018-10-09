/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file config_check.hh
 */

#ifndef CONFIG_CHECK_HH_E2B2E9AA4B484E66C688B724EC0DF0DD//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define CONFIG_CHECK_HH_E2B2E9AA4B484E66C688B724EC0DF0DD

#include <boost/program_options.hpp>

#include <string>
#include <unordered_map>

namespace Epp {
namespace Contact {

class ConfigCheck
{
public:
    ConfigCheck() = default;
    template <typename>
    bool is_type_of()const;
    struct Empty;//specialization is_type_of<Empty>
    ConfigCheck& set_name(const std::string&);
    template <typename>
    static void add_options_description(boost::program_options::options_description& options_description);
    template <typename>
    const std::string& get_value()const;
    template <typename>
    ConfigCheck& set_all_values(const boost::program_options::variables_map&);
    template <typename>
    static ConfigCheck get_default();
    using KeyValue = std::unordered_map<std::string, std::string>;
private:
    std::string name_;
    KeyValue options_;
};

}//namespace Epp::Contact
}//namespace Epp

#endif//CONFIG_CHECK_HH_E2B2E9AA4B484E66C688B724EC0DF0DD