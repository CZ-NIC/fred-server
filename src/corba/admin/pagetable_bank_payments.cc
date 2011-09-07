#include "pagetable_bank_payments.h"

ccReg_Payments_i::ccReg_Payments_i(
        Fred::Banking::PaymentList *list):
    list_(list)
{
}

ccReg_Payments_i::~ccReg_Payments_i()
{
    TRACE("[CALL] ccReg_Payments_i::~ccReg_Payments_i()");
}

ccReg::Filters::Compound_ptr
ccReg_Payments_i::add()
{
    TRACE("[CALL] ccReg_Payments_i::add()");
    Database::Filters::BankPayment *filter =
        new Database::Filters::BankPaymentImpl();
    uf.addFilter(filter);
    return it.addE(filter);
}

Registry::Table::ColumnHeaders *
ccReg_Payments_i::getColumnHeaders()
{
    Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
    ch->length(numColumns());
//    COLHEAD(ch, 0, "Id", CT_OID);
//    COLHEAD(ch, 1, "Statement Head", CT_OID);
    COLHEAD(ch, 0, "Account Number", CT_OTHER);
    COLHEAD(ch, 1, "Bank Code", CT_OTHER);
//    COLHEAD(ch, 4, "Code", CT_OTHER);
//    COLHEAD(ch, 5, "Type", CT_OTHER);
//    COLHEAD(ch, 6, "Const. Symbol", CT_OTHER);
    COLHEAD(ch, 2, "Var. Symbol", CT_OTHER);
//    COLHEAD(ch, 3, "Spec. Symbol", CT_OTHER);
    COLHEAD(ch, 3, "Price", CT_OTHER);
//    COLHEAD(ch, 10, "Account Evid", CT_OTHER);
    COLHEAD(ch, 4, "Account Date", CT_OTHER);
    COLHEAD(ch, 5, "Memo", CT_OTHER);
    COLHEAD(ch, 6, "Invoice", CT_OID);
    COLHEAD(ch, 7, "Account Name", CT_OTHER);
    COLHEAD(ch, 8, "Create Time", CT_OTHER);
    COLHEAD(ch, 9, "Destination Account", CT_OTHER);

    /*
    COLHEAD(ch, 9, "Date", CT_OTHER);
    COLHEAD(ch, 10, "Memo", CT_OTHER);
    COLHEAD(ch, 11, "Invoice", CT_OID);
     */

    return ch;
}

Registry::TableRow *
ccReg_Payments_i::getRow(CORBA::UShort row)
    throw (Registry::Table::INVALID_ROW)
{
    Logging::Context ctx(base_context_);

    const Fred::Banking::Payment *data = list_->get(row);
    if (!data) {
        throw Registry::Table::INVALID_ROW();
    }
    Registry::TableRow *tr = new Registry::TableRow;

    tr->length(numColumns());

    MAKE_OID(oid_id, data->getId(), C_STR(data->getId()), FT_STATEMENTITEM);
    MAKE_OID(oid_statement_id, data->getStatementId(), C_STR(data->getStatementId()), FT_STATEMENTHEAD);
    MAKE_OID(oid_invoice_id, data->getAdvanceInvoiceId(), C_STR(data->getInvoicePrefix()), FT_INVOICE);

 //   (*tr)[0] <<= oid_id;
 //   (*tr)[1] <<= oid_statement_id;
    (*tr)[0] <<= C_STR(data->getAccountNumber());
    (*tr)[1] <<= C_STR(data->getBankCode());
//    (*tr)[4] <<= C_STR(data->getCode());
//    (*tr)[5] <<= C_STR(data->getType());
//    (*tr)[6] <<= C_STR(data->getKonstSym());
    (*tr)[2] <<= C_STR(data->getVarSymb());
//    (*tr)[8] <<= C_STR(data->getSpecSymb());
    (*tr)[3] <<= formatMoney(data->getPrice()).c_str();
//    (*tr)[10] <<= C_STR(data->getAccountEvid());
    (*tr)[4] <<= C_STR(data->getAccountDate());
    (*tr)[5] <<= C_STR(data->getAccountMemo());
    (*tr)[6] <<= oid_invoice_id;
    (*tr)[7] <<= C_STR(data->getAccountName());
    (*tr)[8] <<= C_STR(data->getCrTime());
    (*tr)[9] <<= C_STR(data->getDestAccount());

    return tr;
}

