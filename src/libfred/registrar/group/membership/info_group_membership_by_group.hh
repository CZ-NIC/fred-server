#ifndef INFO_GROUP_MEMBERSHIP_BY_GROUP_HH_
#define INFO_GROUP_MEMBERSHIP_BY_GROUP_HH_

#include "src/libfred/registrar/group/membership/registrar_group_membership_types.hh"
#include "src/libfred/opcontext.hh"

#include <vector>

namespace LibFred {
namespace Registrar {

class InfoGroupMembershipByGroup
{
private:
    unsigned long long group_id;

public:
    InfoGroupMembershipByGroup(unsigned long long _group_id)
    : group_id(_group_id)
    {}

    std::vector<GroupMembershipByGroup> exec(OperationContext& ctx);
};

} // namespace Registrar
} // namespace LibFred

#endif
