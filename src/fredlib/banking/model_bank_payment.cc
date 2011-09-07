#include "model_bank_payment.h"

std::string ModelBankPayment::table_name = "bank_payment";

DEFINE_PRIMARY_KEY(ModelBankPayment, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelBankPayment, unsigned long long, statementId, m_statementId, table_name, "statement_id",.setForeignKey() )
DEFINE_BASIC_FIELD(ModelBankPayment, unsigned long long, accountId, m_accountId, table_name, "account_id",.setForeignKey() )
DEFINE_BASIC_FIELD(ModelBankPayment, std::string, accountNumber, m_accountNumber, table_name, "account_number", .setNotNull())
DEFINE_BASIC_FIELD(ModelBankPayment, std::string, bankCodeId, m_bankCodeId, table_name, "bank_code", .setForeignKey())
DEFINE_BASIC_FIELD(ModelBankPayment, int, code, m_code, table_name, "code", )
DEFINE_BASIC_FIELD(ModelBankPayment, int, type, m_type, table_name, "type", .setDefault())
DEFINE_BASIC_FIELD(ModelBankPayment, int, status, m_status, table_name, "status", )
DEFINE_BASIC_FIELD(ModelBankPayment, std::string, konstSym, m_konstSym, table_name, "konstsym", )
DEFINE_BASIC_FIELD(ModelBankPayment, std::string, varSymb, m_varSymb, table_name, "varsymb", )
DEFINE_BASIC_FIELD(ModelBankPayment, std::string, specSymb, m_specSymb, table_name, "specsymb", )
DEFINE_BASIC_FIELD(ModelBankPayment, std::string, price, m_price, table_name, "price", .setNotNull())
DEFINE_BASIC_FIELD(ModelBankPayment, std::string, accountEvid, m_accountEvid, table_name, "account_evid", )
DEFINE_BASIC_FIELD(ModelBankPayment, Database::Date, accountDate, m_accountDate, table_name, "account_date", .setNotNull())
DEFINE_BASIC_FIELD(ModelBankPayment, std::string, accountMemo, m_accountMemo, table_name, "account_memo", )
DEFINE_BASIC_FIELD(ModelBankPayment, std::string, accountName, m_accountName, table_name, "account_name", )
DEFINE_BASIC_FIELD(ModelBankPayment, Database::DateTime, crTime, m_crTime, table_name, "crtime", .setDefault())

ModelBankPayment::field_list ModelBankPayment::fields = list_of<ModelBankPayment::field_list::value_type>
    (&ModelBankPayment::id)
    (&ModelBankPayment::statementId)
    (&ModelBankPayment::accountId)
    (&ModelBankPayment::accountNumber)
    (&ModelBankPayment::bankCodeId)
    (&ModelBankPayment::code)
    (&ModelBankPayment::type)
    (&ModelBankPayment::status)
    (&ModelBankPayment::konstSym)
    (&ModelBankPayment::varSymb)
    (&ModelBankPayment::specSymb)
    (&ModelBankPayment::price)
    (&ModelBankPayment::accountEvid)
    (&ModelBankPayment::accountDate)
    (&ModelBankPayment::accountMemo)
    (&ModelBankPayment::accountName)
    (&ModelBankPayment::crTime)
;

