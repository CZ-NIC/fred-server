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