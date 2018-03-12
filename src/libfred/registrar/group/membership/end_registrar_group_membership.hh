#ifndef ENDREGISTRAR_GROUP_MEMBERSHIP_HH_6D432285E8894224BFF3680B7A8ED2A5
#define ENDREGISTRAR_GROUP_MEMBERSHIP_HH_6D432285E8894224BFF3680B7A8ED2A5

#include "src/libfred/opcontext.hh"

namespace LibFred {
namespace Registrar {

class EndRegistrarGroupMembership
{
public:
    EndRegistrarGroupMembership(
        unsigned long long _registrar_id,
        unsigned long long _group_id)
    : registrar_id_(_registrar_id),
      group_id_(_group_id)
    {}

    void exec(OperationContext& _ctx);

private:
    unsigned long long registrar_id_;
    unsigned long long group_id_;

};

} // namespace Registrar
} // namespace LibFred

#endif

