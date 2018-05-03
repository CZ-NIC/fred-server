#ifndef THREAD_SAFE_OUTPUT_HH_78DCBB04004B79212589DA174F0D99A6//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define THREAD_SAFE_OUTPUT_HH_78DCBB04004B79212589DA174F0D99A6

#include <mutex>


namespace Tools {
namespace DiscloseFlagsUpdater {


void safe_cout(const std::string& _message);

void safe_cout_flush();

void safe_cerr(const std::string& _message);


}
}

#endif//THREAD_SAFE_OUTPUT_HH_78DCBB04004B79212589DA174F0D99A6
