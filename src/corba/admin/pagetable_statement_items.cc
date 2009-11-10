#include "pagetable_statement_items.h"

ccReg_StatementItems_i::ccReg_StatementItems_i(
        Register::Banking::ItemList *statementItemList):
    m_statementItemList(statementItemList)
{
}

ccReg_StatementItems_i::~ccReg_StatementItems_i()
{
    TRACE("[CALL] ccReg_StatementItems_i::~ccReg_StatementItems_i()");
}

ccReg::Filters::Compound_ptr
ccReg_StatementItems_i::add()
{
    TRACE("[CALL] ccReg_StatementItems_i::add()");
    Database::Filters::StatementItem *filter =
        new Database::Filters::StatementItemImpl();
    uf.addFilter(filter);
    return it.addE(filter);
}

Registry::Table::ColumnHeaders *
ccReg_StatementItems_i::getColumnHeaders()
{
    Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
    ch->length(12);
    COLHEAD(ch, 0, "Statement Head", CT_OID);
    COLHEAD(ch, 1, "Account Number", CT_OTHER);
    COLHEAD(ch, 2, "Bank Code", CT_OTHER);
    COLHEAD(ch, 3, "Code", CT_OTHER);
    COLHEAD(ch, 4, "Type", CT_OTHER);
    COLHEAD(ch, 5, "Const. Symbol", CT_OTHER);
    COLHEAD(ch, 6, "Var. Symbol", CT_OTHER);
    COLHEAD(ch, 7, "Spec. Symbol", CT_OTHER);
    COLHEAD(ch, 8, "Price", CT_OTHER);
    COLHEAD(ch, 9, "Date", CT_OTHER);
    COLHEAD(ch, 10, "Memo", CT_OTHER);
    COLHEAD(ch, 11, "Invoice", CT_OID);
    return ch;
}

Registry::TableRow *
ccReg_StatementItems_i::getRow(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW)
{
    Logging::Context ctx(base_context_);

    const Register::Banking::StatementItem *item = m_statementItemList->get(row);
    if (!item) {
        throw ccReg::Table::INVALID_ROW();
    }
    Registry::TableRow *tr = new Registry::TableRow;

    tr->length(12);
    (*tr)[0] <<= C_STR(item->getStatementId());
    (*tr)[1] <<= C_STR(item->getAccountNumber());
    (*tr)[2] <<= C_STR(item->getBankCodeId());
    (*tr)[3] <<= C_STR(item->getCode());
    (*tr)[4] <<= C_STR(item->getType());
    (*tr)[5] <<= C_STR(item->getKonstSym());
    (*tr)[6] <<= C_STR(item->getVarSymb());
    (*tr)[7] <<= C_STR(item->getSpecSymb());
    (*tr)[8] <<= formatMoney(item->getPrice()).c_str();
    (*tr)[9] <<= C_STR(item->getAccountDate());
    (*tr)[10] <<= C_STR(item->getAccountMemo());
    (*tr)[11] <<= C_STR(item->getInvoiceId());

    return tr;
}

void
ccReg_StatementItems_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
    Logging::Context ctx(base_context_);

    TRACE(boost::format(
                "[CALL] ccReg_StatementItems_i::sortByColumn(%1%, %2%)")
            % column % dir);
    ccReg_PageTable_i::sortByColumn(column, dir);

    switch (column) {
        case 0:
            m_statementItemList->sort(Register::Banking::IMT_STATEMENT_ID, dir);
            break;
        case 1:
            m_statementItemList->sort(Register::Banking::IMT_ACCOUNT_NUMBER, dir);
            break;
        case 2:
            m_statementItemList->sort(Register::Banking::IMT_BANK_CODE, dir);
            break;
        case 3:
            m_statementItemList->sort(Register::Banking::IMT_TYPE, dir);
            break;
        case 4:
            m_statementItemList->sort(Register::Banking::IMT_CODE, dir);
            break;
        case 5:
            m_statementItemList->sort(Register::Banking::IMT_CONSTSYMB, dir);
            break;
        case 6:
            m_statementItemList->sort(Register::Banking::IMT_VARSYMB, dir);
            break;
        case 7:
            m_statementItemList->sort(Register::Banking::IMT_SPECSYMB, dir);
            break;
        case 8:
            m_statementItemList->sort(Register::Banking::IMT_EVID, dir);
            break;
        case 9:
            m_statementItemList->sort(Register::Banking::IMT_DATE, dir);
            break;
    }
}

ccReg::TID
ccReg_StatementItems_i::getRowId(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW)
{
    Logging::Context ctx(base_context_);

    const Register::Banking::StatementItem *item =
        m_statementItemList->get(row);
    if (!item) {
        throw ccReg::Table::INVALID_ROW();
    }
    return item->getId();
}

char *
ccReg_StatementItems_i::outputCSV()
{
    return CORBA::string_dup("1,1,1");
}

CORBA::Short
ccReg_StatementItems_i::numRows()
{
    Logging::Context ctx(base_context_);

    return m_statementItemList->getSize();
}

CORBA::Short
ccReg_StatementItems_i::numColumns()
{
    return 12;
}

void
ccReg_StatementItems_i::reload()
{
    Logging::Context ctx(base_context_);

    m_statementItemList->reload(uf);
}

void
ccReg_StatementItems_i::clear()
{
    Logging::Context ctx(base_context_);

    ccReg_PageTable_i::clear();
    m_statementItemList->clear();
}

CORBA::ULongLong
ccReg_StatementItems_i::resultSize()
{
    Logging::Context ctx(base_context_);

    TRACE("[CALL] ccReg_StatementItems_i::resultSize()");
    return m_statementItemList->getRealCount(uf);
}

void
ccReg_StatementItems_i::loadFilter(ccReg::TID id)
{
    Logging::Context ctx(base_context_);

    TRACE(boost::format("[CALL] ccReg_StatementItems_i::loadFilter(%1%)") % id);

    ccReg_PageTable_i::loadFilter(id);

    Database::Filters::Union::iterator uit = uf.begin();
    for (; uit != uf.end(); ++uit) {
        Database::Filters::StatementItem *tmp = 
            dynamic_cast<Database::Filters::StatementItem *>(*uit);
        if (tmp) {
            it.addE(tmp);
            TRACE(boost::format("[IN] ccReg_StatementItems_i::loadFilter(%1%): "
                        "loaded filter content = %2%")
                    % id % tmp->getContent());
        }
    }
}

void
ccReg_StatementItems_i::saveFilter(const char *name)
{
    Logging::Context ctx(base_context_);

    TRACE(boost::format("[CALL] ccReg_StatementItems_i::saveFilter(%1%)")
            % name);

    std::auto_ptr<Register::Filter::Manager> tmp_filter_manager(
            Register::Filter::Manager::create());
    tmp_filter_manager->save(Register::Filter::FT_STATEMENTITEM, name, uf);
}

Register::Banking::StatementItem *
ccReg_StatementItems_i::findId(ccReg::TID id)
{
    Logging::Context ctx(base_context_);
    try {
        Register::Banking::StatementItem *statementItem =
            m_statementItemList->getById(id);
        if (statementItem) {
            return statementItem;
        }
        return 0;
    } catch (Register::NOT_FOUND) {
        return 0;
    }
}

CORBA::Boolean
ccReg_StatementItems_i::numRowsOverLimit()
{
    Logging::Context ctx(base_context_);

    return m_statementItemList->isLimited();
}
