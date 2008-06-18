#include "keyset_filter.h"

namespace DBase {
namespace Filters {

// ------------------------------------------------------------
// KeySet implementation
// ------------------------------------------------------------

KeySetImpl::KeySetImpl(): ObjectImpl()
{
    setName("KeySet");
    setType(getType());
}

KeySetImpl::~KeySetImpl()
{
}

Table &
KeySetImpl::joinKeySetTable()
{
    return joinTable("keyset");
}

Value<ID> &
KeySetImpl::addId()
{
    Value<ID> *tmp = new Value<ID>(Column("id", joinKeySetTable()));
    tmp->setName("Id");
    add(tmp);
    return *tmp;
}

Contact &
KeySetImpl::addTechContact()
{
    Contact *tmp = new ContactImpl();
    add(tmp);
    tmp->setName("TechContact");
    tmp->addJoin(
            new Join(
                Column("contact_id", joinTable("keyset_contact_map")),
                SQL_OP_EQ,
                Column("id", tmp->joinObjectRegistryTable())
                )
            );
    tmp->joinOn(
            new Join(
                Column("id", joinKeySetTable()),
                SQL_OP_EQ,
                Column("keyset_id", joinTable("keyset_contact_map"))
                )
            );
    return *tmp;
}

Domain &
KeySetImpl::addDomain()
{
    Domain *tmp = new DomainImpl();
    tmp->setName("Domain");
    tmp->joinOn(
            new Join(
                Column("id", joinKeySetTable()),
                SQL_OP_EQ,
                Column("keyset_id", joinTable("domain"))
                )
            );
    return *tmp;
}

void
KeySetImpl::_joinPolymorphicTables()
{
    ObjectImpl::_joinPolymorphicTables();
    Table *n = findTable("keyset");
    if (n) {
        joins.push_front(new Join(
                    Column("id", joinTable("object_registry")),
                    SQL_OP_EQ,
                    Column("id", *n)));
    }
}

// ------------------------------------------------------------
// KeySet history implementation
// ------------------------------------------------------------

KeySetHistoryImpl::KeySetHistoryImpl() :
    ObjectHistoryImpl()
{
    setName("KeySetHistory");
    setType(getType());
}

KeySetHistoryImpl::~KeySetHistoryImpl()
{
}

Table &
KeySetHistoryImpl::joinKeySetTable()
{
    return joinTable("keyset_history");
}

Value<ID> &
KeySetHistoryImpl::addId()
{
    Value<ID> *tmp = new Value<ID>(Column("id", joinKeySetTable()));
    tmp->setName("Id");
    add(tmp);
    return *tmp;
}

Contact &
KeySetHistoryImpl::addTechContact()
{
    Contact *tmp = new ContactHistoryImpl();
    add(tmp);
    tmp->setName("TechContact");
    tmp->addJoin(
            new Join(
                Column("contact_id", joinTable("keyset_contact_map_history")),
                SQL_OP_EQ,
                Column("id", tmp->joinObjectRegistryTable())
                )
            );
    tmp->joinOn(
            new Join(
                Column("historyid", joinKeySetTable()),
                SQL_OP_EQ,
                Column("historyid", joinTable("keyset_contact_map_history"))
                )
            );
    return *tmp;
}

Domain &
KeySetHistoryImpl::addDomain()
{
    Domain *tmp = new DomainHistoryImpl();
    tmp->setName("Domain");
    tmp->joinOn(
            new Join(
                Column("id", joinKeySetTable()),
                SQL_OP_EQ,
                Column("keyset_id", joinTable("domain"))
                )
            );
    return *tmp;
}

void
KeySetHistoryImpl::_joinPolymorphicTables()
{
    ObjectHistoryImpl::_joinPolymorphicTables();
    Table *n = findTable("keyset_history");
    if (n) {
        joins.push_front(
                new Join(
                    Column("historyid", joinTable("object_registry")),
                    SQL_OP_EQ,
                    Column("historyid", *n)
                    )
                );
    }
}

} // namespace Filters
} // namespace DBase
