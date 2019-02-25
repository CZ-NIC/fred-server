/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef OPTIONS_HH_91218A95D0A09AD248FEFF1DC37C6461//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define OPTIONS_HH_91218A95D0A09AD248FEFF1DC37C6461

#include <string>
#include <cstdint>
#include <boost/optional.hpp>

namespace Tools {
namespace DiscloseFlagsUpdater {


struct GeneralOptions
{
    bool verbose;
    bool dry_run;
    bool progress_display;
    std::int16_t thread_count;
    boost::optional<std::uint64_t> logd_request_id;
    std::string by_registrar;
    std::string db_connect;
};


}
}

#endif//OPTIONS_HH_91218A95D0A09AD248FEFF1DC37C6461
