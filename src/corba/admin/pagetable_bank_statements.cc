#include "pagetable_bank_statements.h"

ccReg_Statements_i::ccReg_Statements_i(
        Fred::Banking::StatementList *list):
    list_(list)
{
}

ccReg_Statements_i::~ccReg_Statements_i()
{
    TRACE("[CALL] ccReg_Statements_i::~ccReg_Statements_i()");
}

ccReg::Filters::Compound_ptr
ccReg_Statements_i::add()
{
    TRACE("[CALL] ccReg_Statements_i::add()");
    Database::Filters::BankStatement *filter =
        new Database::Filters::BankStatementImpl();
    uf.addFilter(filter);
    return it.addE(filter);
}

Registry::Table::ColumnHeaders *
ccReg_Statements_i::getColumnHeaders()
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
ccReg_Statements_i::getRow(CORBA::UShort row)
    throw (Registry::Table::INVALID_ROW)
{
    Logging::Context ctx(base_context_);

    const Fred::Banking::Statement *data = list_->get(row);
    if (!data) {
        throw Registry::Table::INVALID_ROW();
    }
    Registry::TableRow *tr = new Registry::TableRow;

    tr->length(9);
    (*tr)[0] <<= C_STR(data->getAccountId());
    (*tr)[1] <<= C_STR(data->getNum());
    (*tr)[2] <<= C_STR(data->getCreateDate());
    (*tr)[3] <<= C_STR(data->getBalanceOldDate());
    (*tr)[4] <<= C_STR(data->getBalanceOld());
    (*tr)[5] <<= C_STR(data->getBalanceNew());
    (*tr)[6] <<= C_STR(data->getBalanceCredit());
    (*tr)[7] <<= C_STR(data->getBalanceDebet());
    (*tr)[8] <<= C_STR(data->getFileId());

    return tr;
}

void
ccReg_Statements_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
    Logging::Context ctx(base_context_);

    TRACE(boost::format(
                "[CALL] ccReg_Statements_i::sortByColumn(%1%, %2%)")
            % column % dir);
    ccReg_PageTable_i::sortByColumn(column, dir);

    switch (column) {
        case 0:
            list_->sort(Fred::Banking::MT_ACCOUNT_ID, dir);
            break;
        case 1:
            list_->sort(Fred::Banking::MT_NUM, dir);
            break;
        case 2:
            list_->sort(Fred::Banking::MT_CREATE_DATE, dir);
            break;
        case 3:
            list_->sort(Fred::Banking::MT_BALANCE_OLD_DATE, dir);
            break;
        case 4:
            list_->sort(Fred::Banking::MT_BALANCE_OLD, dir);
            break;
        case 5:
            list_->sort(Fred::Banking::MT_BALANCE_NEW, dir);
            break;
        case 6:
            list_->sort(Fred::Banking::MT_BALANCE_CREDIT, dir);
            break;
        case 7:
            list_->sort(Fred::Banking::MT_BALANCE_DEBET, dir);
            break;
        case 8:
            list_->sort(Fred::Banking::MT_FILE_ID, dir);
            break;
      
    }
}

ccReg::TID
ccReg_Statements_i::getRowId(CORBA::UShort row)
    throw (Registry::Table::INVALID_ROW)
{
    Logging::Context ctx(base_context_);

    const Fred::Banking::Statement *data = list_->get(row);
    if (!data) {
        throw Registry::Table::INVALID_ROW();
    }
    return data->getId();
}

char *
ccReg_Statements_i::outputCSV()
{
    return CORBA::string_dup("1,1,1");
}

CORBA::Short
ccReg_Statements_i::numRows()
{
    Logging::Context ctx(base_context_);

    return list_->getSize();
}

CORBA::Short
ccReg_Statements_i::numColumns()
{
    return 12;
}

void
ccReg_Statements_i::reload_worker()
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    list_->setTimeout(query_timeout);
    list_->reload(uf);
}

void
ccReg_Statements_i::clear()
{
    Logging::Context ctx(base_context_);

    ccReg_PageTable_i::clear();
    list_->clear();
}

CORBA::ULongLong
ccReg_Statements_i::resultSize()
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE("[CALL] ccReg_Statements_i::resultSize()");
    return list_->getRealCount(uf);
}

void
ccReg_Statements_i::loadFilter(ccReg::TID id)
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Statements_i::loadFilter(%1%)") % id);

    ccReg_PageTable_i::loadFilter(id);

    Database::Filters::Union::iterator uit = uf.begin();
    for (; uit != uf.end(); ++uit) {
        Database::Filters::BankStatement *tmp =
            dynamic_cast<Database::Filters::BankStatement *>(*uit);
        if (tmp) {
            it.addE(tmp);
            TRACE(boost::format("[IN] ccReg_Statements_i::loadFilter(%1%): "
                        "loaded filter content = %2%")
                    % id % tmp->getContent());
        }
    }
}

void
ccReg_Statements_i::saveFilter(const char *name)
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Statements_i::saveFilter(%1%)")
            % name);

    std::auto_ptr<Fred::Filter::Manager> tmp_filter_manager(
            Fred::Filter::Manager::create());
    tmp_filter_manager->save(Fred::Filter::FT_STATEMENTHEAD, name, uf);
}

Fred::Banking::Statement *
ccReg_Statements_i::findId(ccReg::TID id)
{
    Logging::Context ctx(base_context_);
    try {
        Fred::Banking::Statement *item = list_->getById(id);
        if (item) {
            return item;
        }
        return 0;
    } catch (Fred::NOT_FOUND) {
        return 0;
    }
}

CORBA::Boolean
ccReg_Statements_i::numRowsOverLimit()
{
    Logging::Context ctx(base_context_);

    return list_->isLimited();
}