void
ccReg_Payments_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
    Logging::Context ctx(base_context_);

    TRACE(boost::format(
                "[CALL] ccReg_Payments_i::sortByColumn(%1%, %2%)")
            % column % dir);
    try
    {
    ccReg_PageTable_i::sortByColumn(column, dir);

    switch (column) {
//        case 0:
//            list_->sort(Fred::Banking::IMT_ID, dir);
//            break;
//        case 1:
//            list_->sort(Fred::Banking::IMT_STATEMENT_ID, dir);
//            break;
        case 0:
            list_->sort(Fred::Banking::IMT_ACCOUNT_NUMBER, dir);
            break;
        case 1:
            list_->sort(Fred::Banking::IMT_BANK_CODE, dir);
            break;
//        case 4:
//            list_->sort(Fred::Banking::IMT_TYPE, dir);
//            break;
//        case 5:
//            list_->sort(Fred::Banking::IMT_CODE, dir);
//            break;
//        case 6:
//            list_->sort(Fred::Banking::IMT_CONSTSYMB, dir);
//            break;
        case 2:
            list_->sort(Fred::Banking::IMT_VARSYMB, dir);
            break;
//        case 8:
//            list_->sort(Fred::Banking::IMT_SPECSYMB, dir);
//            break;
        case 3:
            list_->sort(Fred::Banking::IMT_PRICE, dir);
            break;
//        case 10:
//            list_->sort(Fred::Banking::IMT_ACCOUNT_EVID, dir);
//            break;
        case 4:
            list_->sort(Fred::Banking::IMT_ACCOUNT_DATE, dir);
            break;
        case 5:
            list_->sort(Fred::Banking::IMT_ACCOUNT_MEMO, dir);
            break;
        case 7:
            list_->sort(Fred::Banking::IMT_ACCOUNT_NAME, dir);
            break;
        case 8:
            list_->sort(Fred::Banking::IMT_CREATE_TIME, dir);
            break;
        case 9:
            list_->sort(Fred::Banking::IMT_DEST_ACCOUNT, dir);
            break;

    }//switch
    }//try
    catch (const std::exception& ex)
    {
        Logging::Manager::instance_ref()
            .get(PACKAGE)
            .message( ERROR_LOG
                , "ccReg_Payments_i::sortByColumn: std::exception %s", ex.what());
    }
    catch (...)
    {
        Logging::Manager::instance_ref()
            .get(PACKAGE)
            .message( ERROR_LOG
                    , "ccReg_Payments_i::sortByColumn: unknown exception ");
    }
}

ccReg::TID
ccReg_Payments_i::getRowId(CORBA::UShort row)
    throw (Registry::Table::INVALID_ROW)
{
    Logging::Context ctx(base_context_);

    const Fred::Banking::Payment *data = list_->get(row);
    if (!data) {
        throw Registry::Table::INVALID_ROW();
    }
    return data->getId();
}

char *
ccReg_Payments_i::outputCSV()
{
    return CORBA::string_dup("1,1,1");
}

CORBA::Short
ccReg_Payments_i::numRows()
{
    Logging::Context ctx(base_context_);

    return list_->getSize();
}

CORBA::Short
ccReg_Payments_i::numColumns()
{
    return 10;
}

void
ccReg_Payments_i::reload_worker()
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    list_->setTimeout(query_timeout);
    list_->reload(uf);
}

void
ccReg_Payments_i::clear()
{
    Logging::Context ctx(base_context_);

    ccReg_PageTable_i::clear();
    list_->clear();
}

CORBA::ULongLong
ccReg_Payments_i::resultSize()
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE("[CALL] ccReg_Payments_i::resultSize()");
    return list_->getRealCount(uf);
}

void
ccReg_Payments_i::loadFilter(ccReg::TID id)
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Payments_i::loadFilter(%1%)") % id);

    ccReg_PageTable_i::loadFilter(id);

    Database::Filters::Union::iterator uit = uf.begin();
    for (; uit != uf.end(); ++uit) {
        Database::Filters::BankPayment *tmp = 
            dynamic_cast<Database::Filters::BankPayment *>(*uit);
        if (tmp) {
            it.addE(tmp);
            TRACE(boost::format("[IN] ccReg_Payments_i::loadFilter(%1%): "
                        "loaded filter content = %2%")
                    % id % tmp->getContent());
        }
    }
}

void
ccReg_Payments_i::saveFilter(const char *name)
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Payments_i::saveFilter(%1%)")
            % name);

    std::auto_ptr<Fred::Filter::Manager> tmp_filter_manager(
            Fred::Filter::Manager::create());
    tmp_filter_manager->save(Fred::Filter::FT_STATEMENTITEM, name, uf);
}

Fred::Banking::Payment *
ccReg_Payments_i::findId(ccReg::TID id)
{
    Logging::Context ctx(base_context_);
    try {
        Fred::Banking::Payment *data = list_->getById(id);
        if (data) {
            return data;
        }
        return 0;
    } catch (Fred::NOT_FOUND) {
        return 0;
    }
}

CORBA::Boolean
ccReg_Payments_i::numRowsOverLimit()
{
    Logging::Context ctx(base_context_);

    return list_->isLimited();
}
