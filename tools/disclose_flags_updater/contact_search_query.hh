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
#ifndef CONTACT_SEARCH_QUERY_HH_87BCC31D6475B588521C2A5412DE3BFF//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define CONTACT_SEARCH_QUERY_HH_87BCC31D6475B588521C2A5412DE3BFF

#include <string>
#include "tools/disclose_flags_updater/disclose_settings.hh"

namespace Tools {
namespace DiscloseFlagsUpdater {


std::string make_query_search_contact_needs_update(const DiscloseSettings& _discloses);


}
}
#endif//CONTACT_SEARCH_QUERY_HH_87BCC31D6475B588521C2A5412DE3BFF
