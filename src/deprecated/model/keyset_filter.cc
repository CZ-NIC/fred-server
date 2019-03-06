/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/deprecated/model/keyset_filter.hh"

namespace Database {
namespace Filters {

// ------------------------------------------------------------
//  KeySet implementation
// ------------------------------------------------------------

KeySetImpl::KeySetImpl(): ObjectImpl()
{
    setName("KeySet");
    addType().setValue(getType());
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

Value<std::string> &
KeySetImpl::addHandle()
{
    Value<std::string> *tmp = new Value<std::string>(Column("name", joinObjectRegistryTable()));
    add(tmp);
    tmp->addPreValueString("UPPER(");
    tmp->addPostValueString(")");
    tmp->setName("Handle");
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
                Column("contactid", joinTable("keyset_contact_map")),
                SQL_OP_EQ,
                Column("id", tmp->joinObjectRegistryTable())
                )
            );
    tmp->joinOn(
            new Join(
                Column("id", joinKeySetTable()),
                SQL_OP_EQ,
                Column("keysetid", joinTable("keyset_contact_map"))
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
//  KeySet history implementation
// ------------------------------------------------------------

KeySetHistoryImpl::KeySetHistoryImpl() :
    ObjectHistoryImpl()
{
    setName("KeySetHistory");
    addType().setValue(getType());
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

Value<std::string> &
KeySetHistoryImpl::addHandle()
{
    Value<std::string> *tmp = new Value<std::string>(Column("name", joinObjectRegistryTable()));
    add(tmp);
    tmp->addPreValueString("UPPER(");
    tmp->addPostValueString(")");
    tmp->setName("Handle");
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
                Column("id", tmp->joinObjectRegistryTable()),
                SQL_OP_EQ,
                Column("contactid", joinTable("keyset_contact_map_history"))
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


void
KeySetHistoryImpl::_joinPolymorphicTables()
{
    Table *n = findTable("keyset_history");
    if (n) {
        joins.push_front(
                new Join(
                    Column("historyid", joinTable("object_history")),
                    SQL_OP_EQ,
                    Column("historyid", *n)
                    )
                );
    }
    ObjectHistoryImpl::_joinPolymorphicTables();
}

} // namespace Filters
} // namespace Database
