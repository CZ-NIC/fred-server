#include "pagetable_keysets.h"

ccReg_KeySets_i::ccReg_KeySets_i(Register::KeySet::List *kl) : m_kl(kl)
{
}

ccReg_KeySets_i::~ccReg_KeySets_i()
{
    TRACE("[CALL] ccReg_KeySets_i::~ccReg_KeySets_i()");
}

ccReg::Filters::Compound_ptr
ccReg_KeySets_i::add()
{
    TRACE("[CALL] ccReg_KeySets_i::add()");
    it.clearF();
    Database::Filters::KeySet *f = new Database::Filters::KeySetHistoryImpl();
    uf.addFilter(f);
    return it.addE(f);
}

ccReg::Table::ColumnHeaders *
ccReg_KeySets_i::getColumnHeaders()
{
    TRACE("[CALL] ccReg_KeySets_i::getColumnHeaders()");
    ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
    ch->length(4);
    COLHEAD(ch, 0, "Handle", CT_KEYSET_HANDLE);
    COLHEAD(ch, 1, "Create date", CT_OTHER);
    COLHEAD(ch, 2, "Delete date", CT_OTHER);
    COLHEAD(ch, 3, "Registrar", CT_REGISTRAR_HANDLE);
    return ch;
}

ccReg::TableRow *
ccReg_KeySets_i::getRow(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW)
{
    const Register::KeySet::KeySet *k = m_kl->getKeySet(row);
    if (!k)
        throw ccReg::Table::INVALID_ROW();
    ccReg::TableRow *tr = new ccReg::TableRow;
    tr->length(4);
    (*tr)[0] = DUPSTRFUN(k->getHandle);
    (*tr)[1] = DUPSTRDATE(k->getCreateDate);
    (*tr)[2] = DUPSTRDATE(k->getDeleteDate);
    (*tr)[3] = DUPSTRFUN(k->getRegistrarHandle);
    return tr;
}

void
ccReg_KeySets_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
    TRACE(boost::format("[CALL] ccReg_KeySets_i::sortByColumn(%1%, %2%)")
            % column % dir);
    switch (column) {
        case 0:
            m_kl->sort(Register::KeySet::MT_HANDLE, dir);
            sorted_by_ = 0;
            break;
        case 1:
            m_kl->sort(Register::KeySet::MT_CRDATE, dir);
            sorted_by_ = 1;
            break;
        case 2:
            m_kl->sort(Register::KeySet::MT_ERDATE, dir);
            sorted_by_ = 2;
            break;
        case 3:
            m_kl->sort(Register::KeySet::MT_REGISTRAR_HANDLE, dir);
            sorted_by_ = 3;
            break;
    }
}

ccReg::TID
ccReg_KeySets_i::getRowId(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW)
{
    const Register::KeySet::KeySet *k = m_kl->getKeySet(row);
    if (!k)
        throw ccReg::Table::INVALID_ROW();
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
    return m_kl->getCount();
}

CORBA::Short
ccReg_KeySets_i::numColumns()
{
    return 4;
}

void
ccReg_KeySets_i::reload()
{
    TRACE("[CALL] ccReg_KeySets_i::reload()");
    m_kl->reload(uf, dbm);
}

void
ccReg_KeySets_i::clear()
{
    TRACE("[CALL] ccReg_KeySets_i::clear()");
    m_kl->clearFilter();

    ccReg_PageTable_i::clear();
    m_kl->clear();
}

CORBA::ULongLong
ccReg_KeySets_i::resultSize()
{
    TRACE("[CALL] ccReg_KeySets_i::resultSize()");
    return m_kl->getRealCount(uf);
}

void
ccReg_KeySets_i::loadFilter(ccReg::TID id)
{
    TRACE(boost::format("[CALL] ccReg_KeySets_i::loadFilter(%1%)") % id);

    ccReg_PageTable_i::loadFilter(id);

    Database::Filters::Union::iterator uit = uf.begin();
    for (; uit != uf.end(); ++uit)
        it.addE(dynamic_cast<Database::Filters::KeySet *>(*uit));
}

void
ccReg_KeySets_i::saveFilter(const char *name)
{
    TRACE(boost::format("[CALL] ccReg_KeySets_i::saveFilter('%1%')") % name);
    
    std::auto_ptr<Register::Filter::Manager> tmp_filter_manager(
            Register::Filter::Manager::create(dbm));
    tmp_filter_manager->save(Register::Filter::FT_KEYSET, name, uf);
}

Register::KeySet::KeySet *
ccReg_KeySets_i::findId(ccReg::TID id)
{
    try {
        Register::KeySet::KeySet *keyset =
            dynamic_cast<Register::KeySet::KeySet *>(m_kl->findId(id));
        if (keyset)
            return keyset;
        return 0;
    }
    catch (Register::NOT_FOUND) {
        return 0;
    }
}
CORBA::Boolean
ccReg_KeySets_i::numRowsOverLimit()
{
    return m_kl->isLimited();
}
