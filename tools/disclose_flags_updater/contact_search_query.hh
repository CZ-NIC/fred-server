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
