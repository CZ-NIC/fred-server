#ifndef CREATE_REGISTRAR_GROUP_MEMBERSHIP_HH_A101FCFEC90C4CB799AFBE8F53F22EF6
#define CREATE_REGISTRAR_GROUP_MEMBERSHIP_HH_A101FCFEC90C4CB799AFBE8F53F22EF6

#include "src/libfred/opcontext.hh"
#include "src/util/optional_value.hh"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace LibFred {
namespace Registrar {

class CreateRegistrarGroupMembership
{
public:
    CreateRegistrarGroupMembership(
        unsigned long long _registrar_id,
        unsigned long long _group_id,
        const boost::gregorian::date& _member_from,
        Optional<boost::gregorian::date> _member_until = Optional<boost::gregorian::date>())
    : registrar_id_(_registrar_id),
      group_id_(_group_id),
      member_from_(_member_from),
      member_until_(_member_until)
    {}

    unsigned long long exec(OperationContext& _ctx);

private:
    unsigned long long registrar_id_;
    unsigned long long group_id_;
    boost::gregorian::date member_from_;
    Optional<boost::gregorian::date> member_until_;

};

} // namespace Registrar
} // namespace LibFred

#endif
