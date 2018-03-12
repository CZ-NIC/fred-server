#ifndef REGISTRAR_GROUP_MEMBERSHIP_TYPES_HH_D615399AE64341FB8A358AA941296CEB
#define REGISTRAR_GROUP_MEMBERSHIP_TYPES_HH_D615399AE64341FB8A358AA941296CEB

#include <boost/date_time/gregorian/gregorian.hpp>

namespace LibFred {
namespace Registrar {

struct GroupMembershipByGroup
{
    unsigned long long membership_id;
    unsigned long long registrar_id;
    boost::gregorian::date member_from;
    boost::gregorian::date member_until;
};

struct GroupMembershipByRegistrar
{
    unsigned long long membership_id;
    unsigned long long group_id;
    boost::gregorian::date member_from;
    boost::gregorian::date member_until;
};

} // namespace Registrar
} // namespace LibFred

#endif
