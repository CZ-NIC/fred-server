/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#include "src/bin/corba/admin/pagetable_zones.hh"

#include <iostream>
#include <sstream>
#include <utility>

ccReg_Zones_i::ccReg_Zones_i(
        LibFred::Zone::Manager::ZoneListPtr zoneList)
	: m_zoneList(std::move(zoneList))
{
}

ccReg_Zones_i::~ccReg_Zones_i()
{
    TRACE("[CALL] ccReg_Zones_i::~ccReg_Zones_i()");
}

ccReg::Filters::Compound_ptr
ccReg_Zones_i::add()
{
    TRACE("[CALL] ccReg_Zones_i::add()");
    Database::Filters::Zone *filter =
        new Database::Filters::ZoneImpl();
    uf.addFilter(filter);
    return it.addE(filter);
}

Registry::Table::ColumnHeaders *
ccReg_Zones_i::getColumnHeaders()
{
    Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
    ch->length(14);
    COLHEAD(ch, 0, "Zone Name", CT_OID);
    COLHEAD(ch, 1, "Min. extension", CT_OTHER);
    COLHEAD(ch, 2, "Max. extension", CT_OTHER);
    COLHEAD(ch, 3, "Revalidation", CT_OTHER);
    COLHEAD(ch, 4, "Max. dots", CT_OTHER);
    COLHEAD(ch, 5, "Enum", CT_OTHER);
    COLHEAD(ch, 6, "Ttl", CT_OTHER);
    COLHEAD(ch, 7, "Hostmaster", CT_OTHER);
    COLHEAD(ch, 8, "Serial", CT_OTHER);
    COLHEAD(ch, 9, "Refresh", CT_OTHER);
    COLHEAD(ch, 10, "Retry", CT_OTHER);
    COLHEAD(ch, 11, "Expiry", CT_OTHER);
    COLHEAD(ch, 12, "Minimum", CT_OTHER);
    COLHEAD(ch, 13, "Nameserver", CT_OTHER);
    return ch;
}

Registry::TableRow *
ccReg_Zones_i::getRow(CORBA::UShort row)
{
    Logging::Context ctx(base_context_);

    const LibFred::Zone::Zone *z
        = dynamic_cast<LibFred::Zone::Zone *>(m_zoneList->get(row));
    if (!z) {
        throw Registry::Table::INVALID_ROW();
    }
    Registry::TableRow *tr = new Registry::TableRow;

    tr->length(14);

    MAKE_OID(oid_fqdn, z->getId(), C_STR(z->getFqdn()), FT_ZONE)

    (*tr)[0] <<= oid_fqdn;
    (*tr)[1] <<= C_STR(z->getExPeriodMin());
    (*tr)[2] <<= C_STR(z->getExPeriodMax());
    (*tr)[3] <<= C_STR(z->getValPeriod());
    (*tr)[4] <<= C_STR(z->getDotsMax());
    (*tr)[5] <<= C_STR(z->getEnumZone());
    (*tr)[6] <<= C_STR(z->getTtl());
    (*tr)[7] <<= C_STR(z->getHostmaster());
    (*tr)[8] <<= C_STR(z->getSerial());
    (*tr)[9] <<= C_STR(z->getRefresh());
    (*tr)[10] <<= C_STR(z->getUpdateRetr());
    (*tr)[11] <<= C_STR(z->getExpiry());
    (*tr)[12] <<= C_STR(z->getMinimum());
    (*tr)[13] <<= C_STR(z->getNsFqdn());


    return tr;
}

