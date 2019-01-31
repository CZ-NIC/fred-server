#ifndef WORKER_HH_C95601DCE0068406DFB76AB8AE93F573//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define WORKER_HH_C95601DCE0068406DFB76AB8AE93F573

#include "tools/disclose_flags_updater/options.hh"
#include "tools/disclose_flags_updater/disclose_settings.hh"
#include "libfred/opcontext.hh"

#include <vector>
#include <utility>
#include <memory>
#include <atomic>

namespace Tools {
namespace DiscloseFlagsUpdater {


struct ContactUpdateTask
{
    ContactUpdateTask(std::uint64_t _contact_id, bool _contact_hidden_address_allowed)
        : contact_id(_contact_id), contact_hidden_address_allowed(_contact_hidden_address_allowed)
    {
    }

    std::uint64_t contact_id;
    bool contact_hidden_address_allowed;
};

using TaskCollection = std::vector<ContactUpdateTask>;
using TaskRange = std::pair<TaskCollection::const_iterator, TaskCollection::const_iterator>;

class Worker
{
private:
    GeneralOptions opts;
    DiscloseSettings discloses;
    TaskRange range;

    std::uint64_t total_count;
    std::atomic<std::uint64_t> done_count;

    bool started;
    std::atomic<bool> exited;

    std::shared_ptr<::LibFred::OperationContextCreator> ctx;


public:
    Worker(const GeneralOptions& _opts, const DiscloseSettings& _discloses, const TaskRange& _range)
        : opts(_opts), discloses(_discloses), range(_range),
          total_count(_range.second - _range.first), done_count(0),
          started(false), exited(false)
    {
    }

    Worker(const Worker& _other)
    {
        opts = _other.opts;
        discloses = _other.discloses;
        range = _other.range;
        total_count = _other.total_count;
        done_count = _other.done_count.load();
        started = _other.started;
        exited = _other.exited.load();
        ctx = _other.ctx;
    }

    std::uint64_t get_total_count() const

    {
        return total_count;
    }

    std::uint64_t get_done_count() const
    {
        return done_count.load();
    }

    auto get_ctx() -> decltype(ctx)
    {
        return ctx;
    }

    bool has_exited() const
    {
        return exited.load();
    }

    void operator()();
};



}
}

#endif//WORKER_HH_C95601DCE0068406DFB76AB8AE93F573
