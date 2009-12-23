#include "pagetable_statement_heads.h"

ccReg_StatementHeads_i::ccReg_StatementHeads_i(
        Register::Banking::HeadList *statementList):
    m_statementList(statementList)
{
}

ccReg_StatementHeads_i::~ccReg_StatementHeads_i()
{
    TRACE("[CALL] ccReg_StatementHeads_i::~ccReg_StatementHeads_i()");
}

ccReg::Filters::Compound_ptr
ccReg_StatementHeads_i::add()
{
    TRACE("[CALL] ccReg_StatementHeads_i::add()");
    Database::Filters::StatementHead *filter =
        new Database::Filters::StatementHeadImpl();
    uf.addFilter(filter);
    return it.addE(filter);
}

Registry::Table::ColumnHeaders *
ccReg_StatementHeads_i::getColumnHeaders()
{
    Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
    ch->length(9);
    COLHEAD(ch, 0, "Account", CT_OID);
    COLHEAD(ch, 1, "Statement Number", CT_OTHER);
    COLHEAD(ch, 2, "Create Date", CT_OTHER);
    COLHEAD(ch, 3, "Old Balance Date", CT_OTHER);
    COLHEAD(ch, 4, "Old Balance", CT_OTHER);
    COLHEAD(ch, 5, "New Balance", CT_OTHER);
    COLHEAD(ch, 6, "Credit Balance", CT_OTHER);
    COLHEAD(ch, 7, "Debet Balance", CT_OTHER);
    COLHEAD(ch, 8, "File", CT_OID);

    return ch;
}

Registry::TableRow *
ccReg_StatementHeads_i::getRow(CORBA::UShort row)
    throw (ccReg::Table::INVALID_ROW)
{
    Logging::Context ctx(base_context_);

    const Register::Banking::StatementHead *head = m_statementList->get(row);
    if (!head) {
        throw ccReg::Table::INVALID_ROW();
    }
    Registry::TableRow *tr = new Registry::TableRow;

    tr->length(9);
    (*tr)[0] <<= C_STR(head->getAccountId());
    (*tr)[1] <<= C_STR(head->getNum());
    (*tr)[2] <<= C_STR(head->getCreateDate());
    (*tr)[3] <<= C_STR(head->getBalanceOldDate());
    (*tr)[4] <<= C_STR(head->getBalanceOld());
    (*tr)[5] <<= C_STR(head->getBalanceNew());
    (*tr)[6] <<= C_STR(head->getBalanceCredit());
    (*tr)[7] <<= C_STR(head->getBalanceDebet());
    (*tr)[8] <<= C_STR(head->getFileId());

    return tr;
}

void
ccReg_StatementHeads_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
    Logging::Context ctx(base_context_);

    TRACE(boost::format(
                "[CALL] ccReg_StatementHeads_i::sortByColumn(%1%, %2%)")
            % column % dir);
    ccReg_PageTable_i::sortByColumn(column, dir);

    switch (column) {
        case 0:
            m_statementList->sort(Register::Banking::MT_ACCOUNT_ID, dir);
            break;
        case 1:
            m_statementList->sort(Register::Banking::MT_NUM, dir);
            break;
        case 2:
            m_statementList->sort(Register::Banking::MT_CREATE_DATE, dir);
            break;
        case 3:
            m_statementList->sort(Register::Banking::MT_BALANCE_OLD_DATE, dir);
            break;
        case 4:
            m_statementList->sort(Register::Banking::MT_BALANCE_OLD, dir);
            break;
        case 5:
            m_statementList->sort(Register::Banking::MT_BALANCE_NEW, dir);
            break;
        case 6:
            m_statementList->sort(Register::Banking::MT_BALANCE_CREDIT, dir);
            break;
        case 7:
            m_statementList->sort(Register::Banking::MT_BALANCE_DEBET, dir);
            break;
        case 8:
            m_statementList->sort(Register::Banking::MT_FILE_ID, dir);
            break;
      
    }
}

ccReg::TID
ccReg_StatementHeads_i::getRowId(CORBA::UShort row)
    throw (ccReg::Table::INVALID_ROW)
{
    Logging::Context ctx(base_context_);

    const Register::Banking::StatementHead *head =
        m_statementList->get(row);
    if (!head) {
        throw ccReg::Table::INVALID_ROW();
    }
    return head->getId();
}

char *
ccReg_StatementHeads_i::outputCSV()
{
    return CORBA::string_dup("1,1,1");
}

CORBA::Short
ccReg_StatementHeads_i::numRows()
{
    Logging::Context ctx(base_context_);

    return m_statementList->getSize();
}

CORBA::Short
ccReg_StatementHeads_i::numColumns()
{
    return 12;
}

void
ccReg_StatementHeads_i::reload()
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    m_statementList->reload(uf);
}

void
ccReg_StatementHeads_i::clear()
{
    Logging::Context ctx(base_context_);

    ccReg_PageTable_i::clear();
    m_statementList->clear();
}

CORBA::ULongLong
ccReg_StatementHeads_i::resultSize()
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE("[CALL] ccReg_StatementHeads_i::resultSize()");
    return m_statementList->getRealCount(uf);
}

void
ccReg_StatementHeads_i::loadFilter(ccReg::TID id)
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_StatementHeads_i::loadFilter(%1%)") % id);

    ccReg_PageTable_i::loadFilter(id);

    Database::Filters::Union::iterator uit = uf.begin();
    for (; uit != uf.end(); ++uit) {
        Database::Filters::StatementHead *tmp =
            dynamic_cast<Database::Filters::StatementHead *>(*uit);
        if (tmp) {
            it.addE(tmp);
            TRACE(boost::format("[IN] ccReg_StatementHeads_i::loadFilter(%1%): "
                        "loaded filter content = %2%")
                    % id % tmp->getContent());
        }
    }
}

void
ccReg_StatementHeads_i::saveFilter(const char *name)
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_StatementHeads_i::saveFilter(%1%)")
            % name);

    std::auto_ptr<Register::Filter::Manager> tmp_filter_manager(
            Register::Filter::Manager::create());
    tmp_filter_manager->save(Register::Filter::FT_STATEMENTHEAD, name, uf);
}

Register::Banking::StatementHead *
ccReg_StatementHeads_i::findId(ccReg::TID id)
{
    Logging::Context ctx(base_context_);
    try {
        Register::Banking::StatementHead *statementHead =
            m_statementList->getById(id);
        if (statementHead) {
            return statementHead;
        }
        return 0;
    } catch (Register::NOT_FOUND) {
        return 0;
    }
}

CORBA::Boolean
ccReg_StatementHeads_i::numRowsOverLimit()
{
    Logging::Context ctx(base_context_);

    return m_statementList->isLimited();
}

