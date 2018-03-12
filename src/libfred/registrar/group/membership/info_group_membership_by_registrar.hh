#ifndef INFO_GROUP_MEMBERSHIP_BY_REGISTRAR_HH_
#define INFO_GROUP_MEMBERSHIP_BY_REGISTRAR_HH_

#include "src/libfred/registrar/group/membership/registrar_group_membership_types.hh"
#include "src/libfred/opcontext.hh"

#include <vector>

namespace LibFred {
namespace Registrar {

class InfoGroupMembershipByRegistrar
{
private:
    unsigned long long registrar_id;

public:
    InfoGroupMembershipByRegistrar(unsigned long long _registrar_id)
    : registrar_id(_registrar_id)
    {}

    std::vector<GroupMembershipByRegistrar> exec(OperationContext& ctx);
};

} // namespace Registrar
} // namespace LibFred

#endif
