#include "tools/disclose_flags_updater/thread_safe_output.hh"

#include <iostream>
#include <mutex>

namespace Tools {
namespace DiscloseFlagsUpdater {


std::mutex& get_output_mutex()
{
    static std::mutex m;
    return m;
}


void safe_cout(const std::string& _message)
{
    std::lock_guard<std::mutex> lock(get_output_mutex());
    std::cout << _message;
}


void safe_cout_flush()
{
    std::lock_guard<std::mutex> lock(get_output_mutex());
    std::cout.flush();
}


void safe_cerr(const std::string& _message)
{
    std::lock_guard<std::mutex> lock(get_output_mutex());
    std::cerr << _message;
}


}
}
