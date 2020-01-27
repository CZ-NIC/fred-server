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
#include "src/bin/corba/admin/pagetable_keysets.hh"

ccReg_KeySets_i::ccReg_KeySets_i(LibFred::Keyset::List *kl, const Settings *_ptr) : m_kl(kl)
{
    uf.settings(_ptr);
}

ccReg_KeySets_i::~ccReg_KeySets_i()
{
    TRACE("[CALL] ccReg_KeySets_i::~ccReg_KeySets_i()");
}

ccReg::Filters::Compound_ptr
ccReg_KeySets_i::add()
{
  Logging::Context ctx(base_context_);

    TRACE("[CALL] ccReg_KeySets_i::add()");
    Database::Filters::KeySet *f = new Database::Filters::KeySetHistoryImpl();
    uf.addFilter(f);
    return it.addE(f);
}

Registry::Table::ColumnHeaders *
ccReg_KeySets_i::getColumnHeaders()
{
  Logging::Context ctx(base_context_);

    TRACE("[CALL] ccReg_KeySets_i::getColumnHeaders()");
    Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
    ch->length(4);
    COLHEAD(ch, 0, "Handle", CT_OID);
    COLHEAD(ch, 1, "Create date", CT_OTHER);
    COLHEAD(ch, 2, "Delete date", CT_OTHER);
    COLHEAD(ch, 3, "Registrar", CT_OID);
    return ch;
}

Registry::TableRow *
ccReg_KeySets_i::getRow(CORBA::UShort row)
{
  Logging::Context ctx(base_context_);

    const LibFred::Keyset::Keyset *k = m_kl->getKeyset(row);
    if (!k)
        throw Registry::Table::INVALID_ROW();
    Registry::TableRow *tr = new Registry::TableRow;
    tr->length(4);

    MAKE_OID(oid_handle, k->getId(), C_STR(k->getHandle()), FT_KEYSET)
    MAKE_OID(oid_registrar, k->getRegistrarId(), C_STR(k->getRegistrarHandle()), FT_REGISTRAR)

    (*tr)[0] <<= oid_handle;
    (*tr)[1] <<= C_STR(k->getCreateDate());
    (*tr)[2] <<= C_STR(k->getDeleteDate());
    (*tr)[3] <<= oid_registrar;
    return tr;
}

void
ccReg_KeySets_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
  Logging::Context ctx(base_context_);

    TRACE(boost::format("[CALL] ccReg_KeySets_i::sortByColumn(%1%, %2%)")
            % column % dir);
    switch (column) {
        case 0:
            m_kl->sort(LibFred::Keyset::MT_HANDLE, dir);
            sorted_by_ = 0;
            break;
        case 1:
            m_kl->sort(LibFred::Keyset::MT_CRDATE, dir);
            sorted_by_ = 1;
            break;
        case 2:
            m_kl->sort(LibFred::Keyset::MT_ERDATE, dir);
            sorted_by_ = 2;
            break;
        case 3:
            m_kl->sort(LibFred::Keyset::MT_REGISTRAR_HANDLE, dir);
            sorted_by_ = 3;
            break;
    }
}

ccReg::TID
ccReg_KeySets_i::getRowId(CORBA::UShort row)
{
  Logging::Context ctx(base_context_);

    const LibFred::Keyset::Keyset *k = m_kl->getKeyset(row);
    if (!k)
        throw Registry::Table::INVALID_ROW();
    return k->getId();
}

char *
ccReg_KeySets_i::outputCSV()
{
    return CORBA::string_dup("1,1,1");
}

CORBA::Short
ccReg_KeySets_i::numRows()
{
  Logging::Context ctx(base_context_);

    return m_kl->getCount();
}

CORBA::Short
ccReg_KeySets_i::numColumns()
{
  Logging::Context ctx(base_context_);

    return 4;
}

void
ccReg_KeySets_i::reload_worker()
{
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

    TRACE("[CALL] ccReg_KeySets_i::reload_worker()");
    m_kl->setTimeout(query_timeout);
    m_kl->setLimit(limit_);
    m_kl->reload(uf);
    m_kl->deleteDuplicatesId();
}

void
ccReg_KeySets_i::clear()
{
  Logging::Context ctx(base_context_);

    TRACE("[CALL] ccReg_KeySets_i::clear()");
    m_kl->clearFilter();

    ccReg_PageTable_i::clear();
    m_kl->clear();
}

CORBA::ULongLong
ccReg_KeySets_i::resultSize()
{
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

    TRACE("[CALL] ccReg_KeySets_i::resultSize()");
    return m_kl->getRealCount(uf);
}

void
ccReg_KeySets_i::loadFilter(ccReg::TID id)
{
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_KeySets_i::loadFilter(%1%)") % id);

    ccReg_PageTable_i::loadFilter(id);

    Database::Filters::Union::iterator uit = uf.begin();
    for (; uit != uf.end(); ++uit) {
        Database::Filters::KeySet *tmp = dynamic_cast<Database::Filters::KeySet *>(*uit);
        if (tmp) {
            it.addE(tmp);
            TRACE(boost::format("[IN] ccReg_KeySets_i::loadFilter(%1%): loaded filter content = %2%") % id % tmp->getContent());
        }
    }
}

void
ccReg_KeySets_i::saveFilter(const char *name)
{
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_KeySets_i::saveFilter('%1%')") % name);

    std::unique_ptr<LibFred::Filter::Manager> tmp_filter_manager(
            LibFred::Filter::Manager::create());
    tmp_filter_manager->save(LibFred::Filter::FT_KEYSET, name, uf);
}

LibFred::Keyset::Keyset *
ccReg_KeySets_i::findId(ccReg::TID id)
{
  Logging::Context ctx(base_context_);

    try {
        LibFred::Keyset::Keyset *keyset =
            dynamic_cast<LibFred::Keyset::Keyset *>(m_kl->findId(id));
        if (keyset)
            return keyset;
        return 0;
    }
    catch (const LibFred::NOT_FOUND&) {
        return 0;
    }
}
CORBA::Boolean
ccReg_KeySets_i::numRowsOverLimit()
{
  Logging::Context ctx(base_context_);

    return m_kl->isLimited();
}