void
ccReg_Zones_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
    Logging::Context ctx(base_context_);

    TRACE(boost::format(
                "[CALL] ccReg_Zones_i::sortByColumn(%1%, %2%)")
            % column % dir);
    ccReg_PageTable_i::sortByColumn(column, dir);

    switch (column) {
        case 0:
            m_zoneList->sort(LibFred::Zone::MT_FQDN, dir);
            break;
        case 1:
            m_zoneList->sort(LibFred::Zone::MT_EXPERIODMIN, dir);
            break;
        case 2:
            m_zoneList->sort(LibFred::Zone::MT_EXPERIODMAX, dir);
            break;
        case 3:
            m_zoneList->sort(LibFred::Zone::MT_VALPERIOD, dir);
            break;
        case 4:
            m_zoneList->sort(LibFred::Zone::MT_DOTSMAX, dir);
            break;
        case 5:
            m_zoneList->sort(LibFred::Zone::MT_ENUMZONE, dir);
            break;
        case 6:
            m_zoneList->sort(LibFred::Zone::MT_TTL, dir);
            break;
        case 7:
            m_zoneList->sort(LibFred::Zone::MT_HOSTMASTER, dir);
            break;
        case 8:
            m_zoneList->sort(LibFred::Zone::MT_SERIAL, dir);
            break;
        case 9:
            m_zoneList->sort(LibFred::Zone::MT_REFRESH, dir);
            break;
        case 10:
            m_zoneList->sort(LibFred::Zone::MT_UPDATERETR, dir);
            break;
        case 11:
            m_zoneList->sort(LibFred::Zone::MT_EXPIRY, dir);
            break;
        case 12:
            m_zoneList->sort(LibFred::Zone::MT_MINIMUM, dir);
            break;
        case 13:
            m_zoneList->sort(LibFred::Zone::MT_NSFQDN, dir);
            break;
    }
}

ccReg::TID
ccReg_Zones_i::getRowId(CORBA::UShort row)
{
    Logging::Context ctx(base_context_);

    const LibFred::Zone::Zone *z =
            dynamic_cast<LibFred::Zone::Zone *>(m_zoneList->get(row));
    if (!z) {
        throw Registry::Table::INVALID_ROW();
    }
    return z->getId();
}

char *
ccReg_Zones_i::outputCSV()
{
    return CORBA::string_dup("1,1,1");
}

CORBA::Short
ccReg_Zones_i::numRows()
{
    Logging::Context ctx(base_context_);

    return m_zoneList->getSize();
}

CORBA::Short
ccReg_Zones_i::numColumns()
{
    return 12;
}

void
ccReg_Zones_i::reload_worker()
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    m_zoneList->setTimeout(query_timeout);
    m_zoneList->setLimit(limit_);
    m_zoneList->reload(uf);
}

void
ccReg_Zones_i::clear()
{
    Logging::Context ctx(base_context_);

    ccReg_PageTable_i::clear();
    m_zoneList->clear();
}

CORBA::ULongLong
ccReg_Zones_i::resultSize()
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE("[CALL] ccReg_Zones_i::resultSize()");
    return m_zoneList->getRealCount(uf);
}

void
ccReg_Zones_i::loadFilter(ccReg::TID id)
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Zones_i::loadFilter(%1%)") % id);

    ccReg_PageTable_i::loadFilter(id);

    Database::Filters::Union::iterator uit = uf.begin();
    for (; uit != uf.end(); ++uit) {
        Database::Filters::Zone *tmp =
            dynamic_cast<Database::Filters::Zone *>(*uit);
        if (tmp) {
            it.addE(tmp);
            TRACE(boost::format("[IN] ccReg_Zones_i::loadFilter(%1%): "
                        "loaded filter content = %2%")
                    % id % tmp->getContent());
        }
    }
}

void
ccReg_Zones_i::saveFilter(const char *name)
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Zones_i::saveFilter(%1%)")
            % name);

    std::unique_ptr<LibFred::Filter::Manager> tmp_filter_manager(
            LibFred::Filter::Manager::create());
    tmp_filter_manager->save(LibFred::Filter::FT_ZONE, name, uf);
}

LibFred::Zone::Zone *
ccReg_Zones_i::findId(ccReg::TID id)
{
    Logging::Context ctx(base_context_);
    try {
        LibFred::Zone::Zone *z =
                m_zoneList->findId(id);
        if (z) {
            return z;
        }
        return 0;
    } catch (const LibFred::NOT_FOUND&) {
        return 0;
    }
}

CORBA::Boolean
ccReg_Zones_i::numRowsOverLimit()
{
    Logging::Context ctx(base_context_);

    return m_zoneList->isLimited();
}
