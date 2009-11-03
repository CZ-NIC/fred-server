#include "zone_ns_filter.h"

namespace Database {
namespace Filters {

ZoneNsImpl::ZoneNsImpl(bool set_active)
	: Compound()
{
    setName("ZoneNs");
    active = set_active;
}

ZoneNsImpl::~ZoneNsImpl()
{ }

Table &
ZoneNsImpl::joinZoneNsTable()
{
    return joinTable("zone_ns");
}

Value<Database::ID> &
ZoneNsImpl::addId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinZoneNsTable()));
    add(tmp);
    tmp->setName("Id");
    return *tmp;
}

Value<Database::ID> &
ZoneNsImpl::addZoneId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(Column("zone", joinZoneNsTable()));
    add(tmp);
    tmp->setName("ZoneId");
    return *tmp;
}

Value<std::string> &
ZoneNsImpl::addFqdn()
{
    Value<std::string> *tmp = new Value<std::string>(Column("fqdn", joinZoneNsTable()));
    add(tmp);
    tmp->setName("Fqdn");
    return *tmp;
}

Value<std::string> &
ZoneNsImpl::addAddrs()
{
    Value<std::string> *tmp = new Value<std::string>(Column("addrs", joinZoneNsTable()));
    add(tmp);
    tmp->setName("Addrs");
    return *tmp;
}

} // namespace Filters
} // namespace Database
