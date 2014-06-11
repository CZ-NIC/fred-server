/**
 *  @file
 *  misc utils related to contact
 */

#ifndef FREDLIB_CONTACT_UTIL_H_151454412344661
#define FREDLIB_CONTACT_UTIL_H_151454412344661

#include <string>
#include <utility>


namespace Fred
{
namespace ContactUtil
{
    struct ExceptionUnknownContactHistoryId {};

    std::pair<std::string, unsigned long long> contact_hid_to_handle_id_pair(unsigned long long hid);
}
}

#endif // #include guard end
