/**
 *  @file
 *  misc utils related to contact
 */

#ifndef UTIL_HH_8801447BBF01407EA488B90620A42F69
#define UTIL_HH_8801447BBF01407EA488B90620A42F69

#include <string>
#include <utility>


namespace LibFred
{
namespace ContactUtil
{
    struct ExceptionUnknownContactHistoryId {};

    std::pair<std::string, unsigned long long> contact_hid_to_handle_id_pair(unsigned long long hid);
}
}

#endif
