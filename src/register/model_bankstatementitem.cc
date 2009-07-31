#include "model_bankstatementitem.h"

std::string ModelBankStatementItem::table_name = "bank_item";

DEFINE_PRIMARY_KEY(ModelBankStatementItem, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_FOREIGN_KEY(ModelBankStatementItem, ModelBankStatementHead, unsigned long long, statementId, m_statementId, table_name, "statement_id", id, )
DEFINE_BASIC_FIELD(ModelBankStatementItem, std::string, accountNumber, m_accountNumber, table_name, "account_number", .setNotNull())
DEFINE_FOREIGN_KEY(ModelBankStatementItem, ModelEnumBankCode, std::string, bankCodeId, m_bankCodeId, table_name, "bank_code", code, )
DEFINE_BASIC_FIELD(ModelBankStatementItem, int, code, m_code, table_name, "code", )
DEFINE_BASIC_FIELD(ModelBankStatementItem, int, type, m_type, table_name, "type", )
DEFINE_BASIC_FIELD(ModelBankStatementItem, std::string, konstSym, m_konstSym, table_name, "konstsym", )
DEFINE_BASIC_FIELD(ModelBankStatementItem, std::string, varSymb, m_varSymb, table_name, "varsymb", )
DEFINE_BASIC_FIELD(ModelBankStatementItem, std::string, specSymb, m_specSymb, table_name, "specsymb", )
DEFINE_BASIC_FIELD(ModelBankStatementItem, Database::Money, price, m_price, table_name, "price", .setNotNull())
DEFINE_BASIC_FIELD(ModelBankStatementItem, std::string, accountEvid, m_accountEvid, table_name, "account_evid", )
DEFINE_BASIC_FIELD(ModelBankStatementItem, Database::Date, accountDate, m_accountDate, table_name, "account_date", .setNotNull())
DEFINE_BASIC_FIELD(ModelBankStatementItem, std::string, accountMemo, m_accountMemo, table_name, "account_memo", )
DEFINE_FOREIGN_KEY(ModelBankStatementItem, ModelInvoice, unsigned long long, invoiceId, m_invoiceId, table_name, "invoice_id", id, )
DEFINE_BASIC_FIELD(ModelBankStatementItem, std::string, accountName, m_accountName, table_name, "account_name", )
DEFINE_BASIC_FIELD(ModelBankStatementItem, Database::DateTime, crTime, m_crTime, table_name, "crtime", .setDefault().setNotNull())

DEFINE_ONE_TO_ONE(ModelBankStatementItem, ModelBankStatementHead, statement, m_statement, unsigned long long, statementId, m_statementId)
DEFINE_ONE_TO_ONE(ModelBankStatementItem, ModelEnumBankCode, bankCode, m_bankCode, std::string, bankCodeId, m_bankCodeId)
DEFINE_ONE_TO_ONE(ModelBankStatementItem, ModelInvoice, invoice, m_invoice, unsigned long long, invoiceId, m_invoiceId)

ModelBankStatementItem::field_list ModelBankStatementItem::fields = list_of<ModelBankStatementItem::field_list::value_type>
    (&ModelBankStatementItem::id)
    (&ModelBankStatementItem::statementId)
    (&ModelBankStatementItem::accountNumber)
    (&ModelBankStatementItem::bankCodeId)
    (&ModelBankStatementItem::code)
    (&ModelBankStatementItem::type)
    (&ModelBankStatementItem::konstSym)
    (&ModelBankStatementItem::varSymb)
    (&ModelBankStatementItem::specSymb)
    (&ModelBankStatementItem::price)
    (&ModelBankStatementItem::accountEvid)
    (&ModelBankStatementItem::accountDate)
    (&ModelBankStatementItem::accountMemo)
    (&ModelBankStatementItem::invoiceId)
    (&ModelBankStatementItem::accountName)
    (&ModelBankStatementItem::crTime)
;

